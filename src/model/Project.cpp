#include "Project.h"

namespace studoo
{
// ============================================================================
//  Tipos de canal
// ============================================================================
juce::String channelTypeToString (ChannelType t)
{
    switch (t)
    {
        case ChannelType::sampler: return "sampler";
        case ChannelType::synth:   return "synth";
        case ChannelType::bass808: return "808";
        case ChannelType::plugin:  return "plugin";
        case ChannelType::audio:   return "audio";
    }
    return "sampler";
}

ChannelType channelTypeFromString (const juce::String& s)
{
    if (s == "synth")  return ChannelType::synth;
    if (s == "808")    return ChannelType::bass808;
    if (s == "plugin") return ChannelType::plugin;
    if (s == "audio")  return ChannelType::audio;
    return ChannelType::sampler;
}

// ============================================================================
//  Automatización
// ============================================================================
AutomationTarget AutomationTarget::parse (const juce::String& s)
{
    AutomationTarget t;
    auto parts = juce::StringArray::fromTokens (s, ":", "");
    if (parts.size() != 3)
        return t;

    if (parts[0] == "mixer")        t.kind = Kind::mixer;
    else if (parts[0] == "channel") t.kind = Kind::channel;

    t.index = parts[1].getIntValue();

    const auto& p = parts[2];
    if (p == "volume")         t.param = Param::volume;
    else if (p == "pan")       t.param = Param::pan;
    else if (p == "reverbMix") t.param = Param::reverbMix;
    else if (p == "delayMix")  t.param = Param::delayMix;
    else if (p == "cutoff")    t.param = Param::cutoff;
    else if (p == "eqLow")     t.param = Param::eqLow;
    else if (p == "eqHigh")    t.param = Param::eqHigh;
    return t;
}

juce::String AutomationTarget::toString() const
{
    return juce::String (kind == Kind::mixer ? "mixer" : "channel") + ":" + juce::String (index) + ":"
           + paramId (param).toString();
}

juce::Identifier AutomationTarget::paramId (Param p)
{
    switch (p)
    {
        case Param::volume:    return ids::volume;
        case Param::pan:       return ids::pan;
        case Param::reverbMix: return ids::reverbMix;
        case Param::delayMix:  return ids::delayMix;
        case Param::cutoff:    return ids::cutoff;
        case Param::eqLow:     return ids::eqLow;
        case Param::eqHigh:    return ids::eqHigh;
        case Param::invalid:   break;
    }
    return ids::volume;
}

juce::String AutomationTarget::paramName (Param p)
{
    switch (p)
    {
        case Param::volume:    return "Volumen";
        case Param::pan:       return "Panorama";
        case Param::reverbMix: return "Reverb";
        case Param::delayMix:  return "Delay";
        case Param::cutoff:    return "Filtro";
        case Param::eqLow:     return "Graves (EQ)";
        case Param::eqHigh:    return "Agudos (EQ)";
        case Param::invalid:   break;
    }
    return {};
}

float AutomationTarget::fromNormalised (Param p, float v)
{
    v = juce::jlimit (0.0f, 1.0f, v);
    switch (p)
    {
        case Param::pan:    return v * 2.0f - 1.0f;
        case Param::cutoff: return 20.0f * std::pow (1000.0f, v);
        case Param::eqLow:
        case Param::eqHigh: return v * 24.0f - 12.0f;
        default:            return v;
    }
}

float AutomationTarget::toNormalised (Param p, float x)
{
    switch (p)
    {
        case Param::pan:    return juce::jlimit (0.0f, 1.0f, (x + 1.0f) * 0.5f);
        case Param::cutoff: return juce::jlimit (0.0f, 1.0f, std::log (juce::jmax (20.0f, x) / 20.0f) / std::log (1000.0f));
        case Param::eqLow:
        case Param::eqHigh: return juce::jlimit (0.0f, 1.0f, (x + 12.0f) / 24.0f);
        default:            return juce::jlimit (0.0f, 1.0f, x);
    }
}

// ============================================================================
//  Proyecto
// ============================================================================
namespace
{
    const juce::uint32 patternColours[] = { 0xffff8a1f, 0xff4fc3f7, 0xff9ccc65, 0xffba68c8,
                                            0xffffd54f, 0xffef5350, 0xff4db6ac, 0xfff06292 };
}

Project::Project()
{
    resetToDefault();
}

Project::~Project()
{
    state.removeListener (this);
}

juce::String Project::getDisplayName() const
{
    return file == juce::File() ? juce::String::fromUTF8 ("Proyecto sin título")
                                : file.getFileNameWithoutExtension();
}

void Project::setState (juce::ValueTree newState)
{
    state.removeListener (this);
    state = newState;
    ensureStructure();
    state.addListener (this);
    undoManager.clearUndoHistory();
    dirty = false;
    ++version;
    ++generation;
    sendChangeMessage();
}

juce::ValueTree Project::createMixerTrack (int index, const juce::String& name)
{
    juce::ValueTree t (ids::TRACK);
    t.setProperty (ids::index, index, nullptr);
    t.setProperty (ids::name, name, nullptr);
    t.setProperty (ids::volume, 0.8f, nullptr);
    t.setProperty (ids::pan, 0.0f, nullptr);
    t.setProperty (ids::muted, false, nullptr);
    t.setProperty (ids::solo, false, nullptr);
    t.setProperty (ids::eqLow, 0.0f, nullptr);
    t.setProperty (ids::eqMid, 0.0f, nullptr);
    t.setProperty (ids::eqHigh, 0.0f, nullptr);
    t.setProperty (ids::compOn, false, nullptr);
    t.setProperty (ids::compThreshold, -18.0f, nullptr);
    t.setProperty (ids::compRatio, 4.0f, nullptr);
    t.setProperty (ids::delayMix, 0.0f, nullptr);
    t.setProperty (ids::delayTime, 0.75f, nullptr);
    t.setProperty (ids::delayFeedback, 0.35f, nullptr);
    t.setProperty (ids::reverbMix, 0.0f, nullptr);
    t.setProperty (ids::reverbSize, 0.6f, nullptr);
    return t;
}

void Project::ensureStructure()
{
    for (auto id : { ids::CHANNELS, ids::PATTERNS, ids::PLAYLIST, ids::MIXER, ids::AUTOMATION })
        if (! state.getChildWithName (id).isValid())
            state.appendChild (juce::ValueTree (id), nullptr);

    if (! state.hasProperty (ids::bpm))      state.setProperty (ids::bpm, 140.0, nullptr);
    if (! state.hasProperty (ids::nextUid))  state.setProperty (ids::nextUid, 1, nullptr);
    if (! state.hasProperty (ids::playMode)) state.setProperty (ids::playMode, "pattern", nullptr);

    auto mix = mixer();
    for (int i = mix.getNumChildren(); i < numMixerTracks; ++i)
        mix.appendChild (createMixerTrack (i, i == 0 ? juce::String ("Master") : "Pista " + juce::String (i)), nullptr);

    if (patterns().getNumChildren() == 0)
    {
        juce::ValueTree p (ids::PATTERN);
        p.setProperty (ids::uid, newUid(), nullptr);
        p.setProperty (ids::name, juce::String::fromUTF8 ("Patrón 1"), nullptr);
        p.setProperty (ids::length, 16, nullptr);
        p.setProperty (ids::colour, (juce::int64) patternColours[0], nullptr);
        patterns().appendChild (p, nullptr);
    }

    if (! findPattern ((int) state[ids::currentPattern]).isValid())
        state.setProperty (ids::currentPattern, patterns().getChild (0)[ids::uid], nullptr);

    if (! findChannel ((int) state[ids::selectedChannel]).isValid() && channels().getNumChildren() > 0)
        state.setProperty (ids::selectedChannel, channels().getChild (0)[ids::uid], nullptr);
}

void Project::resetToDefault()
{
    state.removeListener (this);
    state = juce::ValueTree (ids::PROJECT);
    state.setProperty (ids::version, 1, nullptr);
    state.setProperty (ids::bpm, 140.0, nullptr);
    state.setProperty (ids::metronome, false, nullptr);
    state.setProperty (ids::playMode, "pattern", nullptr);
    state.setProperty (ids::nextUid, 1, nullptr);
    ensureStructure();

    // Kit de batería incluido + 808 + sintetizador, cada uno en su pista del mezclador
    struct Def { const char* name; const char* sample; int mixer; };
    const Def kit[] = { { "Bombo", "builtin:kick", 1 }, { "Caja", "builtin:snare", 2 },
                        { "Hi-hat", "builtin:hihat", 3 }, { "Palmada", "builtin:clap", 4 } };

    for (auto& d : kit)
    {
        auto ch = addChannel (ChannelType::sampler, d.name, d.mixer);
        ch.setProperty (ids::sample, d.sample, nullptr);
        mixer().getChild (d.mixer).setProperty (ids::name, d.name, nullptr);
    }

    addChannel (ChannelType::bass808, "808", 5);
    mixer().getChild (5).setProperty (ids::name, "808", nullptr);
    addChannel (ChannelType::synth, "Sintetizador", 6);
    mixer().getChild (6).setProperty (ids::name, "Sinte", nullptr);

    // La canción empieza con el patrón 1 colocado al principio de la playlist
    auto pattern = patterns().getChild (0);
    addClip ("pattern", (int) pattern[ids::uid], 0, 0.0, 16.0);

    setState (state);
}

bool Project::loadFromXmlString (const juce::String& xmlText, juce::String& error)
{
    auto xml = juce::parseXML (xmlText);
    if (xml == nullptr)
    {
        error = juce::String::fromUTF8 ("El archivo no es un proyecto válido de Studoo.");
        return false;
    }

    auto tree = juce::ValueTree::fromXml (*xml);
    if (! tree.hasType (ids::PROJECT))
    {
        error = juce::String::fromUTF8 ("El archivo no es un proyecto de Studoo.");
        return false;
    }

    setState (tree);
    return true;
}

bool Project::loadFromFile (const juce::File& f, juce::String& error)
{
    if (! f.existsAsFile())
    {
        error = juce::String::fromUTF8 ("No se encuentra el archivo.");
        return false;
    }

    if (! loadFromXmlString (f.loadFileAsString(), error))
        return false;

    file = f;

    // Si un sample no está en su ruta original, lo buscamos junto al proyecto
    for (auto ch : channels())
    {
        const auto path = ch[ids::sample].toString();
        if (path.isEmpty() || path.startsWith ("builtin:") || juce::File::isAbsolutePath (path) == false)
            continue;
        juce::File sampleFile (path);
        if (! sampleFile.existsAsFile())
        {
            auto local = f.getParentDirectory().getChildFile (sampleFile.getFileName());
            auto inSamples = f.getParentDirectory().getChildFile ("Samples").getChildFile (sampleFile.getFileName());
            if (local.existsAsFile())          ch.setProperty (ids::sample, local.getFullPathName(), nullptr);
            else if (inSamples.existsAsFile()) ch.setProperty (ids::sample, inSamples.getFullPathName(), nullptr);
        }
    }
    dirty = false;
    return true;
}

juce::String Project::toXmlString() const
{
    if (auto xml = state.createXml())
        return xml->toString();
    return {};
}

bool Project::saveToFile (const juce::File& f)
{
    auto text = toXmlString();
    if (text.isEmpty())
        return false;

    // Guardado seguro: primero en un archivo temporal y luego se reemplaza
    juce::TemporaryFile temp (f);
    if (! temp.getFile().replaceWithText (text) || ! temp.overwriteTargetFileWithTemporary())
        return false;

    file = f;
    dirty = false;
    return true;
}

int Project::newUid()
{
    const int uid = juce::jmax (1, (int) state[ids::nextUid]);
    state.setProperty (ids::nextUid, uid + 1, nullptr);
    return uid;
}

// ----------------------------------------------------------------------------
//  Cambios
// ----------------------------------------------------------------------------
void Project::changed (bool affectsSavedData)
{
    if (affectsSavedData)
        dirty = true;
    ++version;
    sendChangeMessage();
}

void Project::valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier& id)
{
    const bool uiOnly = (id == ids::selectedChannel || id == ids::selectedLane || id == ids::nextUid);
    changed (! uiOnly);
}

