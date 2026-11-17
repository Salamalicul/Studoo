// ============================================================================
//  Prueba de seguridad: abre miles de proyectos estropeados o con trampa
//  (generados al azar) y los hace sonar. Studoo no debe cerrarse, ni producir
//  valores no válidos (NaN), ni sonar a un volumen disparado.
//
//  Uso:  StudooFuzzTest [número_de_proyectos] [semilla]
//  Pensado para compilarse con -fsanitize=address,undefined.
// ============================================================================
#include <juce_audio_utils/juce_audio_utils.h>
#include "audio/AudioEngine.h"
#include "model/GenreBeats.h"
#include "model/Project.h"

#include <iostream>

using namespace studoo;

namespace
{
    juce::Random rng;

    // Números normales, extremos y raros
    juce::String weirdNumber()
    {
        static const char* specials[] = { "0", "-0", "1", "-1", "nan", "-nan", "inf", "-inf", "1e308", "-1e308",
                                          "1e-308", "2147483647", "-2147483648", "4294967296", "99999999999999999999",
                                          "", " ", "abc", "0x7fffffff", "1.5", "-0.0001", "127", "128", "255", "256",
                                          "-129", "65535", "1e9", "3.4e38" };
        switch (rng.nextInt (4))
        {
            case 0:  return specials[rng.nextInt ((int) std::size (specials))];
            case 1:  return juce::String (rng.nextInt (300) - 50);
            case 2:  return juce::String (rng.nextDouble() * 2000.0 - 1000.0);
            default: return juce::String ((double) rng.nextInt64() * 1.0e-6);
        }
    }

    juce::String weirdString()
    {
        static const char* specials[] = { "", "builtin:", "builtin:kick", "builtin:trap/kick", "builtin:../../x",
                                          "builtin:trap/", "builtin:/kick", "builtin:noexiste/noexiste",
                                          "/dev/zero", "/dev/urandom", "/etc/passwd", "\\\\servidor\\x.wav", "//srv/x.wav",
                                          "C:\\x.wav", "../../x.wav", "audio", "synth", "bass808", "plugin", "sampler",
                                          "pattern", "song", "mixer:0:volume", "channel:1:cutoff", "mixer:-1:pan",
                                          "mixer:99999:volume", "channel:x:y", "::", "a:b:c:d", "1010101011111111",
                                          "1111111111111111111111111111111111111111111111111111111111111111111111" };
        if (rng.nextInt (5) == 0)
        {
            juce::String s;
            const int n = rng.nextInt (40);
            for (int i = 0; i < n; ++i)
                s << juce::String::charToString ((juce::juce_wchar) (rng.nextInt (0x2000) + 1));
            return s;
        }
        return specials[rng.nextInt ((int) std::size (specials))];
    }

    juce::var weirdValue()
    {
        if (rng.nextInt (3) == 0)
            return weirdString();
        return weirdNumber();
    }

    // Parte de un proyecto de verdad (con bases de géneros) y lo estropea al azar
    juce::ValueTree makeBrokenProject()
    {
        Project p;
        const auto& genreList = genres::all();
        if (rng.nextBool())
            genres::load (p, p.getCurrentPattern(), genreList[(size_t) rng.nextInt ((int) genreList.size())].id);
        if (rng.nextBool())
            p.loadExampleBeat (p.getCurrentPattern());
        if (rng.nextBool())
        {
            AutomationTarget t;
            t.kind = AutomationTarget::Kind::mixer;
            t.index = rng.nextInt (10);
            t.param = AutomationTarget::Param::volume;
            auto lane = p.addAutomationLane (t);
            for (int i = 0; i < 5; ++i)
            {
                juce::ValueTree pt (ids::POINT);
                pt.setProperty (ids::time, rng.nextInt (64), nullptr);
                pt.setProperty (ids::value, rng.nextFloat(), nullptr);
                lane.appendChild (pt, nullptr);
            }
        }

        auto tree = p.getState().createCopy();

        static const juce::Identifier props[] = {
            ids::bpm, ids::playMode, ids::nextUid, ids::currentPattern, ids::selectedChannel, ids::uid, ids::name,
            ids::type, ids::volume, ids::pan, ids::muted, ids::mixer, ids::sample, ids::root, ids::oneShot, ids::wave,
            ids::attack, ids::decay, ids::sustain, ids::release, ids::cutoff, ids::resonance, ids::detune, ids::glide,
            ids::drive, ids::pluginDesc, ids::pluginState, ids::length, ids::channel, ids::data, ids::start,
            ids::pitch, ids::velocity, ids::kind, ids::ref, ids::track, ids::offset, ids::index, ids::solo, ids::eqLow,
            ids::eqMid, ids::eqHigh, ids::compOn, ids::compThreshold, ids::compRatio, ids::delayMix, ids::delayTime,
            ids::delayFeedback, ids::reverbMix, ids::reverbSize, ids::target, ids::time, ids::value, ids::colour };

        // Recorre todo el árbol y cambia, borra o duplica cosas al azar
        std::function<void (juce::ValueTree)> mutate = [&] (juce::ValueTree t)
        {
            for (int i = 0; i < t.getNumChildren(); ++i)
                mutate (t.getChild (i));

            const int changes = rng.nextInt (4);
            for (int c = 0; c < changes; ++c)
            {
                const auto& id = props[rng.nextInt ((int) std::size (props))];
                if (rng.nextInt (6) == 0) t.removeProperty (id, nullptr);
                else                      t.setProperty (id, weirdValue(), nullptr);
            }
            if (t.getNumChildren() > 0 && rng.nextInt (8) == 0)
                t.removeChild (rng.nextInt (t.getNumChildren()), nullptr);
            if (t.getNumChildren() > 0 && rng.nextInt (8) == 0)
                t.appendChild (t.getChild (rng.nextInt (t.getNumChildren())).createCopy(), nullptr);
        };
        if (rng.nextInt (10) != 0)
            mutate (tree);

        // A veces, nodos de tipos inesperados o en sitios inesperados
        if (rng.nextInt (4) == 0)
        {
            static const juce::Identifier types[] = { ids::CHANNEL, ids::PATTERN, ids::NOTE, ids::STEPSET, ids::CLIP,
                                                      ids::TRACK, ids::LANE, ids::POINT, ids::CHANNELS, ids::MIXER };
            for (int i = 0; i < 5; ++i)
            {
                juce::ValueTree n (types[rng.nextInt ((int) std::size (types))]);
                for (int k = 0; k < 6; ++k)
                    n.setProperty (props[rng.nextInt ((int) std::size (props))], weirdValue(), nullptr);
                auto parent = tree;
                if (tree.getNumChildren() > 0 && rng.nextBool())
                    parent = tree.getChild (rng.nextInt (tree.getNumChildren()));
                parent.appendChild (n, nullptr);
            }
        }
        return tree;
    }

