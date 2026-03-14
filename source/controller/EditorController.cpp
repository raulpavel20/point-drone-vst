#include "EditorController.h"

#include "../core/ChartMapping.h"

#include <cmath>
#include <optional>

namespace pointdrone::controller
{
namespace
{
juce::String frequencyText(const std::optional<pointdrone::domain::PointModel>& point)
{
    return point.has_value()
        ? "[FREQ " + juce::String(point->frequencyHz, 1) + " HZ]"
        : "[FREQ --]";
}

juce::String panText(const std::optional<pointdrone::domain::PointModel>& point)
{
    if (! point.has_value())
        return "[PAN --]";

    if (std::abs(point->pan) < 0.01f)
        return "[PAN CENTER]";

    const auto side = point->pan < 0.0f ? "LEFT" : "RIGHT";
    return "[PAN " + juce::String(std::abs(point->pan) * 100.0f, 0) + "% " + side + "]";
}
}

EditorController::EditorController(pointdrone::state::ProjectState& projectState)
    : state(projectState)
{
}

EditorViewState EditorController::getViewState()
{
    syncSelectionWithState();

    const auto model = state.getModel();

    return {
        createChartViewModel(model, selectedPointId),
        createInspectorViewModel(model, selectedPointId)
    };
}

void EditorController::handleChartBackgroundClicked(const float normalizedX, const float normalizedY)
{
    const auto point = state.addPoint(pointdrone::core::xToFrequency(normalizedX),
                                      pointdrone::core::yToPan(normalizedY));
    selectedPointId = point.id;
}

void EditorController::handlePointClicked(const juce::String& pointId)
{
    selectedPointId = pointId;
    syncSelectionWithState();
}

void EditorController::handlePointDragged(const juce::String& pointId, const float normalizedX, const float normalizedY)
{
    selectedPointId = pointId;

    if (! state.updatePointPosition(pointId,
                                    pointdrone::core::xToFrequency(normalizedX),
                                    pointdrone::core::yToPan(normalizedY)))
    {
        syncSelectionWithState();
    }
}

void EditorController::handlePointDoubleClicked(const juce::String& pointId)
{
    if (! state.removePoint(pointId))
        return;

    if (selectedPointId == pointId)
        selectedPointId.clear();
}

void EditorController::handleWaveMixChanged(const pointdrone::domain::WaveMix& waveMix)
{
    syncSelectionWithState();

    if (selectedPointId.isNotEmpty())
        state.updatePointWaveMix(selectedPointId, waveMix);
}

void EditorController::syncSelectionWithState()
{
    if (selectedPointId.isNotEmpty() && ! state.containsPoint(selectedPointId))
        selectedPointId.clear();
}

ChartViewModel EditorController::createChartViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& currentSelectedPointId)
{
    ChartViewModel viewModel;
    viewModel.points.reserve(model.points.size());

    for (const auto& point : model.points)
    {
        viewModel.points.push_back({
            point.id,
            pointdrone::core::frequencyToX(point.frequencyHz),
            pointdrone::core::panToY(point.pan),
            point.id == currentSelectedPointId
        });
    }

    return viewModel;
}

InspectorViewModel EditorController::createInspectorViewModel(const pointdrone::domain::ProjectModel& model, const juce::String& currentSelectedPointId)
{
    std::optional<pointdrone::domain::PointModel> selectedPoint;

    for (const auto& point : model.points)
    {
        if (point.id == currentSelectedPointId)
        {
            selectedPoint = point;
            break;
        }
    }

    InspectorViewModel viewModel;
    viewModel.hasSelection = selectedPoint.has_value();
    viewModel.waveMix = selectedPoint.has_value() ? selectedPoint->waveMix : pointdrone::domain::WaveMix {};
    viewModel.frequencyText = frequencyText(selectedPoint);
    viewModel.panText = panText(selectedPoint);
    return viewModel;
}
}
