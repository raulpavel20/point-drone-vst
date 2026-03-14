#include "ProjectState.h"

namespace pointdrone::state
{
ProjectState::ProjectState()
    : rootState(createDefaultState())
{
}

domain::ProjectModel ProjectState::getModel() const
{
    const juce::ScopedLock lock(mutex);

    domain::ProjectModel model;
    model.outputGain = static_cast<float>(rootState.getProperty(outputGainProperty(), 1.0f));

    for (const auto pointTree : pointsTree())
        model.points.push_back(pointFromValueTree(pointTree));

    for (const auto snapshotTree : snapshotsTree())
        model.snapshots.push_back(snapshotFromValueTree(snapshotTree));

    return model;
}

domain::PointModel ProjectState::addPoint(const float frequencyHz, const float pan)
{
    const juce::ScopedLock lock(mutex);

    domain::PointModel point;
    point.id = juce::Uuid().toString();
    point.frequencyHz = frequencyHz;
    point.pan = pan;

    pointsTree().appendChild(pointToValueTree(point), nullptr);
    return point;
}

bool ProjectState::removePoint(const juce::String& pointId)
{
    const juce::ScopedLock lock(mutex);

    for (auto pointTree : pointsTree())
    {
        if (pointTree.getProperty(idProperty()).toString() != pointId)
            continue;

        pointsTree().removeChild(pointTree, nullptr);
        return true;
    }

    return false;
}

bool ProjectState::updatePointPosition(const juce::String& pointId, const float frequencyHz, const float pan)
{
    const juce::ScopedLock lock(mutex);

    for (auto pointTree : pointsTree())
    {
        if (pointTree.getProperty(idProperty()).toString() != pointId)
            continue;

        pointTree.setProperty(frequencyProperty(), frequencyHz, nullptr);
        pointTree.setProperty(panProperty(), pan, nullptr);
        return true;
    }

    return false;
}

bool ProjectState::updateOutputGain(const float outputGain)
{
    const juce::ScopedLock lock(mutex);
    rootState.setProperty(outputGainProperty(), outputGain, nullptr);
    return true;
}

bool ProjectState::updatePointGain(const juce::String& pointId, const float gain)
{
    const juce::ScopedLock lock(mutex);

    for (auto pointTree : pointsTree())
    {
        if (pointTree.getProperty(idProperty()).toString() != pointId)
            continue;

        pointTree.setProperty(gainProperty(), gain, nullptr);
        return true;
    }

    return false;
}

bool ProjectState::updatePointWaveTimbre(const juce::String& pointId, const domain::WaveTimbre& waveTimbre)
{
    const juce::ScopedLock lock(mutex);

    for (auto pointTree : pointsTree())
    {
        if (pointTree.getProperty(idProperty()).toString() != pointId)
            continue;

        pointTree.setProperty(sinePhaseProperty(), waveTimbre.sinePhase, nullptr);
        pointTree.setProperty(sawShapeProperty(), waveTimbre.sawShape, nullptr);
        pointTree.setProperty(squarePulseWidthProperty(), waveTimbre.squarePulseWidth, nullptr);
        pointTree.setProperty(noiseToneProperty(), waveTimbre.noiseTone, nullptr);
        return true;
    }

    return false;
}

bool ProjectState::updatePointWaveMix(const juce::String& pointId, const domain::WaveMix& waveMix)
{
    const juce::ScopedLock lock(mutex);

    for (auto pointTree : pointsTree())
    {
        if (pointTree.getProperty(idProperty()).toString() != pointId)
            continue;

        pointTree.setProperty(sineProperty(), waveMix.sine, nullptr);
        pointTree.setProperty(sawProperty(), waveMix.saw, nullptr);
        pointTree.setProperty(squareProperty(), waveMix.square, nullptr);
        pointTree.setProperty(noiseProperty(), waveMix.noise, nullptr);
        return true;
    }

    return false;
}

bool ProjectState::containsPoint(const juce::String& pointId) const
{
    const juce::ScopedLock lock(mutex);

    for (const auto pointTree : pointsTree())
    {
        if (pointTree.getProperty(idProperty()).toString() == pointId)
            return true;
    }

    return false;
}

juce::ValueTree ProjectState::copyState() const
{
    const juce::ScopedLock lock(mutex);
    return rootState.createCopy();
}

void ProjectState::replaceState(const juce::ValueTree& newState)
{
    const juce::ScopedLock lock(mutex);
    rootState = newState.isValid() ? newState.createCopy() : createDefaultState();
}

juce::Identifier ProjectState::projectType() { return "PROJECT"; }
juce::Identifier ProjectState::pointsType() { return "POINTS"; }
juce::Identifier ProjectState::pointType() { return "POINT"; }
juce::Identifier ProjectState::snapshotsType() { return "SNAPSHOTS"; }
juce::Identifier ProjectState::snapshotType() { return "SNAPSHOT"; }
juce::Identifier ProjectState::idProperty() { return "id"; }
juce::Identifier ProjectState::frequencyProperty() { return "frequencyHz"; }
juce::Identifier ProjectState::panProperty() { return "pan"; }
juce::Identifier ProjectState::outputGainProperty() { return "outputGain"; }
juce::Identifier ProjectState::gainProperty() { return "gain"; }
juce::Identifier ProjectState::sinePhaseProperty() { return "sinePhase"; }
juce::Identifier ProjectState::sawShapeProperty() { return "sawShape"; }
juce::Identifier ProjectState::squarePulseWidthProperty() { return "squarePulseWidth"; }
juce::Identifier ProjectState::noiseToneProperty() { return "noiseTone"; }
juce::Identifier ProjectState::sineProperty() { return "sine"; }
juce::Identifier ProjectState::sawProperty() { return "saw"; }
juce::Identifier ProjectState::squareProperty() { return "square"; }
juce::Identifier ProjectState::noiseProperty() { return "noise"; }
juce::Identifier ProjectState::nameProperty() { return "name"; }

domain::PointModel ProjectState::pointFromValueTree(const juce::ValueTree& pointTree)
{
    domain::PointModel point;
    point.id = pointTree.getProperty(idProperty()).toString();
    point.frequencyHz = static_cast<float>(pointTree.getProperty(frequencyProperty()));
    point.pan = static_cast<float>(pointTree.getProperty(panProperty()));
    point.gain = static_cast<float>(pointTree.getProperty(gainProperty(), 1.0f));
    point.waveTimbre.sinePhase = static_cast<float>(pointTree.getProperty(sinePhaseProperty(), 0.0f));
    point.waveTimbre.sawShape = static_cast<float>(pointTree.getProperty(sawShapeProperty(), 0.0f));
    point.waveTimbre.squarePulseWidth = static_cast<float>(pointTree.getProperty(squarePulseWidthProperty(), 0.5f));
    point.waveTimbre.noiseTone = static_cast<float>(pointTree.getProperty(noiseToneProperty(), 0.0f));
    point.waveMix.sine = static_cast<float>(pointTree.getProperty(sineProperty()));
    point.waveMix.saw = static_cast<float>(pointTree.getProperty(sawProperty()));
    point.waveMix.square = static_cast<float>(pointTree.getProperty(squareProperty()));
    point.waveMix.noise = static_cast<float>(pointTree.getProperty(noiseProperty()));
    return point;
}

juce::ValueTree ProjectState::pointToValueTree(const domain::PointModel& point)
{
    juce::ValueTree pointTree(pointType());
    pointTree.setProperty(idProperty(), point.id, nullptr);
    pointTree.setProperty(frequencyProperty(), point.frequencyHz, nullptr);
    pointTree.setProperty(panProperty(), point.pan, nullptr);
    pointTree.setProperty(gainProperty(), point.gain, nullptr);
    pointTree.setProperty(sinePhaseProperty(), point.waveTimbre.sinePhase, nullptr);
    pointTree.setProperty(sawShapeProperty(), point.waveTimbre.sawShape, nullptr);
    pointTree.setProperty(squarePulseWidthProperty(), point.waveTimbre.squarePulseWidth, nullptr);
    pointTree.setProperty(noiseToneProperty(), point.waveTimbre.noiseTone, nullptr);
    pointTree.setProperty(sineProperty(), point.waveMix.sine, nullptr);
    pointTree.setProperty(sawProperty(), point.waveMix.saw, nullptr);
    pointTree.setProperty(squareProperty(), point.waveMix.square, nullptr);
    pointTree.setProperty(noiseProperty(), point.waveMix.noise, nullptr);
    return pointTree;
}

domain::SnapshotModel ProjectState::snapshotFromValueTree(const juce::ValueTree& snapshotTree)
{
    domain::SnapshotModel snapshot;
    snapshot.id = snapshotTree.getProperty(idProperty()).toString();
    snapshot.name = snapshotTree.getProperty(nameProperty()).toString();

    for (const auto pointTree : snapshotTree)
    {
        if (pointTree.hasType(pointType()))
            snapshot.points.push_back(pointFromValueTree(pointTree));
    }

    return snapshot;
}

juce::ValueTree ProjectState::snapshotToValueTree(const domain::SnapshotModel& snapshot)
{
    juce::ValueTree snapshotTree(snapshotType());
    snapshotTree.setProperty(idProperty(), snapshot.id, nullptr);
    snapshotTree.setProperty(nameProperty(), snapshot.name, nullptr);

    for (const auto& point : snapshot.points)
        snapshotTree.appendChild(pointToValueTree(point), nullptr);

    return snapshotTree;
}

juce::ValueTree ProjectState::createDefaultState()
{
    juce::ValueTree project(projectType());
    project.setProperty(outputGainProperty(), 1.0f, nullptr);
    project.appendChild(juce::ValueTree(pointsType()), nullptr);
    project.appendChild(juce::ValueTree(snapshotsType()), nullptr);
    return project;
}

juce::ValueTree ProjectState::pointsTree() const
{
    return rootState.getChildWithName(pointsType());
}

juce::ValueTree ProjectState::snapshotsTree() const
{
    return rootState.getChildWithName(snapshotsType());
}
}
