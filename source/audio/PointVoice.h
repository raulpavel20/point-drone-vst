#pragma once

#include "../core/RandomModulator.h"
#include "../domain/PointModel.h"
#include "PointRuntimeTelemetry.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <array>

namespace pointdrone::audio
{
class PointVoice
{
public:
    void prepare(double newSampleRate);
    void render(juce::AudioBuffer<float>& outputBuffer, int numSamples, const domain::PointModel& point);
    PointRuntimeTelemetry getRuntimeTelemetry() const;

private:
    static float panLeftGain(float pan);
    static float panRightGain(float pan);

    double sampleRate = 44100.0;
    double phase = 0.0;
    juce::Random random;
    bool prepared = false;
    juce::String pointId;

    juce::SmoothedValue<float> frequencyHz;
    juce::SmoothedValue<float> pan;
    juce::SmoothedValue<float> gain;
    juce::SmoothedValue<float> sinePhase;
    juce::SmoothedValue<float> sawShape;
    juce::SmoothedValue<float> squarePulseWidth;
    juce::SmoothedValue<float> noiseTone;
    juce::SmoothedValue<float> sine;
    juce::SmoothedValue<float> saw;
    juce::SmoothedValue<float> square;
    juce::SmoothedValue<float> noise;
    std::array<pointdrone::core::RandomModulator, pointdrone::domain::modulationTargetCount> modulators;
    PointRuntimeTelemetry runtimeTelemetry;
    int waveformWriteIndex = 0;
    float filteredNoise = 0.0f;
};
}
