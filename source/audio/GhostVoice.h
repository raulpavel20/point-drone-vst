#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace pointdrone::audio
{
class GhostVoice
{
public:
    void prepare(double newSampleRate);
    void assign(float targetFrequencyHz, float targetStrength, float pan);
    void release();
    void render(juce::AudioBuffer<float>& outputBuffer, int numSamples);
    bool isActive() const;

private:
    static float panLeftGain(float pan);
    static float panRightGain(float pan);

    double sampleRate = 44100.0;
    double phase = 0.0;
    bool active = false;

    juce::SmoothedValue<float> frequencyHz;
    juce::SmoothedValue<float> amplitude;
    juce::SmoothedValue<float> panValue;

    float shimmerPhase = 0.0f;
    float shimmerRate = 0.5f;
    juce::Random random;
};
}
