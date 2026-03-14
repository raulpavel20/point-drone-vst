#pragma once

#include "../controller/EditorViewModels.h"
#include "WaveMixSliders.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace pointdrone::ui
{
class InspectorPanel : public juce::Component
{
public:
    InspectorPanel();

    void paint(juce::Graphics& graphics) override;
    void resized() override;

    void setViewModel(pointdrone::controller::InspectorViewModel newViewModel);

    std::function<void(pointdrone::domain::WaveTimbre)> onWaveTimbreChanged;
    std::function<void(pointdrone::domain::WaveMix)> onWaveMixChanged;
    std::function<void(float)> onGainChanged;

private:
    pointdrone::controller::InspectorViewModel viewModel;
    WaveMixSliders waveTimbreSliders;
    WaveMixSliders waveMixSliders;
    juce::Label gainLabel;
    juce::Slider gainSlider;
    bool updatingFromState = false;
};
}
