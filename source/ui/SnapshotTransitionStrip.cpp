#include "SnapshotTransitionStrip.h"

#include "../core/Theme.h"

#include <utility>

namespace pointdrone::ui
{
namespace
{
constexpr float maximumSnapshotTransitionSeconds = 10.0f;
}

void SnapshotTransitionStrip::paint(juce::Graphics& graphics)
{
    graphics.setColour(pointdrone::core::Theme::outline());
    graphics.drawRect(getLocalBounds(), 1);

    auto bounds = getLocalBounds().reduced(8);
    auto headerBounds = bounds.removeFromTop(24);
    auto footerBounds = bounds.removeFromBottom(18);

    graphics.setColour(pointdrone::core::Theme::text());
    graphics.setFont(14.0f);
    graphics.drawFittedText("[TIME]", headerBounds, juce::Justification::centred, 1);
    graphics.setFont(12.0f);
    graphics.drawFittedText("[" + juce::String(transitionSeconds, 1) + "S]", footerBounds, juce::Justification::centred, 1);

    const auto boundsToDraw = sliderBounds();
    const auto centerX = boundsToDraw.getCentreX();
    graphics.setColour(pointdrone::core::Theme::outline());
    graphics.drawRect(boundsToDraw, 1.0f);
    graphics.drawLine(centerX, boundsToDraw.getY() + 4.0f, centerX, boundsToDraw.getBottom() - 4.0f, 1.0f);

    const auto thumbY = juce::jmap(juce::jlimit(0.0f, maximumSnapshotTransitionSeconds, transitionSeconds),
                                   0.0f,
                                   maximumSnapshotTransitionSeconds,
                                   boundsToDraw.getBottom() - 4.0f,
                                   boundsToDraw.getY() + 4.0f);
    const auto thumbBounds = juce::Rectangle<float>(boundsToDraw.getX() + 3.0f,
                                                    thumbY - 4.0f,
                                                    boundsToDraw.getWidth() - 6.0f,
                                                    8.0f);

    graphics.setColour(pointdrone::core::Theme::accent());
    graphics.fillRect(thumbBounds);
}

void SnapshotTransitionStrip::mouseDown(const juce::MouseEvent& event)
{
    updateFromPosition(event.position);
}

void SnapshotTransitionStrip::mouseDrag(const juce::MouseEvent& event)
{
    updateFromPosition(event.position);
}

void SnapshotTransitionStrip::setTransitionSeconds(const float newTransitionSeconds)
{
    transitionSeconds = juce::jlimit(0.0f, maximumSnapshotTransitionSeconds, newTransitionSeconds);
    repaint();
}

juce::Rectangle<float> SnapshotTransitionStrip::sliderBounds() const
{
    auto bounds = getLocalBounds().reduced(8);
    bounds.removeFromTop(28);
    bounds.removeFromBottom(20);
    return bounds.toFloat();
}

void SnapshotTransitionStrip::updateFromPosition(const juce::Point<float> position)
{
    const auto bounds = sliderBounds();
    const auto normalized = juce::jlimit(0.0f, 1.0f, (bounds.getBottom() - position.y) / bounds.getHeight());
    transitionSeconds = normalized * maximumSnapshotTransitionSeconds;

    if (onTransitionSecondsChanged != nullptr)
        onTransitionSecondsChanged(transitionSeconds);

    repaint();
}
}