// ----------------------------------------------------------------------------
//  Canales
// ----------------------------------------------------------------------------
juce::ValueTree Project::addChannel (ChannelType type, const juce::String& name, int mixerTrack)
{
    juce::ValueTree ch (ids::CHANNEL);
    ch.setProperty (ids::uid, newUid(), nullptr);
    ch.setProperty (ids::name, name, nullptr);
    ch.setProperty (ids::type, channelTypeToString (type), nullptr);
    ch.setProperty (ids::volume, 0.8f, nullptr);
    ch.setProperty (ids::pan, 0.0f, nullptr);
    ch.setProperty (ids::muted, false, nullptr);
    ch.setProperty (ids::mixer, mixerTrack, nullptr);
    ch.setProperty (ids::root, 60, nullptr);

    switch (type)
    {
        case ChannelType::sampler:
            ch.setProperty (ids::sample, "", nullptr);
            ch.setProperty (ids::oneShot, true, nullptr);
            break;
        case ChannelType::synth:
            ch.setProperty (ids::wave, 0, nullptr);
            ch.setProperty (ids::attack, 0.005f, nullptr);
            ch.setProperty (ids::decay, 0.3f, nullptr);
            ch.setProperty (ids::sustain, 0.6f, nullptr);
            ch.setProperty (ids::release, 0.3f, nullptr);
            ch.setProperty (ids::cutoff, 5000.0f, nullptr);
            ch.setProperty (ids::resonance, 0.2f, nullptr);
            ch.setProperty (ids::detune, 8.0f, nullptr);
            break;
        case ChannelType::bass808:
            ch.setProperty (ids::root, 36, nullptr);
            ch.setProperty (ids::decay, 1.5f, nullptr);
            ch.setProperty (ids::glide, 0.08f, nullptr);
            ch.setProperty (ids::drive, 0.3f, nullptr);
            break;
        case ChannelType::plugin:
            ch.setProperty (ids::pluginDesc, "", nullptr);
            ch.setProperty (ids::pluginState, "", nullptr);
            break;
        case ChannelType::audio:
            ch.setProperty (ids::sample, "", nullptr);
            break;
    }

    channels().appendChild (ch, &undoManager);
    return ch;
}

