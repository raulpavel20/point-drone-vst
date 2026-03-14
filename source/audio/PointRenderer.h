#pragma once

#include "../domain/ProjectModel.h"
#include "PointVoice.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <unordered_map>

namespace pointdrone::audio
{
class PointRenderer
{
public:
    void prepare(double sampleRate, int maximumExpectedSamplesPerBlock);
    void render(const domain::ProjectModel& model, juce::AudioBuffer<float>& outputBuffer);

private:
    double currentSampleRate = 44100.0;
    bool hasPrepared = false;
    juce::SmoothedValue<float> outputGain;
    std::unordered_map<std::string, PointVoice> voices;
};
}
