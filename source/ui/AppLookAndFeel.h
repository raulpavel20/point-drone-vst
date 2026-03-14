#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace pointdrone::ui
{
class AppLookAndFeel : public juce::LookAndFeel_V4
{
public:
    AppLookAndFeel();

    juce::Typeface::Ptr getTypefaceForFont(const juce::Font& font) override;
    void drawLinearSlider(juce::Graphics& graphics,
                          int x,
                          int y,
                          int width,
                          int height,
                          float sliderPos,
                          float minSliderPos,
                          float maxSliderPos,
                          const juce::Slider::SliderStyle style,
                          juce::Slider& slider) override;

private:
    juce::Typeface::Ptr customTypeface;
};
}
