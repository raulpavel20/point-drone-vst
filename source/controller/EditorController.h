#pragma once

#include "../state/ProjectState.h"
#include "EditorViewModels.h"

namespace pointdrone::controller
{
class EditorController
{
public:
    explicit EditorController(pointdrone::state::ProjectState& projectState);

    EditorViewState getViewState();

    void handleChartBackgroundClicked(float normalizedX, float normalizedY);
    void handlePointClicked(const juce::String& pointId);
    void handlePointDragged(const juce::String& pointId, float normalizedX, float normalizedY);
    void handlePointDoubleClicked(const juce::String& pointId);
    void handleWaveMixChanged(const pointdrone::domain::WaveMix& waveMix);

private:
    void syncSelectionWithState();

    static ChartViewModel createChartViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& selectedPointId);
    static InspectorViewModel createInspectorViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& selectedPointId);

    pointdrone::state::ProjectState& state;
    juce::String selectedPointId;
};
}
