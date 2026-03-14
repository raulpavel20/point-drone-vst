#pragma once

#include "../domain/PointModel.h"
#include "ModulatableSlider.h"

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
    void setLiveValues(const SliderValues& values);
    void clearLiveValues();
    SliderValues getValues() const;
    void setEnabledState(bool shouldBeEnabled);
    void setModulatedStates(const std::array<bool, 4>& newModulatedStates);

    std::function<void(SliderValues)> onValuesChanged;
    std::function<void(int)> onSliderDoubleClicked;

private:
    void configureSlider(ModulatableSlider& slider, int index);
    void handleSliderChange();
    void applyDisplayedValues();
    ModulatableSlider& sliderAt(int index);
    const ModulatableSlider& sliderAt(int index) const;
    void updateLabelColours();

    juce::Label sineLabel;
    juce::Label sawLabel;
    juce::Label squareLabel;
    juce::Label noiseLabel;

    ModulatableSlider sineSlider;
    ModulatableSlider sawSlider;
    ModulatableSlider squareSlider;
    ModulatableSlider noiseSlider;

    bool enabledState = true;
    std::array<bool, 4> modulatedStates { false, false, false, false };
    SliderValues baseValues { 0.0f, 0.0f, 0.0f, 0.0f };
    SliderValues liveValues { 0.0f, 0.0f, 0.0f, 0.0f };
    bool hasLiveValues = false;
    bool updatingFromState = false;
};
}
