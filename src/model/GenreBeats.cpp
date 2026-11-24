#include "GenreBeats.h"

namespace studoo::genres
{
namespace
{
    // Cómo se escriben las bases (16 letras = 1 compás; con 64 letras se
    // escriben los 4 compases enteros, para hacer variaciones):
    //   Batería:  'x' = golpe, '.' = nada
    //   Bajo:     'x' = nota del acorde (la raíz), 'o' = la misma una octava
    //             más aguda, '5' = la quinta, '-' = mantener la nota, '.' = nada
    //   Acordes:  'x' = tocar el acorde, '-' = mantenerlo, '.' = nada
    struct DrumLine { const char* sound; const char* steps; };

    enum class BassKind { none, synth, bass808 };

    struct Recipe
    {
        GenreInfo info;
        const char* kit;
        std::vector<DrumLine> drums;

        BassKind bassKind;
        const char* bassPreset;   // nombre del preset de sinte o de 808
        const char* bassName;     // nombre del canal
        const char* bassLine;
        int bassRoots[4];         // nota grave de cada compás

        const char* chordPreset;  // nullptr = sin acordes
        const char* chordName;
        const char* chordRhythm;
        int chords[4][4];         // notas de cada compás (0 = ninguna)
    };

    const std::vector<Recipe>& recipes()
    {
        static const std::vector<Recipe> list = {
            // --- Trap: 140 BPM a medio tiempo, hi-hats con rolls, 808 largo ------------------
            { { "trap", "Trap", 140.0 }, "trap",
              { { "kick",    "x......x..x....." },
                { "clap",    "........x......." },
                { "hihat",   "x.x.x.x.x.x.x.x.x.x.x.x.x.x.xxxxx.x.x.x.x.x.x.x.x.x.x.x.xxx.xxxx" },
                { "openhat", "..............x." } },
              BassKind::bass808, "808 clásico", "808", "x------x--x-----", { 36, 32, 39, 34 },
              "Campanas", "Campanas trap", "x-------x-------",
              { { 60, 63, 67, 0 }, { 60, 63, 68, 0 }, { 58, 63, 67, 0 }, { 58, 62, 65, 0 } } },

            // --- Reggaetón: el dembow a 95 BPM ---------------------------------------------
            { { "reggaeton", "Reggaetón", 95.0 }, "reggaeton",
              { { "kick",   "x...x...x...x..." },
                { "snare",  "...x..x....x..x." },
                { "hihat",  "x.x.x.x.x.x.x.x." },
                { "shaker", "..x...x...x...x." },
                { "perc",   "...............x...........x.......x...........x...........x..xx" } },
              BassKind::bass808, "808 corto (punch)", "808 dembow", "x..x..x.x..x..x.", { 45, 41, 36, 43 },
              "Pluck", "Pluck reggaetón", "x..x..x...x..x..",
              { { 57, 60, 64, 69 }, { 57, 60, 65, 69 }, { 55, 60, 64, 67 }, { 55, 59, 62, 67 } } },

            // --- House: four on the floor a 124 BPM ----------------------------------------
            { { "house", "House", 124.0 }, "house",
              { { "kick",    "x...x...x...x..." },
                { "clap",    "....x.......x..." },
                { "openhat", "..x...x...x...x." },
                { "hihat2",  "x...x...x...x..x" },
                { "shaker",  ".x.x.x.x.x.x.x.x" } },
              BassKind::synth, "Bajo sinte", "Bajo house", "..x-..x-..x-..o-", { 45, 38, 41, 40 },
              "Piano eléctrico", "Piano house", "x-.x-.x-..x-.x-.",
              { { 57, 60, 64, 67 }, { 57, 60, 62, 65 }, { 57, 60, 64, 65 }, { 55, 59, 62, 64 } } },

            // --- Boom bap: hip-hop de los 90 a 90 BPM ----------------------------------------
            { { "boombap", "Boom bap (hip-hop)", 90.0 }, "boombap",
              { { "kick",    "x......x..x....." },
                { "snare",   "....x.......x..." },
                { "hihat",   "x.x.x.x.x.x.x.x." },
                { "openhat", "..............x." } },
              BassKind::synth, "Bajo sinte", "Bajo boom bap", "x-----x---x-----", { 38, 43, 36, 45 },
              "Piano eléctrico", "Piano jazz", "x-----..x---....",
              { { 57, 60, 62, 65 }, { 55, 59, 62, 65 }, { 55, 59, 60, 64 }, { 55, 57, 60, 64 } } },

            // --- Lo-fi: relajado a 80 BPM -------------------------------------------------------
            { { "lofi", "Lo-fi", 80.0 }, "lofi",
              { { "kick",   "x.......x.x....." },
                { "snare",  "....x.......x..." },
                { "hihat",  "x.x.x.x.x.x.x.x." },
                { "shaker", ".x.x.x.x.x.x.x.x" } },
              BassKind::synth, "Sub bajo", "Bajo lo-fi", "x-------x-----..", { 41, 40, 38, 36 },
              "Piano eléctrico", "Piano lo-fi", "x---------------",
              { { 57, 60, 64, 65 }, { 55, 59, 62, 64 }, { 57, 60, 62, 65 }, { 55, 59, 60, 64 } } },

            // --- Drill: oscuro, 142 BPM, hi-hats saltarines y 808 que se desliza ---------------
            { { "drill", "Drill", 142.0 }, "drill",
              { { "kick",    "x.........x....." },
                { "snare",   "........x.....x." },
                { "hihat",   "x..x..x.x..x..x." },
                { "openhat", "......x........." } },
              BassKind::bass808, "808 con mucho glide", "808 drill", "x-----x-x-----x-", { 41, 37, 39, 36 },
              "Cuerdas", "Cuerdas drill", "x---------------",
              { { 60, 65, 68, 0 }, { 61, 65, 68, 0 }, { 58, 63, 67, 0 }, { 60, 64, 67, 0 } } },

            // --- Techno: 130 BPM, bombo en cada tiempo y bajo que rueda ------------------------
            { { "techno", "Techno", 130.0 }, "techno",
              { { "kick",    "x...x...x...x..." },
                { "clap",    "....x.......x..." },
                { "openhat", "..x...x...x...x." },
                { "hihat2",  "xxxxxxxxxxxxxxxx" },
                { "perc",    "......x.......x." } },
              BassKind::synth, "Bajo ácido", "Bajo techno", ".xxx.xxx.xxx.xxx", { 45, 45, 41, 43 },
              "Ambiente", "Pad techno", "x---------------",
              { { 57, 60, 64, 0 }, { 57, 60, 64, 0 }, { 57, 60, 65, 0 }, { 55, 59, 62, 0 } } },

            // --- Afrobeat: 105 BPM, congas y shaker ---------------------------------------------
            { { "afrobeat", "Afrobeat", 105.0 }, "afrobeat",
              { { "kick",   "x.....x.x......." },
                { "rim",    "...x..x....x..x." },
                { "shaker", "x.xxx.xxx.xxx.xx" },
                { "conga",  ".....x.x.....x.x" },
                { "hihat",  "x...x...x...x..." } },
              BassKind::synth, "Sub bajo", "Bajo afro", "x-----x-x-------", { 36, 45, 41, 43 },
              "Pluck", "Pluck afro", "..x..x..x...x...",
              { { 55, 60, 64, 67 }, { 57, 60, 64, 69 }, { 57, 60, 65, 69 }, { 55, 59, 62, 67 } } },

            // --- Phonk: 130 BPM, cencerro y 808 distorsionado -----------------------------------
            { { "phonk", "Phonk", 130.0 }, "phonk",
              { { "kick",    "x.......x.x....." },
                { "snare",   "....x.......x..." },
                { "hihat",   "x.x.x.x.x.x.x.x." },
                { "cowbell", "x..x..x...x..x.." },
                { "openhat", "..............x." } },
              BassKind::bass808, "808 distorsionado", "808 phonk", "x-------x-x-----", { 37, 37, 40, 35 },
              nullptr, nullptr, nullptr, {} },

            // --- Pop: 110 BPM, los 4 acordes más famosos ---------------------------------------
            { { "pop", "Pop", 110.0 }, "pop",
              { { "kick",       "x.......x.x....." },
                { "clap",       "....x.......x..." },
                { "hihat",      "x.x.x.x.x.x.x.x." },
                { "tambourine", "....x.......x..." } },
              BassKind::synth, "Bajo sinte", "Bajo pop", "x-x-----x-x-----", { 36, 43, 45, 41 },
              "Piano eléctrico", "Piano pop", "x---x---x---x---",
              { { 55, 60, 64, 0 }, { 55, 59, 62, 0 }, { 57, 60, 64, 0 }, { 57, 60, 65, 0 } } },

            // --- R&B: lento (75 BPM) y suave ----------------------------------------------------
            { { "rnb", "R&B", 75.0 }, "rnb",
              { { "kick",  "x......x..x....." },
                { "snap",  "....x.......x..." },
                { "hihat", "x.x.x.xxx.x.x.x." },
                { "rim",   "..........x....." } },
              BassKind::bass808, "808 sub limpio", "808 R&B", "x-------x--x----", { 39, 38, 36, 34 },
              "Pad cálido", "Pad R&B", "x---------------",
              { { 58, 62, 63, 67 }, { 57, 60, 62, 65 }, { 58, 60, 63, 67 }, { 58, 62, 65, 0 } } },
        };
        return list;
    }