    bool finiteAndSafe (const juce::AudioBuffer<float>& b, float& peak)
    {
        peak = 0.0f;
        for (int ch = 0; ch < b.getNumChannels(); ++ch)
            for (int i = 0; i < b.getNumSamples(); ++i)
            {
                const float v = b.getSample (ch, i);
                if (! std::isfinite (v))
                    return false;
                peak = juce::jmax (peak, std::abs (v));
            }
        return true;
    }
}

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI init;
    const int count = argc > 1 ? juce::jmax (1, juce::String (argv[1]).getIntValue()) : 2000;
    const auto seed = argc > 2 ? juce::String (argv[2]).getLargeIntValue() : (juce::int64) 12345;
    rng.setSeed (seed);

    SampleCache cache;
    AudioEngine engine;
    engine.prepare (44100.0, 512);
    juce::AudioBuffer<float> block (2, 512);

    int failures = 0;
    float worstPeak = 0.0f;

    for (int i = 0; i < count; ++i)
    {
        auto broken = makeBrokenProject();

        // 1) Como texto XML (lo que se lee de un archivo)
        Project project;
        juce::String error;
        auto xml = broken.createXml();
        const bool loaded = xml != nullptr && project.loadFromXmlString (xml->toString(), error);
        if (! loaded)
            continue;

        // 2) Que la interfaz pueda usarlo: patrón actual, canales, longitud...
        auto pattern = project.getCurrentPattern();
        juce::ignoreUnused (Project::getPatternLength (pattern), project.getSongLengthSteps());
        for (auto ch : project.channels())
            juce::ignoreUnused (project.getChannelIndex (ch[ids::uid]), project.findChannel (ch[ids::uid]));
        if (rng.nextBool() && pattern.isValid())
        {
            project.beginTransaction();
            genres::load (project, pattern, genres::all()[(size_t) rng.nextInt ((int) genres::all().size())].id);
            project.getUndo().undo();
        }

        // 3) Hacerlo sonar (patrón y canción)
        engine.updateFromProject (project.getState(), cache);
        engine.setPlayPosition (rng.nextDouble() * 64.0);
        engine.play();
        for (int b = 0; b < 40; ++b)
        {
            if (b == 10)
                engine.previewNote (project.channels().getChild (0)[ids::uid], rng.nextInt (300) - 100, rng.nextFloat() * 3.0f, 0.2);
            engine.render (nullptr, 0, block.getArrayOfWritePointers(), 2, 512);
            float peak = 0.0f;
            if (! finiteAndSafe (block, peak))
            {
                std::cout << "FALLO: audio no válido (NaN) en el proyecto " << i << "\n";
                broken.createXml()->writeTo (juce::File::getCurrentWorkingDirectory().getChildFile ("fallo_" + juce::String (i) + ".studoo"));
                ++failures;
                break;
            }
            worstPeak = juce::jmax (worstPeak, peak);
        }
        engine.stop();

        if (i % 500 == 0)
            std::cout << "  " << i << " proyectos probados...\n" << std::flush;
    }

    std::cout << "Proyectos probados: " << count << "\n"
              << "Pico máximo de volumen: " << worstPeak << "\n"
              << (failures == 0 ? "TODO BIEN\n" : "HAY FALLOS\n");
    return failures == 0 ? 0 : 1;
}
