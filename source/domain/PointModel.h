#pragma once

#include <juce_core/juce_core.h>

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

struct PointModel
{
    juce::String id;
    float frequencyHz = 220.0f;
    float pan = 0.0f;
    float gain = 1.0f;
    WaveTimbre waveTimbre;
    WaveMix waveMix;
};
}
