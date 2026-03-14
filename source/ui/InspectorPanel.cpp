#include "InspectorPanel.h"

#include "../core/Theme.h"

#include <utility>

namespace pointdrone::ui
{
InspectorPanel::InspectorPanel()
    : waveTimbreSliders({ "[PHASE]", "[SHAPE]", "[WIDTH]", "[TONE]" }),
      waveMixSliders({ "[SINE]", "[SAW]", "[SQUARE]", "[NOISE]" })
{
    waveTimbreSliders.onValuesChanged = [this](const WaveMixSliders::SliderValues& values)
    {
        if (onWaveTimbreChanged != nullptr)
        {
            onWaveTimbreChanged({
                values[0],
                values[1],
                values[2],
                values[3]
            });
        }
    };

    waveMixSliders.onValuesChanged = [this](const WaveMixSliders::SliderValues& values)
    {
        if (onWaveMixChanged != nullptr)
        {
            onWaveMixChanged({
                values[0],
                values[1],
                values[2],
                values[3]
            });
        }
    };

    gainLabel.setText("[GAIN]", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);

    gainSlider.setSliderStyle(juce::Slider::LinearVertical);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider.setRange(0.0, 1.0, 0.001);
    gainSlider.onValueChange = [this]
    {
        if (updatingFromState)
            return;

        if (onGainChanged != nullptr)
            onGainChanged(static_cast<float>(gainSlider.getValue()));
    };

    addAndMakeVisible(waveTimbreSliders);
    addAndMakeVisible(waveMixSliders);
    addAndMakeVisible(gainLabel);
    addAndMakeVisible(gainSlider);
    waveTimbreSliders.setEnabledState(false);
    waveMixSliders.setEnabledState(false);
    gainSlider.setEnabled(false);
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
}

void InspectorPanel::resized()
{
    auto bounds = getLocalBounds().reduced(12);
    bounds.removeFromTop(110);

    auto gainBounds = bounds.removeFromRight(48);
    auto rowGap = 16;
    auto rowHeight = (bounds.getHeight() - rowGap) / 2;
    waveTimbreSliders.setBounds(bounds.removeFromTop(rowHeight));
    bounds.removeFromTop(rowGap);
    waveMixSliders.setBounds(bounds);
    gainLabel.setBounds(gainBounds.removeFromBottom(24));
    gainSlider.setBounds(gainBounds.reduced(6, 0));
}

void InspectorPanel::setViewModel(pointdrone::controller::InspectorViewModel newViewModel)
{
    viewModel = std::move(newViewModel);
    const juce::ScopedValueSetter<bool> setter(updatingFromState, true);
    waveTimbreSliders.setEnabledState(viewModel.hasSelection);
    waveTimbreSliders.setValues({
        viewModel.waveTimbre.sinePhase,
        viewModel.waveTimbre.sawShape,
        viewModel.waveTimbre.squarePulseWidth,
        viewModel.waveTimbre.noiseTone
    });
    waveMixSliders.setEnabledState(viewModel.hasSelection);
    waveMixSliders.setValues({
        viewModel.waveMix.sine,
        viewModel.waveMix.saw,
        viewModel.waveMix.square,
        viewModel.waveMix.noise
    });
    gainSlider.setEnabled(viewModel.hasSelection);
    gainSlider.setValue(viewModel.gain, juce::dontSendNotification);
    gainLabel.setColour(juce::Label::textColourId, viewModel.hasSelection ? pointdrone::core::Theme::text() : pointdrone::core::Theme::muted());
    repaint();
}
}
