#pragma once

#include "PointModel.h"
#include "SnapshotModel.h"

#include <vector>

namespace pointdrone::domain
{
struct ProjectModel
{
    float outputGain = 1.0f;
    std::vector<PointModel> points;
    std::vector<SnapshotModel> snapshots;
};
}
