#pragma once

#include "../domain/ProjectModel.h"

#include <juce_data_structures/juce_data_structures.h>

namespace pointdrone::state
{
class ProjectState
{
public:
    ProjectState();

    domain::ProjectModel getModel() const;

    domain::PointModel addPoint(float frequencyHz, float pan);
    bool removePoint(const juce::String& pointId);
    bool updatePointPosition(const juce::String& pointId, float frequencyHz, float pan);
    bool updateOutputGain(float outputGain);
    bool updatePointGain(const juce::String& pointId, float gain);
    bool updatePointModulationEnabled(const juce::String& pointId, domain::ModulationTarget target, bool enabled);
    bool updatePointModulationSettings(const juce::String& pointId,
                                       domain::ModulationTarget target,
                                       const domain::ModulationSettings& settings);
    bool updatePointWaveTimbre(const juce::String& pointId, const domain::WaveTimbre& waveTimbre);
    bool updatePointWaveMix(const juce::String& pointId, const domain::WaveMix& waveMix);
    bool containsPoint(const juce::String& pointId) const;

    juce::ValueTree copyState() const;
    void replaceState(const juce::ValueTree& newState);

private:
    static juce::Identifier projectType();
    static juce::Identifier pointsType();
    static juce::Identifier pointType();
    static juce::Identifier modulationsType();
    static juce::Identifier modulationType();
    static juce::Identifier snapshotsType();
    static juce::Identifier snapshotType();
    static juce::Identifier idProperty();
    static juce::Identifier targetProperty();
    static juce::Identifier enabledProperty();
    static juce::Identifier amplitudeProperty();
    static juce::Identifier modulationFrequencyProperty();
    static juce::Identifier easeProperty();
    static juce::Identifier slantProperty();
    static juce::Identifier cyclicProperty();
    static juce::Identifier jitterProperty();
    static juce::Identifier frequencyProperty();
    static juce::Identifier panProperty();
    static juce::Identifier outputGainProperty();
    static juce::Identifier gainProperty();
    static juce::Identifier sinePhaseProperty();
    static juce::Identifier sawShapeProperty();
    static juce::Identifier squarePulseWidthProperty();
    static juce::Identifier noiseToneProperty();
    static juce::Identifier sineProperty();
    static juce::Identifier sawProperty();
    static juce::Identifier squareProperty();
    static juce::Identifier noiseProperty();
    static juce::Identifier nameProperty();

    static domain::PointModel pointFromValueTree(const juce::ValueTree& pointTree);
    static juce::ValueTree pointToValueTree(const domain::PointModel& point, bool includeModulationEnabled = true);
    static domain::SnapshotModel snapshotFromValueTree(const juce::ValueTree& snapshotTree);
    static juce::ValueTree snapshotToValueTree(const domain::SnapshotModel& snapshot);
    static juce::ValueTree createDefaultState();

    juce::ValueTree pointsTree() const;
    juce::ValueTree snapshotsTree() const;

    mutable juce::CriticalSection mutex;
    juce::ValueTree rootState;
};
}