void Project::removeChannel (int uid)
{
    auto ch = findChannel (uid);
    if (! ch.isValid())
        return;

    // Quitamos también sus pasos, notas, clips y automatizaciones
    for (auto pattern : patterns())
        for (int i = pattern.getNumChildren(); --i >= 0;)
            if ((int) pattern.getChild (i)[ids::channel] == uid)
                pattern.removeChild (i, &undoManager);

    auto pl = playlist();
    for (int i = pl.getNumChildren(); --i >= 0;)
        if (pl.getChild (i)[ids::kind].toString() == "audio" && (int) pl.getChild (i)[ids::ref] == uid)
            pl.removeChild (i, &undoManager);

    auto au = automation();
    for (int i = au.getNumChildren(); --i >= 0;)
    {
        auto t = AutomationTarget::parse (au.getChild (i)[ids::target].toString());
        if (t.kind == AutomationTarget::Kind::channel && t.index == uid)
            au.removeChild (i, &undoManager);
    }

    channels().removeChild (ch, &undoManager);

    if (getSelectedChannel() == uid && channels().getNumChildren() > 0)
        setSelectedChannel ((int) channels().getChild (0)[ids::uid]);
}

juce::ValueTree Project::findChannel (int uid)
{
    return channels().getChildWithProperty (ids::uid, uid);
}

