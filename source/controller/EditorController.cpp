#include "EditorController.h"

#include "../core/ChartMapping.h"
#include "../core/RandomModulator.h"

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
constexpr int modulationPreviewSampleCount = 256;
constexpr float maximumSnapshotTransitionSeconds = 10.0f;

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

juce::String modulationTargetTitle(const pointdrone::domain::ModulationTarget target)
{
    using Target = pointdrone::domain::ModulationTarget;

    switch (target)
    {
        case Target::sinePhase: return "[SINE PHASE MOD]";
        case Target::sawShape: return "[SAW SHAPE MOD]";
        case Target::squarePulseWidth: return "[SQUARE WIDTH MOD]";
        case Target::noiseTone: return "[NOISE TONE MOD]";
        case Target::sine: return "[SINE LEVEL MOD]";
        case Target::saw: return "[SAW LEVEL MOD]";
        case Target::square: return "[SQUARE LEVEL MOD]";
        case Target::noise: return "[NOISE LEVEL MOD]";
        case Target::gain: return "[GAIN MOD]";
    }

    return "[MOD]";
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

std::optional<pointdrone::domain::PointModel> pointById(const std::vector<pointdrone::domain::PointModel>& points, const juce::String& pointId)
{
    for (const auto& point : points)
    {
        if (point.id == pointId)
            return point;
    }

    return std::nullopt;
}

float interpolateLinear(const float startValue, const float endValue, const float amount)
{
    return juce::jmap(juce::jlimit(0.0f, 1.0f, amount), startValue, endValue);
}

float interpolateFrequencyHz(const float startValue, const float endValue, const float amount)
{
    const auto clampedAmount = juce::jlimit(0.0f, 1.0f, amount);
    const auto startLog = std::log(juce::jmax(1.0f, startValue));
    const auto endLog = std::log(juce::jmax(1.0f, endValue));
    return static_cast<float>(std::exp(juce::jmap(clampedAmount, startLog, endLog)));
}

pointdrone::domain::PointModel interpolatePoint(const pointdrone::domain::PointModel& sourcePoint,
                                                const pointdrone::domain::PointModel& targetPoint,
                                                const float amount)
{
    auto point = sourcePoint;
    point.frequencyHz = interpolateFrequencyHz(sourcePoint.frequencyHz, targetPoint.frequencyHz, amount);
    point.pan = interpolateLinear(sourcePoint.pan, targetPoint.pan, amount);
    point.gain = interpolateLinear(sourcePoint.gain, targetPoint.gain, amount);
    point.waveTimbre.sinePhase = interpolateLinear(sourcePoint.waveTimbre.sinePhase, targetPoint.waveTimbre.sinePhase, amount);
    point.waveTimbre.sawShape = interpolateLinear(sourcePoint.waveTimbre.sawShape, targetPoint.waveTimbre.sawShape, amount);
    point.waveTimbre.squarePulseWidth = interpolateLinear(sourcePoint.waveTimbre.squarePulseWidth, targetPoint.waveTimbre.squarePulseWidth, amount);
    point.waveTimbre.noiseTone = interpolateLinear(sourcePoint.waveTimbre.noiseTone, targetPoint.waveTimbre.noiseTone, amount);
    point.waveMix.sine = interpolateLinear(sourcePoint.waveMix.sine, targetPoint.waveMix.sine, amount);
    point.waveMix.saw = interpolateLinear(sourcePoint.waveMix.saw, targetPoint.waveMix.saw, amount);
    point.waveMix.square = interpolateLinear(sourcePoint.waveMix.square, targetPoint.waveMix.square, amount);
    point.waveMix.noise = interpolateLinear(sourcePoint.waveMix.noise, targetPoint.waveMix.noise, amount);

    for (const auto target : pointdrone::domain::allModulationTargets)
    {
        auto& modulation = pointdrone::domain::modulationFor(point, target);
        const auto& sourceModulation = pointdrone::domain::modulationFor(sourcePoint, target);
        const auto& targetModulation = pointdrone::domain::modulationFor(targetPoint, target);
        modulation.settings.amplitude = interpolateLinear(sourceModulation.settings.amplitude, targetModulation.settings.amplitude, amount);
        modulation.settings.frequency = interpolateLinear(sourceModulation.settings.frequency, targetModulation.settings.frequency, amount);
        modulation.settings.ease = interpolateLinear(sourceModulation.settings.ease, targetModulation.settings.ease, amount);
        modulation.settings.slant = interpolateLinear(sourceModulation.settings.slant, targetModulation.settings.slant, amount);
        modulation.settings.cyclic = interpolateLinear(sourceModulation.settings.cyclic, targetModulation.settings.cyclic, amount);
        modulation.settings.jitter = interpolateLinear(sourceModulation.settings.jitter, targetModulation.settings.jitter, amount);
    }

    return point;
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

juce::String EditorController::getSelectedPointId() const
{
    return selectedPointId;
}

void EditorController::clearActiveSnapshotPlayback(const bool keepActiveSlot)
{
    activeSnapshotMorph.reset();

    if (! keepActiveSlot)
        activeSnapshotSlotIndex.reset();
}

PointWavePreviewViewModel EditorController::getLiveWavePreviewViewModel(const pointdrone::audio::PointRuntimeTelemetry& telemetry)
{
    syncSelectionWithState();

    const auto model = state.getModel();
    const auto selectedPoint = selectedPointFromModel(model, selectedPointId);

    if (! selectedPoint.has_value())
        return {};

    auto point = *selectedPoint;
    point.waveTimbre.sinePhase = telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::sinePhase)];
    point.waveTimbre.sawShape = telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::sawShape)];
    point.waveTimbre.squarePulseWidth = telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::squarePulseWidth)];
    point.waveTimbre.noiseTone = telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::noiseTone)];
    point.waveMix.sine = telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::sine)];
    point.waveMix.saw = telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::saw)];
    point.waveMix.square = telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::square)];
    point.waveMix.noise = telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::noise)];
    point.gain = telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::gain)];
    return createWavePreviewViewModel(point);
}

