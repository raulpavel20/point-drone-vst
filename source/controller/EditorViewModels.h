#pragma once

#include "../domain/PointModel.h"
#include "../domain/SnapshotModel.h"

#include <array>
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

struct ChartInteractionViewModel
{
    juce::String pointIdA;
    juce::String pointIdB;
    float strength = 0.0f;
};

struct ChartViewModel
{
    std::vector<ChartPointViewModel> points;
    std::vector<ChartInteractionViewModel> interactions;
};

struct PointWavePreviewViewModel
{
    bool hasSelection = false;
    std::vector<float> samples;
};

struct InspectorModulationViewModel
{
    std::array<bool, 4> waveTimbre { false, false, false, false };
    std::array<bool, 4> waveMix { false, false, false, false };
    bool gain = false;
};

struct InspectorViewModel
{
    juce::String pointId;
    bool hasSelection = false;
    float frequencyHz = 0.0f;
    float pan = 0.0f;
    pointdrone::domain::WaveTimbre waveTimbre;
    pointdrone::domain::WaveMix waveMix;
    float gain = 1.0f;
    InspectorModulationViewModel modulation;
    juce::String frequencyText = "[FREQ --]";
    juce::String panText = "[PAN --]";
};

struct MasterOutputViewModel
{
    float gain = 1.0f;
};

struct SnapshotSlotViewModel
{
    int slotIndex = 0;
    bool hasData = false;
    bool isActive = false;
    juce::String label;
};

struct SnapshotControlsViewModel
{
    std::array<SnapshotSlotViewModel, pointdrone::domain::snapshotSlotCount> slots;
    float transitionSeconds = 0.0f;
};

struct ModulationPopupViewModel
{
    bool visible = false;
    juce::String title = "[MOD]";
    pointdrone::domain::ModulationSettings settings;
    std::vector<float> samples;
};

struct EditorViewState
{
    ChartViewModel chart;
    PointWavePreviewViewModel wavePreview;
    InspectorViewModel inspector;
    MasterOutputViewModel masterOutput;
    SnapshotControlsViewModel snapshotControls;
    ModulationPopupViewModel modulationPopup;
};
}
