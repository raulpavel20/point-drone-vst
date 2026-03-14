#include "PointWavePreview.h"

#include "../core/Theme.h"

#include <utility>

namespace pointdrone::ui
{
void PointWavePreview::paint(juce::Graphics& graphics)
{
    graphics.setColour(pointdrone::core::Theme::outline());
    graphics.drawRect(getLocalBounds(), 1);

    auto bounds = getLocalBounds().reduced(10);
    auto headerBounds = bounds.removeFromTop(24);

    graphics.setColour(pointdrone::core::Theme::text());
    graphics.setFont(14.0f);
    graphics.drawFittedText("[WAVE]", headerBounds, juce::Justification::centred, 1);

    if (! viewModel.hasSelection || viewModel.samples.empty())
    {
        graphics.setColour(pointdrone::core::Theme::muted());
        graphics.drawFittedText("[NO POINT]", bounds, juce::Justification::centred, 1);
        return;
    }

    const auto previewBounds = bounds.toFloat();
    const auto centerX = previewBounds.getCentreX();

    graphics.setColour(pointdrone::core::Theme::muted());
    graphics.drawLine(centerX, previewBounds.getY(), centerX, previewBounds.getBottom(), 1.0f);

    juce::Path waveform;

    for (int sampleIndex = 0; sampleIndex < static_cast<int>(viewModel.samples.size()); ++sampleIndex)
    {
        const auto normalizedY = juce::jmap(static_cast<float>(sampleIndex),
                                            0.0f,
                                            static_cast<float>(viewModel.samples.size() - 1),
                                            previewBounds.getY(),
                                            previewBounds.getBottom());
        const auto x = centerX + viewModel.samples[static_cast<std::size_t>(sampleIndex)] * (previewBounds.getWidth() * 0.42f);

        if (sampleIndex == 0)
            waveform.startNewSubPath(x, normalizedY);
        else
            waveform.lineTo(x, normalizedY);
    }

    graphics.setColour(pointdrone::core::Theme::accent());
    graphics.strokePath(waveform, juce::PathStrokeType(1.5f));
}

void PointWavePreview::setViewModel(pointdrone::controller::PointWavePreviewViewModel newViewModel)
{
    viewModel = std::move(newViewModel);
    repaint();
}
}
