#pragma once

#include "PointModel.h"

#include <vector>

namespace pointdrone::domain
{
struct SnapshotModel
{
    juce::String id;
    juce::String name;
    std::vector<PointModel> points;
};
}