int Project::getChannelIndex (int uid)
{
    return channels().indexOf (findChannel (uid));
}

int Project::findFreeMixerTrack()
{
    for (int t = 1; t < numMixerTracks; ++t)
    {
        bool used = false;
        for (auto ch : channels())
            used = used || (int) ch[ids::mixer] == t;
        if (! used)
            return t;
    }
    return 0;
}

// ----------------------------------------------------------------------------
//  Kits incluidos
// ----------------------------------------------------------------------------
juce::ValueTree Project::addKitSound (const kits::KitInfo& kit, const kits::SoundInfo& sound)
{
    const int track = findFreeMixerTrack();
    const auto name = juce::String::fromUTF8 (sound.name);
    auto ch = addChannel (ChannelType::sampler, name, track);
    ch.setProperty (ids::sample, kits::samplePath (kit, sound), &undoManager);
    if (track > 0)
        getMixerTrack (track).setProperty (ids::name, name.substring (0, 12), &undoManager);
    return ch;
}

static void apply808Preset (juce::ValueTree ch, const kits::Bass808Preset& p, juce::UndoManager* um)
{
    ch.setProperty (ids::decay, p.decay, um);
    ch.setProperty (ids::glide, p.glide, um);
    ch.setProperty (ids::drive, p.drive, um);
}

