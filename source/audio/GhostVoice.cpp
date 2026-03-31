#include "GhostVoice.h"

#include <cmath>

namespace pointdrone::audio
{
void GhostVoice::prepare(const double newSampleRate)
{
    sampleRate = newSampleRate;
    phase = 0.0;
    active = false;
    frequencyHz.reset(sampleRate, 0.05);
    amplitude.reset(sampleRate, 0.08);
    panValue.reset(sampleRate, 0.05);
    frequencyHz.setCurrentAndTargetValue(0.0f);
    amplitude.setCurrentAndTargetValue(0.0f);
    panValue.setCurrentAndTargetValue(0.0f);
    shimmerPhase = random.nextFloat();
    shimmerRate = 0.3f + random.nextFloat() * 0.7f;
}

void GhostVoice::assign(const float targetFrequencyHz, const float targetStrength, const float pan)
{
    if (! active)
    {
        frequencyHz.setCurrentAndTargetValue(targetFrequencyHz);
        panValue.setCurrentAndTargetValue(pan);
        amplitude.setCurrentAndTargetValue(0.0f);
        active = true;
    }

    frequencyHz.setTargetValue(targetFrequencyHz);
    amplitude.setTargetValue(targetStrength * 0.025f);
    panValue.setTargetValue(pan);
}

void GhostVoice::release()
{
    amplitude.setTargetValue(0.0f);
}

void GhostVoice::render(juce::AudioBuffer<float>& outputBuffer, const int numSamples)
{
    if (! active)
        return;

    const auto channelCount = juce::jmin(2, outputBuffer.getNumChannels());

    if (channelCount == 0)
        return;

    for (int i = 0; i < numSamples; ++i)
    {
        const auto freq = juce::jmax(1.0f, frequencyHz.getNextValue());
        const auto amp = amplitude.getNextValue();

        if (amp < 1.0e-6f && amplitude.getTargetValue() < 1.0e-6f)
        {
            active = false;
            return;
        }

        const auto shimmerIncrement = static_cast<float>(static_cast<double>(shimmerRate) / sampleRate);
        shimmerPhase += shimmerIncrement;
        if (shimmerPhase >= 1.0f)
            shimmerPhase -= 1.0f;

        const auto shimmer = 0.5f + 0.5f * std::sin(shimmerPhase * juce::MathConstants<float>::twoPi);

        const auto phaseIncrement = juce::MathConstants<double>::twoPi * static_cast<double>(freq) / sampleRate;
        const auto sample = static_cast<float>(std::sin(phase)) * amp * shimmer;

        const auto currentPan = panValue.getNextValue();
        outputBuffer.addSample(0, i, sample * panLeftGain(currentPan));

        if (channelCount > 1)
            outputBuffer.addSample(1, i, sample * panRightGain(currentPan));

        phase += phaseIncrement;
        while (phase >= juce::MathConstants<double>::twoPi)
            phase -= juce::MathConstants<double>::twoPi;
    }
}

bool GhostVoice::isActive() const
{
    return active;
}

float GhostVoice::panLeftGain(const float pan)
{
    const auto angle = juce::jmap(juce::jlimit(-1.0f, 1.0f, pan), -1.0f, 1.0f, 0.0f, juce::MathConstants<float>::halfPi);
    return std::cos(angle);
}

float GhostVoice::panRightGain(const float pan)
{
    const auto angle = juce::jmap(juce::jlimit(-1.0f, 1.0f, pan), -1.0f, 1.0f, 0.0f, juce::MathConstants<float>::halfPi);
    return std::sin(angle);
}
}
