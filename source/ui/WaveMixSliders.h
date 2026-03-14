#pragma once

#include "../domain/PointModel.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace pointdrone::ui
{
class WaveMixSliders : public juce::Component
{
public:
    WaveMixSliders();

    void resized() override;

    void setWaveMix(const pointdrone::domain::WaveMix& waveMix);
    pointdrone::domain::WaveMix getWaveMix() const;
    void setEnabledState(bool shouldBeEnabled);

    std::function<void(pointdrone::domain::WaveMix)> onWaveMixChanged;

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
