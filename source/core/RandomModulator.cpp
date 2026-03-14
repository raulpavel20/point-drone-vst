#include "RandomModulator.h"

#include <cmath>

namespace pointdrone::core
{
namespace
{
constexpr float minimumModulationHz = 0.03f;
constexpr float maximumModulationHz = 10.0f;
constexpr float minimumPreviewModulationHz = 3.0f;
constexpr float maximumPreviewModulationHz = 7.0f;
constexpr double previewWindowSeconds = 10.0;

float frequencyControlForHz(const float frequencyHz)
{
    return juce::jlimit(0.0f,
                        1.0f,
                        (std::log(juce::jlimit(minimumModulationHz, maximumModulationHz, frequencyHz)) - std::log(minimumModulationHz))
                            / (std::log(maximumModulationHz) - std::log(minimumModulationHz)));
}
}

void RandomModulator::prepare(const double newSampleRate, const int seed)
{
    sampleRate = newSampleRate;
    reset(seed);
}

void RandomModulator::reset(const int seed, const float initialValue)
{
    random.setSeed(seed);
    currentValue = initialValue;
    startValue = initialValue;
    targetValue = initialValue;
    previousPatternTarget = initialValue >= 0.0f ? 1.0f : -1.0f;
    segmentSampleIndex = 0;
    segmentSampleCount = 1;
}

float RandomModulator::getNextValue(const pointdrone::domain::ModulationSettings& settings)
{
    if (segmentSampleIndex >= segmentSampleCount)
        chooseNextTarget(settings);

    const auto progress = juce::jlimit(0.0f,
                                       1.0f,
                                       static_cast<float>(segmentSampleIndex) / static_cast<float>(juce::jmax(1, segmentSampleCount - 1)));
    currentValue = juce::jmap(easedProgress(progress, settings.ease, settings.slant, targetValue >= startValue), startValue, targetValue);
    ++segmentSampleIndex;
    return currentValue;
}

float RandomModulator::modulationFrequencyHz(const float controlValue)
{
    return std::exp(juce::jmap(juce::jlimit(0.0f, 1.0f, controlValue),
                               0.0f,
                               1.0f,
                               std::log(minimumModulationHz),
                               std::log(maximumModulationHz)));
}

float RandomModulator::mapToNormalizedRange(const float modulationValue)
{
    return juce::jlimit(0.0f, 1.0f, 0.5f + (0.5f * juce::jlimit(-1.0f, 1.0f, modulationValue)));
}

int RandomModulator::seedForTarget(const juce::String& pointId, const pointdrone::domain::ModulationTarget target)
{
    return static_cast<int>(pointId.hashCode64())
         + static_cast<int>(pointdrone::domain::modulationIndex(target) * 7919);
}

std::vector<float> RandomModulator::createPreview(const pointdrone::domain::ModulationSettings& settings,
                                                  const int seed,
                                                  const double,
                                                  const int sampleCount)
{
    RandomModulator modulator;
    const auto previewSampleRate = static_cast<double>(sampleCount) / previewWindowSeconds;
    modulator.prepare(previewSampleRate, seed);
    auto previewSettings = settings;
    const auto previewFrequencyHz = std::exp(juce::jmap(juce::jlimit(0.0f, 1.0f, settings.frequency),
                                                        0.0f,
                                                        1.0f,
                                                        std::log(minimumPreviewModulationHz),
                                                        std::log(maximumPreviewModulationHz)));
    previewSettings.frequency = frequencyControlForHz(previewFrequencyHz);

    std::vector<float> samples;
    samples.reserve(static_cast<std::size_t>(sampleCount));

    for (int sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
        samples.push_back(modulator.getNextValue(previewSettings));

    return samples;
}

float RandomModulator::easedProgress(const float progress, const float ease, const float slant, const bool rising)
{
    const auto linear = juce::jlimit(0.0f, 1.0f, progress);
    const auto clampedSlant = juce::jlimit(0.0f, 1.0f, slant);
    const auto directedSlant = rising
        ? juce::jmap(clampedSlant, 0.0f, 1.0f, -0.8f, 0.8f)
        : juce::jmap(clampedSlant, 0.0f, 1.0f, 0.8f, -0.8f);

    auto slanted = linear;

    if (directedSlant > 0.0f)
        slanted = std::pow(linear, 1.0f + directedSlant * 2.2f);
    else if (directedSlant < 0.0f)
        slanted = 1.0f - std::pow(1.0f - linear, 1.0f + (-directedSlant * 2.2f));

    const auto smooth = slanted * slanted * (3.0f - 2.0f * slanted);
    const auto late = std::pow(slanted, juce::jmap(juce::jlimit(0.0f, 1.0f, ease), 0.0f, 1.0f, 1.0f, 3.5f));
    return juce::jmap(juce::jlimit(0.0f, 1.0f, ease), slanted, juce::jmap(ease, 0.0f, 1.0f, smooth, late));
}

void RandomModulator::chooseNextTarget(const pointdrone::domain::ModulationSettings& settings)
{
    const auto amplitude = juce::jlimit(0.0f, 1.0f, settings.amplitude);
    const auto frequency = RandomModulator::modulationFrequencyHz(settings.frequency);
    const auto baseDurationSamples = static_cast<float>(sampleRate / frequency);
    const auto jitterAmount = juce::jmap(random.nextFloat(), -settings.jitter, settings.jitter);
    const auto jitterFactor = juce::jlimit(0.2f, 2.0f, 1.0f + jitterAmount);

    const auto randomTarget = juce::jmap(random.nextFloat(), -amplitude, amplitude);
    previousPatternTarget = previousPatternTarget >= 0.0f ? -1.0f : 1.0f;
    const auto patternedTarget = previousPatternTarget * juce::jmap(random.nextFloat(), amplitude * 0.45f, amplitude);
    const auto cyclic = juce::jlimit(0.0f, 1.0f, settings.cyclic);

    startValue = currentValue;
    targetValue = juce::jlimit(-amplitude, amplitude, juce::jmap(cyclic, randomTarget, patternedTarget));
    segmentSampleCount = juce::jmax(2, juce::roundToInt(baseDurationSamples * jitterFactor));
    segmentSampleIndex = 0;
}
}