void Project::addKit (const kits::KitInfo& kit)
{
    int last = 0;
    for (auto& sound : kit.sounds)
        last = addKitSound (kit, sound)[ids::uid];

    if (kit.bass.present)
    {
        auto bass = addChannel (ChannelType::bass808, "808 " + juce::String::fromUTF8 (kit.name).upToFirstOccurrenceOf (" ", false, false),
                                findFreeMixerTrack());
        apply808Preset (bass, kit.bass, &undoManager);
    }

    if (last != 0)
        setSelectedChannel (last);
}

void Project::useKit (const kits::KitInfo& kit)
{
    // 1) Los canales de batería incluida que ya tienes cambian de sonido (los pasos se quedan)
    juce::StringArray covered;
    for (auto ch : channels())
    {
        if (channelTypeFromString (ch[ids::type].toString()) != ChannelType::sampler)
            continue;
        const auto role = kits::roleOf (ch[ids::sample].toString());
        if (role.isEmpty() || covered.contains (role))
            continue;

        for (auto& sound : kit.sounds)
            if (role == sound.id)
            {
                ch.setProperty (ids::sample, kits::samplePath (kit, sound), &undoManager);
                ch.setProperty (ids::name, juce::String::fromUTF8 (sound.name), &undoManager);
                covered.add (role);
                break;
            }
    }

    // 2) Los sonidos del kit que no tenías se añaden como canales nuevos
    for (auto& sound : kit.sounds)
        if (! covered.contains (sound.id))
            addKitSound (kit, sound);

    // 3) El 808 se ajusta al estilo del kit
    if (kit.bass.present)
    {
        auto bass = findChannelByType (ChannelType::bass808);
        if (! bass.isValid())
            bass = addChannel (ChannelType::bass808, "808", findFreeMixerTrack());
        apply808Preset (bass, kit.bass, &undoManager);
    }
}

int Project::getSelectedChannel()
{
    return (int) state[ids::selectedChannel];
}

void Project::setSelectedChannel (int uid)
{
    state.setProperty (ids::selectedChannel, uid, nullptr);
}

juce::ValueTree Project::findChannelByBuiltin (const juce::String& builtinName)
{
    return channels().getChildWithProperty (ids::sample, builtinName);
}

juce::ValueTree Project::findChannelByType (ChannelType t)
{
    return channels().getChildWithProperty (ids::type, channelTypeToString (t));
}

// ----------------------------------------------------------------------------
//  Patrones
// ----------------------------------------------------------------------------
juce::ValueTree Project::addPattern (const juce::String& name)
{
    const int n = patterns().getNumChildren();
    juce::ValueTree p (ids::PATTERN);
    p.setProperty (ids::uid, newUid(), nullptr);
    p.setProperty (ids::name, name.isNotEmpty() ? name : juce::String::fromUTF8 ("Patrón ") + juce::String (n + 1), nullptr);
    p.setProperty (ids::length, 16, nullptr);
    p.setProperty (ids::colour, (juce::int64) patternColours[(size_t) n % 8], nullptr);
    patterns().appendChild (p, &undoManager);
    return p;
}

juce::ValueTree Project::duplicatePattern (int uid)
{
    auto source = findPattern (uid);
    if (! source.isValid())
        return {};

    auto copy = source.createCopy();
    copy.setProperty (ids::uid, newUid(), nullptr);
    copy.setProperty (ids::name, source[ids::name].toString() + " (copia)", nullptr);
    patterns().appendChild (copy, &undoManager);
    return copy;
}

