#include "PointRenderer.h"

#include "HarmonicAnalyzer.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <unordered_set>

namespace pointdrone::audio
{
namespace
{
float averagePan(float panA, float panB)
{
    return (panA + panB) * 0.5f;
}
}

void PointRenderer::prepare(const double sampleRate, const int maximumExpectedSamplesPerBlock)
{
    currentSampleRate = sampleRate;
    hasPrepared = true;
    outputGain.reset(sampleRate, 0.04);
    outputGain.setCurrentAndTargetValue(1.0f);

    for (auto& [_, voice] : voices)
        voice.prepare(sampleRate);

    for (auto& ghost : ghostVoices)
        ghost.prepare(sampleRate);

    juce::dsp::ProcessSpec spec{};
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(maximumExpectedSamplesPerBlock);
    spec.numChannels = 2;

    chorus.prepare(spec);
    chorus.setCentreDelay(7.0f);
    chorus.setFeedback(0.0f);
    chorus.reset();

    reverb.setSampleRate(sampleRate);
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

    auto interactions = HarmonicAnalyzer::computeInteractions(model.points);

    std::sort(interactions.begin(), interactions.end(), [](const auto& a, const auto& b)
    {
        return a.strength > b.strength;
    });

    std::unordered_map<std::string, float> panByPointId;
    for (const auto& point : model.points)
        panByPointId[point.id.toStdString()] = point.pan;

    const auto interactionsToAssign = std::min(static_cast<int>(interactions.size()), ghostVoicePoolSize);

    for (int i = 0; i < interactionsToAssign; ++i)
    {
        const auto& interaction = interactions[static_cast<std::size_t>(i)];
        const auto panA = panByPointId[interaction.pointIdA.toStdString()];
        const auto panB = panByPointId[interaction.pointIdB.toStdString()];
        ghostVoices[static_cast<std::size_t>(i)].assign(
            interaction.differenceToneHz,
            interaction.strength,
            averagePan(panA, panB));
    }

    for (int i = interactionsToAssign; i < ghostVoicePoolSize; ++i)
        ghostVoices[static_cast<std::size_t>(i)].release();

    for (auto& ghost : ghostVoices)
        ghost.render(outputBuffer, outputBuffer.getNumSamples());

    {
        const juce::SpinLock::ScopedLockType lock(runtimeTelemetryLock);
        runtimeTelemetry = std::move(nextRuntimeTelemetry);
        resonanceInteractions = std::move(interactions);
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

    chorus.setRate(model.chorusRate * 10.0f);
    chorus.setDepth(model.chorusDepth);
    chorus.setMix(model.chorusMix);

    juce::dsp::AudioBlock<float> chorusBlock(outputBuffer);
    juce::dsp::ProcessContextReplacing<float> chorusContext(chorusBlock);
    chorus.process(chorusContext);

    const auto mix = juce::jlimit(0.0f, 1.0f, model.reverbMix);
    juce::Reverb::Parameters reverbParams;
    reverbParams.roomSize = juce::jlimit(0.0f, 1.0f, model.reverbSize);
    reverbParams.damping = juce::jlimit(0.0f, 1.0f, model.reverbDamping);
    reverbParams.wetLevel = mix;
    reverbParams.dryLevel = 1.0f - mix;
    reverbParams.width = 1.0f;
    reverbParams.freezeMode = 0.0f;
    reverb.setParameters(reverbParams);

    if (outputBuffer.getNumChannels() >= 2)
    {
        reverb.processStereo(outputBuffer.getWritePointer(0),
                             outputBuffer.getWritePointer(1),
                             outputBuffer.getNumSamples());
    }
    else if (outputBuffer.getNumChannels() == 1)
    {
        reverb.processMono(outputBuffer.getWritePointer(0),
                           outputBuffer.getNumSamples());
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

std::vector<ResonanceInteraction> PointRenderer::getResonanceInteractions() const
{
    const juce::SpinLock::ScopedLockType lock(runtimeTelemetryLock);
    return resonanceInteractions;
}
}
