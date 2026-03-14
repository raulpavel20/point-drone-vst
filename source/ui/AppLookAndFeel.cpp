#include "AppLookAndFeel.h"

#include "../core/Theme.h"

#include <BinaryData.h>

namespace pointdrone::ui
{
AppLookAndFeel::AppLookAndFeel()
{
    customTypeface = juce::Typeface::createSystemTypefaceFor(BinaryData::ManropeVariableFont_wght_ttf,
                                                             BinaryData::ManropeVariableFont_wght_ttfSize);

    setColour(juce::Slider::backgroundColourId, pointdrone::core::Theme::muted());
    setColour(juce::Slider::trackColourId, pointdrone::core::Theme::accent());
    setColour(juce::Slider::thumbColourId, pointdrone::core::Theme::accent());
    setColour(juce::Slider::textBoxTextColourId, pointdrone::core::Theme::text());
    setColour(juce::Slider::textBoxOutlineColourId, pointdrone::core::Theme::outline());
    setColour(juce::Label::textColourId, pointdrone::core::Theme::text());
    setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
}

juce::Typeface::Ptr AppLookAndFeel::getTypefaceForFont(const juce::Font& font)
{
    return customTypeface != nullptr ? customTypeface : juce::LookAndFeel_V4::getTypefaceForFont(font);
}

void AppLookAndFeel::drawLinearSlider(juce::Graphics& graphics,
                                      const int x,
                                      const int y,
                                      const int width,
                                      const int height,
                                      const float sliderPos,
                                      float,
                                      float,
                                      const juce::Slider::SliderStyle style,
                                      juce::Slider& slider)
{
    if (style != juce::Slider::LinearVertical)
    {
        juce::LookAndFeel_V4::drawLinearSlider(graphics,
                                               x,
                                               y,
                                               width,
                                               height,
                                               sliderPos,
                                               0.0f,
                                               0.0f,
                                               style,
                                               slider);
        return;
    }

    const auto bounds = juce::Rectangle<float>(static_cast<float>(x),
                                               static_cast<float>(y),
                                               static_cast<float>(width),
                                               static_cast<float>(height));
    const auto centerX = bounds.getCentreX();

    graphics.setColour(pointdrone::core::Theme::outline());
    graphics.drawRect(bounds, 1.0f);
    graphics.drawLine(centerX, bounds.getY() + 6.0f, centerX, bounds.getBottom() - 6.0f, 1.0f);

    const auto thumbWidth = juce::jmin(22.0f, bounds.getWidth() - 6.0f);
    const auto thumbHeight = 8.0f;
    const auto thumbBounds = juce::Rectangle<float>(thumbWidth, thumbHeight)
                                 .withCentre({ centerX, sliderPos });

    graphics.setColour(pointdrone::core::Theme::accent());
    graphics.fillRect(thumbBounds);
}
}
