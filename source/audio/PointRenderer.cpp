#include "PointRenderer.h"

#include <cmath>
#include <unordered_set>

namespace pointdrone::audio
{
void PointRenderer::prepare(const double sampleRate, int)
{
    currentSampleRate = sampleRate;
    hasPrepared = true;
    outputGain.reset(sampleRate, 0.04);
    outputGain.setCurrentAndTargetValue(1.0f);

    for (auto& [_, voice] : voices)
        voice.prepare(sampleRate);
}

void PointRenderer::render(const domain::ProjectModel& model, juce::AudioBuffer<float>& outputBuffer)
{
    outputBuffer.clear();

    std::unordered_set<std::string> activeVoiceIds;
    activeVoiceIds.reserve(model.points.size());

    for (const auto& point : model.points)
    {
        const auto voiceId = point.id.toStdString();
        activeVoiceIds.insert(voiceId);

        auto [iterator, inserted] = voices.try_emplace(voiceId);
        auto& voice = iterator->second;

        if (inserted && hasPrepared)
            voice.prepare(currentSampleRate);

        voice.render(outputBuffer, outputBuffer.getNumSamples(), point);
    }

    std::erase_if(voices, [&activeVoiceIds](const auto& item)
    {
        return ! activeVoiceIds.contains(item.first);
    });

    const auto pointCount = static_cast<float>(juce::jmax(1, static_cast<int>(model.points.size())));
    outputBuffer.applyGain(1.0f / std::sqrt(pointCount));
    outputGain.setTargetValue(juce::jlimit(0.0f, 2.0f, model.outputGain));

    for (int sampleIndex = 0; sampleIndex < outputBuffer.getNumSamples(); ++sampleIndex)
    {
        const auto gain = outputGain.getNextValue();

        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
        {
            auto* channelData = outputBuffer.getWritePointer(channel);
            channelData[sampleIndex] = std::tanh(channelData[sampleIndex] * gain);
        }
    }
}
}
