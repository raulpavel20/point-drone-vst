#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace pointdrone::ui
{
class SnapshotTransitionStrip : public juce::Component
{
public:
    void paint(juce::Graphics& graphics) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;

    void setTransitionSeconds(float newTransitionSeconds);

    std::function<void(float)> onTransitionSecondsChanged;

private:
    juce::Rectangle<float> sliderBounds() const;
    void updateFromPosition(juce::Point<float> position);

    float transitionSeconds = 0.0f;
};
}
