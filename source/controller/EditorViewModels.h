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

struct PointWavePreviewViewModel
{
    bool hasSelection = false;
    std::vector<float> samples;
};

struct InspectorViewModel
{
    bool hasSelection = false;
    float frequencyHz = 0.0f;
    float pan = 0.0f;
    pointdrone::domain::WaveTimbre waveTimbre;
    pointdrone::domain::WaveMix waveMix;
    float gain = 1.0f;
    juce::String frequencyText = "[FREQ --]";
    juce::String panText = "[PAN --]";
};

struct MasterOutputViewModel
{
    float gain = 1.0f;
};

struct EditorViewState
{
    ChartViewModel chart;
    PointWavePreviewViewModel wavePreview;
    InspectorViewModel inspector;
    MasterOutputViewModel masterOutput;
};
}
