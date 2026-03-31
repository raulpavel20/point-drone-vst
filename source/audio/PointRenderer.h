#pragma once

#include "../domain/ProjectModel.h"
#include "GhostVoice.h"
#include "PointRuntimeTelemetry.h"
#include "PointVoice.h"
#include "ResonanceInteraction.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

#include <optional>
#include <unordered_map>
#include <vector>

namespace pointdrone::audio
{
class PointRenderer
{
public:
    void prepare(double sampleRate, int maximumExpectedSamplesPerBlock);
    void render(const domain::ProjectModel& model, juce::AudioBuffer<float>& outputBuffer);
    std::optional<PointRuntimeTelemetry> getRuntimeTelemetry(const juce::String& pointId) const;
    std::vector<ResonanceInteraction> getResonanceInteractions() const;

private:
    static constexpr int ghostVoicePoolSize = 12;

    double currentSampleRate = 44100.0;
    bool hasPrepared = false;
    juce::SmoothedValue<float> outputGain;
    std::unordered_map<std::string, PointVoice> voices;
    std::array<GhostVoice, ghostVoicePoolSize> ghostVoices;
    juce::dsp::Chorus<float> chorus;
    juce::Reverb reverb;
    mutable juce::SpinLock runtimeTelemetryLock;
    std::unordered_map<std::string, PointRuntimeTelemetry> runtimeTelemetry;
    std::vector<ResonanceInteraction> resonanceInteractions;
};
}
