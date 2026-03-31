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
    float chorusRate = 1.0f;
    float chorusDepth = 0.0f;
    float chorusMix = 0.0f;
    float reverbMix = 0.0f;
    float reverbSize = 0.5f;
    float reverbDamping = 0.5f;
    float snapshotTransitionSeconds = 0.0f;
    std::vector<PointModel> points;
    std::array<SnapshotModel, snapshotSlotCount> snapshots;
};
}
