#include "PointVoice.h"

#include <cmath>

namespace pointdrone::audio
{
namespace
{
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
}

void PointVoice::render(juce::AudioBuffer<float>& outputBuffer, const int numSamples, const domain::PointModel& point)
{
    if (! prepared)
    {
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

        float mixedSample = 0.0f;

        const auto wrappedPhase = std::fmod(static_cast<float>(phase), juce::MathConstants<float>::twoPi);
        const auto rawSawSample = (wrappedPhase / juce::MathConstants<float>::pi) - 1.0f;
        const auto sineSample = std::sin(wrappedPhase + currentSinePhase * juce::MathConstants<float>::twoPi);
        const auto sawSample = rawSawSample + currentSawShape * (triangleSample(rawSawSample) - rawSawSample);
        const auto squareSample = wrappedPhase < pulseWidth(currentSquarePulseWidth) * juce::MathConstants<float>::twoPi ? 1.0f : -1.0f;
        const auto noiseSample = lowPassNoise(random.nextFloat() * 2.0f - 1.0f,
                                              noiseCutoffHz(currentNoiseTone),
                                              sampleRate,
                                              filteredNoise);

        mixedSample = (sineLevel * sineSample)
                    + (sawLevel * sawSample)
                    + (squareLevel * squareSample)
                    + (noiseLevel * noiseSample);

        mixedSample *= 0.18f * gain.getNextValue();

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
