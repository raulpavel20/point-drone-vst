#pragma once

#include "../domain/PointModel.h"
#include "ResonanceInteraction.h"

#include <cmath>
#include <vector>

namespace pointdrone::audio
{
class HarmonicAnalyzer
{
public:
    static std::vector<ResonanceInteraction> computeInteractions(const std::vector<domain::PointModel>& points)
    {
        std::vector<ResonanceInteraction> result;

        for (std::size_t i = 0; i < points.size(); ++i)
        {
            for (std::size_t j = i + 1; j < points.size(); ++j)
            {
                const auto& a = points[i];
                const auto& b = points[j];
                const auto strength = computePairStrength(a.frequencyHz, b.frequencyHz, a.gain, b.gain);

                if (strength < minStrengthThreshold)
                    continue;

                const auto fLow = std::min(a.frequencyHz, b.frequencyHz);
                const auto fHigh = std::max(a.frequencyHz, b.frequencyHz);

                result.push_back({
                    a.id,
                    b.id,
                    strength,
                    fHigh - fLow,
                    fHigh + fLow
                });
            }
        }

        return result;
    }

private:
    static constexpr float minStrengthThreshold = 0.05f;
    static constexpr float centsSigma = 15.0f;

    struct HarmonicRatio
    {
        float ratio;
        float weight;
    };

    static constexpr std::array<HarmonicRatio, 10> targetRatios {{
        { 1.0f,                1.0f / 1.0f },
        { 2.0f,                1.0f / 2.0f },
        { 3.0f / 2.0f,        1.0f / 4.0f },
        { 4.0f / 3.0f,        1.0f / 6.0f },
        { 5.0f / 4.0f,        1.0f / 8.0f },
        { 5.0f / 3.0f,        1.0f / 7.0f },
        { 6.0f / 5.0f,        1.0f / 10.0f },
        { 3.0f,                1.0f / 3.0f },
        { 4.0f,                1.0f / 4.0f },
        { 5.0f,                1.0f / 5.0f },
    }};

    static float ratioToCents(float ratio, float target)
    {
        return 1200.0f * std::log2(ratio / target);
    }

    static float gaussianFalloff(float cents)
    {
        return std::exp(-(cents * cents) / (2.0f * centsSigma * centsSigma));
    }

    static float computePairStrength(float freqA, float freqB, float gainA, float gainB)
    {
        if (freqA <= 0.0f || freqB <= 0.0f)
            return 0.0f;

        const auto fHigh = std::max(freqA, freqB);
        const auto fLow = std::min(freqA, freqB);
        const auto ratio = fHigh / fLow;

        float bestScore = 0.0f;

        for (const auto& [targetRatio, weight] : targetRatios)
        {
            const auto cents = std::abs(ratioToCents(ratio, targetRatio));
            const auto closeness = gaussianFalloff(cents);
            const auto score = weight * closeness;

            if (score > bestScore)
                bestScore = score;
        }

        const auto gainFactor = std::sqrt(juce::jlimit(0.0f, 1.0f, gainA)
                                        * juce::jlimit(0.0f, 1.0f, gainB));

        return juce::jlimit(0.0f, 1.0f, bestScore * gainFactor);
    }
};
}
