#include "InspectorPanel.h"

#include "../core/Theme.h"

#include <cmath>
#include <utility>

namespace pointdrone::ui
{
InspectorPanel::InspectorPanel()
    : waveTimbreSliders({ "[PHASE]", "[SHAPE]", "[WIDTH]", "[TONE]" }),
      waveMixSliders({ "[SINE]", "[SAW]", "[SQUARE]", "[NOISE]" })
{
    waveTimbreSliders.onValuesChanged = [this](const WaveMixSliders::SliderValues& values)
    {
        if (onWaveTimbreChanged != nullptr)
        {
            onWaveTimbreChanged({
                values[0],
                values[1],
                values[2],
                values[3]
            });
        }
    };

    waveMixSliders.onValuesChanged = [this](const WaveMixSliders::SliderValues& values)
    {
        if (onWaveMixChanged != nullptr)
        {
            onWaveMixChanged({
                values[0],
                values[1],
                values[2],
                values[3]
            });
        }
    };

    gainLabel.setText("[GAIN]", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);

    gainSlider.setSliderStyle(juce::Slider::LinearVertical);
    gainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    gainSlider.setRange(0.0, 1.0, 0.001);
    gainSlider.onValueChange = [this]
    {
        if (updatingFromState)
            return;

        if (onGainChanged != nullptr)
            onGainChanged(static_cast<float>(gainSlider.getValue()));
    };

    inputEditor.setVisible(false);
    inputEditor.setJustification(juce::Justification::centredLeft);
    inputEditor.setIndents(0, 0);
    inputEditor.setBorder(juce::BorderSize<int>(0));
    inputEditor.setSelectAllWhenFocused(true);
    inputEditor.onReturnKey = [this] { submitEditing(); };
    inputEditor.onEscapeKey = [this] { cancelEditing(); };
    inputEditor.onFocusLost = [this] { submitEditing(); };

    addAndMakeVisible(waveTimbreSliders);
    addAndMakeVisible(waveMixSliders);
    addAndMakeVisible(gainLabel);
    addAndMakeVisible(gainSlider);
    addChildComponent(inputEditor);
    waveTimbreSliders.setEnabledState(false);
    waveMixSliders.setEnabledState(false);
    gainSlider.setEnabled(false);
}

void InspectorPanel::paint(juce::Graphics& graphics)
{
    graphics.setColour(pointdrone::core::Theme::outline());
    graphics.drawRect(getLocalBounds(), 1);

    graphics.setColour(pointdrone::core::Theme::text());
    graphics.setFont(16.0f);
    graphics.drawFittedText("[POINT]", getLocalBounds().removeFromTop(28).reduced(12, 0), juce::Justification::centredLeft, 1);

    auto contentBounds = getLocalBounds().reduced(12);
    contentBounds.removeFromTop(36);

    if (! viewModel.hasSelection)
    {
        graphics.setColour(pointdrone::core::Theme::muted());
        graphics.setFont(14.0f);
        graphics.drawFittedText("[SELECT A POINT]", contentBounds.removeFromTop(22), juce::Justification::centredLeft, 1);
        graphics.drawFittedText("[CLICK THE CHART TO ADD ONE]", contentBounds.removeFromTop(22), juce::Justification::centredLeft, 1);
        return;
    }

    graphics.setColour(pointdrone::core::Theme::text());
    graphics.setFont(14.0f);
    graphics.drawFittedText(viewModel.frequencyText, frequencyTextBounds(), juce::Justification::centredLeft, 1);
    graphics.drawFittedText(viewModel.panText, panTextBounds(), juce::Justification::centredLeft, 1);
}

void InspectorPanel::resized()
{
    auto bounds = controlsBounds();

    auto gainBounds = bounds.removeFromRight(48);
    auto rowGap = 16;
    auto rowHeight = (bounds.getHeight() - rowGap) / 2;
    waveTimbreSliders.setBounds(bounds.removeFromTop(rowHeight));
    bounds.removeFromTop(rowGap);
    waveMixSliders.setBounds(bounds);
    gainLabel.setBounds(gainBounds.removeFromBottom(24));
    gainSlider.setBounds(gainBounds.reduced(6, 0));

    if (editingField == EditableField::frequency)
        inputEditor.setBounds(frequencyTextBounds());
    else if (editingField == EditableField::pan)
        inputEditor.setBounds(panTextBounds());
}

void InspectorPanel::mouseDoubleClick(const juce::MouseEvent& event)
{
    if (! viewModel.hasSelection)
        return;

    if (frequencyTextBounds().contains(event.getPosition()))
    {
        beginEditing(EditableField::frequency);
        return;
    }

    if (panTextBounds().contains(event.getPosition()))
        beginEditing(EditableField::pan);
}

void InspectorPanel::setViewModel(pointdrone::controller::InspectorViewModel newViewModel)
{
    viewModel = std::move(newViewModel);

    if (! viewModel.hasSelection)
        hideEditor();

    const juce::ScopedValueSetter<bool> setter(updatingFromState, true);
    waveTimbreSliders.setEnabledState(viewModel.hasSelection);
    waveTimbreSliders.setValues({
        viewModel.waveTimbre.sinePhase,
        viewModel.waveTimbre.sawShape,
        viewModel.waveTimbre.squarePulseWidth,
        viewModel.waveTimbre.noiseTone
    });
    waveMixSliders.setEnabledState(viewModel.hasSelection);
    waveMixSliders.setValues({
        viewModel.waveMix.sine,
        viewModel.waveMix.saw,
        viewModel.waveMix.square,
        viewModel.waveMix.noise
    });
    gainSlider.setEnabled(viewModel.hasSelection);
    gainSlider.setValue(viewModel.gain, juce::dontSendNotification);
    gainLabel.setColour(juce::Label::textColourId, viewModel.hasSelection ? pointdrone::core::Theme::text() : pointdrone::core::Theme::muted());
    repaint();
}

juce::Rectangle<int> InspectorPanel::frequencyTextBounds() const
{
    auto bounds = getLocalBounds().reduced(12);
    bounds.removeFromTop(36);
    return bounds.removeFromTop(22);
}

juce::Rectangle<int> InspectorPanel::panTextBounds() const
{
    auto bounds = getLocalBounds().reduced(12);
    bounds.removeFromTop(58);
    return bounds.removeFromTop(22);
}

juce::Rectangle<int> InspectorPanel::controlsBounds() const
{
    auto bounds = getLocalBounds().reduced(12);
    bounds.removeFromTop(110);
    return bounds;
}

juce::String InspectorPanel::frequencyEditorText() const
{
    return juce::String(viewModel.frequencyHz, 2).trimCharactersAtEnd("0").trimCharactersAtEnd(".");
}

juce::String InspectorPanel::panEditorText() const
{
    const auto panPercent = static_cast<int>(std::round(viewModel.pan * 100.0f));
    return juce::String(panPercent);
}

void InspectorPanel::beginEditing(const EditableField field)
{
    editingField = field;
    inputEditor.setText(field == EditableField::frequency ? frequencyEditorText() : panEditorText(),
                        juce::dontSendNotification);
    inputEditor.setBounds(field == EditableField::frequency ? frequencyTextBounds() : panTextBounds());
    inputEditor.setVisible(true);
    inputEditor.grabKeyboardFocus();
    inputEditor.selectAll();
    repaint();
}

void InspectorPanel::submitEditing()
{
    if (editingField == EditableField::none)
        return;

    auto accepted = false;
    const auto text = inputEditor.getText();

    if (editingField == EditableField::frequency && onFrequencyInputSubmitted != nullptr)
        accepted = onFrequencyInputSubmitted(text);
    else if (editingField == EditableField::pan && onPanInputSubmitted != nullptr)
        accepted = onPanInputSubmitted(text);

    hideEditor();

    if (! accepted)
        repaint();
}

void InspectorPanel::cancelEditing()
{
    hideEditor();
    repaint();
}

void InspectorPanel::hideEditor()
{
    editingField = EditableField::none;
    inputEditor.setVisible(false);
}
}
