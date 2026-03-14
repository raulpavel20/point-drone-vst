#pragma once

#include <juce_core/juce_core.h>

#include <array>

namespace pointdrone::domain
{
struct WaveMix
{
    float sine = 1.0f;
    float saw = 0.0f;
    float square = 0.0f;
    float noise = 0.0f;
};

struct WaveTimbre
{
    float sinePhase = 0.0f;
    float sawShape = 0.0f;
    float squarePulseWidth = 0.5f;
    float noiseTone = 0.0f;
};

enum class ModulationTarget
{
    sinePhase,
    sawShape,
    squarePulseWidth,
    noiseTone,
    sine,
    saw,
    square,
    noise,
    gain
};

inline constexpr std::size_t modulationTargetCount = 9;

inline constexpr std::array<ModulationTarget, modulationTargetCount> allModulationTargets {
    ModulationTarget::sinePhase,
    ModulationTarget::sawShape,
    ModulationTarget::squarePulseWidth,
    ModulationTarget::noiseTone,
    ModulationTarget::sine,
    ModulationTarget::saw,
    ModulationTarget::square,
    ModulationTarget::noise,
    ModulationTarget::gain
};

struct ModulationSettings
{
    float amplitude = 0.15f;
    float frequency = 0.25f;
    float ease = 0.5f;
    float slant = 0.5f;
    float cyclic = 0.0f;
    float jitter = 0.0f;
};

struct ParameterModulation
{
    bool enabled = false;
    ModulationSettings settings;
};

struct PointModel
{
    juce::String id;
    float frequencyHz = 220.0f;
    float pan = 0.0f;
    float gain = 1.0f;
    WaveTimbre waveTimbre;
    WaveMix waveMix;
    std::array<ParameterModulation, modulationTargetCount> modulations;
};

inline std::size_t modulationIndex(const ModulationTarget target)
{
    return static_cast<std::size_t>(target);
}

inline ParameterModulation& modulationFor(PointModel& point, const ModulationTarget target)
{
    return point.modulations[modulationIndex(target)];
}

inline const ParameterModulation& modulationFor(const PointModel& point, const ModulationTarget target)
{
    return point.modulations[modulationIndex(target)];
}
}
