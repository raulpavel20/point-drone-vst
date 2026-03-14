#pragma once

#include "../controller/EditorViewModels.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <optional>

namespace pointdrone::ui
{
class ChartComponent : public juce::Component
{
public:
    void paint(juce::Graphics& graphics) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;

    void setViewModel(pointdrone::controller::ChartViewModel newViewModel);

    std::function<void(float, float)> onBackgroundClicked;
    std::function<void(juce::String)> onPointClicked;
    std::function<void(juce::String)> onPointDoubleClicked;
    std::function<void(juce::String, float, float)> onPointDragged;

private:
    enum class DragAxisLock
    {
        none,
        x,
        y
    };

    juce::Rectangle<int> chartBounds() const;
    juce::Rectangle<float> plotBounds() const;
    juce::Point<float> pointToPosition(const pointdrone::controller::ChartPointViewModel& point) const;
    std::optional<pointdrone::controller::ChartPointViewModel> hitTestPoint(juce::Point<float> position) const;
    juce::Point<float> normalizedPosition(juce::Point<float> position) const;

    pointdrone::controller::ChartViewModel viewModel;
    std::optional<juce::String> draggedPointId;
    std::optional<juce::Point<float>> dragStartPosition;
    DragAxisLock dragAxisLock = DragAxisLock::none;
};
}