EditorViewState EditorController::getViewState()
{
    syncSelectionWithState();

    const auto model = state.getModel();

    return {
        createChartViewModel(model, selectedPointId),
        createWavePreviewViewModel(model, selectedPointId),
        createInspectorViewModel(model, selectedPointId),
        createMasterOutputViewModel(model),
        createSnapshotControlsViewModel(model),
        createModulationPopupViewModel(model, selectedPointId, editingModulationTarget)
    };
}

void EditorController::handleChartBackgroundClicked(const float normalizedX, const float normalizedY)
{
    clearActiveSnapshotPlayback();
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
    clearActiveSnapshotPlayback();
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
    clearActiveSnapshotPlayback();
    if (! state.removePoint(pointId))
        return;

    if (selectedPointId == pointId)
        selectedPointId.clear();
}

void EditorController::handleWaveTimbreChanged(const pointdrone::domain::WaveTimbre& waveTimbre)
{
    clearActiveSnapshotPlayback();
    syncSelectionWithState();

    if (selectedPointId.isNotEmpty())
        state.updatePointWaveTimbre(selectedPointId, waveTimbre);
}

void EditorController::handleWaveMixChanged(const pointdrone::domain::WaveMix& waveMix)
{
    clearActiveSnapshotPlayback();
    syncSelectionWithState();

    if (selectedPointId.isNotEmpty())
        state.updatePointWaveMix(selectedPointId, waveMix);
}

void EditorController::handleGainChanged(const float gain)
{
    clearActiveSnapshotPlayback();
    syncSelectionWithState();

    if (selectedPointId.isNotEmpty())
        state.updatePointGain(selectedPointId, gain);
}

void EditorController::handleOutputGainChanged(const float gain)
{
    state.updateOutputGain(gain);
}

void EditorController::handleModulationRequested(const pointdrone::domain::ModulationTarget target)
{
    syncSelectionWithState();

    if (selectedPointId.isEmpty())
        return;

    const auto model = state.getModel();
    const auto selectedPoint = selectedPointFromModel(model, selectedPointId);

    if (! selectedPoint.has_value())
        return;

    if (! pointdrone::domain::modulationFor(*selectedPoint, target).enabled)
        state.updatePointModulationEnabled(selectedPointId, target, true);

    editingModulationTarget = target;
}

void EditorController::handleModulationDisabled()
{
    syncSelectionWithState();

    if (selectedPointId.isEmpty() || ! editingModulationTarget.has_value())
        return;

    state.updatePointModulationEnabled(selectedPointId, *editingModulationTarget, false);
    editingModulationTarget.reset();
}

void EditorController::handleModulationPopupClosed()
{
    editingModulationTarget.reset();
}

void EditorController::handleModulationSettingsChanged(const pointdrone::domain::ModulationSettings& settings)
{
    clearActiveSnapshotPlayback();
    syncSelectionWithState();

    if (selectedPointId.isEmpty() || ! editingModulationTarget.has_value())
        return;

    state.updatePointModulationSettings(selectedPointId, *editingModulationTarget, settings);
}

void EditorController::handleSnapAllPointsToSemitone()
{
    clearActiveSnapshotPlayback();
    const auto model = state.getModel();

    for (const auto& point : model.points)
        state.updatePointPosition(point.id, snappedSemitoneFrequency(point.frequencyHz), point.pan);
}

