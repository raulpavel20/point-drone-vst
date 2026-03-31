#pragma once

#include "../controller/EditorViewModels.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <functional>
#include <optional>

namespace pointdrone::ui
{
class FieldStrip : public juce::Component
{
public:
    void paint(juce::Graphics& graphics) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;

    void setViewModel(pointdrone::controller::FieldViewModel newViewModel);

    std::function<void(float)> onChorusRateChanged;
    std::function<void(float)> onChorusDepthChanged;
    std::function<void(float)> onChorusMixChanged;
    std::function<void(float)> onReverbMixChanged;
    std::function<void(float)> onReverbSizeChanged;
    std::function<void(float)> onReverbDampingChanged;

private:
    enum class Knob { chorusRate, chorusDepth, chorusMix, reverbMix, reverbSize, reverbDamping };

    struct KnobLayout
    {
        juce::Rectangle<float> bounds;
        juce::String label;
        Knob id;
    };

    std::vector<KnobLayout> getKnobLayouts() const;
    float valueForKnob(Knob knob) const;
    float defaultForKnob(Knob knob) const;
    void setValueForKnob(Knob knob, float value);
    void notifyKnob(Knob knob, float value);

    static void paintKnob(juce::Graphics& graphics, juce::Rectangle<float> bounds,
                           const juce::String& label, float value);

    pointdrone::controller::FieldViewModel viewModel;
    std::optional<Knob> activeKnob;
    std::optional<float> dragStartValue;
};
}
