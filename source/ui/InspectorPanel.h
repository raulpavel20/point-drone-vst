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

    std::function<void(pointdrone::domain::WaveMix)> onWaveMixChanged;

private:
    pointdrone::controller::InspectorViewModel viewModel;
    WaveMixSliders waveMixSliders;
};
}
