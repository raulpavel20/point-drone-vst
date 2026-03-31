#include "PointDroneAudioProcessorEditor.h"

#include "../core/Theme.h"
#include "PointDroneAudioProcessor.h"

#include <utility>

namespace pointdrone::plugin
{
PointDroneAudioProcessorEditor::PointDroneAudioProcessorEditor(PointDroneAudioProcessor& pluginProcessor)
    : AudioProcessorEditor(&pluginProcessor),
      audioProcessor(pluginProcessor),
      controller(pluginProcessor.getProjectState()),
      snapToSemitoneButton("|||")
{
    setLookAndFeel(&lookAndFeel);

    snapToSemitoneButton.setTooltip("Snap all points to nearest semitone");
    snapToSemitoneButton.setClickingTogglesState(false);
    snapToSemitoneButton.setConnectedEdges(juce::Button::ConnectedOnLeft
                                           | juce::Button::ConnectedOnRight
                                           | juce::Button::ConnectedOnTop
                                           | juce::Button::ConnectedOnBottom);
    snapToSemitoneButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    snapToSemitoneButton.setColour(juce::TextButton::buttonOnColourId, pointdrone::core::Theme::accent().withAlpha(0.18f));
    snapToSemitoneButton.setColour(juce::TextButton::textColourOffId, pointdrone::core::Theme::text());
    snapToSemitoneButton.setColour(juce::TextButton::textColourOnId, pointdrone::core::Theme::accent());
    snapToSemitoneButton.onClick = [this]
    {
        controller.handleSnapAllPointsToSemitone();
        refreshViews();
    };

    snapshotSlots.onSlotPressed = [this](const int slotIndex, const bool saveRequested)
    {
        controller.handleSnapshotSlotPressed(slotIndex, saveRequested);
        refreshViews();
    };

    snapshotTransitionStrip.onTransitionSecondsChanged = [this](const float seconds)
    {
        controller.handleSnapshotTransitionSecondsChanged(seconds);
        refreshViews();
    };

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

    inspectorPanel.onWaveTimbreChanged = [this](const pointdrone::domain::WaveTimbre& waveTimbre)
    {
        controller.handleWaveTimbreChanged(waveTimbre);
        refreshViews();
    };

    inspectorPanel.onGainChanged = [this](const float gain)
    {
        controller.handleGainChanged(gain);
        refreshViews();
    };
    inspectorPanel.onModulationRequested = [this](const pointdrone::domain::ModulationTarget target)
    {
        controller.handleModulationRequested(target);
        refreshViews();
    };

    fieldStrip.onChorusRateChanged = [this](const float rate)
    {
        controller.handleChorusRateChanged(rate);
        refreshViews();
    };
    fieldStrip.onChorusDepthChanged = [this](const float depth)
    {
        controller.handleChorusDepthChanged(depth);
        refreshViews();
    };
    fieldStrip.onChorusMixChanged = [this](const float mix)
    {
        controller.handleChorusMixChanged(mix);
        refreshViews();
    };
    fieldStrip.onReverbMixChanged = [this](const float mix)
    {
        controller.handleReverbMixChanged(mix);
        refreshViews();
    };
    fieldStrip.onReverbSizeChanged = [this](const float size)
    {
        controller.handleReverbSizeChanged(size);
        refreshViews();
    };
    fieldStrip.onReverbDampingChanged = [this](const float damping)
    {
        controller.handleReverbDampingChanged(damping);
        refreshViews();
    };

    masterOutputStrip.onGainChanged = [this](const float gain)
    {
        controller.handleOutputGainChanged(gain);
        refreshViews();
    };
    masterOutputStrip.setMeterSupplier([&pluginProcessor]
    {
        return pluginProcessor.getOutputMeterLevels();
    });

    inspectorPanel.onFrequencyInputSubmitted = [this](juce::String text)
    {
        const auto accepted = controller.handleFrequencyInputSubmitted(text);

        if (accepted)
            refreshViews();

        return accepted;
    };

    inspectorPanel.onPanInputSubmitted = [this](juce::String text)
    {
        const auto accepted = controller.handlePanInputSubmitted(text);

        if (accepted)
            refreshViews();

        return accepted;
    };

    modulationPopup.onSettingsChanged = [this](const pointdrone::domain::ModulationSettings& settings)
    {
        controller.handleModulationSettingsChanged(settings);
        refreshViews();
    };
    modulationPopup.onDisableRequested = [this]
    {
        controller.handleModulationDisabled();
        refreshViews();
    };
    modulationPopup.onCloseRequested = [this]
    {
        controller.handleModulationPopupClosed();
        refreshViews();
    };

    addAndMakeVisible(snapshotSlots);
    addAndMakeVisible(snapshotTransitionStrip);
    addAndMakeVisible(chartComponent);
    addAndMakeVisible(pointWavePreview);
    addAndMakeVisible(inspectorPanel);
    addAndMakeVisible(fieldStrip);
    addAndMakeVisible(masterOutputStrip);
    addChildComponent(modulationPopup);
    addAndMakeVisible(snapToSemitoneButton);
    snapToSemitoneButton.toFront(false);

    setSize(1280, 620);
    refreshViews();
    startTimerHz(30);
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

    auto masterOutputBounds = bounds.removeFromRight(56);
    bounds.removeFromRight(12);
    auto fieldStripBounds = bounds.removeFromRight(72);
    bounds.removeFromRight(12);
    auto inspectorBounds = bounds.removeFromRight(280);
    auto previewBounds = bounds.removeFromRight(110);
    auto chartBounds = bounds;
    auto snapshotSlotsBounds = chartBounds.removeFromLeft(54);
    chartBounds.removeFromLeft(10);
    auto snapshotTransitionBounds = chartBounds.removeFromLeft(48);
    chartBounds.removeFromLeft(12);
    snapToSemitoneButton.setBounds(chartBounds.getX() + 12, chartBounds.getY() + 10, 24, 24);
    snapshotSlots.setBounds(snapshotSlotsBounds);
    snapshotTransitionStrip.setBounds(snapshotTransitionBounds);
    masterOutputStrip.setBounds(masterOutputBounds);
    fieldStrip.setBounds(fieldStripBounds);
    inspectorPanel.setBounds(inspectorBounds);
    pointWavePreview.setBounds(previewBounds);
    chartComponent.setBounds(chartBounds);
    modulationPopup.setBounds(getLocalBounds().withSizeKeepingCentre(540, 270).translated(0, 24));
}

void PointDroneAudioProcessorEditor::refreshViews()
{
    auto viewState = controller.getViewState();
    chartComponent.setViewModel(std::move(viewState.chart));
    pointWavePreview.setViewModel(std::move(viewState.wavePreview));
    inspectorPanel.setViewModel(std::move(viewState.inspector));
    fieldStrip.setViewModel(std::move(viewState.field));
    masterOutputStrip.setViewModel(std::move(viewState.masterOutput));
    snapshotSlots.setViewModel(std::move(viewState.snapshotControls.slots));
    snapshotTransitionStrip.setTransitionSeconds(viewState.snapshotControls.transitionSeconds);
    modulationPopup.setViewModel(std::move(viewState.modulationPopup));

    if (cachedTelemetry.has_value() && cachedTelemetryPointId == controller.getSelectedPointId())
        applyRuntimeTelemetry(*cachedTelemetry);

    if (modulationPopup.isVisible())
        modulationPopup.toFront(false);
}

void PointDroneAudioProcessorEditor::timerCallback()
{
    const auto currentTimestampMs = juce::Time::getMillisecondCounterHiRes();
    const auto deltaSeconds = lastTimerTimestampMs > 0.0 ? (currentTimestampMs - lastTimerTimestampMs) * 0.001 : 0.0;
    lastTimerTimestampMs = currentTimestampMs;

    if (controller.advanceSnapshotMorph(deltaSeconds))
        refreshViews();

    const auto rawInteractions = audioProcessor.getResonanceInteractions();
    std::vector<pointdrone::controller::ChartInteractionViewModel> chartInteractions;
    chartInteractions.reserve(rawInteractions.size());

    for (const auto& ri : rawInteractions)
        chartInteractions.push_back({ ri.pointIdA, ri.pointIdB, ri.strength });

    chartComponent.setInteractions(std::move(chartInteractions));

    const auto selectedPointId = controller.getSelectedPointId();

    if (const auto telemetry = audioProcessor.getPointRuntimeTelemetry(selectedPointId); telemetry.has_value())
    {
        cachedTelemetryPointId = selectedPointId;
        cachedTelemetry = telemetry;
        applyRuntimeTelemetry(*telemetry);
        return;
    }

    cachedTelemetryPointId.clear();
    cachedTelemetry.reset();
}

void PointDroneAudioProcessorEditor::applyRuntimeTelemetry(const pointdrone::audio::PointRuntimeTelemetry& telemetry)
{
    inspectorPanel.setLiveModulationValues({
                                               telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::sinePhase)],
                                               telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::sawShape)],
                                               telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::squarePulseWidth)],
                                               telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::noiseTone)]
                                           },
                                           {
                                               telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::sine)],
                                               telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::saw)],
                                               telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::square)],
                                               telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::noise)]
                                           },
                                           telemetry.modulatedValues[pointdrone::domain::modulationIndex(pointdrone::domain::ModulationTarget::gain)]);
    pointWavePreview.setViewModel(controller.getLiveWavePreviewViewModel(telemetry));
}
}
