#pragma once

#include "PointModel.h"
#include "SnapshotModel.h"

#include <array>
#include <vector>

namespace pointdrone::domain
{
struct ProjectModel
{
    float outputGain = 1.0f;
    float snapshotTransitionSeconds = 0.0f;
    std::vector<PointModel> points;
    std::array<SnapshotModel, snapshotSlotCount> snapshots;
};
}
