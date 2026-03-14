#pragma once

#include "../audio/PointRenderer.h"
#include "../state/ProjectState.h"

#include <juce_audio_processors/juce_audio_processors.h>

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

private:
    pointdrone::state::ProjectState projectState;
    pointdrone::audio::PointRenderer renderer;
};
}
