#include "SnapshotSlots.h"

#include "../core/Theme.h"

#include <utility>

namespace pointdrone::ui
{
void SnapshotSlots::paint(juce::Graphics& graphics)
{
    graphics.setColour(pointdrone::core::Theme::outline());
    graphics.drawRect(getLocalBounds(), 1);

    for (std::size_t slotIndex = 0; slotIndex < viewModel.size(); ++slotIndex)
    {
        const auto bounds = slotBounds(static_cast<int>(slotIndex));
        const auto& slot = viewModel[slotIndex];

        if (slot.isActive)
        {
            graphics.setColour(pointdrone::core::Theme::accent().withAlpha(0.16f));
            graphics.fillRect(bounds);
        }

        graphics.setColour(slot.hasData || slot.isActive ? pointdrone::core::Theme::accent() : pointdrone::core::Theme::outline());
        graphics.drawRect(bounds, 1);
        graphics.setColour(slot.hasData || slot.isActive ? pointdrone::core::Theme::text() : pointdrone::core::Theme::muted());
        graphics.setFont(14.0f);
        graphics.drawFittedText(slot.label, bounds, juce::Justification::centred, 1);
    }
}

void SnapshotSlots::mouseUp(const juce::MouseEvent& event)
{
    if (onSlotPressed == nullptr)
        return;

    for (std::size_t slotIndex = 0; slotIndex < viewModel.size(); ++slotIndex)
    {
        if (slotBounds(static_cast<int>(slotIndex)).contains(event.getPosition()))
        {
            onSlotPressed(static_cast<int>(slotIndex), event.mods.isShiftDown());
            return;
        }
    }
}

void SnapshotSlots::setViewModel(SlotViewModels newViewModel)
{
    viewModel = std::move(newViewModel);
    repaint();
}

juce::Rectangle<int> SnapshotSlots::slotBounds(const int slotIndex) const
{
    auto bounds = getLocalBounds().reduced(6);
    const auto gap = 8;
    const auto slotHeight = (bounds.getHeight() - (gap * (static_cast<int>(viewModel.size()) - 1))) / static_cast<int>(viewModel.size());
    return juce::Rectangle<int>(bounds.getX(),
                                bounds.getY() + (slotIndex * (slotHeight + gap)),
                                bounds.getWidth(),
                                slotHeight);
}
}
