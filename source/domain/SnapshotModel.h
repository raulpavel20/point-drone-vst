#pragma once

#include "PointModel.h"

#include <vector>

namespace pointdrone::domain
{
inline constexpr std::size_t snapshotSlotCount = 4;

struct SnapshotModel
{
    juce::String id;
    int slotIndex = 0;
    bool hasData = false;
    juce::String name;
    std::vector<PointModel> points;
};
}
