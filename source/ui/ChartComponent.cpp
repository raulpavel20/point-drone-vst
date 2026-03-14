#include "ChartComponent.h"

#include "../core/ChartMapping.h"
#include "../core/Theme.h"

#include <cmath>
#include <utility>

namespace pointdrone::ui
{
namespace
{
constexpr float axisLockThreshold = 0.015f;
}

void ChartComponent::paint(juce::Graphics& graphics)
{
    auto bounds = chartBounds();
    auto topBounds = bounds.removeFromTop(22);
    auto bottomBounds = bounds.removeFromBottom(22);
    const auto currentPlotBounds = bounds.toFloat();

    graphics.setColour(pointdrone::core::Theme::outline());
    graphics.drawRect(currentPlotBounds, 1.0f);

    graphics.setColour(pointdrone::core::Theme::muted());
    graphics.setFont(13.0f);
    graphics.drawFittedText("[PITCH / HZ]", bottomBounds, juce::Justification::centred, 1);
    graphics.drawFittedText("[L]", topBounds.removeFromRight(28), juce::Justification::centredRight, 1);
    graphics.drawFittedText("[R]", bottomBounds.removeFromRight(28), juce::Justification::centredRight, 1);

    graphics.setColour(pointdrone::core::Theme::muted().withAlpha(0.5f));
    for (const auto frequency : { 55.0f, 110.0f, 220.0f, 440.0f, 880.0f, 1760.0f })
    {
        const auto x = currentPlotBounds.getX() + pointdrone::core::frequencyToX(static_cast<float>(frequency)) * currentPlotBounds.getWidth();
        graphics.drawVerticalLine(juce::roundToInt(x), currentPlotBounds.getY(), currentPlotBounds.getBottom());
    }

    graphics.drawHorizontalLine(juce::roundToInt(currentPlotBounds.getCentreY()), currentPlotBounds.getX(), currentPlotBounds.getRight());

    for (const auto& point : viewModel.points)
    {
        const auto position = pointToPosition(point);
        const auto isSelected = point.isSelected;
        const auto radius = isSelected ? 7.0f : 5.0f;

        graphics.setColour(isSelected ? pointdrone::core::Theme::selected() : pointdrone::core::Theme::text());
        graphics.drawEllipse(position.x - radius, position.y - radius, radius * 2.0f, radius * 2.0f, 1.5f);
        graphics.drawLine(position.x - radius - 4.0f, position.y, position.x + radius + 4.0f, position.y, 1.0f);
        graphics.drawLine(position.x, position.y - radius - 4.0f, position.x, position.y + radius + 4.0f, 1.0f);
    }
}

void ChartComponent::mouseDown(const juce::MouseEvent& event)
{
    const auto currentPlotBounds = plotBounds();

    draggedPointId.reset();
    dragStartPosition.reset();
    dragAxisLock = DragAxisLock::none;

    if (! currentPlotBounds.contains(event.position))
        return;

    if (const auto hitPoint = hitTestPoint(event.position); hitPoint.has_value())
    {
        draggedPointId = hitPoint->id;
        dragStartPosition = { hitPoint->normalizedX, hitPoint->normalizedY };

        if (onPointClicked != nullptr)
            onPointClicked(hitPoint->id);
    }
    else
    {
        const auto normalizedX = (event.position.x - currentPlotBounds.getX()) / currentPlotBounds.getWidth();
        const auto normalizedY = (event.position.y - currentPlotBounds.getY()) / currentPlotBounds.getHeight();

        if (onBackgroundClicked != nullptr)
            onBackgroundClicked(normalizedX, normalizedY);
    }

    repaint();
}

void ChartComponent::mouseDrag(const juce::MouseEvent& event)
{
    if (! draggedPointId.has_value() || ! dragStartPosition.has_value())
        return;

    auto dragPosition = normalizedPosition(event.position);

    if (event.mods.isShiftDown())
    {
        const auto deltaX = dragPosition.x - dragStartPosition->x;
        const auto deltaY = dragPosition.y - dragStartPosition->y;

        if (dragAxisLock == DragAxisLock::none
            && (std::abs(deltaX) >= axisLockThreshold || std::abs(deltaY) >= axisLockThreshold))
        {
            dragAxisLock = std::abs(deltaX) >= std::abs(deltaY) ? DragAxisLock::x : DragAxisLock::y;
        }

        if (dragAxisLock == DragAxisLock::x)
            dragPosition.y = dragStartPosition->y;
        else if (dragAxisLock == DragAxisLock::y)
            dragPosition.x = dragStartPosition->x;
    }
    else
    {
        dragAxisLock = DragAxisLock::none;
    }

    if (onPointDragged != nullptr)
        onPointDragged(*draggedPointId, dragPosition.x, dragPosition.y);
}

void ChartComponent::mouseDoubleClick(const juce::MouseEvent& event)
{
    const auto currentPlotBounds = plotBounds();

    if (! currentPlotBounds.contains(event.position))
        return;

    const auto hitPoint = hitTestPoint(event.position);

    if (! hitPoint.has_value())
        return;

    if (onPointDoubleClicked != nullptr)
        onPointDoubleClicked(hitPoint->id);

    repaint();
}

void ChartComponent::mouseUp(const juce::MouseEvent&)
{
    draggedPointId.reset();
    dragStartPosition.reset();
    dragAxisLock = DragAxisLock::none;
}

void ChartComponent::setViewModel(pointdrone::controller::ChartViewModel newViewModel)
{
    viewModel = std::move(newViewModel);
    repaint();
}

juce::Rectangle<int> ChartComponent::chartBounds() const
{
    return getLocalBounds().reduced(12, 12);
}

juce::Rectangle<float> ChartComponent::plotBounds() const
{
    auto bounds = chartBounds();
    bounds.removeFromTop(22);
    bounds.removeFromBottom(22);
    return bounds.toFloat();
}

juce::Point<float> ChartComponent::pointToPosition(const pointdrone::controller::ChartPointViewModel& point) const
{
    const auto currentPlotBounds = plotBounds();
    return {
        currentPlotBounds.getX() + point.normalizedX * currentPlotBounds.getWidth(),
        currentPlotBounds.getY() + point.normalizedY * currentPlotBounds.getHeight()
    };
}

std::optional<pointdrone::controller::ChartPointViewModel> ChartComponent::hitTestPoint(const juce::Point<float> position) const
{
    for (const auto& point : viewModel.points)
    {
        if (pointToPosition(point).getDistanceFrom(position) <= 10.0f)
            return point;
    }

    return std::nullopt;
}

juce::Point<float> ChartComponent::normalizedPosition(const juce::Point<float> position) const
{
    const auto currentPlotBounds = plotBounds();

    return {
        (position.x - currentPlotBounds.getX()) / currentPlotBounds.getWidth(),
        (position.y - currentPlotBounds.getY()) / currentPlotBounds.getHeight()
    };
}
}
