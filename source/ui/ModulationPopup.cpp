#include "ModulationPopup.h"

#include "../core/RandomModulator.h"
#include "../core/Theme.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <utility>

namespace pointdrone::ui
{
ModulationPopup::ModulationPopup()
{
    static constexpr std::array<const char*, 6> labels { "[AMP]", "[FREQ]", "[EASE]", "[SLANT]", "[CYCLIC]", "[JITTER]" };

    for (std::size_t index = 0; index < knobs.size(); ++index)
    {
        knobs[index].setSliderStyle(juce::Slider::RotaryVerticalDrag);
        knobs[index].setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        knobs[index].setRange(0.0, 1.0, 0.001);
        knobs[index].onValueChange = [this] { emitSettingsChanged(); };
        addAndMakeVisible(knobs[index]);

        knobLabels[index].setText(labels[index], juce::dontSendNotification);
        knobLabels[index].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(knobLabels[index]);
    }

    manualButton.setButtonText("[MANUAL]");
    manualButton.onClick = [this]
    {
        if (onDisableRequested != nullptr)
            onDisableRequested();
    };
    addAndMakeVisible(manualButton);

    closeButton.setButtonText("[CLOSE]");
    closeButton.onClick = [this]
    {
        if (onCloseRequested != nullptr)
            onCloseRequested();
    };
    addAndMakeVisible(closeButton);

    setVisible(false);
    startTimerHz(60);
}

void ModulationPopup::paint(juce::Graphics& graphics)
{
    if (! isVisible())
        return;

    graphics.setColour(pointdrone::core::Theme::background().brighter(0.06f));
    graphics.fillRect(getLocalBounds());
    graphics.setColour(pointdrone::core::Theme::outline());
    graphics.drawRect(getLocalBounds(), 1);

    graphics.setColour(pointdrone::core::Theme::text());
    graphics.setFont(14.0f);
    graphics.drawFittedText(viewModel.title, 12, 10, getWidth() - 180, 22, juce::Justification::centredLeft, 1);
    graphics.setColour(pointdrone::core::Theme::muted());
    graphics.setFont(12.0f);
    graphics.drawFittedText(amplitudeInfoText() + "   " + frequencyInfoText() + "   [WINDOW 10.0S]",
                            12,
                            30,
                            getWidth() - 24,
                            16,
                            juce::Justification::centredLeft,
                            1);

    const auto currentPreviewBounds = previewBounds();
    graphics.setColour(pointdrone::core::Theme::outline());
    graphics.drawRect(currentPreviewBounds, 1.0f);

    if (displayedSamples.empty())
        return;

    const auto leftX = currentPreviewBounds.getX() + 6.0f;
    const auto rightX = currentPreviewBounds.getRight() - 6.0f;
    const auto topGuideY = currentPreviewBounds.getY() + 6.0f;
    const auto bottomGuideY = currentPreviewBounds.getBottom() - 6.0f;
    const auto centerY = juce::jmap(0.5f, 0.0f, 1.0f, bottomGuideY, topGuideY);
    graphics.setColour(pointdrone::core::Theme::muted().withAlpha(0.55f));
    graphics.drawLine(leftX, topGuideY, rightX, topGuideY, 1.0f);
    graphics.drawLine(leftX, bottomGuideY, rightX, bottomGuideY, 1.0f);
    graphics.setColour(pointdrone::core::Theme::muted());
    graphics.drawLine(leftX, centerY, rightX, centerY, 1.2f);
    graphics.setFont(11.0f);
    graphics.drawFittedText("[MAX]",
                            juce::Rectangle<int>(static_cast<int>(currentPreviewBounds.getX()) + 4,
                                                 static_cast<int>(topGuideY) - 8,
                                                 52,
                                                 14),
                            juce::Justification::centredLeft,
                            1);
    graphics.drawFittedText("[MID]",
                            juce::Rectangle<int>(static_cast<int>(currentPreviewBounds.getX()) + 4,
                                                 static_cast<int>(centerY) - 8,
                                                 52,
                                                 14),
                            juce::Justification::centredLeft,
                            1);
    graphics.drawFittedText("[MIN]",
                            juce::Rectangle<int>(static_cast<int>(currentPreviewBounds.getX()) + 4,
                                                 static_cast<int>(bottomGuideY) - 8,
                                                 52,
                                                 14),
                            juce::Justification::centredLeft,
                            1);

    juce::Path waveform;
    juce::Path fillPath;

    for (int sampleIndex = 0; sampleIndex < static_cast<int>(displayedSamples.size()); ++sampleIndex)
    {
        const auto x = juce::jmap(static_cast<float>(sampleIndex),
                                  0.0f,
                                  static_cast<float>(displayedSamples.size() - 1),
                                  leftX,
                                  rightX);
        const auto y = juce::jmap(juce::jlimit(0.0f, 1.0f, displayedSamples[static_cast<std::size_t>(sampleIndex)]),
                                  0.0f,
                                  1.0f,
                                  bottomGuideY,
                                  topGuideY);

        if (sampleIndex == 0)
        {
            waveform.startNewSubPath(x, y);
            fillPath.startNewSubPath(x, centerY);
            fillPath.lineTo(x, y);
        }
        else
        {
            waveform.lineTo(x, y);
            fillPath.lineTo(x, y);
        }
    }

    fillPath.lineTo(rightX, centerY);
    fillPath.closeSubPath();

    graphics.setColour(pointdrone::core::Theme::accent().withAlpha(0.12f));
    graphics.fillPath(fillPath);
    graphics.setColour(pointdrone::core::Theme::accent());
    graphics.strokePath(waveform, juce::PathStrokeType(1.8f));
}

void ModulationPopup::resized()
{
    auto bounds = getLocalBounds().reduced(12);
    auto headerBounds = bounds.removeFromTop(28);
    closeButton.setBounds(headerBounds.removeFromRight(74));
    manualButton.setBounds(headerBounds.removeFromRight(88));
    bounds.removeFromTop(8);

    auto knobsBounds = bounds.removeFromTop(88);
    const auto columnWidth = knobsBounds.getWidth() / static_cast<int>(knobs.size());

    for (std::size_t index = 0; index < knobs.size(); ++index)
    {
        auto columnBounds = knobsBounds.removeFromLeft(columnWidth);
        auto labelBounds = columnBounds.removeFromBottom(22);
        knobLabels[index].setBounds(labelBounds);
        knobs[index].setBounds(columnBounds.reduced(6, 0));
    }
}

void ModulationPopup::setViewModel(pointdrone::controller::ModulationPopupViewModel newViewModel)
{
    viewModel = std::move(newViewModel);
    setVisible(viewModel.visible);

    if (! viewModel.visible)
    {
        displayedSamples.clear();
        targetSamples.clear();
        return;
    }

    const juce::ScopedValueSetter<bool> setter(updatingFromState, true);
    knobs[0].setValue(viewModel.settings.amplitude, juce::dontSendNotification);
    knobs[1].setValue(viewModel.settings.frequency, juce::dontSendNotification);
    knobs[2].setValue(viewModel.settings.ease, juce::dontSendNotification);
    knobs[3].setValue(viewModel.settings.slant, juce::dontSendNotification);
    knobs[4].setValue(viewModel.settings.cyclic, juce::dontSendNotification);
    knobs[5].setValue(viewModel.settings.jitter, juce::dontSendNotification);
    targetSamples = viewModel.samples;

    if (displayedSamples.size() != targetSamples.size())
        displayedSamples = targetSamples;

    repaint();
}

void ModulationPopup::timerCallback()
{
    if (! isVisible() || displayedSamples.empty() || displayedSamples.size() != targetSamples.size())
        return;

    auto changed = false;

    for (std::size_t index = 0; index < displayedSamples.size(); ++index)
    {
        auto& displayed = displayedSamples[index];
        const auto target = targetSamples[index];
        const auto next = displayed + ((target - displayed) * 0.22f);

        if (std::abs(target - displayed) > 0.0005f)
        {
            displayed = next;
            changed = true;
        }
        else
        {
            displayed = target;
        }
    }

    if (changed)
        repaint();
}

pointdrone::domain::ModulationSettings ModulationPopup::currentSettings() const
{
    pointdrone::domain::ModulationSettings settings;
    settings.amplitude = static_cast<float>(knobs[0].getValue());
    settings.frequency = static_cast<float>(knobs[1].getValue());
    settings.ease = static_cast<float>(knobs[2].getValue());
    settings.slant = static_cast<float>(knobs[3].getValue());
    settings.cyclic = static_cast<float>(knobs[4].getValue());
    settings.jitter = static_cast<float>(knobs[5].getValue());
    return settings;
}

juce::Rectangle<float> ModulationPopup::previewBounds() const
{
    auto bounds = getLocalBounds().reduced(12);
    bounds.removeFromTop(118);
    bounds.removeFromBottom(8);
    return bounds.toFloat();
}

juce::String ModulationPopup::amplitudeInfoText() const
{
    return "[SPAN " + juce::String(std::round(viewModel.settings.amplitude * 100.0f)) + "%]";
}

juce::String ModulationPopup::frequencyInfoText() const
{
    return "[FREQ " + juce::String(pointdrone::core::RandomModulator::modulationFrequencyHz(viewModel.settings.frequency), 2) + " HZ]";
}

void ModulationPopup::emitSettingsChanged()
{
    if (updatingFromState)
        return;

    viewModel.settings = currentSettings();

    if (onSettingsChanged != nullptr)
        onSettingsChanged(viewModel.settings);
}
}