    const Recipe* findRecipe (const juce::String& id)
    {
        for (auto& r : recipes())
            if (id == r.info.id)
                return &r;
        return nullptr;
    }

    // Letra del paso 'step' (0..63): las líneas de 16 letras se repiten cada compás
    char charAt (const char* line, int step)
    {
        const int len = (int) std::strlen (line);
        return len == 0 ? '.' : line[step % len];
    }
}

const std::vector<GenreInfo>& all()
{
    static const std::vector<GenreInfo> list = []
    {
        std::vector<GenreInfo> v;
        for (auto& r : recipes())
            v.push_back (r.info);
        return v;
    }();
    return list;
}

juce::String nameOf (const juce::String& id)
{
    auto* r = findRecipe (id);
    return r != nullptr ? juce::String::fromUTF8 (r->info.name) : juce::String();
}

bool load (Project& project, juce::ValueTree pattern, const juce::String& id)
{
    const auto* recipe = findRecipe (id);
    const auto* kit = recipe != nullptr ? kits::find (recipe->kit) : nullptr;
    if (kit == nullptr || ! pattern.isValid())
        return false;

    auto* um = project.um();

    // Busca (o crea) el canal de un sonido del kit
    auto kitChannel = [&] (const juce::String& soundId) -> juce::ValueTree
    {
        for (auto& sound : kit->sounds)
            if (soundId == sound.id)
            {
                auto ch = project.findChannelByBuiltin (kits::samplePath (*kit, sound));
                return ch.isValid() ? ch : project.addKitSound (*kit, sound);
            }
        return {};
    };

    auto rename = [&] (juce::ValueTree ch, const juce::String& name)
    {
        ch.setProperty (ids::name, name, um);
        const int track = ch[ids::mixer];
        if (track > 0)
            project.getMixerTrack (track).setProperty (ids::name, name.substring (0, 12), um);
    };

    // Busca (o crea) un sinte o un 808 con nombre propio a partir de un preset
    auto instrument = [&] (BassKind kind, const char* preset, const char* name) -> juce::ValueTree
    {
        const auto channelName = juce::String::fromUTF8 (name);
        auto ch = project.channels().getChildWithProperty (ids::name, channelName);
        if (ch.isValid())
            return ch;

        const auto presetName = juce::String::fromUTF8 (preset);
        if (kind == BassKind::bass808)
        {
            for (auto& p : kits::bass808Presets())
                if (juce::String::fromUTF8 (p.name) == presetName)
                    ch = project.add808Preset (p);
        }
        else
        {
            for (auto& p : kits::synthPresets())
                if (juce::String::fromUTF8 (p.name) == presetName)
                    ch = project.addSynthPreset (p);
        }
        if (ch.isValid())
            rename (ch, channelName);
        return ch;
    };

    constexpr int bars = 4, length = bars * 16;
    project.clearPattern (pattern);
    pattern.setProperty (ids::length, length, um);
    project.getState().setProperty (ids::bpm, recipe->info.bpm, um);

    // Batería
    for (auto& line : recipe->drums)
        if (auto ch = kitChannel (line.sound); ch.isValid())
            for (int s = 0; s < length; ++s)
                if (charAt (line.steps, s) == 'x')
                    project.setStep (pattern, (int) ch[ids::uid], s, true);

    // Lee una línea con '-' (mantener) y llama a addNote con cada nota y su duración
    auto forEachNote = [&] (const char* line, auto&& addNote)
    {
        for (int s = 0; s < length; ++s)
        {
            const char c = charAt (line, s);
            if (c == '.' || c == '-')
                continue;
            int len = 1;
            while (s + len < length && (s + len) % 16 != 0 && charAt (line, s + len) == '-')
                ++len;
            addNote (s, len, c);
        }
    };

    // Bajo
    juce::ValueTree bass, keys;
    if (recipe->bassKind != BassKind::none)
        if (bass = instrument (recipe->bassKind, recipe->bassPreset, recipe->bassName); bass.isValid())
            forEachNote (recipe->bassLine, [&] (int s, int len, char c)
            {
                const int root = recipe->bassRoots[s / 16];
                const int pitch = root + (c == 'o' ? 12 : c == '5' ? 7 : 0);
                project.addNote (pattern, bass[ids::uid], (double) s, (double) len, pitch, 0.9f);
            });

    // Acordes
    if (recipe->chordPreset != nullptr)
        if (keys = instrument (BassKind::synth, recipe->chordPreset, recipe->chordName); keys.isValid())
            forEachNote (recipe->chordRhythm, [&] (int s, int len, char)
            {
                for (int note : recipe->chords[s / 16])
                    if (note > 0)
                        project.addNote (pattern, keys[ids::uid], (double) s, (double) len, note,
                                         s % 16 == 0 ? 0.8f : 0.68f);
            });

    // Swing: el lo-fi y el boom bap "bailan"; el resto va recto
    const juce::String gid (recipe->info.id);
    project.getState().setProperty (ids::swing, gid == "lofi" ? 0.3 : gid == "boombap" ? 0.2 : 0.0, um);

    // Sidechain: en house y techno el bajo y los acordes "bombean" con el bombo
    if ((gid == "house" || gid == "techno") && ! recipe->drums.empty())
        if (auto kick = kitChannel (recipe->drums.front().sound); kick.isValid())
        {
            // ¿Suena algo de esta pista en el patrón? (sin contar al canal 'except')
            auto trackInUse = [&] (int track, const juce::ValueTree& except)
            {
                for (auto ch : project.channels())
                {
                    if ((int) ch[ids::mixer] != track || ch == except)
                        continue;
                    for (auto item : pattern)
                        if ((int) item[ids::channel] == (int) ch[ids::uid]
                            && (item.hasType (ids::NOTE) || (item.hasType (ids::STEPSET) && item[ids::data].toString().containsChar ('1'))))
                            return true;
                }
                return false;
            };
            // Para que el sidechain funcione, el bombo, el bajo y los acordes necesitan pista propia
            auto ownTrack = [&] (juce::ValueTree ch, int avoid)
            {
                if (! ch.isValid())
                    return;
                const int current = ch[ids::mixer];
                if (current > 0 && current != avoid && ! trackInUse (current, ch))
                    return;
                for (int t = numMixerTracks - 1; t >= 1; --t)
                    if (t != avoid && ! trackInUse (t, ch))
                    {
                        ch.setProperty (ids::mixer, t, um);
                        rename (ch, ch[ids::name].toString());
                        return;
                    }
            };
            ownTrack (kick, 0);
            const int kickTrack = kick[ids::mixer];
            ownTrack (bass, kickTrack);
            ownTrack (keys, kickTrack);
            if (keys.isValid() && bass.isValid() && (int) keys[ids::mixer] == (int) bass[ids::mixer])
                keys = {};   // comparten pista: el sidechain del bajo ya los agacha a los dos

            for (auto& ch : { bass, keys })
            {
                const int track = ch.isValid() ? (int) ch[ids::mixer] : 0;
                if (kickTrack > 0 && track > 0 && track != kickTrack)
                {
                    auto t = project.getMixerTrack (track);
                    t.setProperty (ids::sidechainSource, kickTrack, um);
                    t.setProperty (ids::sidechain, ch == bass ? 0.7 : 0.5, um);
                }
            }
        }

    return true;
}
}