void EditorController::handleSnapshotSlotPressed(const int slotIndex, const bool saveRequested)
{
    if (slotIndex < 0 || slotIndex >= static_cast<int>(pointdrone::domain::snapshotSlotCount))
        return;

    if (saveRequested)
    {
        state.saveSnapshotSlot(slotIndex);
        return;
    }

    const auto model = state.getModel();
    const auto targetSnapshot = model.snapshots[static_cast<std::size_t>(slotIndex)];

    if (! targetSnapshot.hasData)
        return;

    activeSnapshotSlotIndex = slotIndex;

    if (model.snapshotTransitionSeconds <= 0.0f)
    {
        clearActiveSnapshotPlayback(true);
        state.applySnapshotPoints(targetSnapshot.points);
        return;
    }

    SnapshotMorphState morphState;
    morphState.targetSlotIndex = slotIndex;
    morphState.durationSeconds = model.snapshotTransitionSeconds;

    for (const auto& point : model.points)
    {
        if (const auto targetPoint = pointById(targetSnapshot.points, point.id); targetPoint.has_value())
        {
            morphState.sourcePoints.push_back(point);
            morphState.targetPoints.push_back(*targetPoint);
        }
    }

    if (morphState.sourcePoints.empty())
    {
        clearActiveSnapshotPlayback(true);
        return;
    }

    activeSnapshotMorph = std::move(morphState);
}

void EditorController::handleSnapshotTransitionSecondsChanged(const float seconds)
{
    state.updateSnapshotTransitionSeconds(juce::jlimit(0.0f, maximumSnapshotTransitionSeconds, seconds));
}

bool EditorController::advanceSnapshotMorph(const double deltaSeconds)
{
    if (! activeSnapshotMorph.has_value())
        return false;

    auto& morphState = *activeSnapshotMorph;
    morphState.elapsedSeconds = juce::jmin(morphState.durationSeconds,
                                           morphState.elapsedSeconds + static_cast<float>(juce::jmax(0.0, deltaSeconds)));

    const auto amount = morphState.durationSeconds <= 0.0f
        ? 1.0f
        : juce::jlimit(0.0f, 1.0f, morphState.elapsedSeconds / morphState.durationSeconds);

    std::vector<pointdrone::domain::PointModel> interpolatedPoints;
    interpolatedPoints.reserve(morphState.sourcePoints.size());

    for (std::size_t index = 0; index < morphState.sourcePoints.size(); ++index)
        interpolatedPoints.push_back(interpolatePoint(morphState.sourcePoints[index], morphState.targetPoints[index], amount));

    const auto changed = state.applySnapshotPoints(interpolatedPoints);

    if (amount >= 1.0f)
        activeSnapshotMorph.reset();

    return changed;
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

    if (! state.updatePointPosition(selectedPointId, *parsedFrequency, selectedPoint->pan))
        return false;

    clearActiveSnapshotPlayback();
    return true;
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

    if (! state.updatePointPosition(selectedPointId, selectedPoint->frequencyHz, *parsedPan))
        return false;

    clearActiveSnapshotPlayback();
    return true;
}

void EditorController::syncSelectionWithState()
{
    if (selectedPointId.isNotEmpty() && ! state.containsPoint(selectedPointId))
    {
        selectedPointId.clear();
        editingModulationTarget.reset();
    }
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

    if (! selectedPoint.has_value())
        return {};

    return createWavePreviewViewModel(*selectedPoint);
}

PointWavePreviewViewModel EditorController::createWavePreviewViewModel(const pointdrone::domain::PointModel& point)
{
    PointWavePreviewViewModel viewModel;
    viewModel.hasSelection = true;
    viewModel.samples.reserve(previewSampleCount);

    juce::Random random(point.id.hashCode64());
    float filteredNoise = 0.0f;

    for (int sampleIndex = 0; sampleIndex < previewSampleCount; ++sampleIndex)
    {
        const auto phase = juce::jmap(static_cast<float>(sampleIndex),
                                      0.0f,
                                      static_cast<float>(previewSampleCount - 1),
                                      0.0f,
                                      previewCycles * juce::MathConstants<float>::twoPi);
        const auto wrappedPhase = std::fmod(phase, juce::MathConstants<float>::twoPi);

        const auto rawSawSample = (wrappedPhase / juce::MathConstants<float>::pi) - 1.0f;
        const auto sineSample = std::sin(wrappedPhase + point.waveTimbre.sinePhase * juce::MathConstants<float>::twoPi);
        const auto sawSample = rawSawSample + point.waveTimbre.sawShape * (triangleSample(rawSawSample) - rawSawSample);
        const auto squareSample = wrappedPhase < pulseWidth(point.waveTimbre.squarePulseWidth) * juce::MathConstants<float>::twoPi ? 1.0f : -1.0f;
        const auto noiseSample = lowPassNoise(random.nextFloat() * 2.0f - 1.0f,
                                              noiseCutoffHz(point.waveTimbre.noiseTone),
                                              filteredNoise);

        const auto mixedSample = (point.waveMix.sine * sineSample)
                               + (point.waveMix.saw * sawSample)
                               + (point.waveMix.square * squareSample)
                               + (point.waveMix.noise * noiseSample);

        viewModel.samples.push_back(juce::jlimit(-1.0f, 1.0f, mixedSample * point.gain));
    }

    return viewModel;
}

