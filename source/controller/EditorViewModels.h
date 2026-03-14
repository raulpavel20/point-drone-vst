#pragma once

#include "../domain/PointModel.h"

#include <vector>

namespace pointdrone::controller
{
struct ChartPointViewModel
{
    juce::String id;
    float normalizedX = 0.0f;
    float normalizedY = 0.5f;
    bool isSelected = false;
};

struct ChartViewModel
{
    std::vector<ChartPointViewModel> points;
};

struct InspectorViewModel
{
    bool hasSelection = false;
    pointdrone::domain::WaveMix waveMix;
    juce::String frequencyText = "[FREQ --]";
    juce::String panText = "[PAN --]";
};

struct EditorViewState
{
    ChartViewModel chart;
    InspectorViewModel inspector;
};
}
