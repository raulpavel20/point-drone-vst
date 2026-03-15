#pragma once

#include "../controller/EditorViewModels.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <array>

namespace pointdrone::ui
{
class SnapshotSlots : public juce::Component
{
public:
    using SlotViewModels = std::array<pointdrone::controller::SnapshotSlotViewModel, pointdrone::domain::snapshotSlotCount>;

    void paint(juce::Graphics& graphics) override;
    void mouseUp(const juce::MouseEvent& event) override;

    void setViewModel(SlotViewModels newViewModel);

    std::function<void(int, bool)> onSlotPressed;

private:
    juce::Rectangle<int> slotBounds(int slotIndex) const;

    SlotViewModels viewModel;
};
}
