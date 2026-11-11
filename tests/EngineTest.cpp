// ============================================================================
//  Pruebas automáticas de Studoo (sin ventana ni tarjeta de sonido).
//  Comprueban el motor de audio, los instrumentos, el mezclador, la
//  automatización, guardar/abrir, deshacer y la exportación a WAV.
//
//  Uso:  StudooEngineTest [carpeta_para_archivos_de_prueba]
// ============================================================================
#include <juce_audio_utils/juce_audio_utils.h>
#include "audio/AudioEngine.h"
#include "model/Project.h"

#include <iostream>

using namespace studoo;

static int failures = 0;

static void check (bool condition, const char* what)
{
    std::cout << (condition ? "  OK    " : "  FALLO ") << what << "\n";
    if (! condition)
        ++failures;
}

static constexpr double sampleRate = 44100.0;
static constexpr int blockSize = 512;

// Genera 'seconds' segundos de audio
static juce::AudioBuffer<float> renderSeconds (AudioEngine& engine, double seconds)
{
    const int total = (int) (sampleRate * seconds);
    juce::AudioBuffer<float> out (2, total);
    out.clear();
    juce::AudioBuffer<float> block (2, blockSize);
    for (int pos = 0; pos < total; pos += blockSize)
    {
        const int n = juce::jmin (blockSize, total - pos);
        engine.render (nullptr, 0, block.getArrayOfWritePointers(), 2, n);
        out.copyFrom (0, pos, block, 0, 0, n);
        out.copyFrom (1, pos, block, 1, 0, n);
    }
    return out;
}

