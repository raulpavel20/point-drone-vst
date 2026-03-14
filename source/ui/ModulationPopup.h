#pragma once

#include "../controller/EditorViewModels.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>

namespace pointdrone::ui
{
class ModulationPopup : public juce::Component,
                        private juce::Timer
{
public:
    ModulationPopup();

    void paint(juce::Graphics& graphics) override;
    void resized() override;

    void setViewModel(pointdrone::controller::ModulationPopupViewModel newViewModel);

    std::function<void(pointdrone::domain::ModulationSettings)> onSettingsChanged;
    std::function<void()> onDisableRequested;
    std::function<void()> onCloseRequested;

private:
    void timerCallback() override;
    pointdrone::domain::ModulationSettings currentSettings() const;
    juce::Rectangle<float> previewBounds() const;
    juce::String amplitudeInfoText() const;
    juce::String frequencyInfoText() const;
    void emitSettingsChanged();

    pointdrone::controller::ModulationPopupViewModel viewModel;
    std::vector<float> displayedSamples;
    std::vector<float> targetSamples;
    std::array<juce::Slider, 6> knobs;
    std::array<juce::Label, 6> knobLabels;
    juce::TextButton manualButton;
    juce::TextButton closeButton;
    bool updatingFromState = false;
};
}
