#pragma once

#include <juce_core/juce_core.h>

#include <cmath>

namespace pointdrone::core
{
inline constexpr float minFrequencyHz = 30.0f;
inline constexpr float maxFrequencyHz = 4000.0f;

inline float xToFrequency(const float normalizedX)
{
    const auto clamped = juce::jlimit(0.0f, 1.0f, normalizedX);
    return minFrequencyHz * std::pow(maxFrequencyHz / minFrequencyHz, clamped);
}

inline float frequencyToX(const float frequencyHz)
{
    const auto clamped = juce::jlimit(minFrequencyHz, maxFrequencyHz, frequencyHz);
    return std::log(clamped / minFrequencyHz) / std::log(maxFrequencyHz / minFrequencyHz);
}

inline float yToPan(const float normalizedY)
{
    return juce::jlimit(-1.0f, 1.0f, juce::jmap(normalizedY, 0.0f, 1.0f, -1.0f, 1.0f));
}

inline float panToY(const float pan)
{
    return juce::jmap(juce::jlimit(-1.0f, 1.0f, pan), -1.0f, 1.0f, 0.0f, 1.0f);
}
}
