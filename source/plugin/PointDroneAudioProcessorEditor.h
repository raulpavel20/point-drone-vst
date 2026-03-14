#pragma once

#include "../controller/EditorController.h"
#include "../ui/AppLookAndFeel.h"
#include "../ui/ChartComponent.h"
#include "../ui/InspectorPanel.h"
#include "../ui/PointWavePreview.h"

#include <juce_audio_processors/juce_audio_processors.h>

namespace pointdrone::plugin
{
class PointDroneAudioProcessor;

class PointDroneAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PointDroneAudioProcessorEditor(PointDroneAudioProcessor&);
    ~PointDroneAudioProcessorEditor() override;

    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    void refreshViews();

    pointdrone::ui::AppLookAndFeel lookAndFeel;
    pointdrone::controller::EditorController controller;
    pointdrone::ui::ChartComponent chartComponent;
    pointdrone::ui::PointWavePreview pointWavePreview;
    pointdrone::ui::InspectorPanel inspectorPanel;
};
}
