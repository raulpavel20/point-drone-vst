#pragma once

#include "PointModel.h"
#include "SnapshotModel.h"

#include <vector>

namespace pointdrone::domain
{
struct ProjectModel
{
    std::vector<PointModel> points;
    std::vector<SnapshotModel> snapshots;
};
}
