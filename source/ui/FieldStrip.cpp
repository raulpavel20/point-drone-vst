#include "FieldStrip.h"

#include "../core/Theme.h"

#include <cmath>
#include <utility>

namespace pointdrone::ui
{
namespace
{
constexpr float knobStartAngle = juce::MathConstants<float>::pi * 0.75f;
constexpr float knobEndAngle = juce::MathConstants<float>::pi * 2.25f;
constexpr float dragSensitivity = 0.005f;

float angleForValue(const float value)
{
    return knobStartAngle + juce::jlimit(0.0f, 1.0f, value) * (knobEndAngle - knobStartAngle);
}
}

void FieldStrip::paint(juce::Graphics& graphics)
{
    graphics.setColour(pointdrone::core::Theme::outline());
    graphics.drawRect(getLocalBounds(), 1);

    auto bounds = getLocalBounds().reduced(8);

    auto chorusHeaderBounds = bounds.removeFromTop(18);
    graphics.setColour(pointdrone::core::Theme::text());
    graphics.setFont(11.0f);
    graphics.drawFittedText("[CHORUS]", chorusHeaderBounds, juce::Justification::centred, 1);

    const auto layouts = getKnobLayouts();

    for (int i = 0; i < 3 && i < static_cast<int>(layouts.size()); ++i)
        paintKnob(graphics, layouts[static_cast<std::size_t>(i)].bounds,
                  layouts[static_cast<std::size_t>(i)].label,
                  valueForKnob(layouts[static_cast<std::size_t>(i)].id));

    if (layouts.size() >= 4)
    {
        const auto gapTop = layouts[2].bounds.getBottom();
        const auto gapBottom = layouts[3].bounds.getY();
        auto reverbHeaderBounds = juce::Rectangle<float>(
            static_cast<float>(getLocalBounds().getX() + 8),
            gapTop + 1.0f,
            static_cast<float>(getLocalBounds().getWidth() - 16),
            gapBottom - gapTop - 2.0f);
        graphics.setColour(pointdrone::core::Theme::text());
        graphics.setFont(11.0f);
        graphics.drawFittedText("[REVERB]", reverbHeaderBounds.toNearestInt(), juce::Justification::centred, 1);
    }

    for (int i = 3; i < static_cast<int>(layouts.size()); ++i)
        paintKnob(graphics, layouts[static_cast<std::size_t>(i)].bounds,
                  layouts[static_cast<std::size_t>(i)].label,
                  valueForKnob(layouts[static_cast<std::size_t>(i)].id));
}

void FieldStrip::paintKnob(juce::Graphics& graphics, const juce::Rectangle<float> bounds,
                            const juce::String& label, const float value)
{
    const auto knobSize = juce::jmin(bounds.getWidth(), bounds.getHeight() - 16.0f);
    const auto knobArea = bounds.withTrimmedBottom(16.0f)
                               .withSizeKeepingCentre(knobSize, knobSize);
    const auto centre = knobArea.getCentre();
    const auto radius = knobSize * 0.5f;
    const auto trackRadius = radius - 3.0f;

    graphics.setColour(pointdrone::core::Theme::outline().withAlpha(0.4f));
    juce::Path trackArc;
    trackArc.addCentredArc(centre.x, centre.y, trackRadius, trackRadius,
                           0.0f, knobStartAngle, knobEndAngle, true);
    graphics.strokePath(trackArc, juce::PathStrokeType(2.0f));

    const auto valueAngle = angleForValue(value);

    if (value > 0.005f)
    {
        graphics.setColour(pointdrone::core::Theme::accent().withAlpha(0.5f));
        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, trackRadius, trackRadius,
                               0.0f, knobStartAngle, valueAngle, true);
        graphics.strokePath(valueArc, juce::PathStrokeType(2.5f));
    }

    const auto indicatorLength = trackRadius - 4.0f;
    const auto indicatorX = centre.x + indicatorLength * std::cos(valueAngle - juce::MathConstants<float>::halfPi);
    const auto indicatorY = centre.y + indicatorLength * std::sin(valueAngle - juce::MathConstants<float>::halfPi);

    graphics.setColour(pointdrone::core::Theme::accent());
    graphics.drawLine(centre.x, centre.y, indicatorX, indicatorY, 2.0f);

    graphics.setColour(pointdrone::core::Theme::text());
    graphics.fillEllipse(centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);

    auto labelBounds = juce::Rectangle<float>(bounds.getX(), knobArea.getBottom() + 2.0f,
                                               bounds.getWidth(), 14.0f);
    graphics.setColour(pointdrone::core::Theme::muted());
    graphics.setFont(10.0f);
    graphics.drawFittedText(label, labelBounds.toNearestInt(), juce::Justification::centred, 1);
}

void FieldStrip::mouseDown(const juce::MouseEvent& event)
{
    activeKnob.reset();
    dragStartValue.reset();

    for (const auto& layout : getKnobLayouts())
    {
        if (layout.bounds.contains(event.position))
        {
            activeKnob = layout.id;
            dragStartValue = valueForKnob(layout.id);
            return;
        }
    }
}