void Project::removePattern (int uid)
{
    if (patterns().getNumChildren() <= 1)
        return;

    auto p = findPattern (uid);
    if (! p.isValid())
        return;

    auto pl = playlist();
    for (int i = pl.getNumChildren(); --i >= 0;)
        if (pl.getChild (i)[ids::kind].toString() == "pattern" && (int) pl.getChild (i)[ids::ref] == uid)
            pl.removeChild (i, &undoManager);

    patterns().removeChild (p, &undoManager);

    if (getCurrentPatternUid() == uid)
        setCurrentPattern ((int) patterns().getChild (0)[ids::uid]);
}

juce::ValueTree Project::findPattern (int uid)
{
    return patterns().getChildWithProperty (ids::uid, uid);
}

juce::ValueTree Project::getCurrentPattern()
{
    auto p = findPattern (getCurrentPatternUid());
    return p.isValid() ? p : patterns().getChild (0);
}

int Project::getCurrentPatternUid()
{
    return (int) state[ids::currentPattern];
}

void Project::setCurrentPattern (int uid)
{
    state.setProperty (ids::currentPattern, uid, nullptr);
}

int Project::getPatternLength (const juce::ValueTree& pattern)
{
    return juce::jlimit (4, 256, (int) pattern.getProperty (ids::length, 16));
}

juce::ValueTree Project::getStepSet (juce::ValueTree pattern, int channelUid, bool create)
{
    for (auto c : pattern)
        if (c.hasType (ids::STEPSET) && (int) c[ids::channel] == channelUid)
            return c;

    if (! create)
        return {};

    juce::ValueTree s (ids::STEPSET);
    s.setProperty (ids::channel, channelUid, nullptr);
    s.setProperty (ids::data, "", nullptr);
    pattern.appendChild (s, &undoManager);
    return s;
}

bool Project::getStep (const juce::ValueTree& pattern, int channelUid, int step)
{
    for (auto c : pattern)
        if (c.hasType (ids::STEPSET) && (int) c[ids::channel] == channelUid)
        {
            const auto data = c[ids::data].toString();
            return step < data.length() && data[step] == '1';
        }
    return false;
}

void Project::setStep (juce::ValueTree pattern, int channelUid, int step, bool on)
{
    if (! pattern.isValid() || step < 0 || step >= 256)
        return;

    auto set = getStepSet (pattern, channelUid, true);
    auto data = set[ids::data].toString();
    while (data.length() <= step)
        data << '0';

    if ((data[step] == '1') == on)
        return;

    data = data.substring (0, step) + (on ? "1" : "0") + data.substring (step + 1);
    set.setProperty (ids::data, data, &undoManager);
}

void Project::clearPattern (juce::ValueTree pattern)
{
    for (int i = pattern.getNumChildren(); --i >= 0;)
        pattern.removeChild (i, &undoManager);
}

void Project::loadExampleBeat (juce::ValueTree pattern)
{
    clearPattern (pattern);
    pattern.setProperty (ids::length, 16, &undoManager);

    auto setSteps = [&] (const juce::ValueTree& ch, std::initializer_list<int> steps)
    {
        if (ch.isValid())
            for (int s : steps)
                setStep (pattern, (int) ch[ids::uid], s, true);
    };

    setSteps (findChannelByBuiltin ("builtin:kick"),  { 0, 6, 8, 11 });
    setSteps (findChannelByBuiltin ("builtin:snare"), { 4, 12 });
    setSteps (findChannelByBuiltin ("builtin:hihat"), { 0, 2, 4, 6, 8, 10, 12, 14, 15 });
    setSteps (findChannelByBuiltin ("builtin:clap"),  { 12 });

    // Una línea de 808 en Do menor que acompaña al bombo
    auto bass = findChannelByType (ChannelType::bass808);
    if (bass.isValid())
    {
        const int uid = bass[ids::uid];
        addNote (pattern, uid, 0.0, 6.0, 36);
        addNote (pattern, uid, 6.0, 2.0, 36);
        addNote (pattern, uid, 8.0, 3.0, 39);
        addNote (pattern, uid, 11.0, 5.0, 34);
    }
}

