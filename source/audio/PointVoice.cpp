#include "PointVoice.h"

#include <cmath>

namespace pointdrone::audio
{
namespace
{
float modulatedValue(const float baseValue,
                     pointdrone::core::RandomModulator& modulator,
                     const pointdrone::domain::ParameterModulation& modulation)
{
    if (! modulation.enabled)
        return juce::jlimit(0.0f, 1.0f, baseValue);

    return pointdrone::core::RandomModulator::mapToNormalizedRange(modulator.getNextValue(modulation.settings));
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

float lowPassNoise(const float input, const float cutoffHz, const double sampleRate, float& state)
{
    const auto coefficient = std::exp(-juce::MathConstants<double>::twoPi * static_cast<double>(cutoffHz) / sampleRate);
    state = static_cast<float>((1.0 - coefficient) * static_cast<double>(input) + coefficient * static_cast<double>(state));
    return state;
}
}

void PointVoice::prepare(const double newSampleRate)
{
    sampleRate = newSampleRate;
    frequencyHz.reset(sampleRate, 0.03);
    pan.reset(sampleRate, 0.03);
    gain.reset(sampleRate, 0.03);
    sinePhase.reset(sampleRate, 0.03);
    sawShape.reset(sampleRate, 0.03);
    squarePulseWidth.reset(sampleRate, 0.03);
    noiseTone.reset(sampleRate, 0.03);
    sine.reset(sampleRate, 0.03);
    saw.reset(sampleRate, 0.03);
    square.reset(sampleRate, 0.03);
    noise.reset(sampleRate, 0.03);
    filteredNoise = 0.0f;
    prepared = false;
    pointId.clear();
    runtimeTelemetry = {};
    waveformWriteIndex = 0;
}

void PointVoice::render(juce::AudioBuffer<float>& outputBuffer, const int numSamples, const domain::PointModel& point)
{
    if (! prepared || pointId != point.id)
    {
        pointId = point.id;
        frequencyHz.setCurrentAndTargetValue(point.frequencyHz);
        pan.setCurrentAndTargetValue(point.pan);
        gain.setCurrentAndTargetValue(point.gain);
        sinePhase.setCurrentAndTargetValue(point.waveTimbre.sinePhase);
        sawShape.setCurrentAndTargetValue(point.waveTimbre.sawShape);
        squarePulseWidth.setCurrentAndTargetValue(point.waveTimbre.squarePulseWidth);
        noiseTone.setCurrentAndTargetValue(point.waveTimbre.noiseTone);
        sine.setCurrentAndTargetValue(point.waveMix.sine);
        saw.setCurrentAndTargetValue(point.waveMix.saw);
        square.setCurrentAndTargetValue(point.waveMix.square);
        noise.setCurrentAndTargetValue(point.waveMix.noise);
        for (const auto target : domain::allModulationTargets)
            modulators[domain::modulationIndex(target)].prepare(sampleRate, pointdrone::core::RandomModulator::seedForTarget(point.id, target));
        prepared = true;
    }

    frequencyHz.setTargetValue(point.frequencyHz);
    pan.setTargetValue(point.pan);
    gain.setTargetValue(point.gain);
    sinePhase.setTargetValue(point.waveTimbre.sinePhase);
    sawShape.setTargetValue(point.waveTimbre.sawShape);
    squarePulseWidth.setTargetValue(point.waveTimbre.squarePulseWidth);
    noiseTone.setTargetValue(point.waveTimbre.noiseTone);
    sine.setTargetValue(point.waveMix.sine);
    saw.setTargetValue(point.waveMix.saw);
    square.setTargetValue(point.waveMix.square);
    noise.setTargetValue(point.waveMix.noise);

    const auto channelCount = juce::jmin(2, outputBuffer.getNumChannels());

    if (channelCount == 0)
        return;

    for (int sampleIndex = 0; sampleIndex < numSamples; ++sampleIndex)
    {
        const auto currentFrequency = juce::jmax(1.0f, frequencyHz.getNextValue());
        const auto phaseIncrement = juce::MathConstants<double>::twoPi * static_cast<double>(currentFrequency) / sampleRate;

        const auto sineLevel = sine.getNextValue();
        const auto sawLevel = saw.getNextValue();
        const auto squareLevel = square.getNextValue();
        const auto noiseLevel = noise.getNextValue();
        const auto currentSinePhase = sinePhase.getNextValue();
        const auto currentSawShape = sawShape.getNextValue();
        const auto currentSquarePulseWidth = squarePulseWidth.getNextValue();
        const auto currentNoiseTone = noiseTone.getNextValue();
        const auto modulatedSinePhase = modulatedValue(currentSinePhase,
                                                      modulators[domain::modulationIndex(domain::ModulationTarget::sinePhase)],
                                                      domain::modulationFor(point, domain::ModulationTarget::sinePhase));
        const auto modulatedSawShape = modulatedValue(currentSawShape,
                                                     modulators[domain::modulationIndex(domain::ModulationTarget::sawShape)],
                                                     domain::modulationFor(point, domain::ModulationTarget::sawShape));
        const auto modulatedSquarePulseWidth = modulatedValue(currentSquarePulseWidth,
                                                              modulators[domain::modulationIndex(domain::ModulationTarget::squarePulseWidth)],
                                                              domain::modulationFor(point, domain::ModulationTarget::squarePulseWidth));
        const auto modulatedNoiseTone = modulatedValue(currentNoiseTone,
                                                       modulators[domain::modulationIndex(domain::ModulationTarget::noiseTone)],
                                                       domain::modulationFor(point, domain::ModulationTarget::noiseTone));
        const auto modulatedSineLevel = modulatedValue(sineLevel,
                                                       modulators[domain::modulationIndex(domain::ModulationTarget::sine)],
                                                       domain::modulationFor(point, domain::ModulationTarget::sine));
        const auto modulatedSawLevel = modulatedValue(sawLevel,
                                                      modulators[domain::modulationIndex(domain::ModulationTarget::saw)],
                                                      domain::modulationFor(point, domain::ModulationTarget::saw));
        const auto modulatedSquareLevel = modulatedValue(squareLevel,
                                                         modulators[domain::modulationIndex(domain::ModulationTarget::square)],
                                                         domain::modulationFor(point, domain::ModulationTarget::square));
        const auto modulatedNoiseLevel = modulatedValue(noiseLevel,
                                                        modulators[domain::modulationIndex(domain::ModulationTarget::noise)],
                                                        domain::modulationFor(point, domain::ModulationTarget::noise));
        const auto modulatedGain = modulatedValue(gain.getNextValue(),
                                                  modulators[domain::modulationIndex(domain::ModulationTarget::gain)],
                                                  domain::modulationFor(point, domain::ModulationTarget::gain));
        runtimeTelemetry.modulatedValues[domain::modulationIndex(domain::ModulationTarget::sinePhase)] = modulatedSinePhase;
        runtimeTelemetry.modulatedValues[domain::modulationIndex(domain::ModulationTarget::sawShape)] = modulatedSawShape;
        runtimeTelemetry.modulatedValues[domain::modulationIndex(domain::ModulationTarget::squarePulseWidth)] = modulatedSquarePulseWidth;
        runtimeTelemetry.modulatedValues[domain::modulationIndex(domain::ModulationTarget::noiseTone)] = modulatedNoiseTone;
        runtimeTelemetry.modulatedValues[domain::modulationIndex(domain::ModulationTarget::sine)] = modulatedSineLevel;
        runtimeTelemetry.modulatedValues[domain::modulationIndex(domain::ModulationTarget::saw)] = modulatedSawLevel;
        runtimeTelemetry.modulatedValues[domain::modulationIndex(domain::ModulationTarget::square)] = modulatedSquareLevel;
        runtimeTelemetry.modulatedValues[domain::modulationIndex(domain::ModulationTarget::noise)] = modulatedNoiseLevel;
        runtimeTelemetry.modulatedValues[domain::modulationIndex(domain::ModulationTarget::gain)] = modulatedGain;

        float mixedSample = 0.0f;

        const auto wrappedPhase = std::fmod(static_cast<float>(phase), juce::MathConstants<float>::twoPi);
        const auto rawSawSample = (wrappedPhase / juce::MathConstants<float>::pi) - 1.0f;
        const auto sineSample = std::sin(wrappedPhase + modulatedSinePhase * juce::MathConstants<float>::twoPi);
        const auto sawSample = rawSawSample + modulatedSawShape * (triangleSample(rawSawSample) - rawSawSample);
        const auto squareSample = wrappedPhase < pulseWidth(modulatedSquarePulseWidth) * juce::MathConstants<float>::twoPi ? 1.0f : -1.0f;
        const auto noiseSample = lowPassNoise(random.nextFloat() * 2.0f - 1.0f,
                                              noiseCutoffHz(modulatedNoiseTone),
                                              sampleRate,
                                              filteredNoise);

        mixedSample = (modulatedSineLevel * sineSample)
                    + (modulatedSawLevel * sawSample)
                    + (modulatedSquareLevel * squareSample)
                    + (modulatedNoiseLevel * noiseSample);

        mixedSample *= 0.18f * modulatedGain;
        runtimeTelemetry.waveform[static_cast<std::size_t>(waveformWriteIndex)] = juce::jlimit(-1.0f, 1.0f, mixedSample);
        waveformWriteIndex = (waveformWriteIndex + 1) % pointRuntimeWaveformSampleCount;
        runtimeTelemetry.active = true;

        const auto currentPan = pan.getNextValue();
        const auto leftGain = panLeftGain(currentPan);
        const auto rightGain = panRightGain(currentPan);

        outputBuffer.addSample(0, sampleIndex, mixedSample * leftGain);

        if (channelCount > 1)
            outputBuffer.addSample(1, sampleIndex, mixedSample * rightGain);

        phase += phaseIncrement;

        while (phase >= juce::MathConstants<double>::twoPi)
            phase -= juce::MathConstants<double>::twoPi;
    }
}

PointRuntimeTelemetry PointVoice::getRuntimeTelemetry() const
{
    auto telemetry = runtimeTelemetry;
    std::array<float, pointRuntimeWaveformSampleCount> orderedWaveform {};

    for (int index = 0; index < pointRuntimeWaveformSampleCount; ++index)
        orderedWaveform[static_cast<std::size_t>(index)] = runtimeTelemetry.waveform[static_cast<std::size_t>((waveformWriteIndex + index) % pointRuntimeWaveformSampleCount)];

    telemetry.waveform = orderedWaveform;
    return telemetry;
}

float PointVoice::panLeftGain(const float panValue)
{
    const auto angle = juce::jmap(juce::jlimit(-1.0f, 1.0f, panValue), -1.0f, 1.0f, 0.0f, juce::MathConstants<float>::halfPi);
    return std::cos(angle);
}

float PointVoice::panRightGain(const float panValue)
{
    const auto angle = juce::jmap(juce::jlimit(-1.0f, 1.0f, panValue), -1.0f, 1.0f, 0.0f, juce::MathConstants<float>::halfPi);
    return std::sin(angle);
}
}
