#pragma once

#include "../domain/PointModel.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>

namespace pointdrone::ui
{
class WaveMixSliders : public juce::Component
{
public:
    using SliderValues = std::array<float, 4>;

    explicit WaveMixSliders(std::array<juce::String, 4> labels);

    void resized() override;

    void setValues(const SliderValues& values);
    SliderValues getValues() const;
    void setEnabledState(bool shouldBeEnabled);

    std::function<void(SliderValues)> onValuesChanged;

private:
    void configureSlider(juce::Slider& slider);
    void handleSliderChange();

    juce::Label sineLabel;
    juce::Label sawLabel;
    juce::Label squareLabel;
    juce::Label noiseLabel;

    juce::Slider sineSlider;
    juce::Slider sawSlider;
    juce::Slider squareSlider;
    juce::Slider noiseSlider;

    bool updatingFromState = false;
};
}
