#pragma once

#include <juce_core/juce_core.h>

namespace pointdrone::audio
{
struct ResonanceInteraction
{
    juce::String pointIdA;
    juce::String pointIdB;
    float strength = 0.0f;
    float differenceToneHz = 0.0f;
    float sumToneHz = 0.0f;
};
}
