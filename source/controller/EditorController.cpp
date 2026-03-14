#include "EditorController.h"

#include "../core/ChartMapping.h"

#include <cmath>
#include <optional>
#include <regex>

namespace pointdrone::controller
{
namespace
{
constexpr int previewSampleCount = 192;
constexpr float previewCycles = 2.0f;
constexpr double previewSampleRate = 44100.0;

juce::String noteIndicatorText(const float frequencyHz)
{
    static constexpr std::array<const char*, 12> noteNames { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    const auto midiNoteFloat = 69.0 + (12.0 * std::log2(static_cast<double>(frequencyHz) / 440.0));
    auto nearestMidiNote = static_cast<int>(std::round(midiNoteFloat));
    auto cents = static_cast<int>(std::round((midiNoteFloat - static_cast<double>(nearestMidiNote)) * 100.0));

    if (cents >= 50)
    {
        ++nearestMidiNote;
        cents -= 100;
    }
    else if (cents <= -50)
    {
        --nearestMidiNote;
        cents += 100;
    }

    const auto noteIndex = ((nearestMidiNote % static_cast<int>(noteNames.size())) + static_cast<int>(noteNames.size()))
                         % static_cast<int>(noteNames.size());
    const auto octave = (nearestMidiNote / 12) - 1;
    auto text = juce::String(noteNames[static_cast<size_t>(noteIndex)]) + juce::String(octave);

    if (cents != 0)
        text << (cents > 0 ? "+" : "") << juce::String(cents) << "c";

    return text;
}

juce::String frequencyValueText(const float frequencyHz)
{
    return juce::String(frequencyHz, 1).trimCharactersAtEnd("0").trimCharactersAtEnd(".");
}

juce::String frequencyText(const std::optional<pointdrone::domain::PointModel>& point)
{
    return point.has_value()
        ? "[FREQ " + frequencyValueText(point->frequencyHz) + " HZ / " + noteIndicatorText(point->frequencyHz) + "]"
        : "[FREQ --]";
}

juce::String panText(const std::optional<pointdrone::domain::PointModel>& point)
{
    if (! point.has_value())
        return "[PAN --]";

    if (std::abs(point->pan) < 0.01f)
        return "[PAN CENTER]";

    const auto side = point->pan < 0.0f ? "LEFT" : "RIGHT";
    return "[PAN " + juce::String(std::abs(point->pan) * 100.0f, 0) + "% " + side + "]";
}

std::optional<pointdrone::domain::PointModel> selectedPointFromModel(const pointdrone::domain::ProjectModel& model, const juce::String& currentSelectedPointId)
{
    for (const auto& point : model.points)
    {
        if (point.id == currentSelectedPointId)
            return point;
    }

    return std::nullopt;
}

float triangleSample(const float sawSample)
{
    return 1.0f - 2.0f * std::abs(sawSample);
}

float pulseWidth(const float rawPulseWidth)
{
    return juce::jmap(juce::jlimit(0.0f, 1.0f, rawPulseWidth), 0.0f, 1.0f, 0.1f, 0.9f);
}

float noiseCutoffHz(const float rawNoiseTone)
{
    return std::exp(juce::jmap(juce::jlimit(0.0f, 1.0f, rawNoiseTone),
                               0.0f,
                               1.0f,
                               std::log(120.0f),
                               std::log(18000.0f)));
}

float lowPassNoise(const float input, const float cutoffHz, float& state)
{
    const auto coefficient = std::exp(-juce::MathConstants<double>::twoPi * static_cast<double>(cutoffHz) / previewSampleRate);
    state = static_cast<float>((1.0 - coefficient) * static_cast<double>(input) + coefficient * static_cast<double>(state));
    return state;
}

float snappedSemitoneFrequency(const float frequencyHz)
{
    const auto midiNoteFloat = 69.0 + (12.0 * std::log2(static_cast<double>(frequencyHz) / 440.0));
    const auto nearestMidiNote = static_cast<int>(std::round(midiNoteFloat));
    const auto snappedFrequency = 440.0 * std::pow(2.0, (static_cast<double>(nearestMidiNote) - 69.0) / 12.0);
    return juce::jlimit(pointdrone::core::minFrequencyHz,
                        pointdrone::core::maxFrequencyHz,
                        static_cast<float>(snappedFrequency));
}

std::optional<float> parseFrequencyInput(const juce::String& rawText)
{
    auto text = rawText.trim().toUpperCase().removeCharacters(" ");

    if (text.isEmpty())
        return std::nullopt;

    static const std::regex notePattern("^([A-G])(#?)(-?\\d+)$");
    std::smatch noteMatch;
    const std::string noteText = text.toStdString();

    if (std::regex_match(noteText, noteMatch, notePattern))
    {
        const auto letter = noteMatch[1].str()[0];
        const auto sharp = noteMatch[2].str() == "#";
        const auto octave = std::stoi(noteMatch[3].str());

        int semitone = 0;

        switch (letter)
        {
            case 'C': semitone = 0; break;
            case 'D': semitone = 2; break;
            case 'E': semitone = 4; break;
            case 'F': semitone = 5; break;
            case 'G': semitone = 7; break;
            case 'A': semitone = 9; break;
            case 'B': semitone = 11; break;
            default: return std::nullopt;
        }

        if (sharp)
            ++semitone;

        const auto midiNote = (octave + 1) * 12 + semitone;
        const auto frequencyHz = 440.0 * std::pow(2.0, (static_cast<double>(midiNote) - 69.0) / 12.0);
        return juce::jlimit(pointdrone::core::minFrequencyHz,
                            pointdrone::core::maxFrequencyHz,
                            static_cast<float>(frequencyHz));
    }

    if (text.endsWith("HZ"))
        text = text.dropLastCharacters(2);

    text = text.replaceCharacter(',', '.');

    static const std::regex numericPattern("^\\d+(\\.\\d+)?$");
    const std::string numericText = text.toStdString();

    if (! std::regex_match(numericText, numericPattern))
        return std::nullopt;

    return juce::jlimit(pointdrone::core::minFrequencyHz,
                        pointdrone::core::maxFrequencyHz,
                        static_cast<float>(std::stod(numericText)));
}

std::optional<float> parsePanInput(const juce::String& rawText)
{
    auto text = rawText.trim().removeCharacters(" ");

    if (text.isEmpty())
        return std::nullopt;

    if (text.endsWith("%"))
        text = text.dropLastCharacters(1);

    auto sign = 1.0f;

    if (text.startsWithChar('+'))
        text = text.substring(1);
    else if (text.startsWithChar('-'))
    {
        sign = -1.0f;
        text = text.substring(1);
    }

    text = text.replaceCharacter(',', '.');

    static const std::regex numericPattern("^\\d+(\\.\\d+)?$");
    const std::string numericText = text.toStdString();

    if (! std::regex_match(numericText, numericPattern))
        return std::nullopt;

    const auto percentage = juce::jlimit(0.0f, 100.0f, static_cast<float>(std::stod(numericText)));
    return juce::jlimit(-1.0f, 1.0f, sign * (percentage / 100.0f));
}
}

EditorController::EditorController(pointdrone::state::ProjectState& projectState)
    : state(projectState)
{
}

EditorViewState EditorController::getViewState()
{
    syncSelectionWithState();

    const auto model = state.getModel();

    return {
        createChartViewModel(model, selectedPointId),
        createWavePreviewViewModel(model, selectedPointId),
        createInspectorViewModel(model, selectedPointId),
        createMasterOutputViewModel(model)
    };
}

void EditorController::handleChartBackgroundClicked(const float normalizedX, const float normalizedY)
{
    const auto point = state.addPoint(pointdrone::core::xToFrequency(normalizedX),
                                      pointdrone::core::yToPan(normalizedY));
    selectedPointId = point.id;
}

void EditorController::handlePointClicked(const juce::String& pointId)
{
    selectedPointId = pointId;
    syncSelectionWithState();
}

void EditorController::handlePointDragged(const juce::String& pointId, const float normalizedX, const float normalizedY)
{
    selectedPointId = pointId;

    if (! state.updatePointPosition(pointId,
                                    pointdrone::core::xToFrequency(normalizedX),
                                    pointdrone::core::yToPan(normalizedY)))
    {
        syncSelectionWithState();
    }
}

void EditorController::handlePointDoubleClicked(const juce::String& pointId)
{
    if (! state.removePoint(pointId))
        return;

    if (selectedPointId == pointId)
        selectedPointId.clear();
}

void EditorController::handleWaveTimbreChanged(const pointdrone::domain::WaveTimbre& waveTimbre)
{
    syncSelectionWithState();

    if (selectedPointId.isNotEmpty())
        state.updatePointWaveTimbre(selectedPointId, waveTimbre);
}

void EditorController::handleWaveMixChanged(const pointdrone::domain::WaveMix& waveMix)
{
    syncSelectionWithState();

    if (selectedPointId.isNotEmpty())
        state.updatePointWaveMix(selectedPointId, waveMix);
}

void EditorController::handleGainChanged(const float gain)
{
    syncSelectionWithState();

    if (selectedPointId.isNotEmpty())
        state.updatePointGain(selectedPointId, gain);
}

void EditorController::handleOutputGainChanged(const float gain)
{
    state.updateOutputGain(gain);
}

void EditorController::handleSnapAllPointsToSemitone()
{
    const auto model = state.getModel();

    for (const auto& point : model.points)
        state.updatePointPosition(point.id, snappedSemitoneFrequency(point.frequencyHz), point.pan);
}

bool EditorController::handleFrequencyInputSubmitted(const juce::String& text)
{
    syncSelectionWithState();

    if (selectedPointId.isEmpty())
        return false;

    const auto model = state.getModel();
    const auto selectedPoint = selectedPointFromModel(model, selectedPointId);
    const auto parsedFrequency = parseFrequencyInput(text);

    if (! selectedPoint.has_value() || ! parsedFrequency.has_value())
        return false;

    return state.updatePointPosition(selectedPointId, *parsedFrequency, selectedPoint->pan);
}

bool EditorController::handlePanInputSubmitted(const juce::String& text)
{
    syncSelectionWithState();

    if (selectedPointId.isEmpty())
        return false;

    const auto model = state.getModel();
    const auto selectedPoint = selectedPointFromModel(model, selectedPointId);
    const auto parsedPan = parsePanInput(text);

    if (! selectedPoint.has_value() || ! parsedPan.has_value())
        return false;

    return state.updatePointPosition(selectedPointId, selectedPoint->frequencyHz, *parsedPan);
}

void EditorController::syncSelectionWithState()
{
    if (selectedPointId.isNotEmpty() && ! state.containsPoint(selectedPointId))
        selectedPointId.clear();
}

ChartViewModel EditorController::createChartViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& currentSelectedPointId)
{
    ChartViewModel viewModel;
    viewModel.points.reserve(model.points.size());

    for (const auto& point : model.points)
    {
        viewModel.points.push_back({
            point.id,
            pointdrone::core::frequencyToX(point.frequencyHz),
            pointdrone::core::panToY(point.pan),
            point.id == currentSelectedPointId
        });
    }

    return viewModel;
}

PointWavePreviewViewModel EditorController::createWavePreviewViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& currentSelectedPointId)
{
    const auto selectedPoint = selectedPointFromModel(model, currentSelectedPointId);
    PointWavePreviewViewModel viewModel;

    if (! selectedPoint.has_value())
        return viewModel;

    viewModel.hasSelection = true;
    viewModel.samples.reserve(previewSampleCount);

    juce::Random random(selectedPoint->id.hashCode64());
    float filteredNoise = 0.0f;

    for (int sampleIndex = 0; sampleIndex < previewSampleCount; ++sampleIndex)
    {
        const auto phase = juce::jmap(static_cast<float>(sampleIndex),
                                      0.0f,
                                      static_cast<float>(previewSampleCount - 1),
                                      0.0f,
                                      previewCycles * juce::MathConstants<float>::twoPi);
        const auto wrappedPhase = std::fmod(phase, juce::MathConstants<float>::twoPi);

        float mixedSample = 0.0f;

        const auto rawSawSample = (wrappedPhase / juce::MathConstants<float>::pi) - 1.0f;
        const auto sineSample = std::sin(wrappedPhase + selectedPoint->waveTimbre.sinePhase * juce::MathConstants<float>::twoPi);
        const auto sawSample = rawSawSample + selectedPoint->waveTimbre.sawShape * (triangleSample(rawSawSample) - rawSawSample);
        const auto squareSample = wrappedPhase < pulseWidth(selectedPoint->waveTimbre.squarePulseWidth) * juce::MathConstants<float>::twoPi ? 1.0f : -1.0f;
        const auto noiseSample = lowPassNoise(random.nextFloat() * 2.0f - 1.0f,
                                              noiseCutoffHz(selectedPoint->waveTimbre.noiseTone),
                                              filteredNoise);

        mixedSample = (selectedPoint->waveMix.sine * sineSample)
                    + (selectedPoint->waveMix.saw * sawSample)
                    + (selectedPoint->waveMix.square * squareSample)
                    + (selectedPoint->waveMix.noise * noiseSample);

        viewModel.samples.push_back(juce::jlimit(-1.0f, 1.0f, mixedSample * selectedPoint->gain));
    }

    return viewModel;
}

InspectorViewModel EditorController::createInspectorViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& currentSelectedPointId)
{
    const auto selectedPoint = selectedPointFromModel(model, currentSelectedPointId);

    InspectorViewModel viewModel;
    viewModel.hasSelection = selectedPoint.has_value();
    viewModel.frequencyHz = selectedPoint.has_value() ? selectedPoint->frequencyHz : 0.0f;
    viewModel.pan = selectedPoint.has_value() ? selectedPoint->pan : 0.0f;
    viewModel.waveTimbre = selectedPoint.has_value() ? selectedPoint->waveTimbre : pointdrone::domain::WaveTimbre {};
    viewModel.waveMix = selectedPoint.has_value() ? selectedPoint->waveMix : pointdrone::domain::WaveMix {};
    viewModel.gain = selectedPoint.has_value() ? selectedPoint->gain : 1.0f;
    viewModel.frequencyText = frequencyText(selectedPoint);
    viewModel.panText = panText(selectedPoint);
    return viewModel;
}

MasterOutputViewModel EditorController::createMasterOutputViewModel(const pointdrone::domain::ProjectModel& model)
{
    MasterOutputViewModel viewModel;
    viewModel.gain = model.outputGain;
    return viewModel;
}
}
