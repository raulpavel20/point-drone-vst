#include "PointVoice.h"

#include <cmath>

namespace pointdrone::audio
{
void PointVoice::prepare(const double newSampleRate)
{
    sampleRate = newSampleRate;
    frequencyHz.reset(sampleRate, 0.03);
    pan.reset(sampleRate, 0.03);
    sine.reset(sampleRate, 0.03);
    saw.reset(sampleRate, 0.03);
    square.reset(sampleRate, 0.03);
    noise.reset(sampleRate, 0.03);
    prepared = false;
}

void PointVoice::render(juce::AudioBuffer<float>& outputBuffer, const int numSamples, const domain::PointModel& point)
{
    if (! prepared)
    {
        frequencyHz.setCurrentAndTargetValue(point.frequencyHz);
        pan.setCurrentAndTargetValue(point.pan);
        sine.setCurrentAndTargetValue(point.waveMix.sine);
        saw.setCurrentAndTargetValue(point.waveMix.saw);
        square.setCurrentAndTargetValue(point.waveMix.square);
        noise.setCurrentAndTargetValue(point.waveMix.noise);
        prepared = true;
    }

    frequencyHz.setTargetValue(point.frequencyHz);
    pan.setTargetValue(point.pan);
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

        const auto total = mixWeightTotal({ sineLevel, sawLevel, squareLevel, noiseLevel });

        float mixedSample = 0.0f;

        if (total > 0.0f)
        {
            const auto normalizedSine = sineLevel / total;
            const auto normalizedSaw = sawLevel / total;
            const auto normalizedSquare = squareLevel / total;
            const auto normalizedNoise = noiseLevel / total;

            const auto sineSample = std::sin(static_cast<float>(phase));
            const auto sawSample = static_cast<float>((phase / juce::MathConstants<double>::pi) - 1.0);
            const auto squareSample = phase < juce::MathConstants<double>::pi ? 1.0f : -1.0f;
            const auto noiseSample = random.nextFloat() * 2.0f - 1.0f;

            mixedSample = (normalizedSine * sineSample)
                        + (normalizedSaw * sawSample)
                        + (normalizedSquare * squareSample)
                        + (normalizedNoise * noiseSample);
        }

        mixedSample *= 0.18f;

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

float PointVoice::mixWeightTotal(const domain::WaveMix& waveMix)
{
    return juce::jmax(0.0f, waveMix.sine)
         + juce::jmax(0.0f, waveMix.saw)
         + juce::jmax(0.0f, waveMix.square)
         + juce::jmax(0.0f, waveMix.noise);
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
