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
    void handleChorusRateChanged(float rate);
    void handleChorusDepthChanged(float depth);
    void handleChorusMixChanged(float mix);
    void handleReverbMixChanged(float mix);
    void handleReverbSizeChanged(float size);
    void handleReverbDampingChanged(float damping);
    void handleModulationRequested(pointdrone::domain::ModulationTarget target);
    void handleModulationDisabled();
    void handleModulationPopupClosed();
    void handleModulationSettingsChanged(const pointdrone::domain::ModulationSettings& settings);
    void handleSnapAllPointsToSemitone();
    void handleSnapshotSlotPressed(int slotIndex, bool saveRequested);
    void handleSnapshotTransitionSecondsChanged(float seconds);
    bool advanceSnapshotMorph(double deltaSeconds);
    bool handleFrequencyInputSubmitted(const juce::String& text);
    bool handlePanInputSubmitted(const juce::String& text);

private:
    struct SnapshotMorphState
    {
        int targetSlotIndex = -1;
        float elapsedSeconds = 0.0f;
        float durationSeconds = 0.0f;
        std::vector<pointdrone::domain::PointModel> sourcePoints;
        std::vector<pointdrone::domain::PointModel> targetPoints;
    };

    void clearActiveSnapshotPlayback(bool keepActiveSlot = false);
    void syncSelectionWithState();

    static ChartViewModel createChartViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& selectedPointId);
    static PointWavePreviewViewModel createWavePreviewViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& selectedPointId);
    static PointWavePreviewViewModel createWavePreviewViewModel(const pointdrone::domain::PointModel& point);
    static InspectorViewModel createInspectorViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& selectedPointId);
    static FieldViewModel createFieldViewModel(const pointdrone::domain::ProjectModel& model);
    static MasterOutputViewModel createMasterOutputViewModel(const pointdrone::domain::ProjectModel& model);
    SnapshotControlsViewModel createSnapshotControlsViewModel(const pointdrone::domain::ProjectModel& model) const;
    static ModulationPopupViewModel createModulationPopupViewModel(const pointdrone::domain::ProjectModel& model,
                                                                  const juce::String& selectedPointId,
                                                                  const std::optional<pointdrone::domain::ModulationTarget>& editingModulationTarget);

    pointdrone::state::ProjectState& state;
    juce::String selectedPointId;
    std::optional<pointdrone::domain::ModulationTarget> editingModulationTarget;
    std::optional<int> activeSnapshotSlotIndex;
    std::optional<SnapshotMorphState> activeSnapshotMorph;
};
}
