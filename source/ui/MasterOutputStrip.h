#pragma once

#include "../controller/EditorViewModels.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>
#include <functional>

namespace pointdrone::ui
{
class MasterOutputStrip : public juce::Component,
                          private juce::Timer
{
public:
    MasterOutputStrip();

    void paint(juce::Graphics& graphics) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;

    void setViewModel(pointdrone::controller::MasterOutputViewModel newViewModel);
    void setMeterSupplier(std::function<std::array<float, 2>()> newMeterSupplier);

    std::function<void(float)> onGainChanged;

private:
    void timerCallback() override;
    juce::Rectangle<float> meterBounds() const;
    void updateGainFromPosition(juce::Point<float> position);

    pointdrone::controller::MasterOutputViewModel viewModel;
    std::function<std::array<float, 2>()> meterSupplier;
    std::array<float, 2> meterLevels { 0.0f, 0.0f };
};
}
