#pragma once

#include "../domain/PointModel.h"

#include <array>

namespace pointdrone::audio
{
inline constexpr int pointRuntimeWaveformSampleCount = 96;

struct PointRuntimeTelemetry
{
    bool active = false;
    std::array<float, pointdrone::domain::modulationTargetCount> modulatedValues {};
    std::array<float, pointRuntimeWaveformSampleCount> waveform {};
};
}
