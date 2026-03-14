#pragma once

#include <juce_graphics/juce_graphics.h>

namespace pointdrone::core
{
struct Theme
{
    static juce::Colour background() { return juce::Colours::black; }
    static juce::Colour accent() { return juce::Colour::fromString("FF5EE9C4"); }
    static juce::Colour text() { return juce::Colour::fromRGBA(255, 255, 255, 220); }
    static juce::Colour muted() { return juce::Colour::fromRGBA(255, 255, 255, 100); }
    static juce::Colour outline() { return juce::Colour::fromRGBA(255, 255, 255, 140); }
    static juce::Colour selected() { return accent(); }
};
}
