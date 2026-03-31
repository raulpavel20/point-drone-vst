#pragma once

#include "../audio/PointRenderer.h"
#include "../audio/PointRuntimeTelemetry.h"
#include "../audio/ResonanceInteraction.h"
#include "../state/ProjectState.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <atomic>
#include <optional>
#include <vector>

namespace pointdrone::plugin
{
class PointDroneAudioProcessor : public juce::AudioProcessor
{
public:
    PointDroneAudioProcessor();

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    pointdrone::state::ProjectState& getProjectState();
    std::array<float, 2> getOutputMeterLevels() const;
    std::optional<pointdrone::audio::PointRuntimeTelemetry> getPointRuntimeTelemetry(const juce::String& pointId) const;
    std::vector<pointdrone::audio::ResonanceInteraction> getResonanceInteractions() const;

private:
    std::atomic<float> leftMeterLevel = 0.0f;
    std::atomic<float> rightMeterLevel = 0.0f;
    float smoothedLeftMeterLevel = 0.0f;
    float smoothedRightMeterLevel = 0.0f;
    pointdrone::state::ProjectState projectState;
    pointdrone::audio::PointRenderer renderer;
};
}
