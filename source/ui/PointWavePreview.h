#pragma once

#include "../controller/EditorViewModels.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace pointdrone::ui
{
class PointWavePreview : public juce::Component
{
public:
    void paint(juce::Graphics& graphics) override;

    void setViewModel(pointdrone::controller::PointWavePreviewViewModel newViewModel);

private:
    pointdrone::controller::PointWavePreviewViewModel viewModel;
};
}
