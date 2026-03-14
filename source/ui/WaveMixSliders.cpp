#include "WaveMixSliders.h"

#include "../core/Theme.h"

namespace pointdrone::ui
{
WaveMixSliders::WaveMixSliders()
{
    sineLabel.setText("[SINE]", juce::dontSendNotification);
    sawLabel.setText("[SAW]", juce::dontSendNotification);
    squareLabel.setText("[SQUARE]", juce::dontSendNotification);
    noiseLabel.setText("[NOISE]", juce::dontSendNotification);

    for (auto* label : { &sineLabel, &sawLabel, &squareLabel, &noiseLabel })
    {
        label->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*label);
    }

    configureSlider(sineSlider);
    configureSlider(sawSlider);
    configureSlider(squareSlider);
    configureSlider(noiseSlider);
}

void WaveMixSliders::resized()
{
    const auto bounds = getLocalBounds();
    const auto columnWidth = bounds.getWidth() / 4;

    auto layoutColumn = [](juce::Rectangle<int> columnBounds, juce::Slider& slider, juce::Label& label)
    {
        auto labelBounds = columnBounds.removeFromBottom(24);
        label.setBounds(labelBounds);
        slider.setBounds(columnBounds.reduced(6, 0));
    };

    auto remaining = bounds;
    layoutColumn(remaining.removeFromLeft(columnWidth), sineSlider, sineLabel);
    layoutColumn(remaining.removeFromLeft(columnWidth), sawSlider, sawLabel);
    layoutColumn(remaining.removeFromLeft(columnWidth), squareSlider, squareLabel);
    layoutColumn(remaining, noiseSlider, noiseLabel);
}

void WaveMixSliders::setWaveMix(const pointdrone::domain::WaveMix& waveMix)
{
    const juce::ScopedValueSetter<bool> setter(updatingFromState, true);
    sineSlider.setValue(waveMix.sine, juce::dontSendNotification);
    sawSlider.setValue(waveMix.saw, juce::dontSendNotification);
    squareSlider.setValue(waveMix.square, juce::dontSendNotification);
    noiseSlider.setValue(waveMix.noise, juce::dontSendNotification);
}

pointdrone::domain::WaveMix WaveMixSliders::getWaveMix() const
{
    return {
        static_cast<float>(sineSlider.getValue()),
        static_cast<float>(sawSlider.getValue()),
        static_cast<float>(squareSlider.getValue()),
        static_cast<float>(noiseSlider.getValue())
    };
}

void WaveMixSliders::setEnabledState(const bool shouldBeEnabled)
{
    sineSlider.setEnabled(shouldBeEnabled);
    sawSlider.setEnabled(shouldBeEnabled);
    squareSlider.setEnabled(shouldBeEnabled);
    noiseSlider.setEnabled(shouldBeEnabled);

    sineLabel.setColour(juce::Label::textColourId, shouldBeEnabled ? pointdrone::core::Theme::text() : pointdrone::core::Theme::muted());
    sawLabel.setColour(juce::Label::textColourId, shouldBeEnabled ? pointdrone::core::Theme::text() : pointdrone::core::Theme::muted());
    squareLabel.setColour(juce::Label::textColourId, shouldBeEnabled ? pointdrone::core::Theme::text() : pointdrone::core::Theme::muted());
    noiseLabel.setColour(juce::Label::textColourId, shouldBeEnabled ? pointdrone::core::Theme::text() : pointdrone::core::Theme::muted());
}

void WaveMixSliders::configureSlider(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setRange(0.0, 1.0, 0.001);
    slider.onValueChange = [this] { handleSliderChange(); };
    addAndMakeVisible(slider);
}

void WaveMixSliders::handleSliderChange()
{
    if (updatingFromState)
        return;

    if (onWaveMixChanged != nullptr)
        onWaveMixChanged(getWaveMix());
}
}
