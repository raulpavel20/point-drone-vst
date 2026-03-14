#include "InspectorPanel.h"

#include "../core/Theme.h"

#include <utility>

namespace pointdrone::ui
{
InspectorPanel::InspectorPanel()
{
    waveMixSliders.onWaveMixChanged = [this](const pointdrone::domain::WaveMix& waveMix)
    {
        if (onWaveMixChanged != nullptr)
            onWaveMixChanged(waveMix);
    };

    addAndMakeVisible(waveMixSliders);
    waveMixSliders.setEnabledState(false);
}

void InspectorPanel::paint(juce::Graphics& graphics)
{
    graphics.setColour(pointdrone::core::Theme::outline());
    graphics.drawRect(getLocalBounds(), 1);

    graphics.setColour(pointdrone::core::Theme::text());
    graphics.setFont(16.0f);
    graphics.drawFittedText("[POINT]", getLocalBounds().removeFromTop(28).reduced(12, 0), juce::Justification::centredLeft, 1);

    auto contentBounds = getLocalBounds().reduced(12);
    contentBounds.removeFromTop(36);

    if (! viewModel.hasSelection)
    {
        graphics.setColour(pointdrone::core::Theme::muted());
        graphics.setFont(14.0f);
        graphics.drawFittedText("[SELECT A POINT]", contentBounds.removeFromTop(22), juce::Justification::centredLeft, 1);
        graphics.drawFittedText("[CLICK THE CHART TO ADD ONE]", contentBounds.removeFromTop(22), juce::Justification::centredLeft, 1);
        return;
    }

    graphics.setColour(pointdrone::core::Theme::text());
    graphics.setFont(14.0f);
    graphics.drawFittedText(viewModel.frequencyText, contentBounds.removeFromTop(22), juce::Justification::centredLeft, 1);
    graphics.drawFittedText(viewModel.panText, contentBounds.removeFromTop(22), juce::Justification::centredLeft, 1);
    contentBounds.removeFromTop(12);
    graphics.drawFittedText("[WAVE MIX]", contentBounds.removeFromTop(20), juce::Justification::centredLeft, 1);
}

void InspectorPanel::resized()
{
    auto bounds = getLocalBounds().reduced(12);
    bounds.removeFromTop(110);
    waveMixSliders.setBounds(bounds);
}

void InspectorPanel::setViewModel(pointdrone::controller::InspectorViewModel newViewModel)
{
    viewModel = std::move(newViewModel);
    waveMixSliders.setEnabledState(viewModel.hasSelection);
    waveMixSliders.setWaveMix(viewModel.waveMix);
    repaint();
}
}
