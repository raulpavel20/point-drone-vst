#include "MasterOutputStrip.h"

#include "../core/Theme.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <cmath>
#include <utility>

namespace pointdrone::ui
{
namespace
{
constexpr float maxOutputGain = 2.0f;

float meterLevelFromLinear(const float linearLevel)
{
    const auto decibels = juce::Decibels::gainToDecibels(juce::jmax(linearLevel, 0.00001f), -48.0f);
    return juce::jlimit(0.0f, 1.0f, juce::jmap(decibels, -48.0f, 0.0f, 0.0f, 1.0f));
}
}

MasterOutputStrip::MasterOutputStrip()
{
    startTimerHz(30);
}

void MasterOutputStrip::paint(juce::Graphics& graphics)
{
    graphics.setColour(pointdrone::core::Theme::outline());
    graphics.drawRect(getLocalBounds(), 1);

    auto bounds = getLocalBounds().reduced(8);
    auto headerBounds = bounds.removeFromTop(24);

    graphics.setColour(pointdrone::core::Theme::text());
    graphics.setFont(14.0f);
    graphics.drawFittedText("[OUT]", headerBounds, juce::Justification::centred, 1);

    const auto sliderArea = meterBounds();
    const auto leftMeterBounds = sliderArea.withWidth((sliderArea.getWidth() - 2.0f) * 0.5f);
    const auto rightMeterBounds = juce::Rectangle<float>(leftMeterBounds.getRight() + 2.0f,
                                                         sliderArea.getY(),
                                                         leftMeterBounds.getWidth(),
                                                         sliderArea.getHeight());

    graphics.setColour(pointdrone::core::Theme::outline());
    graphics.drawRect(sliderArea, 1.0f);

    const auto unityY = juce::jmap(1.0f, 0.0f, maxOutputGain, sliderArea.getBottom() - 4.0f, sliderArea.getY() + 4.0f);
    graphics.setColour(pointdrone::core::Theme::muted().withAlpha(0.7f));
    graphics.drawLine(sliderArea.getX() + 2.0f, unityY, sliderArea.getRight() - 2.0f, unityY, 1.0f);
    graphics.drawLine(leftMeterBounds.getRight() + 1.0f, sliderArea.getY() + 1.0f, leftMeterBounds.getRight() + 1.0f, sliderArea.getBottom() - 1.0f, 1.0f);

    const auto leftHeight = leftMeterBounds.getHeight() * meterLevelFromLinear(meterLevels[0]);
    const auto rightHeight = rightMeterBounds.getHeight() * meterLevelFromLinear(meterLevels[1]);

    graphics.setColour(pointdrone::core::Theme::accent().withAlpha(0.18f));
    graphics.fillRect(leftMeterBounds.withTop(leftMeterBounds.getBottom() - leftHeight).reduced(1.0f, 1.0f));
    graphics.fillRect(rightMeterBounds.withTop(rightMeterBounds.getBottom() - rightHeight).reduced(1.0f, 1.0f));

    const auto thumbY = juce::jmap(juce::jlimit(0.0f, maxOutputGain, viewModel.gain),
                                   0.0f,
                                   maxOutputGain,
                                   sliderArea.getBottom() - 4.0f,
                                   sliderArea.getY() + 4.0f);
    const auto thumbBounds = juce::Rectangle<float>(sliderArea.getX() + 3.0f,
                                                    thumbY - 4.0f,
                                                    sliderArea.getWidth() - 6.0f,
                                                    8.0f);

    graphics.setColour(pointdrone::core::Theme::accent());
    graphics.fillRect(thumbBounds);
}

void MasterOutputStrip::mouseDown(const juce::MouseEvent& event)
{
    updateGainFromPosition(event.position);
}

void MasterOutputStrip::mouseDrag(const juce::MouseEvent& event)
{
    updateGainFromPosition(event.position);
}

void MasterOutputStrip::mouseDoubleClick(const juce::MouseEvent&)
{
    viewModel.gain = 1.0f;

    if (onGainChanged != nullptr)
        onGainChanged(viewModel.gain);

    repaint();
}

void MasterOutputStrip::setViewModel(pointdrone::controller::MasterOutputViewModel newViewModel)
{
    viewModel = std::move(newViewModel);
    repaint();
}

void MasterOutputStrip::setMeterSupplier(std::function<std::array<float, 2>()> newMeterSupplier)
{
    meterSupplier = std::move(newMeterSupplier);
}

void MasterOutputStrip::timerCallback()
{
    if (meterSupplier == nullptr)
        return;

    meterLevels = meterSupplier();
    repaint();
}

juce::Rectangle<float> MasterOutputStrip::meterBounds() const
{
    auto bounds = getLocalBounds().reduced(8);
    bounds.removeFromTop(28);
    bounds.removeFromBottom(8);
    return bounds.toFloat();
}

void MasterOutputStrip::updateGainFromPosition(const juce::Point<float> position)
{
    const auto bounds = meterBounds();
    const auto normalized = juce::jlimit(0.0f, 1.0f, (bounds.getBottom() - position.y) / bounds.getHeight());
    const auto gain = normalized * maxOutputGain;
    viewModel.gain = gain;

    if (onGainChanged != nullptr)
        onGainChanged(gain);

    repaint();
}
}
