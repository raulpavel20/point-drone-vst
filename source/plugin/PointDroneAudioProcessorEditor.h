#pragma once

#include "../controller/EditorController.h"
#include "../ui/AppLookAndFeel.h"
#include "../ui/ChartComponent.h"
#include "../ui/InspectorPanel.h"
#include "../ui/MasterOutputStrip.h"
#include "../ui/ModulationPopup.h"
#include "../ui/PointWavePreview.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <optional>

namespace pointdrone::plugin
{
class PointDroneAudioProcessor;

class PointDroneAudioProcessorEditor : public juce::AudioProcessorEditor,
                                       private juce::Timer
{
public:
    explicit PointDroneAudioProcessorEditor(PointDroneAudioProcessor&);
    ~PointDroneAudioProcessorEditor() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void applyRuntimeTelemetry(const pointdrone::audio::PointRuntimeTelemetry& telemetry);
    void refreshViews();
    void timerCallback() override;

    PointDroneAudioProcessor& audioProcessor;
    pointdrone::ui::AppLookAndFeel lookAndFeel;
    pointdrone::controller::EditorController controller;
    juce::TextButton snapToSemitoneButton;
    pointdrone::ui::ChartComponent chartComponent;
    pointdrone::ui::PointWavePreview pointWavePreview;
    pointdrone::ui::InspectorPanel inspectorPanel;
    pointdrone::ui::MasterOutputStrip masterOutputStrip;
    pointdrone::ui::ModulationPopup modulationPopup;
    juce::String cachedTelemetryPointId;
    std::optional<pointdrone::audio::PointRuntimeTelemetry> cachedTelemetry;
};
}
