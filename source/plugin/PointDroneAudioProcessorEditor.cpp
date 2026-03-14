#include "PointDroneAudioProcessorEditor.h"

#include "../core/Theme.h"
#include "PointDroneAudioProcessor.h"

#include <utility>

namespace pointdrone::plugin
{
PointDroneAudioProcessorEditor::PointDroneAudioProcessorEditor(PointDroneAudioProcessor& audioProcessor)
    : AudioProcessorEditor(&audioProcessor),
      controller(audioProcessor.getProjectState())
{
    setLookAndFeel(&lookAndFeel);

    chartComponent.onBackgroundClicked = [this](const float normalizedX, const float normalizedY)
    {
        controller.handleChartBackgroundClicked(normalizedX, normalizedY);
        refreshViews();
    };

    chartComponent.onPointClicked = [this](const juce::String& pointId)
    {
        controller.handlePointClicked(pointId);
        refreshViews();
    };

    chartComponent.onPointDragged = [this](const juce::String& pointId, const float normalizedX, const float normalizedY)
    {
        controller.handlePointDragged(pointId, normalizedX, normalizedY);
        refreshViews();
    };

    chartComponent.onPointDoubleClicked = [this](const juce::String& pointId)
    {
        controller.handlePointDoubleClicked(pointId);
        refreshViews();
    };

    inspectorPanel.onWaveMixChanged = [this](const pointdrone::domain::WaveMix& waveMix)
    {
        controller.handleWaveMixChanged(waveMix);
        refreshViews();
    };

    inspectorPanel.onGainChanged = [this](const float gain)
    {
        controller.handleGainChanged(gain);
        refreshViews();
    };

    addAndMakeVisible(chartComponent);
    addAndMakeVisible(pointWavePreview);
    addAndMakeVisible(inspectorPanel);

    setSize(1080, 620);
    refreshViews();
}

PointDroneAudioProcessorEditor::~PointDroneAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void PointDroneAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    graphics.fillAll(pointdrone::core::Theme::background());

    graphics.setColour(pointdrone::core::Theme::text());
    graphics.setFont(18.0f);
    graphics.drawFittedText("[POINT DRONE]", 16, 10, getWidth() - 32, 24, juce::Justification::centredLeft, 1);

    graphics.setColour(pointdrone::core::Theme::muted());
    graphics.setFont(13.0f);
    graphics.drawFittedText("[CLICK TO ADD / CLICK TO SELECT]", 16, 34, getWidth() - 32, 18, juce::Justification::centredLeft, 1);
}

void PointDroneAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(16);
    bounds.removeFromTop(56);

    auto inspectorBounds = bounds.removeFromRight(300);
    auto previewBounds = bounds.removeFromRight(110);
    inspectorPanel.setBounds(inspectorBounds);
    pointWavePreview.setBounds(previewBounds);
    chartComponent.setBounds(bounds);
}

void PointDroneAudioProcessorEditor::refreshViews()
{
    auto viewState = controller.getViewState();
    chartComponent.setViewModel(std::move(viewState.chart));
    pointWavePreview.setViewModel(std::move(viewState.wavePreview));
    inspectorPanel.setViewModel(std::move(viewState.inspector));
}
}
