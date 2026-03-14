#pragma once

#include "../controller/EditorViewModels.h"
#include "../domain/PointModel.h"
#include "ModulatableSlider.h"
#include "WaveMixSliders.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace pointdrone::ui
{
class InspectorPanel : public juce::Component
{
public:
    InspectorPanel();

    void paint(juce::Graphics& graphics) override;
    void resized() override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;

    void setViewModel(pointdrone::controller::InspectorViewModel newViewModel);
    void setLiveModulationValues(const std::array<float, 4>& waveTimbreValues,
                                 const std::array<float, 4>& waveMixValues,
                                 float gainValue);

    std::function<void(pointdrone::domain::WaveTimbre)> onWaveTimbreChanged;
    std::function<void(pointdrone::domain::WaveMix)> onWaveMixChanged;
    std::function<void(float)> onGainChanged;
    std::function<void(pointdrone::domain::ModulationTarget)> onModulationRequested;
    std::function<bool(juce::String)> onFrequencyInputSubmitted;
    std::function<bool(juce::String)> onPanInputSubmitted;

private:
    enum class EditableField
    {
        none,
        frequency,
        pan
    };

    juce::Rectangle<int> frequencyTextBounds() const;
    juce::Rectangle<int> panTextBounds() const;
    juce::Rectangle<int> controlsBounds() const;
    juce::String frequencyEditorText() const;
    juce::String panEditorText() const;
    void beginEditing(EditableField field);
    void submitEditing();
    void cancelEditing();
    void hideEditor();

    pointdrone::controller::InspectorViewModel viewModel;
    WaveMixSliders waveTimbreSliders;
    WaveMixSliders waveMixSliders;
    juce::Label gainLabel;
    ModulatableSlider gainSlider;
    juce::TextEditor inputEditor;
    EditableField editingField = EditableField::none;
    juce::String currentPointId;
    float baseGainValue = 1.0f;
    float liveGainValue = 1.0f;
    bool hasLiveGainValue = false;
    bool updatingFromState = false;
};
}
