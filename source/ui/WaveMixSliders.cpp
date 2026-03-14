#include "WaveMixSliders.h"

#include "../core/Theme.h"

namespace pointdrone::ui
{
WaveMixSliders::WaveMixSliders(std::array<juce::String, 4> labels)
{
    sineLabel.setText(labels[0], juce::dontSendNotification);
    sawLabel.setText(labels[1], juce::dontSendNotification);
    squareLabel.setText(labels[2], juce::dontSendNotification);
    noiseLabel.setText(labels[3], juce::dontSendNotification);

    for (auto* label : { &sineLabel, &sawLabel, &squareLabel, &noiseLabel })
    {
        label->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(*label);
    }

    configureSlider(sineSlider, 0);
    configureSlider(sawSlider, 1);
    configureSlider(squareSlider, 2);
    configureSlider(noiseSlider, 3);
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

void WaveMixSliders::setValues(const SliderValues& values)
{
    baseValues = values;
    applyDisplayedValues();
}

void WaveMixSliders::setLiveValues(const SliderValues& values)
{
    liveValues = values;
    hasLiveValues = true;
    applyDisplayedValues();
}

void WaveMixSliders::clearLiveValues()
{
    hasLiveValues = false;
    applyDisplayedValues();
}

WaveMixSliders::SliderValues WaveMixSliders::getValues() const
{
    return baseValues;
}

void WaveMixSliders::setEnabledState(const bool shouldBeEnabled)
{
    enabledState = shouldBeEnabled;
    sineSlider.setEnabled(shouldBeEnabled);
    sawSlider.setEnabled(shouldBeEnabled);
    squareSlider.setEnabled(shouldBeEnabled);
    noiseSlider.setEnabled(shouldBeEnabled);
    updateLabelColours();
}

void WaveMixSliders::setModulatedStates(const std::array<bool, 4>& newModulatedStates)
{
    modulatedStates = newModulatedStates;
    sineSlider.setModulationEnabled(modulatedStates[0]);
    sawSlider.setModulationEnabled(modulatedStates[1]);
    squareSlider.setModulationEnabled(modulatedStates[2]);
    noiseSlider.setModulationEnabled(modulatedStates[3]);
    sineSlider.setDisplayingLiveValue(modulatedStates[0]);
    sawSlider.setDisplayingLiveValue(modulatedStates[1]);
    squareSlider.setDisplayingLiveValue(modulatedStates[2]);
    noiseSlider.setDisplayingLiveValue(modulatedStates[3]);
    applyDisplayedValues();
    updateLabelColours();
}

void WaveMixSliders::configureSlider(ModulatableSlider& slider, const int index)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setRange(0.0, 1.0, 0.001);
    slider.onValueChange = [this] { handleSliderChange(); };
    slider.onModulationDoubleClick = [this, index]
    {
        if (onSliderDoubleClicked != nullptr)
            onSliderDoubleClicked(index);
    };
    addAndMakeVisible(slider);
}

void WaveMixSliders::handleSliderChange()
{
    if (updatingFromState)
        return;

    for (int index = 0; index < 4; ++index)
    {
        if (! modulatedStates[static_cast<std::size_t>(index)])
            baseValues[static_cast<std::size_t>(index)] = static_cast<float>(sliderAt(index).getValue());
    }

    if (onValuesChanged != nullptr)
        onValuesChanged(getValues());
}

void WaveMixSliders::applyDisplayedValues()
{
    const juce::ScopedValueSetter<bool> setter(updatingFromState, true);
    sineSlider.setValue(modulatedStates[0] && hasLiveValues ? liveValues[0] : baseValues[0], juce::dontSendNotification);
    sawSlider.setValue(modulatedStates[1] && hasLiveValues ? liveValues[1] : baseValues[1], juce::dontSendNotification);
    squareSlider.setValue(modulatedStates[2] && hasLiveValues ? liveValues[2] : baseValues[2], juce::dontSendNotification);
    noiseSlider.setValue(modulatedStates[3] && hasLiveValues ? liveValues[3] : baseValues[3], juce::dontSendNotification);
}

ModulatableSlider& WaveMixSliders::sliderAt(const int index)
{
    switch (index)
    {
        case 0: return sineSlider;
        case 1: return sawSlider;
        case 2: return squareSlider;
        default: return noiseSlider;
    }
}

const ModulatableSlider& WaveMixSliders::sliderAt(const int index) const
{
    switch (index)
    {
        case 0: return sineSlider;
        case 1: return sawSlider;
        case 2: return squareSlider;
        default: return noiseSlider;
    }
}

void WaveMixSliders::updateLabelColours()
{
    auto colourForIndex = [this](const int index)
    {
        if (! enabledState)
            return pointdrone::core::Theme::muted();

        return modulatedStates[static_cast<std::size_t>(index)]
            ? pointdrone::core::Theme::accent()
            : pointdrone::core::Theme::text();
    };

    sineLabel.setColour(juce::Label::textColourId, colourForIndex(0));
    sawLabel.setColour(juce::Label::textColourId, colourForIndex(1));
    squareLabel.setColour(juce::Label::textColourId, colourForIndex(2));
    noiseLabel.setColour(juce::Label::textColourId, colourForIndex(3));
}
}
