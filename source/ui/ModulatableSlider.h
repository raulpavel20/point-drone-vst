#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace pointdrone::ui
{
class ModulatableSlider : public juce::Slider
{
public:
    std::function<void()> onModulationDoubleClick;

    void setModulationEnabled(const bool shouldBeModulated)
    {
        modulationEnabled = shouldBeModulated;
    }

    bool isModulationEnabled() const
    {
        return modulationEnabled;
    }

    void setDisplayingLiveValue(const bool shouldDisplayLiveValue)
    {
        displayingLiveValue = shouldDisplayLiveValue;
    }

    bool isDisplayingLiveValue() const
    {
        return displayingLiveValue;
    }

    void mouseDown(const juce::MouseEvent& event) override
    {
        if (! modulationEnabled)
            juce::Slider::mouseDown(event);
    }

    void mouseDrag(const juce::MouseEvent& event) override
    {
        if (! modulationEnabled)
            juce::Slider::mouseDrag(event);
    }

    void mouseDoubleClick(const juce::MouseEvent& event) override
    {
        if (onModulationDoubleClick != nullptr)
            onModulationDoubleClick();

        juce::Slider::mouseDoubleClick(event);
    }

private:
    bool modulationEnabled = false;
    bool displayingLiveValue = false;
};
}