void FieldStrip::mouseDrag(const juce::MouseEvent& event)
{
    if (! activeKnob.has_value() || ! dragStartValue.has_value())
        return;

    const auto delta = -event.getDistanceFromDragStartY() * dragSensitivity;
    const auto newValue = juce::jlimit(0.0f, 1.0f, *dragStartValue + delta);

    if (std::abs(newValue - valueForKnob(*activeKnob)) > 1e-6f)
    {
        setValueForKnob(*activeKnob, newValue);
        notifyKnob(*activeKnob, newValue);
        repaint();
    }
}

void FieldStrip::mouseDoubleClick(const juce::MouseEvent& event)
{
    for (const auto& layout : getKnobLayouts())
    {
        if (layout.bounds.contains(event.position))
        {
            const auto def = defaultForKnob(layout.id);
            setValueForKnob(layout.id, def);
            notifyKnob(layout.id, def);
            repaint();
            return;
        }
    }
}

void FieldStrip::setViewModel(pointdrone::controller::FieldViewModel newViewModel)
{
    viewModel = std::move(newViewModel);
    repaint();
}

std::vector<FieldStrip::KnobLayout> FieldStrip::getKnobLayouts() const
{
    auto bounds = getLocalBounds().reduced(8);
    bounds.removeFromTop(20);

    constexpr auto knobCount = 6;
    constexpr auto spacing = 3.0f;
    constexpr auto sectionGap = 22.0f;
    const auto totalGaps = spacing * static_cast<float>(knobCount - 1) + sectionGap;
    const auto knobHeight = (static_cast<float>(bounds.getHeight()) - totalGaps) / static_cast<float>(knobCount);

    std::vector<KnobLayout> layouts;
    layouts.reserve(knobCount);

    auto area = bounds.toFloat();

    layouts.push_back({ area.removeFromTop(knobHeight), "[RATE]", Knob::chorusRate });
    area.removeFromTop(spacing);
    layouts.push_back({ area.removeFromTop(knobHeight), "[DEPTH]", Knob::chorusDepth });
    area.removeFromTop(spacing);
    layouts.push_back({ area.removeFromTop(knobHeight), "[MIX]", Knob::chorusMix });

    area.removeFromTop(sectionGap);

    layouts.push_back({ area.removeFromTop(knobHeight), "[MIX]", Knob::reverbMix });
    area.removeFromTop(spacing);
    layouts.push_back({ area.removeFromTop(knobHeight), "[SIZE]", Knob::reverbSize });
    area.removeFromTop(spacing);
    layouts.push_back({ area.removeFromTop(knobHeight), "[DAMP]", Knob::reverbDamping });

    return layouts;
}

float FieldStrip::valueForKnob(const Knob knob) const
{
    switch (knob)
    {
        case Knob::chorusRate: return viewModel.chorusRate;
        case Knob::chorusDepth: return viewModel.chorusDepth;
        case Knob::chorusMix: return viewModel.chorusMix;
        case Knob::reverbMix: return viewModel.reverbMix;
        case Knob::reverbSize: return viewModel.reverbSize;
        case Knob::reverbDamping: return viewModel.reverbDamping;
    }

    return 0.0f;
}

float FieldStrip::defaultForKnob(const Knob knob) const
{
    switch (knob)
    {
        case Knob::chorusRate: return 1.0f;
        case Knob::chorusDepth: return 0.0f;
        case Knob::chorusMix: return 0.0f;
        case Knob::reverbMix: return 0.0f;
        case Knob::reverbSize: return 0.5f;
        case Knob::reverbDamping: return 0.5f;
    }

    return 0.0f;
}

void FieldStrip::setValueForKnob(const Knob knob, const float value)
{
    switch (knob)
    {
        case Knob::chorusRate: viewModel.chorusRate = value; break;
        case Knob::chorusDepth: viewModel.chorusDepth = value; break;
        case Knob::chorusMix: viewModel.chorusMix = value; break;
        case Knob::reverbMix: viewModel.reverbMix = value; break;
        case Knob::reverbSize: viewModel.reverbSize = value; break;
        case Knob::reverbDamping: viewModel.reverbDamping = value; break;
    }
}

void FieldStrip::notifyKnob(const Knob knob, const float value)
{
    switch (knob)
    {
        case Knob::chorusRate:
            if (onChorusRateChanged != nullptr) onChorusRateChanged(value);
            break;
        case Knob::chorusDepth:
            if (onChorusDepthChanged != nullptr) onChorusDepthChanged(value);
            break;
        case Knob::chorusMix:
            if (onChorusMixChanged != nullptr) onChorusMixChanged(value);
            break;
        case Knob::reverbMix:
            if (onReverbMixChanged != nullptr) onReverbMixChanged(value);
            break;
        case Knob::reverbSize:
            if (onReverbSizeChanged != nullptr) onReverbSizeChanged(value);
            break;
        case Knob::reverbDamping:
            if (onReverbDampingChanged != nullptr) onReverbDampingChanged(value);
            break;
    }
}
}