static bool isFinite (const juce::AudioBuffer<float>& b)
{
    for (int ch = 0; ch < b.getNumChannels(); ++ch)
        for (int i = 0; i < b.getNumSamples(); ++i)
            if (! std::isfinite (b.getSample (ch, i)))
                return false;
    return true;
}

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI init;
    SampleCache cache;

    juce::File outDir = argc > 1 ? juce::File::getCurrentWorkingDirectory().getChildFile (argv[1])
                                 : juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("StudooTest");
    outDir.createDirectory();

    Project project;
    AudioEngine engine;
    engine.prepare (sampleRate, blockSize);

    std::cout << "Proyecto y secuenciador\n";
    check (project.channels().getNumChildren() == 6, "El proyecto nuevo tiene 6 canales");
    check (project.playlist().getNumChildren() == 1, "La playlist empieza con 1 clip");

    engine.updateFromProject (project.getState(), cache);
    check (renderSeconds (engine, 0.2).getMagnitude (0, (int) (sampleRate * 0.2)) < 1.0e-6f, "Silencio con el patrón vacío y parado");

    project.loadExampleBeat (project.getCurrentPattern());
    engine.updateFromProject (project.getState(), cache);
    engine.play();
    auto beat = renderSeconds (engine, 2.0);
    check (beat.getRMSLevel (0, 0, beat.getNumSamples()) > 0.02f, "El beat de ejemplo suena");
    check (beat.getMagnitude (0, 0, 2000) > 0.1f, "El bombo suena en el primer paso");
    check (beat.getMagnitude (0, 0, beat.getNumSamples()) <= 1.0f, "Sin saturación (pico <= 1)");
    check (isFinite (beat), "Sin valores no válidos (NaN)");
    engine.stop();

    // --- Mute -------------------------------------------------------------------------
    for (auto ch : project.channels())
        ch.setProperty (ids::muted, true, nullptr);
    engine.updateFromProject (project.getState(), cache);
    engine.play();
    renderSeconds (engine, 0.1); // deja que termine lo que sonaba
    engine.stop();
    engine.play();
    check (renderSeconds (engine, 1.0).getMagnitude (0, (int) sampleRate) < 1.0e-4f, "Silencio con todos los canales en mute");
    engine.stop();
    for (auto ch : project.channels())
        ch.setProperty (ids::muted, false, nullptr);

    // --- Deshacer / rehacer --------------------------------------------------------------
    std::cout << "Deshacer y rehacer\n";
    {
        auto pattern = project.getCurrentPattern();
        const int kickUid = project.channels().getChild (0)[ids::uid];
        project.beginTransaction();
        project.setStep (pattern, kickUid, 3, true);
        check (project.getStep (pattern, kickUid, 3), "Paso activado");
        project.getUndo().undo();
        check (! project.getStep (pattern, kickUid, 3), "Ctrl+Z lo deshace");
        project.getUndo().redo();
        check (project.getStep (pattern, kickUid, 3), "Ctrl+Y lo rehace");
    }

    // --- Guardar y abrir ------------------------------------------------------------------
    std::cout << "Guardar y abrir\n";
    {
        auto file = outDir.getChildFile ("prueba.studoo");
        check (project.saveToFile (file), "Se guarda el proyecto");
        Project loaded;
        juce::String error;
        check (loaded.loadFromFile (file, error), "Se abre el proyecto");
        check (loaded.channels().getNumChildren() == project.channels().getNumChildren(), "Mismos canales");
        check (loaded.getState().isEquivalentTo (project.getState()), "Todo igual tras guardar y abrir");
    }

    // --- Instrumentos -----------------------------------------------------------------------
    std::cout << "Instrumentos\n";
    for (auto type : { ChannelType::synth, ChannelType::bass808 })
    {
        Project p;
        auto pattern = p.getCurrentPattern();
        auto ch = p.findChannelByType (type);
        p.addNote (pattern, ch[ids::uid], 0.0, 8.0, type == ChannelType::synth ? 60 : 36);
        AudioEngine e;
        e.prepare (sampleRate, blockSize);
        e.updateFromProject (p.getState(), cache);
        e.play();
        auto audio = renderSeconds (e, 0.5);
        check (audio.getRMSLevel (0, 0, audio.getNumSamples()) > 0.01f && isFinite (audio),
               type == ChannelType::synth ? "El sintetizador suena" : "El 808 suena");
    }

    // --- Modo canción, efectos y automatización -------------------------------------------------
    std::cout << "Playlist, mezclador y automatización\n";
    {
        project.getState().setProperty (ids::playMode, "song", nullptr);
        project.addClip ("pattern", project.getCurrentPatternUid(), 1, 16.0, 16.0);
        check (juce::approximatelyEqual (project.getSongLengthSteps(), 32.0), "La canción dura 2 compases");

        for (int t = 1; t < numMixerTracks; ++t)
        {
            auto track = project.getMixerTrack (t);
            track.setProperty (ids::reverbMix, 0.5f, nullptr);
            track.setProperty (ids::delayMix, 0.4f, nullptr);
            track.setProperty (ids::compOn, true, nullptr);
            track.setProperty (ids::eqLow, 6.0f, nullptr);
            track.setProperty (ids::eqHigh, -6.0f, nullptr);
        }
        engine.updateFromProject (project.getState(), cache);
        engine.play();
        auto song = renderSeconds (engine, 2.0);
        engine.stop();
        check (song.getRMSLevel (0, 0, song.getNumSamples()) > 0.02f, "La canción suena con efectos");
        check (isFinite (song) && song.getMagnitude (0, 0, song.getNumSamples()) <= 1.0f, "Los efectos no rompen el sonido");

        // Automatización: el volumen del Master baja a 0
        AutomationTarget target;
        target.kind = AutomationTarget::Kind::mixer;
        target.index = 0;
        target.param = AutomationTarget::Param::volume;
        auto lane = project.addAutomationLane (target);
        for (auto point : lane)
            point.setProperty (ids::value, 0.0f, nullptr);
        engine.updateFromProject (project.getState(), cache);
        engine.play();
        renderSeconds (engine, 0.05);
        auto automated = renderSeconds (engine, 1.0);
        engine.stop();
        check (automated.getMagnitude (0, 0, automated.getNumSamples()) < 1.0e-3f, "La automatización de volumen funciona");
        project.automation().removeAllChildren (nullptr);
        engine.updateFromProject (project.getState(), cache);
    }

    // --- Kits incluidos -------------------------------------------------------------------
    std::cout << "Kits incluidos\n";
    {
        int total = 0, ok = 0;
        for (auto& kit : kits::all())
            for (auto& sound : kit.sounds)
            {
                ++total;
                auto sample = cache.get (kits::samplePath (kit, sound));
                if (sample != nullptr && sample->buffer.getNumSamples() > 100 && isFinite (sample->buffer)
                    && sample->buffer.getMagnitude (0, sample->buffer.getNumSamples()) > 0.5f
                    && sample->buffer.getMagnitude (0, sample->buffer.getNumSamples()) <= 1.0f)
                    ++ok;
                else
                    std::cout << "    sonido con problemas: " << kit.id << "/" << sound.id << "\n";
            }
        std::cout << "    " << kits::all().size() << " kits, " << total << " sonidos\n";
        check (total == ok && total >= 200, "Todos los sonidos de los kits se generan bien");

        // "Usar este kit" cambia los sonidos y conserva el ritmo
        Project p;
        auto pattern = p.getCurrentPattern();
        p.loadExampleBeat (pattern);
        const int channelsBefore = p.channels().getNumChildren();
        const int kickUid = p.findChannelByBuiltin ("builtin:kick")[ids::uid];
        p.beginTransaction();
        p.useKit (*kits::find ("trap"));
        auto kick = p.findChannel (kickUid);
        check (kick[ids::sample].toString() == "builtin:trap/kick", "Usar el kit Trap cambia el sonido del bombo");
        check (p.getStep (pattern, kickUid, 0) && p.getStep (pattern, kickUid, 8), "El ritmo del bombo se conserva");
        // Bombo, caja, hi-hat y palmada ya existían: se añaden solo los demás sonidos del kit
        check (p.channels().getNumChildren() == channelsBefore + (int) kits::find ("trap")->sounds.size() - 4,
               "Se añaden los sonidos del kit que faltaban");
        check ((float) p.findChannelByType (ChannelType::bass808)[ids::decay] > 2.0f, "El 808 se ajusta al estilo trap");
        p.getUndo().undo();
        check (p.findChannel (kickUid)[ids::sample].toString() == "builtin:kick", "Ctrl+Z deshace el cambio de kit");

        // El kit suena
        Project q;
        q.channels().removeAllChildren (nullptr);
        q.addKit (*kits::find ("reggaeton"));
        auto qp = q.getCurrentPattern();
        for (auto ch : q.channels())
            for (int st : { 0, 4, 8, 12 })
                q.setStep (qp, ch[ids::uid], st, true);
        AudioEngine e;
        e.prepare (sampleRate, blockSize);
        e.updateFromProject (q.getState(), cache);
        e.play();
        auto audio = renderSeconds (e, 1.0);
        check (audio.getRMSLevel (0, 0, audio.getNumSamples()) > 0.03f && isFinite (audio), "El kit Reggaetón suena en el secuenciador");

        // Sonidos de sintetizador preparados y tipos de 808
        int presetsOk = 0, presetsTotal = 0;
        auto testPreset = [&] (std::function<juce::ValueTree (Project&)> add, int note)
        {
            ++presetsTotal;
            Project pp;
            pp.channels().removeAllChildren (nullptr);
            auto ch = add (pp);
            pp.addNote (pp.getCurrentPattern(), ch[ids::uid], 0.0, 8.0, note);
            AudioEngine pe;
            pe.prepare (sampleRate, blockSize);
            pe.updateFromProject (pp.getState(), cache);
            pe.play();
            auto out = renderSeconds (pe, 1.0);
            if (out.getRMSLevel (0, 0, out.getNumSamples()) > 0.005f && isFinite (out) && out.getMagnitude (0, 0, out.getNumSamples()) <= 1.0f)
                ++presetsOk;
        };
        for (auto& sp : kits::synthPresets())
            testPreset ([&sp] (Project& pp) { return pp.addSynthPreset (sp); }, sp.root);
        for (auto& bp : kits::bass808Presets())
            testPreset ([&bp] (Project& pp) { return pp.add808Preset (bp); }, 36);
        std::cout << "    " << presetsTotal << " sonidos de sintetizador y 808\n";
        check (presetsOk == presetsTotal, "Todos los sonidos de sintetizador y los tipos de 808 suenan");

        // Muestras de cada kit para escucharlas
        for (auto& kit : kits::all())
        {
            juce::AudioBuffer<float> demo (1, (int) (sampleRate * 0.5) * (int) kit.sounds.size());
            demo.clear();
            int pos = 0;
            for (auto& sound : kit.sounds)
            {
                auto sample = cache.get (kits::samplePath (kit, sound));
                if (sample != nullptr)
                    demo.copyFrom (0, pos, sample->buffer, 0, 0, juce::jmin (sample->buffer.getNumSamples(), (int) (sampleRate * 0.5)));
                pos += (int) (sampleRate * 0.5);
            }
            juce::StringArray soundIds;
            for (auto& sound : kit.sounds)
                soundIds.add (sound.id);
            outDir.getChildFile (juce::String ("kit_") + kit.id + ".txt").replaceWithText (soundIds.joinIntoString (","));
            auto f = outDir.getChildFile (juce::String ("kit_") + kit.id + ".wav");
            f.deleteFile();
            juce::WavAudioFormat wav;
            std::unique_ptr<juce::OutputStream> stream (f.createOutputStream());
            if (auto w = wav.createWriterFor (stream, juce::AudioFormatWriterOptions{}.withSampleRate (sampleRate).withNumChannels (1).withBitsPerSample (16)))
                w->writeFromAudioSampleBuffer (demo, 0, demo.getNumSamples());
        }
    }

    // --- Exportar ------------------------------------------------------------------------------
    std::cout << "Exportar\n";
    {
        auto wav = outDir.getChildFile ("cancion.wav");
        juce::String error;
        const bool ok = engine.renderSongToFile (wav, 44100.0, nullptr, error);
        check (ok && wav.existsAsFile(), "Se exporta la canción a WAV");

        juce::AudioFormatManager fm;
        fm.registerBasicFormats();
        std::unique_ptr<juce::AudioFormatReader> reader (fm.createReaderFor (wav));
        // 32 pasos a 140 BPM = 3,43 s + 2 s de cola
        const double seconds = reader != nullptr ? reader->lengthInSamples / reader->sampleRate : 0.0;
        check (seconds > 5.0 && seconds < 5.8, "La duración del WAV es correcta");
        std::cout << "  WAV de prueba: " << wav.getFullPathName() << "\n";
    }

    // --- Plugin VST3 (opcional: StudooEngineTest carpeta ruta/al/plugin.vst3) --------------------
    if (argc > 2)
    {
        std::cout << "Plugin VST3\n";
        juce::AudioPluginFormatManager formats;
        formats.addDefaultFormats();
        juce::OwnedArray<juce::PluginDescription> types;
        for (auto* format : formats.getFormats())
            format->findAllTypesForFile (types, juce::File::getCurrentWorkingDirectory().getChildFile (argv[2]).getFullPathName());
        check (types.size() > 0, "Se encuentra el plugin VST3");

        if (types.size() > 0)
        {
            juce::String error;
            auto instance = formats.createPluginInstance (*types[0], sampleRate, blockSize, error);
            check (instance != nullptr, "Se abre el plugin VST3");

            Project p;
            auto ch = p.addChannel (ChannelType::plugin, types[0]->name, 0);
            p.addNote (p.getCurrentPattern(), ch[ids::uid], 0.0, 8.0, 69);
            AudioEngine e;
            e.prepare (sampleRate, blockSize);
            e.setChannelPlugin (ch[ids::uid], std::move (instance));
            e.updateFromProject (p.getState(), cache);
            e.play();
            auto audio = renderSeconds (e, 0.5);
            check (audio.getRMSLevel (0, 0, audio.getNumSamples()) > 0.05f, "El plugin VST3 suena con las notas del piano roll");

            // Al borrar el canal, el plugin se libera
            p.removeChannel (ch[ids::uid]);
            e.updateFromProject (p.getState(), cache);
            check (e.getChannelPlugin (ch[ids::uid]) == nullptr, "El plugin se libera al borrar el canal");
        }
    }

    std::cout << (failures == 0 ? "\nTodas las pruebas han pasado.\n" : "\nHay pruebas que han fallado.\n");
    return failures == 0 ? 0 : 1;
}
