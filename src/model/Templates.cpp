#include "Templates.h"
#include "GenreBeats.h"

namespace studoo::templates
{
const std::vector<TemplateInfo>& all()
{
    static const std::vector<TemplateInfo> list = {
        { "vacia",     "Canción vacía con estructura", "Sin sonidos: solo los marcadores (intro, verso, estribillo...) y las pistas con nombre.", "" },
        { "trap",      "Trap",      "Base de trap a 140 BPM, 808 con sidechain y sitio para la voz.", "trap" },
        { "reggaeton", "Reggaetón", "Dembow a 95 BPM con bajo y acordes, lista para cantar encima.", "reggaeton" },
        { "house",     "House",     "Bombo en cada tiempo a 124 BPM, con intro y drop.", "house" },
        { "lofi",      "Lo-fi",     "Lo-fi relajado a 80 BPM, con un poco de reverb.", "lofi" },
        { "drill",     "Drill",     "Drill a 142 BPM con 808 deslizante.", "drill" },
        { "pop",       "Pop",       "Pop a 110 BPM con verso, pre-estribillo y estribillo.", "pop" },
        { "voz",       "Grabar mi voz", "Una base de trap sencilla y la pista de la voz preparada (puerta, compresor y reverb).", "trap" },
    };
    return list;
}

bool apply (Project& project, const juce::String& id)
{
    const TemplateInfo* info = nullptr;
    for (auto& t : all())
        if (id == t.id)
            info = &t;
    if (info == nullptr)
        return false;

    project.resetToDefault();
    auto& state = project.getState();
    auto* um = (juce::UndoManager*) nullptr;   // una plantilla es un proyecto nuevo: no se deshace
    const bool hasBeat = juce::String (info->genre).isNotEmpty();
    const int bar = project.getStepsPerBar();

    auto base = project.getCurrentPattern();
    base.setProperty (ids::name, "Beat", um);
    if (hasBeat)
        genres::load (project, base, info->genre);
    else
        project.clearPattern (base);

    // Quita los canales que no suenan en la base (los del kit por defecto que el género no usa)
    if (hasBeat)
    {
        juce::Array<int> unused;
        for (auto ch : project.channels())
        {
            const int uid = ch[ids::uid];
            bool used = Project::patternHasNotes (base, uid);
            for (int s = 0; s < Project::getPatternLength (base) && ! used; ++s)
                used = project.getStep (base, uid, s);
            if (! used)
                unused.add (uid);
        }
        if (unused.size() < project.channels().getNumChildren())
            for (int uid : unused)
                project.removeChannel (uid);
    }

    // Intro: la misma base sin la batería (solo bajo y acordes, si los hay)
    juce::ValueTree intro;
    if (hasBeat)
    {
        intro = project.duplicatePattern (base[ids::uid]);
        intro.setProperty (ids::name, "Intro", um);
        intro.setProperty (ids::colour, (juce::int64) 0xff4fc3f7, um);
        for (int i = intro.getNumChildren(); --i >= 0;)
            if (intro.getChild (i).hasType (ids::STEPSET))
                intro.removeChild (i, um);
        bool hasNotes = false;
        for (auto c : intro)
            hasNotes = hasNotes || c.hasType (ids::NOTE);
        if (! hasNotes)
        {
            project.removePattern (intro[ids::uid]);
            intro = {};
        }
    }

    // Estructura: Intro (4) · Verso (8) · Estribillo (8) · Verso (8) · Estribillo (8) · Final (4)
    struct Part { const char* name; int bars; bool full; };
    const std::vector<Part> parts = juce::String (info->id) == "house"
        ? std::vector<Part> { { "Intro", 8, false }, { "Drop", 16, true }, { "Puente", 8, false }, { "Drop", 16, true }, { "Final", 4, false } }
        : juce::String (info->id) == "pop"
            ? std::vector<Part> { { "Intro", 4, false }, { "Verso", 8, true }, { "Pre-estribillo", 4, true },
                                  { "Estribillo", 8, true }, { "Verso", 8, true }, { "Estribillo", 8, true }, { "Final", 4, false } }
            : std::vector<Part> { { "Intro", 4, false }, { "Verso", 8, true }, { "Estribillo", 8, true },
                                  { "Verso", 8, true }, { "Estribillo", 8, true }, { "Final", 4, false } };

    auto playlist = project.playlist();
    playlist.removeAllChildren (um);
    auto markers = state.getOrCreateChildWithName (ids::MARKERS, um);
    markers.removeAllChildren (um);
    const double patternLength = Project::getPatternLength (base);
    double at = 0.0;
    for (auto& part : parts)
    {
        juce::ValueTree m (ids::MARKER);
        m.setProperty (ids::time, at, um);
        m.setProperty (ids::name, juce::String::fromUTF8 (part.name), um);
        markers.appendChild (m, um);
        const double len = part.bars * bar;
        if (hasBeat)
        {
            auto pattern = (! part.full && intro.isValid()) ? intro : base;
            // Bloques del tamaño del patrón, uno detrás de otro
            for (double t = 0.0; t < len - 1.0e-6; t += patternLength)
                project.addClip ("pattern", pattern[ids::uid], 0, at + t, juce::jmin (patternLength, len - t));
        }
        at += len;
    }

    playlist.setProperty ("tname0", "Beat", um);
    playlist.setProperty ("tname1", "Voz", um);
    playlist.setProperty ("tname2", "Coros", um);
    playlist.setProperty ("tname3", "Efectos", um);
    playlist.setProperty ("tgroup1", "Voces", um);
    playlist.setProperty ("tgroup2", "Voces", um);

    // Nombres del mezclador: cada pista se llama como su instrumento (las libres, "Pista N")
    for (int t = 1; t < numMixerTracks; ++t)
    {
        juce::String name = "Pista " + juce::String (t);
        for (auto ch : project.channels())
            if ((int) ch[ids::mixer] == t)
            {
                name = ch[ids::name].toString();
                break;
            }
        project.getMixerTrack (t).setProperty (ids::name, name, um);
    }

    // Mezcla inicial: la voz va a la primera pista libre
    int lastUsed = 0;
    for (auto ch : project.channels())
        lastUsed = juce::jmax (lastUsed, (int) ch[ids::mixer]);
    const int voiceTrack = lastUsed + 1 < numMixerTracks ? lastUsed + 1 : juce::jmax (1, project.findFreeMixerTrack());
    auto voice = project.getMixerTrack (voiceTrack);
    voice.setProperty (ids::name, "Voz", um);
    voice.setProperty (ids::reverbMix, 0.18f, um);
    voice.setProperty (ids::compOn, true, um);
    if (juce::String (info->id) == "voz")
    {
        voice.setProperty (ids::gateThreshold, -50.0f, um);
        voice.setProperty (ids::compThreshold, -20.0f, um);
        voice.setProperty (ids::tuneAmount, 0.3f, um);
    }
    auto master = project.getMixerTrack (0);
    master.setProperty (ids::maximize, hasBeat ? 3.0f : 0.0f, um);
    if (juce::String (info->id) == "lofi")
        for (auto ch : project.channels())
            if (channelTypeFromString (ch[ids::type].toString()) == ChannelType::synth)
                project.getMixerTrack ((int) ch[ids::mixer]).setProperty (ids::reverbMix, 0.3f, um);
    // El bajo se agacha cuando suena el bombo (sidechain)
    juce::ValueTree kick;
    for (auto ch : project.channels())
    {
        const auto sample = ch[ids::sample].toString();
        if (! kick.isValid() && sample.startsWith ("builtin:") && (sample.endsWith (":kick") || sample.endsWith ("/kick")))
            kick = ch;
    }
    for (auto ch : project.channels())
        if (channelTypeFromString (ch[ids::type].toString()) == ChannelType::bass808 && kick.isValid() && (int) ch[ids::mixer] > 0
            && (int) ch[ids::mixer] != (int) kick[ids::mixer] && (int) kick[ids::mixer] > 0)
        {
            auto t = project.getMixerTrack ((int) ch[ids::mixer]);
            t.setProperty (ids::sidechain, 0.35f, um);
            t.setProperty (ids::sidechainSource, (int) kick[ids::mixer], um);
        }

    state.setProperty (ids::playMode, hasBeat ? "song" : "pattern", um);
    project.setCurrentPattern (base[ids::uid]);
    project.getUndo().clearUndoHistory();
    return true;
}
}
