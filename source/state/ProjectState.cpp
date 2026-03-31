#include "ProjectState.h"

namespace pointdrone::state
{
namespace
{
constexpr float maximumSnapshotTransitionSeconds = 10.0f;

juce::String snapshotIdForSlot(const int slotIndex)
{
    return "snapshot-slot-" + juce::String(slotIndex + 1);
}

juce::String snapshotNameForSlot(const int slotIndex)
{
    return "[SNAP " + juce::String(slotIndex + 1) + "]";
}

juce::ValueTree findModulationTree(const juce::ValueTree& pointTree, const juce::Identifier modulationsType, const juce::Identifier targetProperty, const domain::ModulationTarget target)
{
    if (const auto modulationsTree = pointTree.getChildWithName(modulationsType); modulationsTree.isValid())
    {
        for (const auto modulationTree : modulationsTree)
        {
            if (static_cast<int>(modulationTree.getProperty(targetProperty, -1)) == static_cast<int>(target))
                return modulationTree;
        }
    }

    return {};
}

domain::SnapshotModel emptySnapshotForSlot(const int slotIndex)
{
    domain::SnapshotModel snapshot;
    snapshot.id = snapshotIdForSlot(slotIndex);
    snapshot.slotIndex = slotIndex;
    snapshot.name = snapshotNameForSlot(slotIndex);
    return snapshot;
}
}

ProjectState::ProjectState()
    : rootState(createDefaultState())
{
}

domain::ProjectModel ProjectState::getModel() const
{
    const juce::ScopedLock lock(mutex);

    domain::ProjectModel model;
    model.outputGain = static_cast<float>(rootState.getProperty(outputGainProperty(), 1.0f));
    model.chorusRate = static_cast<float>(rootState.getProperty(chorusRateProperty(), 1.0f));
    model.chorusDepth = static_cast<float>(rootState.getProperty(chorusDepthProperty(), 0.0f));
    model.chorusMix = static_cast<float>(rootState.getProperty(chorusMixProperty(), 0.0f));
    model.reverbMix = static_cast<float>(rootState.getProperty(reverbMixProperty(), 0.0f));
    model.reverbSize = static_cast<float>(rootState.getProperty(reverbSizeProperty(), 0.5f));
    model.reverbDamping = static_cast<float>(rootState.getProperty(reverbDampingProperty(), 0.5f));
    model.snapshotTransitionSeconds = static_cast<float>(rootState.getProperty(snapshotTransitionSecondsProperty(), 0.0f));

    for (std::size_t slotIndex = 0; slotIndex < domain::snapshotSlotCount; ++slotIndex)
        model.snapshots[slotIndex] = emptySnapshotForSlot(static_cast<int>(slotIndex));

    for (const auto pointTree : pointsTree())
        model.points.push_back(pointFromValueTree(pointTree));

    for (const auto snapshotTree : snapshotsTree())
    {
        auto snapshot = snapshotFromValueTree(snapshotTree);

        if (snapshot.slotIndex >= 0 && snapshot.slotIndex < static_cast<int>(domain::snapshotSlotCount))
            model.snapshots[static_cast<std::size_t>(snapshot.slotIndex)] = std::move(snapshot);
    }

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

bool ProjectState::updateChorusRate(const float rate)
{
    const juce::ScopedLock lock(mutex);
    rootState.setProperty(chorusRateProperty(), juce::jlimit(0.0f, 1.0f, rate), nullptr);
    return true;
}

bool ProjectState::updateChorusDepth(const float depth)
{
    const juce::ScopedLock lock(mutex);
    rootState.setProperty(chorusDepthProperty(), juce::jlimit(0.0f, 1.0f, depth), nullptr);
    return true;
}

bool ProjectState::updateChorusMix(const float mix)
{
    const juce::ScopedLock lock(mutex);
    rootState.setProperty(chorusMixProperty(), juce::jlimit(0.0f, 1.0f, mix), nullptr);
    return true;
}

bool ProjectState::updateReverbMix(const float mix)
{
    const juce::ScopedLock lock(mutex);
    rootState.setProperty(reverbMixProperty(), juce::jlimit(0.0f, 1.0f, mix), nullptr);
    return true;
}

bool ProjectState::updateReverbSize(const float size)
{
    const juce::ScopedLock lock(mutex);
    rootState.setProperty(reverbSizeProperty(), juce::jlimit(0.0f, 1.0f, size), nullptr);
    return true;
}

bool ProjectState::updateReverbDamping(const float damping)
{
    const juce::ScopedLock lock(mutex);
    rootState.setProperty(reverbDampingProperty(), juce::jlimit(0.0f, 1.0f, damping), nullptr);
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

bool ProjectState::updatePointModulationEnabled(const juce::String& pointId, const domain::ModulationTarget target, const bool enabled)
{
    const juce::ScopedLock lock(mutex);

    for (auto pointTree : pointsTree())
    {
        if (pointTree.getProperty(idProperty()).toString() != pointId)
            continue;

        if (auto modulationTree = findModulationTree(pointTree, modulationsType(), targetProperty(), target); modulationTree.isValid())
        {
            modulationTree.setProperty(enabledProperty(), enabled, nullptr);
            return true;
        }
    }

    return false;
}

bool ProjectState::updatePointModulationSettings(const juce::String& pointId,
                                                 const domain::ModulationTarget target,
                                                 const domain::ModulationSettings& settings)
{
    const juce::ScopedLock lock(mutex);

    for (auto pointTree : pointsTree())
    {
        if (pointTree.getProperty(idProperty()).toString() != pointId)
            continue;

        if (auto modulationTree = findModulationTree(pointTree, modulationsType(), targetProperty(), target); modulationTree.isValid())
        {
            modulationTree.setProperty(amplitudeProperty(), settings.amplitude, nullptr);
            modulationTree.setProperty(modulationFrequencyProperty(), settings.frequency, nullptr);
            modulationTree.setProperty(easeProperty(), settings.ease, nullptr);
            modulationTree.setProperty(slantProperty(), settings.slant, nullptr);
            modulationTree.setProperty(cyclicProperty(), settings.cyclic, nullptr);
            modulationTree.setProperty(jitterProperty(), settings.jitter, nullptr);
            return true;
        }
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

bool ProjectState::updateSnapshotTransitionSeconds(const float seconds)
{
    const juce::ScopedLock lock(mutex);
    rootState.setProperty(snapshotTransitionSecondsProperty(), juce::jlimit(0.0f, maximumSnapshotTransitionSeconds, seconds), nullptr);
    return true;
}

bool ProjectState::saveSnapshotSlot(const int slotIndex)
{
    if (slotIndex < 0 || slotIndex >= static_cast<int>(domain::snapshotSlotCount))
        return false;

    const juce::ScopedLock lock(mutex);
    auto snapshot = emptySnapshotForSlot(slotIndex);
    snapshot.hasData = true;

    for (const auto pointTree : pointsTree())
        snapshot.points.push_back(pointFromValueTree(pointTree));

    const auto newSnapshotTree = snapshotToValueTree(snapshot);
    auto snapshots = snapshotsTree();
    auto existingSnapshotTree = snapshotTreeForSlot(slotIndex);

    if (existingSnapshotTree.isValid())
    {
        const auto existingIndex = snapshots.indexOf(existingSnapshotTree);
        snapshots.removeChild(existingSnapshotTree, nullptr);
        snapshots.addChild(newSnapshotTree, existingIndex, nullptr);
        return true;
    }

    snapshots.appendChild(newSnapshotTree, nullptr);
    return true;
}

std::optional<domain::SnapshotModel> ProjectState::getSnapshotSlot(const int slotIndex) const
{
    if (slotIndex < 0 || slotIndex >= static_cast<int>(domain::snapshotSlotCount))
        return std::nullopt;

    const juce::ScopedLock lock(mutex);
    const auto snapshotTree = snapshotTreeForSlot(slotIndex);

    if (! snapshotTree.isValid())
        return std::nullopt;

    auto snapshot = snapshotFromValueTree(snapshotTree);

    if (! snapshot.hasData)
        return std::nullopt;

    return snapshot;
}

bool ProjectState::applySnapshotPoints(const std::vector<domain::PointModel>& points)
{
    const juce::ScopedLock lock(mutex);
    auto changed = false;

    for (auto pointTree : pointsTree())
    {
        const auto pointId = pointTree.getProperty(idProperty()).toString();

        for (const auto& point : points)
        {
            if (point.id != pointId)
                continue;

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

            for (const auto target : domain::allModulationTargets)
            {
                if (auto modulationTree = findModulationTree(pointTree, modulationsType(), targetProperty(), target); modulationTree.isValid())
                {
                    const auto& modulation = domain::modulationFor(point, target);
                    modulationTree.setProperty(amplitudeProperty(), modulation.settings.amplitude, nullptr);
                    modulationTree.setProperty(modulationFrequencyProperty(), modulation.settings.frequency, nullptr);
                    modulationTree.setProperty(easeProperty(), modulation.settings.ease, nullptr);
                    modulationTree.setProperty(slantProperty(), modulation.settings.slant, nullptr);
                    modulationTree.setProperty(cyclicProperty(), modulation.settings.cyclic, nullptr);
                    modulationTree.setProperty(jitterProperty(), modulation.settings.jitter, nullptr);
                }
            }

            changed = true;
            break;
        }
    }

    return changed;
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

    if (! rootState.hasProperty(outputGainProperty()))
        rootState.setProperty(outputGainProperty(), 1.0f, nullptr);

    if (! rootState.hasProperty(chorusRateProperty()))
        rootState.setProperty(chorusRateProperty(), 1.0f, nullptr);

    if (! rootState.hasProperty(chorusDepthProperty()))
        rootState.setProperty(chorusDepthProperty(), 0.0f, nullptr);

    if (! rootState.hasProperty(chorusMixProperty()))
        rootState.setProperty(chorusMixProperty(), 0.0f, nullptr);

    if (! rootState.hasProperty(reverbMixProperty()))
        rootState.setProperty(reverbMixProperty(), 0.0f, nullptr);

    if (! rootState.hasProperty(reverbSizeProperty()))
        rootState.setProperty(reverbSizeProperty(), 0.5f, nullptr);

    if (! rootState.hasProperty(reverbDampingProperty()))
        rootState.setProperty(reverbDampingProperty(), 0.5f, nullptr);

    if (! rootState.hasProperty(snapshotTransitionSecondsProperty()))
        rootState.setProperty(snapshotTransitionSecondsProperty(), 0.0f, nullptr);

    if (! pointsTree().isValid())
        rootState.appendChild(juce::ValueTree(pointsType()), nullptr);

    if (! snapshotsTree().isValid())
        rootState.appendChild(juce::ValueTree(snapshotsType()), nullptr);

    auto snapshots = snapshotsTree();

    for (int childIndex = 0; childIndex < snapshots.getNumChildren(); ++childIndex)
    {
        auto snapshotTree = snapshots.getChild(childIndex);

        if (! snapshotTree.hasProperty(slotProperty()))
            snapshotTree.setProperty(slotProperty(), childIndex, nullptr);

        const auto slotIndex = static_cast<int>(snapshotTree.getProperty(slotProperty(), childIndex));

        if (! snapshotTree.hasProperty(idProperty()))
            snapshotTree.setProperty(idProperty(), snapshotIdForSlot(slotIndex), nullptr);

        if (! snapshotTree.hasProperty(hasDataProperty()))
            snapshotTree.setProperty(hasDataProperty(), snapshotTree.getNumChildren() > 0, nullptr);

        if (! snapshotTree.hasProperty(nameProperty()))
            snapshotTree.setProperty(nameProperty(), snapshotNameForSlot(slotIndex), nullptr);
    }

    for (std::size_t slotIndex = 0; slotIndex < domain::snapshotSlotCount; ++slotIndex)
    {
        if (! snapshotTreeForSlot(static_cast<int>(slotIndex)).isValid())
            snapshots.appendChild(snapshotToValueTree(emptySnapshotForSlot(static_cast<int>(slotIndex))), nullptr);
    }
}

juce::Identifier ProjectState::projectType() { return "PROJECT"; }
juce::Identifier ProjectState::pointsType() { return "POINTS"; }
juce::Identifier ProjectState::pointType() { return "POINT"; }
juce::Identifier ProjectState::modulationsType() { return "MODULATIONS"; }
juce::Identifier ProjectState::modulationType() { return "MODULATION"; }
juce::Identifier ProjectState::snapshotsType() { return "SNAPSHOTS"; }
juce::Identifier ProjectState::snapshotType() { return "SNAPSHOT"; }
juce::Identifier ProjectState::idProperty() { return "id"; }
juce::Identifier ProjectState::slotProperty() { return "slot"; }
juce::Identifier ProjectState::hasDataProperty() { return "hasData"; }
juce::Identifier ProjectState::targetProperty() { return "target"; }
juce::Identifier ProjectState::enabledProperty() { return "enabled"; }
juce::Identifier ProjectState::amplitudeProperty() { return "amplitude"; }
juce::Identifier ProjectState::modulationFrequencyProperty() { return "modulationFrequency"; }
juce::Identifier ProjectState::easeProperty() { return "ease"; }
juce::Identifier ProjectState::slantProperty() { return "slant"; }
juce::Identifier ProjectState::cyclicProperty() { return "cyclic"; }
juce::Identifier ProjectState::jitterProperty() { return "jitter"; }
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
juce::Identifier ProjectState::chorusRateProperty() { return "chorusRate"; }
juce::Identifier ProjectState::chorusDepthProperty() { return "chorusDepth"; }
juce::Identifier ProjectState::chorusMixProperty() { return "chorusMix"; }
juce::Identifier ProjectState::reverbMixProperty() { return "reverbMix"; }
juce::Identifier ProjectState::reverbSizeProperty() { return "reverbSize"; }
juce::Identifier ProjectState::reverbDampingProperty() { return "reverbDamping"; }
juce::Identifier ProjectState::snapshotTransitionSecondsProperty() { return "snapshotTransitionSeconds"; }

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

    if (const auto modulationsTree = pointTree.getChildWithName(modulationsType()); modulationsTree.isValid())
    {
        for (const auto modulationTree : modulationsTree)
        {
            const auto targetValue = static_cast<int>(modulationTree.getProperty(targetProperty(), -1));

            if (targetValue < 0 || targetValue >= static_cast<int>(domain::modulationTargetCount))
                continue;

            auto& modulation = point.modulations[static_cast<std::size_t>(targetValue)];
            modulation.enabled = static_cast<bool>(modulationTree.getProperty(enabledProperty(), false));
            modulation.settings.amplitude = static_cast<float>(modulationTree.getProperty(amplitudeProperty(), domain::ModulationSettings {}.amplitude));
            modulation.settings.frequency = static_cast<float>(modulationTree.getProperty(modulationFrequencyProperty(), domain::ModulationSettings {}.frequency));
            modulation.settings.ease = static_cast<float>(modulationTree.getProperty(easeProperty(), domain::ModulationSettings {}.ease));
            modulation.settings.slant = static_cast<float>(modulationTree.getProperty(slantProperty(), domain::ModulationSettings {}.slant));
            modulation.settings.cyclic = static_cast<float>(modulationTree.getProperty(cyclicProperty(), domain::ModulationSettings {}.cyclic));
            modulation.settings.jitter = static_cast<float>(modulationTree.getProperty(jitterProperty(), domain::ModulationSettings {}.jitter));
        }
    }

    return point;
}

juce::ValueTree ProjectState::pointToValueTree(const domain::PointModel& point, const bool includeModulationEnabled)
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

    juce::ValueTree modulationsTree(modulationsType());

    for (const auto target : domain::allModulationTargets)
    {
        const auto& modulation = domain::modulationFor(point, target);
        juce::ValueTree modulationTree(modulationType());
        modulationTree.setProperty(targetProperty(), static_cast<int>(target), nullptr);

        if (includeModulationEnabled)
            modulationTree.setProperty(enabledProperty(), modulation.enabled, nullptr);

        modulationTree.setProperty(amplitudeProperty(), modulation.settings.amplitude, nullptr);
        modulationTree.setProperty(modulationFrequencyProperty(), modulation.settings.frequency, nullptr);
        modulationTree.setProperty(easeProperty(), modulation.settings.ease, nullptr);
        modulationTree.setProperty(slantProperty(), modulation.settings.slant, nullptr);
        modulationTree.setProperty(cyclicProperty(), modulation.settings.cyclic, nullptr);
        modulationTree.setProperty(jitterProperty(), modulation.settings.jitter, nullptr);
        modulationsTree.appendChild(modulationTree, nullptr);
    }

    pointTree.appendChild(modulationsTree, nullptr);
    return pointTree;
}

domain::SnapshotModel ProjectState::snapshotFromValueTree(const juce::ValueTree& snapshotTree)
{
    domain::SnapshotModel snapshot;
    snapshot.slotIndex = static_cast<int>(snapshotTree.getProperty(slotProperty(), 0));
    snapshot.id = snapshotTree.getProperty(idProperty(), snapshotIdForSlot(snapshot.slotIndex)).toString();
    snapshot.hasData = static_cast<bool>(snapshotTree.getProperty(hasDataProperty(), false));
    snapshot.name = snapshotTree.getProperty(nameProperty(), snapshotNameForSlot(snapshot.slotIndex)).toString();

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
    snapshotTree.setProperty(slotProperty(), snapshot.slotIndex, nullptr);
    snapshotTree.setProperty(hasDataProperty(), snapshot.hasData, nullptr);
    snapshotTree.setProperty(nameProperty(), snapshot.name, nullptr);

    for (const auto& point : snapshot.points)
        snapshotTree.appendChild(pointToValueTree(point, false), nullptr);

    return snapshotTree;
}

juce::ValueTree ProjectState::createDefaultState()
{
    juce::ValueTree project(projectType());
    project.setProperty(outputGainProperty(), 1.0f, nullptr);
    project.setProperty(chorusRateProperty(), 1.0f, nullptr);
    project.setProperty(chorusDepthProperty(), 0.0f, nullptr);
    project.setProperty(chorusMixProperty(), 0.0f, nullptr);
    project.setProperty(reverbMixProperty(), 0.0f, nullptr);
    project.setProperty(reverbSizeProperty(), 0.5f, nullptr);
    project.setProperty(reverbDampingProperty(), 0.5f, nullptr);
    project.setProperty(snapshotTransitionSecondsProperty(), 0.0f, nullptr);
    project.appendChild(juce::ValueTree(pointsType()), nullptr);

    juce::ValueTree snapshots(snapshotsType());

    for (std::size_t slotIndex = 0; slotIndex < domain::snapshotSlotCount; ++slotIndex)
        snapshots.appendChild(snapshotToValueTree(emptySnapshotForSlot(static_cast<int>(slotIndex))), nullptr);

    project.appendChild(snapshots, nullptr);
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

juce::ValueTree ProjectState::snapshotTreeForSlot(const int slotIndex) const
{
    auto snapshots = snapshotsTree();

    for (const auto snapshotTree : snapshots)
    {
        if (static_cast<int>(snapshotTree.getProperty(slotProperty(), -1)) == slotIndex)
            return snapshotTree;
    }

    if (slotIndex >= 0 && slotIndex < snapshots.getNumChildren())
        return snapshots.getChild(slotIndex);

    return {};
}
}
