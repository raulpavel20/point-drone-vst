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
    std::unordered_map<std::string, PointRuntimeTelemetry> nextRuntimeTelemetry;
    activeVoiceIds.reserve(model.points.size());
    nextRuntimeTelemetry.reserve(model.points.size());

    for (const auto& point : model.points)
    {
        const auto voiceId = point.id.toStdString();
        activeVoiceIds.insert(voiceId);

        auto [iterator, inserted] = voices.try_emplace(voiceId);
        auto& voice = iterator->second;

        if (inserted && hasPrepared)
            voice.prepare(currentSampleRate);

        voice.render(outputBuffer, outputBuffer.getNumSamples(), point);
        nextRuntimeTelemetry[voiceId] = voice.getRuntimeTelemetry();
    }

    std::erase_if(voices, [&activeVoiceIds](const auto& item)
    {
        return ! activeVoiceIds.contains(item.first);
    });

    {
        const juce::SpinLock::ScopedLockType lock(runtimeTelemetryLock);
        runtimeTelemetry = std::move(nextRuntimeTelemetry);
    }

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

std::optional<PointRuntimeTelemetry> PointRenderer::getRuntimeTelemetry(const juce::String& pointId) const
{
    const juce::SpinLock::ScopedLockType lock(runtimeTelemetryLock);
    const auto iterator = runtimeTelemetry.find(pointId.toStdString());

    if (iterator == runtimeTelemetry.end())
        return std::nullopt;

    return iterator->second;
}
}
