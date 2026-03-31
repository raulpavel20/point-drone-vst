#include "PointDroneAudioProcessor.h"

#include "PointDroneAudioProcessorEditor.h"

namespace pointdrone::plugin
{
PointDroneAudioProcessor::PointDroneAudioProcessor()
    : juce::AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void PointDroneAudioProcessor::prepareToPlay(const double sampleRate, const int samplesPerBlock)
{
    renderer.prepare(sampleRate, samplesPerBlock);
    leftMeterLevel.store(0.0f);
    rightMeterLevel.store(0.0f);
    smoothedLeftMeterLevel = 0.0f;
    smoothedRightMeterLevel = 0.0f;
}

void PointDroneAudioProcessor::releaseResources()
{
}

bool PointDroneAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void PointDroneAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    renderer.render(projectState.getModel(), buffer);

    const auto leftPeak = buffer.getNumChannels() > 0 ? buffer.getMagnitude(0, 0, buffer.getNumSamples()) : 0.0f;
    const auto rightPeak = buffer.getNumChannels() > 1 ? buffer.getMagnitude(1, 0, buffer.getNumSamples()) : leftPeak;

    smoothedLeftMeterLevel = leftPeak > smoothedLeftMeterLevel
        ? leftPeak
        : juce::jmax(leftPeak, smoothedLeftMeterLevel * 0.92f);
    smoothedRightMeterLevel = rightPeak > smoothedRightMeterLevel
        ? rightPeak
        : juce::jmax(rightPeak, smoothedRightMeterLevel * 0.92f);

    leftMeterLevel.store(smoothedLeftMeterLevel);
    rightMeterLevel.store(smoothedRightMeterLevel);
}

juce::AudioProcessorEditor* PointDroneAudioProcessor::createEditor()
{
    return new PointDroneAudioProcessorEditor(*this);
}

bool PointDroneAudioProcessor::hasEditor() const
{
    return true;
}

const juce::String PointDroneAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PointDroneAudioProcessor::acceptsMidi() const
{
    return true;
}

bool PointDroneAudioProcessor::producesMidi() const
{
    return false;
}

bool PointDroneAudioProcessor::isMidiEffect() const
{
    return false;
}

double PointDroneAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PointDroneAudioProcessor::getNumPrograms()
{
    return 1;
}

int PointDroneAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PointDroneAudioProcessor::setCurrentProgram(int)
{
}

const juce::String PointDroneAudioProcessor::getProgramName(int)
{
    return {};
}

void PointDroneAudioProcessor::changeProgramName(int, const juce::String&)
{
}

void PointDroneAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (const auto xml = projectState.copyState().createXml())
        copyXmlToBinary(*xml, destData);
}

void PointDroneAudioProcessor::setStateInformation(const void* data, const int sizeInBytes)
{
    const auto xml = getXmlFromBinary(data, sizeInBytes);

    if (xml == nullptr)
        return;

    projectState.replaceState(juce::ValueTree::fromXml(*xml));
}

pointdrone::state::ProjectState& PointDroneAudioProcessor::getProjectState()
{
    return projectState;
}

std::array<float, 2> PointDroneAudioProcessor::getOutputMeterLevels() const
{
    return { leftMeterLevel.load(), rightMeterLevel.load() };
}

std::optional<pointdrone::audio::PointRuntimeTelemetry> PointDroneAudioProcessor::getPointRuntimeTelemetry(const juce::String& pointId) const
{
    if (pointId.isEmpty())
        return std::nullopt;

    return renderer.getRuntimeTelemetry(pointId);
}

std::vector<pointdrone::audio::ResonanceInteraction> PointDroneAudioProcessor::getResonanceInteractions() const
{
    return renderer.getResonanceInteractions();
}
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new pointdrone::plugin::PointDroneAudioProcessor();
}
