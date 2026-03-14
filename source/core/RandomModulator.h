#pragma once

#include "../domain/PointModel.h"

#include <juce_core/juce_core.h>

#include <vector>

namespace pointdrone::core
{
class RandomModulator
{
public:
    void prepare(double newSampleRate, int seed);
    void reset(int seed, float initialValue = 0.0f);
    float getNextValue(const pointdrone::domain::ModulationSettings& settings);
    static float modulationFrequencyHz(float controlValue);
    static float mapToNormalizedRange(float modulationValue);
    static int seedForTarget(const juce::String& pointId, pointdrone::domain::ModulationTarget target);

    static std::vector<float> createPreview(const pointdrone::domain::ModulationSettings& settings,
                                            int seed,
                                            double sampleRate,
                                            int sampleCount);

private:
    static float easedProgress(float progress, float ease, float slant, bool rising);
    void chooseNextTarget(const pointdrone::domain::ModulationSettings& settings);

    double sampleRate = 44100.0;
    juce::Random random;
    float currentValue = 0.0f;
    float startValue = 0.0f;
    float targetValue = 0.0f;
    float previousPatternTarget = 0.0f;
    int segmentSampleIndex = 0;
    int segmentSampleCount = 1;
};
}
