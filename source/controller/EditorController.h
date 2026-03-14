#pragma once

#include "../audio/PointRuntimeTelemetry.h"
#include <optional>

#include "../state/ProjectState.h"
#include "EditorViewModels.h"

namespace pointdrone::controller
{
class EditorController
{
public:
    explicit EditorController(pointdrone::state::ProjectState& projectState);

    EditorViewState getViewState();
    juce::String getSelectedPointId() const;
    PointWavePreviewViewModel getLiveWavePreviewViewModel(const pointdrone::audio::PointRuntimeTelemetry& telemetry);

    void handleChartBackgroundClicked(float normalizedX, float normalizedY);
    void handlePointClicked(const juce::String& pointId);
    void handlePointDragged(const juce::String& pointId, float normalizedX, float normalizedY);
    void handlePointDoubleClicked(const juce::String& pointId);
    void handleWaveTimbreChanged(const pointdrone::domain::WaveTimbre& waveTimbre);
    void handleWaveMixChanged(const pointdrone::domain::WaveMix& waveMix);
    void handleGainChanged(float gain);
    void handleOutputGainChanged(float gain);
    void handleModulationRequested(pointdrone::domain::ModulationTarget target);
    void handleModulationDisabled();
    void handleModulationPopupClosed();
    void handleModulationSettingsChanged(const pointdrone::domain::ModulationSettings& settings);
    void handleSnapAllPointsToSemitone();
    bool handleFrequencyInputSubmitted(const juce::String& text);
    bool handlePanInputSubmitted(const juce::String& text);

private:
    void syncSelectionWithState();

    static ChartViewModel createChartViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& selectedPointId);
    static PointWavePreviewViewModel createWavePreviewViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& selectedPointId);
    static PointWavePreviewViewModel createWavePreviewViewModel(const pointdrone::domain::PointModel& point);
    static InspectorViewModel createInspectorViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& selectedPointId);
    static MasterOutputViewModel createMasterOutputViewModel(const pointdrone::domain::ProjectModel& model);
    static ModulationPopupViewModel createModulationPopupViewModel(const pointdrone::domain::ProjectModel& model,
                                                                  const juce::String& selectedPointId,
                                                                  const std::optional<pointdrone::domain::ModulationTarget>& editingModulationTarget);

    pointdrone::state::ProjectState& state;
    juce::String selectedPointId;
    std::optional<pointdrone::domain::ModulationTarget> editingModulationTarget;
};
}