InspectorViewModel EditorController::createInspectorViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& currentSelectedPointId)
{
    const auto selectedPoint = selectedPointFromModel(model, currentSelectedPointId);

    InspectorViewModel viewModel;
    viewModel.pointId = selectedPoint.has_value() ? selectedPoint->id : juce::String {};
    viewModel.hasSelection = selectedPoint.has_value();
    viewModel.frequencyHz = selectedPoint.has_value() ? selectedPoint->frequencyHz : 0.0f;
    viewModel.pan = selectedPoint.has_value() ? selectedPoint->pan : 0.0f;
    viewModel.waveTimbre = selectedPoint.has_value() ? selectedPoint->waveTimbre : pointdrone::domain::WaveTimbre {};
    viewModel.waveMix = selectedPoint.has_value() ? selectedPoint->waveMix : pointdrone::domain::WaveMix {};
    viewModel.gain = selectedPoint.has_value() ? selectedPoint->gain : 1.0f;

    if (selectedPoint.has_value())
    {
        viewModel.modulation.waveTimbre = {
            pointdrone::domain::modulationFor(*selectedPoint, pointdrone::domain::ModulationTarget::sinePhase).enabled,
            pointdrone::domain::modulationFor(*selectedPoint, pointdrone::domain::ModulationTarget::sawShape).enabled,
            pointdrone::domain::modulationFor(*selectedPoint, pointdrone::domain::ModulationTarget::squarePulseWidth).enabled,
            pointdrone::domain::modulationFor(*selectedPoint, pointdrone::domain::ModulationTarget::noiseTone).enabled
        };
        viewModel.modulation.waveMix = {
            pointdrone::domain::modulationFor(*selectedPoint, pointdrone::domain::ModulationTarget::sine).enabled,
            pointdrone::domain::modulationFor(*selectedPoint, pointdrone::domain::ModulationTarget::saw).enabled,
            pointdrone::domain::modulationFor(*selectedPoint, pointdrone::domain::ModulationTarget::square).enabled,
            pointdrone::domain::modulationFor(*selectedPoint, pointdrone::domain::ModulationTarget::noise).enabled
        };
        viewModel.modulation.gain = pointdrone::domain::modulationFor(*selectedPoint, pointdrone::domain::ModulationTarget::gain).enabled;
    }

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

SnapshotControlsViewModel EditorController::createSnapshotControlsViewModel(const pointdrone::domain::ProjectModel& model) const
{
    SnapshotControlsViewModel viewModel;
    viewModel.transitionSeconds = model.snapshotTransitionSeconds;

    for (std::size_t slotIndex = 0; slotIndex < pointdrone::domain::snapshotSlotCount; ++slotIndex)
    {
        viewModel.slots[slotIndex].slotIndex = static_cast<int>(slotIndex);
        viewModel.slots[slotIndex].hasData = model.snapshots[slotIndex].hasData;
        viewModel.slots[slotIndex].isActive = activeSnapshotSlotIndex.has_value() && *activeSnapshotSlotIndex == static_cast<int>(slotIndex);
        viewModel.slots[slotIndex].label = "[" + juce::String(static_cast<int>(slotIndex) + 1) + "]";
    }

    return viewModel;
}

ModulationPopupViewModel EditorController::createModulationPopupViewModel(const pointdrone::domain::ProjectModel& model,
                                                                          const juce::String& currentSelectedPointId,
                                                                          const std::optional<pointdrone::domain::ModulationTarget>& currentEditingModulationTarget)
{
    ModulationPopupViewModel viewModel;

    if (! currentEditingModulationTarget.has_value())
        return viewModel;

    const auto selectedPoint = selectedPointFromModel(model, currentSelectedPointId);

    if (! selectedPoint.has_value())
        return viewModel;

    const auto& modulation = pointdrone::domain::modulationFor(*selectedPoint, *currentEditingModulationTarget);
    viewModel.visible = true;
    viewModel.title = modulationTargetTitle(*currentEditingModulationTarget);
    viewModel.settings = modulation.settings;
    viewModel.samples = pointdrone::core::RandomModulator::createPreview(modulation.settings,
                                                                         pointdrone::core::RandomModulator::seedForTarget(selectedPoint->id,
                                                                                                                         *currentEditingModulationTarget),
                                                                         previewSampleRate,
                                                                         modulationPreviewSampleCount);

    for (auto& sample : viewModel.samples)
        sample = pointdrone::core::RandomModulator::mapToNormalizedRange(sample);

    return viewModel;
}
}