juce::ValueTree Project::addNote (juce::ValueTree pattern, int channelUid, double start,
                                  double length, int pitch, float velocity)
{
    juce::ValueTree n (ids::NOTE);
    n.setProperty (ids::channel, channelUid, nullptr);
    n.setProperty (ids::start, start, nullptr);
    n.setProperty (ids::length, length, nullptr);
    n.setProperty (ids::pitch, juce::jlimit (0, 127, pitch), nullptr);
    n.setProperty (ids::velocity, velocity, nullptr);
    pattern.appendChild (n, &undoManager);
    return n;
}

bool Project::patternHasNotes (const juce::ValueTree& pattern, int channelUid)
{
    for (auto c : pattern)
        if (c.hasType (ids::NOTE) && (int) c[ids::channel] == channelUid)
            return true;
    return false;
}

// ----------------------------------------------------------------------------
//  Playlist
// ----------------------------------------------------------------------------
juce::ValueTree Project::addClip (const juce::String& kind, int ref, int track,
                                  double start, double length, double offset)
{
    juce::ValueTree c (ids::CLIP);
    c.setProperty (ids::uid, newUid(), nullptr);
    c.setProperty (ids::kind, kind, nullptr);
    c.setProperty (ids::ref, ref, nullptr);
    c.setProperty (ids::track, juce::jlimit (0, numPlaylistTracks - 1, track), nullptr);
    c.setProperty (ids::start, juce::jmax (0.0, start), nullptr);
    c.setProperty (ids::length, juce::jmax (1.0, length), nullptr);
    c.setProperty (ids::offset, offset, nullptr);
    playlist().appendChild (c, &undoManager);
    return c;
}

double Project::getSongLengthSteps()
{
    double end = 0.0;
    for (auto c : playlist())
        end = juce::jmax (end, (double) c[ids::start] + (double) c[ids::length]);

    // Redondeamos al compás siguiente
    return juce::jmax ((double) stepsPerBar, std::ceil (end / stepsPerBar) * stepsPerBar);
}

// ----------------------------------------------------------------------------
//  Mezclador y automatización
// ----------------------------------------------------------------------------
juce::ValueTree Project::getMixerTrack (int index)
{
    return mixer().getChild (index);
}

juce::ValueTree Project::addAutomationLane (const AutomationTarget& target)
{
    juce::ValueTree lane (ids::LANE);
    lane.setProperty (ids::uid, newUid(), nullptr);
    lane.setProperty (ids::target, target.toString(), nullptr);
    lane.setProperty (ids::name, describeTarget (target), nullptr);

    // Empieza con dos puntos al valor actual del parámetro
    float current = 0.8f;
    juce::ValueTree owner = target.kind == AutomationTarget::Kind::mixer ? getMixerTrack (target.index)
                                                                           : findChannel (target.index);
    if (owner.isValid())
        current = (float) owner.getProperty (AutomationTarget::paramId (target.param), current);

    const float norm = AutomationTarget::toNormalised (target.param, current);
    for (double t : { 0.0, getSongLengthSteps() })
    {
        juce::ValueTree p (ids::POINT);
        p.setProperty (ids::time, t, nullptr);
        p.setProperty (ids::value, norm, nullptr);
        lane.appendChild (p, nullptr);
    }

    automation().appendChild (lane, &undoManager);
    state.setProperty (ids::selectedLane, lane[ids::uid], nullptr);
    return lane;
}

juce::String Project::describeTarget (const AutomationTarget& t)
{
    juce::String owner;
    if (t.kind == AutomationTarget::Kind::mixer)
    {
        auto track = getMixerTrack (t.index);
        owner = t.index == 0 ? juce::String ("Master") : "Pista " + juce::String (t.index) + " (" + track[ids::name].toString() + ")";
    }
    else
    {
        owner = "Canal " + findChannel (t.index)[ids::name].toString();
    }
    return owner + juce::String::fromUTF8 (" · ") + AutomationTarget::paramName (t.param);
}
}
