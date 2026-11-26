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
#include "model/MidiIO.h"
#include "audio/MixerTrack.h"
#include "audio/Onsets.h"

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

// Plugin de prueba que retrasa el sonido 1000 muestras (y lo dice, como los plugins de verdad)
class LatentDelayPlugin : public juce::AudioPluginInstance
{
public:
    LatentDelayPlugin() : AudioPluginInstance (BusesProperties().withInput ("In", juce::AudioChannelSet::stereo(), true)
                                                             .withOutput ("Out", juce::AudioChannelSet::stereo(), true))
    {
        setLatencySamples (latency);
    }
    void fillInPluginDescription (juce::PluginDescription& d) const override { d.name = "Retraso de prueba"; }
    const juce::String getName() const override { return "Retraso de prueba"; }
    void prepareToPlay (double, int) override { line.setSize (2, 4096); line.clear(); pos = 0; }
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>& b, juce::MidiBuffer&) override
    {
        for (int i = 0; i < b.getNumSamples(); ++i)
        {
            for (int ch = 0; ch < 2; ++ch)
            {
                const float in = b.getSample (ch, i);
                b.setSample (ch, i, line.getSample (ch, (pos - latency + 4096) % 4096));
                line.setSample (ch, pos, in);
            }
            pos = (pos + 1) % 4096;
        }
    }
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    juce::AudioProcessorEditor* createEditor() override { return nullptr; }
    bool hasEditor() const override { return false; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override {}
    void setStateInformation (const void*, int) override {}
    static constexpr int latency = 1000;
private:
    juce::AudioBuffer<float> line;
    int pos = 0;
};

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
    auto engineOwned = std::make_unique<AudioEngine>();
    auto& engine = *engineOwned;
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
        auto eOwned = std::make_unique<AudioEngine>();
        auto& e = *eOwned;
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
        auto eOwned = std::make_unique<AudioEngine>();
        auto& e = *eOwned;
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
            auto peOwned = std::make_unique<AudioEngine>();
            auto& pe = *peOwned;
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
    // --- Studoo 2.0: sampler, swing, sidechain y efectos nuevos ----------------------------------
    std::cout << "Funciones nuevas (sampler, swing, sidechain, efectos)\n";
    {
        // Prepara un proyecto con solo el bombo en los pasos indicados y lo hace sonar
        auto renderKick = [&cache] (std::function<void (Project&, juce::ValueTree kick)> setup, double seconds)
        {
            Project p;
            auto kick = p.findChannelByBuiltin ("builtin:kick");
            setup (p, kick);
            auto eOwned = std::make_unique<AudioEngine>();
            auto& e = *eOwned;
            e.prepare (sampleRate, blockSize);
            e.updateFromProject (p.getState(), cache);
            e.play();
            return renderSeconds (e, seconds);
        };
        auto lastSound = [] (const juce::AudioBuffer<float>& b)
        {
            for (int i = b.getNumSamples(); --i >= 0;)
                if (std::abs (b.getSample (0, i)) > 1.0e-3f)
                    return i;
            return 0;
        };
        auto firstSound = [] (const juce::AudioBuffer<float>& b)
        {
            for (int i = 0; i < b.getNumSamples(); ++i)
                if (std::abs (b.getSample (0, i)) > 1.0e-2f)
                    return i;
            return -1;
        };
        auto stepAt = [] (Project& p, const juce::ValueTree& ch, int step) { p.setStep (p.getCurrentPattern(), ch[ids::uid], step, true); };

        auto normal = renderKick ([&] (Project& p, juce::ValueTree k) { stepAt (p, k, 0); }, 1.0);
        auto reversed = renderKick ([&] (Project& p, juce::ValueTree k) { stepAt (p, k, 0); k.setProperty (ids::reverse, true, nullptr); }, 1.0);
        check (normal.getMagnitude (0, 0, 1500) > reversed.getMagnitude (0, 0, 1500) * 2.0f,
               "Sample al revés: empieza flojo (por la cola)");
        auto higher = renderKick ([&] (Project& p, juce::ValueTree k) { stepAt (p, k, 0); k.setProperty (ids::sampleTune, 12.0f, nullptr); }, 1.0);
        check (lastSound (higher) < lastSound (normal) * 0.7, "Tono +12: el sample suena más agudo y dura la mitad");
        auto sliced = renderKick ([&] (Project& p, juce::ValueTree k) { stepAt (p, k, 0); k.setProperty (ids::slices, 4, nullptr); }, 1.0);
        check (lastSound (sliced) < lastSound (normal) * 0.45, "Trocear en 4: cada nota toca un trozo");
        auto trimmed = renderKick ([&] (Project& p, juce::ValueTree k) { stepAt (p, k, 0); k.setProperty (ids::sampleEnd, 0.3f, nullptr); }, 1.0);
        check (lastSound (trimmed) < lastSound (normal) * 0.5, "Recortar el final del sample");
        check (isFinite (reversed) && isFinite (higher) && isFinite (sliced), "El sampler nuevo no da valores raros");

        // Swing: el paso 2 (una semicorchea "par") se retrasa medio paso
        const double samplesPerStep = sampleRate * 60.0 / 140.0 / 4.0;
        auto straight = renderKick ([&] (Project& p, juce::ValueTree k) { stepAt (p, k, 1); }, 0.6);
        auto swung = renderKick ([&] (Project& p, juce::ValueTree k) { stepAt (p, k, 1); p.getState().setProperty (ids::swing, 1.0f, nullptr); }, 0.6);
        check (std::abs (firstSound (straight) - samplesPerStep) < 200, "Sin swing, el paso 2 suena a su hora");
        check (std::abs (firstSound (swung) - samplesPerStep * 1.5) < 200, "Con swing, el paso 2 se retrasa");

        // Sidechain: un pad largo se agacha cuando suena el bombo
        auto withPad = [&] (float amount)
        {
            return renderKick ([&] (Project& p, juce::ValueTree k)
            {
                for (int s : { 0, 4, 8, 12 }) stepAt (p, k, s);
                auto pad = p.findChannelByType (ChannelType::synth);
                pad.setProperty (ids::sustain, 1.0f, nullptr);
                p.addNote (p.getCurrentPattern(), pad[ids::uid], 0.0, 16.0, 60);
                auto padTrack = p.getMixerTrack ((int) pad[ids::mixer]);
                padTrack.setProperty (ids::sidechain, amount, nullptr);
                padTrack.setProperty (ids::sidechainSource, (int) k[ids::mixer], nullptr);
                // La pista del bombo, en silencio: así se oye solo el pad (el sidechain la sigue "oyendo")
                p.getMixerTrack ((int) k[ids::mixer]).setProperty (ids::muted, true, nullptr);
            }, 1.5);
        };
        auto dry = withPad (0.0f), ducked = withPad (1.0f);
        check (ducked.getRMSLevel (0, 0, ducked.getNumSamples()) < dry.getRMSLevel (0, 0, dry.getNumSamples()) * 0.95f,
               "Sidechain: el pad baja cuando suena el bombo");

        // Limitador del Master: con todo a tope no hay picos recortados
        auto loud = [&] (bool limiter)
        {
            return renderKick ([&] (Project& p, juce::ValueTree k)
            {
                for (int s : { 0, 4, 8, 12 }) stepAt (p, k, s);
                k.setProperty (ids::volume, 1.25f, nullptr);
                p.getMixerTrack ((int) k[ids::mixer]).setProperty (ids::volume, 1.25f, nullptr);
                p.getMixerTrack (0).setProperty (ids::volume, 1.25f, nullptr);
                p.getMixerTrack (0).setProperty (ids::limiter, limiter, nullptr);
            }, 1.0);
        };
        auto countClipped = [] (const juce::AudioBuffer<float>& b)
        {
            int n = 0;
            for (int i = 0; i < b.getNumSamples(); ++i)
                n += std::abs (b.getSample (0, i)) >= 0.999f ? 1 : 0;
            return n;
        };
        const int clippedOff = countClipped (loud (false)), clippedOn = countClipped (loud (true));
        check (clippedOff > 50 && clippedOn < clippedOff / 10, "El limitador evita que el sonido se recorte");

        // Filtro, distorsión y chorus de pista
        auto withFx = [&] (const juce::Identifier& id, float value)
        {
            return renderKick ([&] (Project& p, juce::ValueTree k)
            {
                auto hat = p.findChannelByBuiltin ("builtin:hihat");
                for (int s = 0; s < 16; s += 2) stepAt (p, hat, s);
                p.getMixerTrack ((int) hat[ids::mixer]).setProperty (id, value, nullptr);
                juce::ignoreUnused (k);
            }, 1.0);
        };
        auto hats = withFx (ids::trackFilter, 0.0f), dark = withFx (ids::trackFilter, -1.0f);
        check (dark.getRMSLevel (0, 0, dark.getNumSamples()) < hats.getRMSLevel (0, 0, hats.getNumSamples()) * 0.3f,
               "Filtro de pista: quita los agudos del hi-hat");
        auto dist = withFx (ids::distortion, 1.0f), chorus = withFx (ids::chorusMix, 1.0f);
        check (isFinite (dist) && isFinite (chorus) && dist.getRMSLevel (0, 0, dist.getNumSamples()) > 0.01f
               && chorus.getRMSLevel (0, 0, chorus.getNumSamples()) > 0.01f, "Distorsión y chorus suenan sin romper nada");

        // 16 pistas y envíos
        auto rms = [] (const juce::AudioBuffer<float>& b) { return b.getRMSLevel (0, 0, b.getNumSamples()); };
        auto kickOn = [&] (std::function<void (Project&, juce::ValueTree)> extra)
        {
            return renderKick ([&] (Project& p, juce::ValueTree k)
            {
                for (int s : { 0, 4, 8, 12 }) stepAt (p, k, s);
                extra (p, k);
            }, 1.0);
        };
        {
            Project p;
            check (p.getMixerTrack (16).isValid() && (int) p.getMixerTrack (16)[ids::index] == 16, "El mezclador tiene 16 pistas (más el Master)");
        }
        auto onLast = kickOn ([] (Project&, juce::ValueTree k) { k.setProperty (ids::mixer, 16, nullptr); });
        check (rms (onLast) > 0.01f, "Un canal en la pista 16 suena");
        auto direct = kickOn ([] (Project&, juce::ValueTree) {});
        auto bussed = kickOn ([] (Project& p, juce::ValueTree k)
        {
            auto t = p.getMixerTrack ((int) k[ids::mixer]);
            t.setProperty (ids::sendTo, 12, nullptr);
            t.setProperty (ids::sendLevel, 1.0f, nullptr);
            t.setProperty (ids::sendOnly, true, nullptr);
            p.getMixerTrack (12).setProperty (ids::volume, 0.8f, nullptr);
        });
        check (std::abs (rms (bussed) - rms (direct) * 0.8f) < rms (direct) * 0.15f, "Solo envío: el bombo suena a través de la pista 12");
        auto bussedMuted = kickOn ([] (Project& p, juce::ValueTree k)
        {
            auto t = p.getMixerTrack ((int) k[ids::mixer]);
            t.setProperty (ids::sendTo, 12, nullptr);
            t.setProperty (ids::sendLevel, 1.0f, nullptr);
            t.setProperty (ids::sendOnly, true, nullptr);
            p.getMixerTrack (12).setProperty (ids::muted, true, nullptr);
        });
        check (rms (bussedMuted) < 1.0e-4f, "Si la pista de destino está en silencio, el envío tampoco suena");
        auto doubled = kickOn ([] (Project& p, juce::ValueTree k)
        {
            auto t = p.getMixerTrack ((int) k[ids::mixer]);
            t.setProperty (ids::sendTo, 12, nullptr);
            t.setProperty (ids::sendLevel, 1.0f, nullptr);
        });
        check (rms (doubled) > rms (direct) * 1.3f, "Envío normal: se oye la pista y además la copia");
        auto loop = kickOn ([] (Project& p, juce::ValueTree k)
        {
            const int t = k[ids::mixer];
            p.getMixerTrack (t).setProperty (ids::sendTo, t, nullptr);           // a sí misma: no se permite
            p.getMixerTrack (t).setProperty (ids::sendLevel, 1.0f, nullptr);
            p.getMixerTrack (12).setProperty (ids::sendTo, 3, nullptr);          // hacia atrás: no se permite
            p.getMixerTrack (12).setProperty (ids::sendLevel, 1.0f, nullptr);
        });
        check (isFinite (loop) && std::abs (rms (loop) - rms (direct)) < rms (direct) * 0.05f, "Los envíos imposibles (bucles) se ignoran");
        auto soloSend = kickOn ([] (Project& p, juce::ValueTree k)
        {
            auto t = p.getMixerTrack ((int) k[ids::mixer]);
            t.setProperty (ids::sendTo, 12, nullptr);
            t.setProperty (ids::sendLevel, 1.0f, nullptr);
            t.setProperty (ids::sendOnly, true, nullptr);
            t.setProperty (ids::solo, true, nullptr);
        });
        check (rms (soloSend) > 0.01f, "Solo en una pista con envío: también se oye su destino");

        // Proyecto antiguo con 8 pistas: se abre con 16
        {
            Project old;
            auto mix = old.getState().getChildWithName (ids::MIXER);
            for (int i = mix.getNumChildren(); --i >= 9;)
                mix.removeChild (i, nullptr);
            juce::String error;
            Project p;
            check (p.loadFromXmlString (old.toXmlString(), error) && p.getMixerTrack (16).isValid(),
                   "Un proyecto de antes (8 pistas) se abre con 16 pistas");
        }

        // Grabaciones: volumen y fundidos del clip
        auto audioClip = [&] (float gain, double fadeIn)
        {
            Project p;
            auto voice = p.addChannel (ChannelType::audio, "Voz", 7);
            voice.setProperty (ids::sample, "builtin:kick", nullptr);
            auto clip = p.addClip ("audio", voice[ids::uid], 0, 0.0, 8.0);
            clip.setProperty (ids::clipGain, gain, nullptr);
            clip.setProperty (ids::fadeIn, fadeIn, nullptr);
            p.getState().setProperty (ids::playMode, "song", nullptr);
            for (auto ch : p.channels())
                if (ch != voice) ch.setProperty (ids::muted, true, nullptr);
            auto eOwned = std::make_unique<AudioEngine>();
            auto& e = *eOwned;
            e.prepare (sampleRate, blockSize);
            e.updateFromProject (p.getState(), cache);
            e.play();
            return renderSeconds (e, 0.5);
        };
        auto full = audioClip (1.0f, 0.0), half = audioClip (0.5f, 0.0), faded = audioClip (1.0f, 4.0);
        check (rms (full) > 0.01f, "Una grabación en la playlist suena");
        check (std::abs (rms (half) - rms (full) * 0.5f) < rms (full) * 0.1f, "Volumen del clip al 50%");
        check (faded.getMagnitude (0, 0, 2000) < full.getMagnitude (0, 0, 2000) * 0.3f, "Fundido de entrada: empieza bajito");

        // Estirar al tempo y cambiar el tono de una grabación (con un "la" de 440 Hz de 1 segundo)
        {
            auto wav = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("studoo-tono-440.wav");
            {
                juce::AudioBuffer<float> tone (1, 44100);
                for (int i = 0; i < tone.getNumSamples(); ++i)
                    tone.setSample (0, i, 0.5f * (float) std::sin (juce::MathConstants<double>::twoPi * 440.0 * i / 44100.0));
                wav.deleteFile();
                juce::WavAudioFormat fmt;
                std::unique_ptr<juce::AudioFormatWriter> w (fmt.createWriterFor (new juce::FileOutputStream (wav), 44100.0, 1, 16, {}, 0));
                if (w != nullptr) w->writeFromAudioSampleBuffer (tone, 0, tone.getNumSamples());
            }
            auto play = [&] (bool stretch, int semis, double bpm)
            {
                Project p;
                p.getState().setProperty (ids::bpm, bpm, nullptr);
                auto voice = p.addChannel (ChannelType::audio, "Tono", 7);
                voice.setProperty (ids::sample, wav.getFullPathName(), nullptr);
                auto clip = p.addClip ("audio", voice[ids::uid], 0, 0.0, 32.0);
                clip.setProperty (ids::stretch, stretch, nullptr);
                clip.setProperty (ids::recordBpm, 120.0, nullptr);
                clip.setProperty (ids::clipPitch, semis, nullptr);
                p.getState().setProperty (ids::playMode, "song", nullptr);
                for (auto ch : p.channels())
                    if (ch != voice) ch.setProperty (ids::muted, true, nullptr);
                auto eOwned = std::make_unique<AudioEngine>();
                auto& e = *eOwned;
                e.prepare (sampleRate, blockSize);
                e.updateFromProject (p.getState(), cache);
                e.play();
                return renderSeconds (e, 2.5);
            };
            auto lastLoud = [] (const juce::AudioBuffer<float>& b)
            {
                for (int i = b.getNumSamples(); --i >= 0;)
                    if (std::abs (b.getSample (0, i)) > 0.05f) return i;
                return 0;
            };
            // Frecuencia aproximada contando cruces por cero entre 0,2 s y 0,7 s
            auto freq = [] (const juce::AudioBuffer<float>& b)
            {
                int crossings = 0;
                for (int i = 8820; i < 30870; ++i)
                    crossings += (b.getSample (0, i - 1) < 0.0f) != (b.getSample (0, i) < 0.0f) ? 1 : 0;
                return crossings / 2.0 / 0.5;
            };
            auto normal = play (false, 0, 60.0), slow = play (true, 0, 60.0), up = play (false, 12, 120.0);
            check (std::abs (freq (normal) - 440.0) < 15.0, "La grabación suena a su tono (440 Hz)");
            check (lastLoud (slow) > lastLoud (normal) * 1.7 && std::abs (freq (slow) - 440.0) < 25.0,
                   "Seguir el tempo: a la mitad de BPM dura el doble y no cambia el tono");
            check (std::abs (freq (up) - 880.0) < 40.0 && std::abs (lastLoud (up) - lastLoud (normal)) < 4410,
                   "Tono +12: una octava más aguda y dura lo mismo");
            check (isFinite (slow) && isFinite (up), "Estirar y cambiar el tono no da valores raros");
            wav.deleteFile();
        }

        // Tanda A: patrones largos, editor de pasos, stems y glide del sinte
        {
            auto firstAbove = [] (const juce::AudioBuffer<float>& b, float th)
            {
                for (int i = 0; i < b.getNumSamples(); ++i)
                    if (std::abs (b.getSample (0, i)) > th) return i;
                return -1;
            };
            // Un patrón de 8 compases: el paso 100 suena a su hora
            auto longPattern = renderKick ([&] (Project& p, juce::ValueTree k)
            {
                p.getCurrentPattern().setProperty (ids::length, 128, nullptr);
                p.getState().setProperty (ids::bpm, 240.0, nullptr);
                stepAt (p, k, 100);
            }, 6.6);
            check (std::abs (firstAbove (longPattern, 0.01f) - sampleRate * 60.0 / 240.0 / 4.0 * 100) < 300,
                   "Patrón de 8 compases: el paso 100 suena a su hora");

            // Editor de pasos: fuerza 30 suena más flojo; fuerza 0 no suena
            auto loudStep = renderKick ([&] (Project& p, juce::ValueTree k) { stepAt (p, k, 0); }, 0.5);
            auto softStep = renderKick ([&] (Project& p, juce::ValueTree k)
            {
                stepAt (p, k, 0);
                p.setStepValue (p.getCurrentPattern(), k[ids::uid], 0, ids::stepVel, 30);
            }, 0.5);
            auto mutedStep = renderKick ([&] (Project& p, juce::ValueTree k)
            {
                stepAt (p, k, 0);
                p.setStepValue (p.getCurrentPattern(), k[ids::uid], 0, ids::stepVel, 0);
            }, 0.5);
            check (rms (softStep) < rms (loudStep) * 0.6f && rms (mutedStep) < 1.0e-5f, "Editor de pasos: la fuerza de cada golpe cambia el volumen");
            auto upStep = renderKick ([&] (Project& p, juce::ValueTree k)
            {
                stepAt (p, k, 0);
                p.setStepValue (p.getCurrentPattern(), k[ids::uid], 0, ids::stepPitch, 12);
            }, 1.0);
            auto baseStep = renderKick ([&] (Project& p, juce::ValueTree k) { stepAt (p, k, 0); }, 1.0);
            check (lastSound (upStep) < lastSound (baseStep) * 0.7, "Editor de pasos: +12 semitonos suena más agudo");
            {
                Project p;
                const int uid = p.channels().getChild (0)[ids::uid];
                p.setStepValue (p.getCurrentPattern(), uid, 5, ids::stepVel, 500);
                p.setStepValue (p.getCurrentPattern(), uid, 6, ids::stepPitch, -99);
                check (p.getStepValue (p.getCurrentPattern(), uid, 5, ids::stepVel) == 100
                       && p.getStepValue (p.getCurrentPattern(), uid, 6, ids::stepPitch) == -24
                       && p.getStepValue (p.getCurrentPattern(), uid, 7, ids::stepVel) == 85, "Editor de pasos: los valores se limitan y tienen valor por defecto");
            }

            // Exportar por pistas: con el bombo en su pista, la pista de la caja queda en silencio
            {
                Project p;
                auto kick = p.findChannelByBuiltin ("builtin:kick");
                auto snare = p.findChannelByBuiltin ("builtin:snare");
                p.setStep (p.getCurrentPattern(), kick[ids::uid], 0, true);
                p.setStep (p.getCurrentPattern(), snare[ids::uid], 4, true);
                auto eOwned = std::make_unique<AudioEngine>();
                auto& e = *eOwned;
                e.prepare (sampleRate, blockSize);
                e.updateFromProject (p.getState(), cache);
                e.setStemTrack ((int) snare[ids::mixer]);
                e.play();
                auto onlySnare = renderSeconds (e, 0.5);
                const int snareStart = (int) (samplesPerStep * 4);
                check (onlySnare.getMagnitude (0, 0, snareStart - 200) < 1.0e-4f && onlySnare.getMagnitude (0, snareStart, 4000) > 0.01f,
                       "Exportar por pistas: solo suena la pista elegida");
            }

            // Sinte mono con glide: la segunda nota empieza con el tono de la primera
            {
                auto glideRender = [&] (float glide)
                {
                    Project p;
                    auto synth = p.findChannelByType (ChannelType::synth);
                    synth.setProperty (ids::mono, true, nullptr);
                    synth.setProperty (ids::glide, glide, nullptr);
                    synth.setProperty (ids::wave, 2, nullptr);
                    synth.setProperty (ids::sustain, 1.0f, nullptr);
                    synth.setProperty (ids::cutoff, 18000.0f, nullptr);
                    p.addNote (p.getCurrentPattern(), synth[ids::uid], 0.0, 5.0, 57);   // La3 (220 Hz)
                    p.addNote (p.getCurrentPattern(), synth[ids::uid], 4.0, 8.0, 69);   // La4 (440 Hz), solapada
                    for (auto ch : p.channels()) if (ch != synth) ch.setProperty (ids::muted, true, nullptr);
                    auto eOwned = std::make_unique<AudioEngine>();
                    auto& e = *eOwned;
                    e.prepare (sampleRate, blockSize);
                    e.updateFromProject (p.getState(), cache);
                    e.play();
                    return renderSeconds (e, 1.2);
                };
                auto crossings = [] (const juce::AudioBuffer<float>& b, int from, int len)
                {
                    int c = 0;
                    for (int i = from + 1; i < from + len; ++i)
                        c += (b.getSample (0, i - 1) < 0.0f) != (b.getSample (0, i) < 0.0f) ? 1 : 0;
                    return c / 2.0 / (len / sampleRate);
                };
                const int second = (int) (samplesPerStep * 4);
                auto noGlide = glideRender (0.0f), withGlide = glideRender (0.5f);
                const double fNo = crossings (noGlide, second + 600, 2205), fGlide = crossings (withGlide, second + 600, 2205);
                check (fNo > 380.0 && fGlide < fNo * 0.8, "Sinte mono: con glide la nota se desliza (empieza más grave)");
                check (crossings (withGlide, (int) (sampleRate * 1.0), 4410) > 400.0, "Sinte mono: al final llega a la nota nueva");
            }

            // Tanda D: notas con panorama, afinación y deslizamiento; capas
            {
                auto crossings = [] (const juce::AudioBuffer<float>& b, int from, int len)
                {
                    int c = 0;
                    for (int i = from + 1; i < from + len; ++i)
                        c += (b.getSample (0, i - 1) < 0.0f) != (b.getSample (0, i) < 0.0f) ? 1 : 0;
                    return c / 2.0 / (len / sampleRate);
                };
                auto renderNotes = [&] (std::function<void (Project&, juce::ValueTree)> setup, double seconds)
                {
                    Project p;
                    auto synth = p.findChannelByType (ChannelType::synth);
                    synth.setProperty (ids::wave, 2, nullptr);
                    synth.setProperty (ids::sustain, 1.0f, nullptr);
                    synth.setProperty (ids::cutoff, 18000.0f, nullptr);
                    for (auto ch : p.channels()) if (ch != synth) ch.setProperty (ids::muted, true, nullptr);
                    setup (p, synth);
                    auto eOwned = std::make_unique<AudioEngine>();
                    auto& e = *eOwned;
                    e.prepare (sampleRate, blockSize);
                    e.updateFromProject (p.getState(), cache);
                    e.play();
                    return renderSeconds (e, seconds);
                };
                auto left = renderNotes ([] (Project& p, juce::ValueTree s)
                {
                    auto n = p.addNote (p.getCurrentPattern(), s[ids::uid], 0.0, 4.0, 69);
                    n.setProperty (ids::notePan, -1.0f, nullptr);
                }, 0.3);
                check (left.getRMSLevel (0, 0, 8000) > 0.01f && left.getRMSLevel (1, 0, 8000) < left.getRMSLevel (0, 0, 8000) * 0.05f,
                       "Panorama por nota: la nota a la izquierda solo suena por la izquierda");
                auto fine = renderNotes ([] (Project& p, juce::ValueTree s)
                {
                    auto n = p.addNote (p.getCurrentPattern(), s[ids::uid], 0.0, 8.0, 69);
                    n.setProperty (ids::noteFine, 100.0f, nullptr);   // +1 semitono
                }, 0.6);
                check (std::abs (crossings (fine, 4410, 17640) - 466.2) < 8.0, "Afinación fina: +100 cents sube un semitono");
                auto slid = renderNotes ([] (Project& p, juce::ValueTree s)
                {
                    p.addNote (p.getCurrentPattern(), s[ids::uid], 0.0, 4.0, 57);
                    auto n = p.addNote (p.getCurrentPattern(), s[ids::uid], 4.0, 8.0, 69);
                    n.setProperty (ids::slide, true, nullptr);
                }, 1.2);
                const int second = (int) (samplesPerStep * 4);
                check (crossings (slid, second + 300, 1500) < 380.0 && crossings (slid, (int) (sampleRate * 1.0), 4410) > 420.0,
                       "Nota deslizante: empieza en el tono de la anterior y llega a la suya");
                auto layered = renderNotes ([] (Project& p, juce::ValueTree s)
                {
                    // El sinte está en mute, pero el 808 (que suena) es una capa de otro canal con notas
                    auto bass = p.findChannelByType (ChannelType::bass808);
                    bass.setProperty (ids::muted, false, nullptr);
                    auto lead = p.addChannel (ChannelType::synth, "Capa", 9);
                    lead.setProperty (ids::volume, 0.0f, nullptr);
                    lead.setProperty (ids::layers, bass[ids::uid].toString(), nullptr);
                    s.setProperty (ids::muted, true, nullptr);
                    p.addNote (p.getCurrentPattern(), lead[ids::uid], 0.0, 4.0, 45);
                }, 0.4);
                check (layered.getRMSLevel (0, 0, 8000) > 0.02f, "Capas: las notas de un canal hacen sonar también sus capas");
            }

            // Modo directo: un patrón lanzado suena; al pararlo, se calla en el siguiente compás
            {
                Project p;
                auto kick = p.findChannelByBuiltin ("builtin:kick");
                const int patUid = p.getCurrentPatternUid();
                for (int st = 0; st < 16; st += 2) p.setStep (p.getCurrentPattern(), kick[ids::uid], st, true);
                auto other = p.addPattern ("Vacío");
                p.setCurrentPattern (other[ids::uid]);   // el patrón actual está vacío: solo suena lo lanzado
                auto eOwned = std::make_unique<AudioEngine>();
                auto& e = *eOwned;
                e.prepare (sampleRate, blockSize);
                e.updateFromProject (p.getState(), cache);
                auto silentNormal = (e.play(), renderSeconds (e, 0.3));
                e.stop();
                e.setLiveMode (true);
                e.toggleLivePattern (patUid);
                e.play();
                auto launched = renderSeconds (e, 0.5);
                check (silentNormal.getRMSLevel (0, 0, silentNormal.getNumSamples()) < 1.0e-4f
                       && launched.getRMSLevel (0, 0, launched.getNumSamples()) > 0.02f && e.getLiveState (patUid) == 1,
                       "Modo directo: el patrón lanzado suena");
                e.toggleLivePattern (patUid);
                auto stopping = renderSeconds (e, 0.2);
                juce::ignoreUnused (stopping);
                const bool waits = e.getLiveState (patUid) == 3;
                renderSeconds (e, 2.0);   // pasa el final del compás (a 140 BPM, 1,7 s)
                auto after = renderSeconds (e, 0.5);
                check (waits && e.getLiveState (patUid) == 0 && after.getMagnitude (0, 0, after.getNumSamples()) < 0.01f,
                       "Modo directo: al pararlo espera al compás y luego se calla");
            }
        }

        // Tanda B: efectos avanzados de pista (autotune, puerta, EQ paramétrico, ritmo, maximizador)
        {
            // Procesa 'seconds' de un seno por una pista del mezclador con estos ajustes
            auto runTrack = [] (TrackParams p, double hz, float amp, double seconds, bool playing = false, bool analyse = false)
            {
                MixerTrack track;
                track.prepare (sampleRate, blockSize);
                track.ensureRhythm();
                track.setAnalysePitch (analyse);
                const int total = (int) (seconds * sampleRate);
                juce::AudioBuffer<float> out (2, total);
                double phase = 0.0, stepPos = 0.0;
                const double sps = sampleRate * 60.0 / 120.0 / 4.0;
                for (int pos = 0; pos < total; pos += blockSize)
                {
                    const int n = juce::jmin (blockSize, total - pos);
                    auto& b = track.getBuffer();
                    for (int i = 0; i < n; ++i)
                    {
                        const float v = amp * (float) std::sin (phase);
                        phase += juce::MathConstants<double>::twoPi * hz / sampleRate;
                        b.setSample (0, i, v);
                        b.setSample (1, i, v);
                    }
                    track.setTransport (stepPos, sps, playing);
                    track.process (n, p, 120.0, false);
                    stepPos += n / sps;
                    out.copyFrom (0, pos, b, 0, 0, n);
                    out.copyFrom (1, pos, b, 1, 0, n);
                }
                return std::make_pair (out, track.getDetectedHz());
            };
            auto freqOf = [] (const juce::AudioBuffer<float>& b, int from, int len)
            {
                int c = 0;
                for (int i = from + 1; i < from + len; ++i)
                    c += (b.getSample (0, i - 1) < 0.0f) != (b.getSample (0, i) < 0.0f) ? 1 : 0;
                return c / 2.0 / (len / sampleRate);
            };
            TrackParams base;
            base.volume = 1.0f;
            base.pan = 0.0f;

            auto [plain, detected] = runTrack (base, 450.0, 0.3f, 1.0, false, true);
            check (std::abs (detected - 450.0f) < 5.0f, "Afinador: detecta un tono de 450 Hz");
            auto tuned = base;
            tuned.tuneAmount = 1.0f;
            auto [corrected, d2] = runTrack (tuned, 450.0, 0.3f, 1.5);
            juce::ignoreUnused (d2);
            check (std::abs (freqOf (corrected, 44100, 22050) - 440.0) < 6.0 && isFinite (corrected),
                   "Autotune: 450 Hz se corrige a 440 Hz (La)");
            auto inScale = tuned;
            inScale.tuneScale = 1;   // Do mayor: Fa# (370 Hz) no está; va a Fa (349) o Sol (392)
            auto [scaled, d3] = runTrack (inScale, 370.0, 0.3f, 1.5);
            juce::ignoreUnused (d3);
            const double fs = freqOf (scaled, 44100, 22050);
            check (std::abs (fs - 349.2) < 6.0 || std::abs (fs - 392.0) < 6.0, "Autotune con escala: solo deja notas de la escala");

            auto gated = base;
            gated.gateThreshold = -30.0f;
            auto [quiet, d4] = runTrack (gated, 200.0, 0.005f, 0.5);   // unos -46 dB: por debajo del umbral
            auto [loud, d5] = runTrack (gated, 200.0, 0.3f, 0.5);
            juce::ignoreUnused (d4, d5);
            check (quiet.getRMSLevel (0, 11025, 11025) < 0.0005f && loud.getRMSLevel (0, 11025, 11025) > 0.15f,
                   "Puerta de ruido: corta lo flojo y deja pasar lo fuerte");

            auto boosted = base;
            boosted.peqFreq[1] = 1000.0f;
            boosted.peqGain[1] = 12.0f;
            auto [flat, d6] = runTrack (base, 1000.0, 0.05f, 0.3);
            auto [peak, d7] = runTrack (boosted, 1000.0, 0.05f, 0.3);
            auto [away, d8] = runTrack (boosted, 100.0, 0.05f, 0.3);
            juce::ignoreUnused (d6, d7, d8);
            check (peak.getRMSLevel (0, 4410, 8820) > flat.getRMSLevel (0, 4410, 8820) * 3.0f
                   && away.getRMSLevel (0, 4410, 8820) < flat.getRMSLevel (0, 4410, 8820) * 1.3f,
                   "EQ paramétrico: +12 dB en 1 kHz sube esa frecuencia y no las demás");

            auto halfTime = base;
            halfTime.rhythmMode = 1;
            auto [slow, d9] = runTrack (halfTime, 440.0, 0.3f, 1.0, true);
            juce::ignoreUnused (d9);
            check (std::abs (freqOf (slow, 4410, 8820) - 220.0) < 10.0, "Half-time: suena a mitad de velocidad (una octava más grave)");
            auto trance = base;
            trance.rhythmMode = 4;
            auto [chopped, d10] = runTrack (trance, 440.0, 0.3f, 1.0, true);
            juce::ignoreUnused (d10);
            const float ratio = chopped.getRMSLevel (0, 0, 44100) / plain.getRMSLevel (0, 0, 44100);
            check (ratio > 0.55f && ratio < 0.85f, "Puerta rítmica: el sonido se corta a ritmo");
            for (int mode = 2; mode <= 6; ++mode)
            {
                auto m = base;
                m.rhythmMode = mode;
                auto [o, d] = runTrack (m, 300.0, 0.3f, 2.5, true);
                juce::ignoreUnused (d);
                if (! isFinite (o) || o.getMagnitude (0, 0, o.getNumSamples()) > 1.0f)
                    check (false, ("Efecto de ritmo " + juce::String (mode) + " no da valores raros").toRawUTF8());
            }

            auto maxed = base;
            maxed.maximize = 12.0f;
            auto [hot, d11] = runTrack (maxed, 200.0, 0.3f, 0.5);
            juce::ignoreUnused (d11);
            check (hot.getRMSLevel (0, 11025, 11025) > plain.getRMSLevel (0, 11025, 11025) * 2.0f
                   && hot.getMagnitude (0, 11025, 11025) < 0.95f, "Maximizador: más volumen sin pasar del máximo");

            // Tanda D: flanger, phaser, transitorios, multibanda
            auto diff = [] (const juce::AudioBuffer<float>& a, const juce::AudioBuffer<float>& b)
            {
                double d = 0.0;
                for (int i = 0; i < juce::jmin (a.getNumSamples(), b.getNumSamples()); ++i)
                    d += std::abs (a.getSample (0, i) - b.getSample (0, i));
                return d / a.getNumSamples();
            };
            {
                auto [ref, r0] = runTrack (base, 300.0, 0.3f, 0.5);
                juce::ignoreUnused (r0);
                const char* names[] = { "Flanger", "Phaser", "Multibanda", "Transitorios" };
                for (int k = 0; k < 4; ++k)
                {
                    auto m = base;
                    if (k == 0) m.flangerMix = 1.0f;
                    if (k == 1) m.phaserMix = 1.0f;
                    if (k == 2) m.multiband = 1.0f;
                    if (k == 3) { m.transAttack = 1.0f; m.transSustain = -1.0f; }
                    auto [o, dd] = runTrack (m, 300.0, 0.3f, 0.5);
                    juce::ignoreUnused (dd);
                    check (isFinite (o) && o.getMagnitude (0, 0, o.getNumSamples()) < 2.0f && diff (o, ref) > 0.002,
                           (juce::String (names[k]) + ": cambia el sonido sin romperlo").toRawUTF8());
                }
            }
            {
                // Transitorios: un golpe seco con "Golpe" al máximo tiene el principio más fuerte que la cola
                TransientShaper ts;
                ts.prepare (sampleRate);
                juce::AudioBuffer<float> hit (2, 8820);
                for (int i = 0; i < hit.getNumSamples(); ++i)
                {
                    const float v = (i < 4410 ? 0.0f : 0.5f) * (float) std::sin (i * 0.05);
                    hit.setSample (0, i, v); hit.setSample (1, i, v);
                }
                auto shaped = hit;
                ts.process (shaped, shaped.getNumSamples(), 1.0f, 0.0f);
                check (shaped.getMagnitude (0, 4410, 400) > hit.getMagnitude (0, 4410, 400) * 1.3f
                       && shaped.getMagnitude (0, 8000, 800) < hit.getMagnitude (0, 8000, 800) * 1.15f,
                       "Transitorios: sube el golpe y deja igual lo que sigue");
            }
            {
                juce::AudioBuffer<float> st (2, 1000);
                for (int i = 0; i < 1000; ++i) { st.setSample (0, i, 0.5f); st.setSample (1, i, -0.1f); }
                auto mono = st;
                widenStereo (mono, 1000, 0.0f);
                auto wide = st;
                widenStereo (wide, 1000, 2.0f);
                check (std::abs (mono.getSample (0, 10) - mono.getSample (1, 10)) < 1.0e-5f
                       && std::abs (wide.getSample (0, 10) - wide.getSample (1, 10)) > 0.6f,
                       "Ancho estéreo: a 0 es mono, a 200% más separado");
            }
            {
                // Vocoder: sin voz no suena; con voz, el sinte suena
                Vocoder v;
                v.prepare (sampleRate);
                juce::AudioBuffer<float> synth (2, 22050), voice (2, 22050), silent (2, 22050);
                silent.clear();
                for (int i = 0; i < 22050; ++i)
                {
                    const float saw = (float) (2.0 * std::fmod (i * 110.0 / sampleRate, 1.0) - 1.0) * 0.3f;
                    synth.setSample (0, i, saw); synth.setSample (1, i, saw);
                    const float vo = 0.3f * (float) (std::sin (i * 0.06) * std::sin (i * 0.0007));
                    voice.setSample (0, i, vo); voice.setSample (1, i, vo);
                }
                auto a = synth, b = synth;
                v.process (a, silent, 22050, 1.0f);
                v.reset();
                v.process (b, voice, 22050, 1.0f);
                check (a.getRMSLevel (0, 11025, 11025) < 0.002f && b.getRMSLevel (0, 11025, 11025) > 0.02f && isFinite (b),
                       "Vocoder: el sinte solo suena cuando habla la voz");
            }
        }

        // Multisample: cada nota usa el sample más cercano
        {
            auto makeTone = [] (const juce::String& name, double hz)
            {
                auto f = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile (name);
                juce::AudioBuffer<float> tone (1, 44100);
                for (int i = 0; i < tone.getNumSamples(); ++i)
                    tone.setSample (0, i, 0.5f * (float) std::sin (juce::MathConstants<double>::twoPi * hz * i / 44100.0));
                f.deleteFile();
                juce::WavAudioFormat fmt;
                std::unique_ptr<juce::AudioFormatWriter> w (fmt.createWriterFor (new juce::FileOutputStream (f), 44100.0, 1, 16, {}, 0));
                if (w != nullptr) w->writeFromAudioSampleBuffer (tone, 0, tone.getNumSamples());
                return f;
            };
            auto low = makeTone ("studoo-zona-220.wav", 220.0), high = makeTone ("studoo-zona-880.wav", 880.0);
            auto playNote = [&] (int note)
            {
                Project p;
                auto ch = p.addChannel (ChannelType::sampler, "Multi", 7);
                ch.setProperty (ids::sample, low.getFullPathName(), nullptr);
                ch.setProperty (ids::oneShot, false, nullptr);
                for (auto [file, root, lo, hi] : { std::make_tuple (low, 57, 0, 69), std::make_tuple (high, 81, 70, 127) })
                {
                    juce::ValueTree z (ids::ZONE);
                    z.setProperty (ids::sample, file.getFullPathName(), nullptr);
                    z.setProperty (ids::root, root, nullptr);
                    z.setProperty (ids::lo, lo, nullptr);
                    z.setProperty (ids::hi, hi, nullptr);
                    ch.appendChild (z, nullptr);
                }
                for (auto c : p.channels()) if (c != ch) c.setProperty (ids::muted, true, nullptr);
                p.addNote (p.getCurrentPattern(), ch[ids::uid], 0.0, 4.0, note);
                auto eOwned = std::make_unique<AudioEngine>();
                auto& e = *eOwned;
                e.prepare (sampleRate, blockSize);
                e.updateFromProject (p.getState(), cache);
                e.play();
                auto b = renderSeconds (e, 0.4);
                int c = 0;
                for (int i = 4410 + 1; i < 4410 + 8820; ++i)
                    c += (b.getSample (0, i - 1) < 0.0f) != (b.getSample (0, i) < 0.0f) ? 1 : 0;
                return c / 2.0 / 0.2;
            };
            const double f57 = playNote (57), f69 = playNote (69), f81 = playNote (81);
            check (std::abs (f57 - 220.0) < 10.0 && std::abs (f81 - 880.0) < 30.0 && std::abs (f69 - 440.0) < 15.0,
                   "Multisample: cada nota usa su sample (y los de en medio se afinan desde el más cercano)");
            low.deleteFile();
            high.deleteFile();
        }

        // Trocear por golpes y tomas silenciadas
        {
            juce::AudioBuffer<float> loop (1, 44100);
            loop.clear();
            for (int hit : { 0, 11025, 22050, 33075 })   // 4 golpes, uno cada cuarto de segundo
                for (int i = 0; i < 3000; ++i)
                    loop.setSample (0, hit + i, 0.8f * std::exp (-i / 600.0f) * (float) std::sin (i * 0.3));
            auto points = onsets::detect (loop, 44100.0, 0.0f, 1.0f, 32);
            bool near = points.size() == 4;
            for (size_t i = 0; near && i < 4; ++i)
                near = std::abs (points[i] - 0.25f * (float) i) < 0.02f;
            check (near, "Trocear por golpes: encuentra los 4 golpes del loop");

            Project p;
            auto kick = p.findChannelByBuiltin ("builtin:kick");
            kick.setProperty (ids::slices, 3, nullptr);
            kick.setProperty (ids::slicePoints, "0,0.2,0.5", nullptr);
            auto snap = Snapshot::build (p.getState());
            check (snap->channels[(size_t) p.getChannelIndex (kick[ids::uid])].sampler.numSlicePoints == 3, "Los golpes detectados se usan como trozos");
            kick.setProperty (ids::slicePoints, "0,0.7,0.5", nullptr);   // desordenados: no valen
            snap = Snapshot::build (p.getState());
            check (snap->channels[(size_t) p.getChannelIndex (kick[ids::uid])].sampler.numSlicePoints == 0, "Golpes en mal orden: se ignoran");

            auto voice = p.addChannel (ChannelType::audio, "Voz", 7);
            voice.setProperty (ids::sample, "builtin:kick", nullptr);
            auto take1 = p.addClip ("audio", voice[ids::uid], 0, 0.0, 8.0);
            auto take2 = p.addClip ("audio", voice[ids::uid], 1, 0.0, 8.0);
            take2.setProperty (ids::clipMuted, true, nullptr);
            juce::ignoreUnused (take1);
            snap = Snapshot::build (p.getState());
            int audioClips = 0;
            for (auto& c : snap->clips) audioClips += c.isAudio ? 1 : 0;
            check (audioClips == 1, "Tomas: el trozo silenciado no suena");
        }

        // Bloques de automatización en la Playlist: la curva se repite en cada bloque
        {
            Project p;
            AutomationTarget t;
            t.kind = AutomationTarget::Kind::mixer;
            t.index = 1;
            t.param = AutomationTarget::Param::volume;
            auto lane = p.addAutomationLane (t);
            lane.removeAllChildren (nullptr);
            for (auto [time, value] : { std::make_pair (0.0, 0.0f), std::make_pair (16.0, 1.0f) })
            {
                juce::ValueTree pt (ids::POINT);
                pt.setProperty (ids::time, time, nullptr);
                pt.setProperty (ids::value, value, nullptr);
                lane.appendChild (pt, nullptr);
            }
            p.addClip ("automation", lane[ids::uid], 0, 32.0, 16.0);
            p.addClip ("automation", lane[ids::uid], 0, 64.0, 16.0);
            auto snap = Snapshot::build (p.getState());
            check (snap->lanes.size() == 1 && snap->lanes[0].blocks.size() == 2, "Bloques de automatización: se leen en el Snapshot");
            const auto& l = snap->lanes[0];
            check (std::abs (l.valueAt (0.0) - 0.0f) < 0.01f && std::abs (l.valueAt (40.0) - 0.5f) < 0.02f
                   && std::abs (l.valueAt (50.0) - 1.0f) < 0.01f && std::abs (l.valueAt (72.0) - 0.5f) < 0.02f,
                   "Bloques de automatización: la curva empieza en cada bloque y fuera se queda el último valor");
            check (snap->songLength >= 80.0, "Los bloques de automatización cuentan en la duración de la canción");
        }

        // Compensación del retraso de los plugins: dos bombos iguales, uno con un plugin que
        // retrasa 1000 muestras; el otro se espera y suenan a la vez (un solo golpe)
        {
            auto firstHit = [] (const juce::AudioBuffer<float>& b)
            {
                for (int i = 0; i < b.getNumSamples(); ++i)
                    if (std::abs (b.getSample (0, i)) > 0.02f) return i;
                return -1;
            };
            Project p;
            auto kick = p.findChannelByBuiltin ("builtin:kick");
            for (auto c : p.channels()) if (c != kick) c.setProperty (ids::muted, true, nullptr);
            auto twin = p.addChannel (ChannelType::sampler, "Bombo 2", 12);
            twin.setProperty (ids::sample, "builtin:kick", nullptr);
            p.setStep (p.getCurrentPattern(), kick[ids::uid], 4, true);
            p.setStep (p.getCurrentPattern(), twin[ids::uid], 4, true);
            auto eOwned = std::make_unique<AudioEngine>();
            auto& e = *eOwned;
            e.prepare (sampleRate, blockSize);
            e.updateFromProject (p.getState(), cache);
            e.setMixerPlugin ((int) kick[ids::mixer], std::make_unique<LatentDelayPlugin>(), 1);
            e.play();
            auto out = renderSeconds (e, 0.6);
            const int expected = (int) (samplesPerStep * 4) + LatentDelayPlugin::latency;
            const int hit = firstHit (out);
            check (std::abs (hit - expected) < 64 && out.getMagnitude (0, 0, expected - 64) < 0.02f,
                   "Compensación del retraso: la pista sin plugin espera y los dos bombos suenan a la vez");
        }

        // Zona en bucle (modo canción): la reproducción vuelve al principio de la zona
        {
            Project p;
            p.getState().setProperty (ids::playMode, "song", nullptr);
            p.addClip ("pattern", p.getCurrentPatternUid(), 0, 0.0, 64.0);
            p.getState().setProperty (ids::loopOn, true, nullptr);
            p.getState().setProperty (ids::loopStart, 16.0, nullptr);
            p.getState().setProperty (ids::loopEnd, 32.0, nullptr);
            auto eOwned = std::make_unique<AudioEngine>();
            auto& e = *eOwned;
            e.prepare (sampleRate, blockSize);
            e.updateFromProject (p.getState(), cache);
            e.setPlayPosition (20.0);
            e.play();
            bool inside = true;
            for (int k = 0; k < 40; ++k)
            {
                renderSeconds (e, 0.1);
                const double pos = e.getPlayPosition();
                inside = inside && pos >= 16.0 - 1.0e-6 && pos < 32.0;
            }
            check (inside, "Bucle: la canción se repite dentro de la zona elegida");
        }

        // Compás y cambios de tempo
        {
            Project p;
            auto& st = p.getState();
            check (p.getStepsPerBar() == 16, "Compás 4/4: 16 pasos por compás");
            st.setProperty (ids::timeSigNum, 3, nullptr);
            check (p.getStepsPerBar() == 12, "Compás 3/4: 12 pasos por compás");
            st.setProperty (ids::timeSigNum, 6, nullptr);
            st.setProperty (ids::timeSigDen, 8, nullptr);
            check (p.getStepsPerBar() == 12, "Compás 6/8: 12 pasos por compás");
            st.setProperty (ids::timeSigNum, 99, nullptr);
            st.setProperty (ids::timeSigDen, 3, nullptr);
            check (p.getStepsPerBar() == 28, "Un compás imposible se limita (7/4 como mucho)");

            // Línea de tempo a 70 BPM (el proyecto está a 140): todo va a la mitad de velocidad
            auto withTempo = [&] (bool lane)
            {
                Project q;
                auto kick = q.findChannelByBuiltin ("builtin:kick");
                for (auto ch : q.channels()) if (ch != kick) ch.setProperty (ids::muted, true, nullptr);
                q.setStep (q.getCurrentPattern(), kick[ids::uid], 4, true);
                q.getState().setProperty (ids::playMode, "song", nullptr);
                if (lane)
                {
                    AutomationTarget t;
                    t.kind = AutomationTarget::Kind::project;
                    t.param = AutomationTarget::Param::bpm;
                    auto l = q.addAutomationLane (t);
                    for (auto point : l)
                        point.setProperty (ids::value, AutomationTarget::toNormalised (t.param, 70.0f), nullptr);
                }
                auto eOwned = std::make_unique<AudioEngine>();
                auto& e = *eOwned;
                e.prepare (sampleRate, blockSize);
                e.updateFromProject (q.getState(), cache);
                e.play();
                auto b = renderSeconds (e, 1.2);
                for (int i = 0; i < b.getNumSamples(); ++i)
                    if (std::abs (b.getSample (0, i)) > 0.01f) return i;
                return -1;
            };
            const int normal = withTempo (false), slowed = withTempo (true);
            check (std::abs (normal - samplesPerStep * 4) < 300 && std::abs (slowed - samplesPerStep * 8) < 600,
                   "Automatización de tempo: a 70 BPM el paso 5 llega el doble de tarde");
            check (AutomationTarget::parse ("project:0:bpm").isValid() && ! AutomationTarget::parse ("project:0:volume").isValid()
                   && ! AutomationTarget::parse ("mixer:1:bpm").isValid(), "El tempo solo se automatiza en la canción");
        }

        // Controles MIDI conectados: se guardan con el proyecto y se limpia lo que no vale
        {
            Project p;
            auto links = p.getState().getChildWithName (ids::MIDILINKS);
            juce::ValueTree link (ids::LINK);
            link.setProperty (ids::cc, 21, nullptr);
            link.setProperty (ids::kind, "mixer", nullptr);
            link.setProperty (ids::index, 3, nullptr);
            link.setProperty (ids::param, "volume", nullptr);
            links.appendChild (link, nullptr);
            links.appendChild (juce::ValueTree ("TRAMPA"), nullptr);
            juce::String error;
            Project q;
            const bool loaded = q.loadFromXmlString (p.toXmlString(), error);
            auto back = q.getState().getChildWithName (ids::MIDILINKS);
            check (loaded && back.getNumChildren() == 1 && (int) back.getChild (0)[ids::cc] == 21,
                   "Los controles MIDI conectados se guardan (y se quita lo que no es un control)");
        }

        // MIDI: exportar un patrón y volver a importarlo
        {
            Project p;
            auto synth = p.findChannelByType (ChannelType::synth);
            auto kick = p.findChannelByBuiltin ("builtin:kick");
            p.setStep (p.getCurrentPattern(), kick[ids::uid], 4, true);
            p.addNote (p.getCurrentPattern(), synth[ids::uid], 0.0, 2.0, 60, 0.5f);
            p.addNote (p.getCurrentPattern(), synth[ids::uid], 20.0, 1.5, 67, 1.0f);
            auto midi = midiio::patternToMidi (p, p.getCurrentPattern());
            juce::MemoryOutputStream out;
            check (midi.writeTo (out) && midi.getNumTracks() == 3, "Exportar MIDI: una pista de tempo y una por canal con notas");

            juce::MemoryInputStream in (out.getData(), out.getDataSize(), false);
            juce::MidiFile back;
            Project q;
            auto target = q.findChannelByType (ChannelType::synth);
            auto pattern = q.getCurrentPattern();
            const int count = back.readFrom (in) ? midiio::importIntoPattern (q, pattern, target[ids::uid], back) : 0;
            bool found60 = false, found67 = false;
            for (auto n : pattern)
                if (n.hasType (ids::NOTE))
                {
                    found60 = found60 || ((int) n[ids::pitch] == 60 && std::abs ((double) n[ids::start]) < 0.01 && std::abs ((double) n[ids::length] - 2.0) < 0.3);
                    found67 = found67 || ((int) n[ids::pitch] == 67 && std::abs ((double) n[ids::start] - 20.0) < 0.01);
                }
            check (count == 3 && found60 && found67 && Project::getPatternLength (pattern) == 32,
                   "Importar MIDI: vuelven las notas en su sitio y el patrón se alarga");

            juce::MidiFile garbage;
            juce::MemoryInputStream bad ("esto no es un midi", 18, false);
            check (! garbage.readFrom (bad), "Un archivo que no es MIDI se rechaza");
        }

        // Automatizar controles de instrumentos
        {
            using P = AutomationTarget::Param;
            bool ok = true;
            for (auto param : { P::resonance, P::lfoRate, P::lfoAmount, P::drive, P::sampleTune, P::sendLevel })
            {
                AutomationTarget t;
                t.kind = AutomationTarget::Kind::channel;
                t.index = 7;
                t.param = param;
                ok = ok && AutomationTarget::parse (t.toString()).param == param;
                const float v = AutomationTarget::fromNormalised (param, 0.6f);
                ok = ok && std::abs (AutomationTarget::toNormalised (param, v) - 0.6f) < 0.03f;
            }
            check (ok, "Las automatizaciones nuevas (LFO, resonancia, 808, tono, envío) se guardan y se leen bien");

            auto withLane = [&] (float value)
            {
                Project p;
                auto bass = p.findChannelByType (ChannelType::bass808);
                p.addNote (p.getCurrentPattern(), bass[ids::uid], 0.0, 8.0, 36);
                p.getState().setProperty (ids::playMode, "song", nullptr);
                AutomationTarget t;
                t.kind = AutomationTarget::Kind::channel;
                t.index = bass[ids::uid];
                t.param = P::drive;
                auto lane = p.addAutomationLane (t);
                for (auto point : lane)
                    point.setProperty (ids::value, value, nullptr);
                for (auto ch : p.channels())
                    if (ch != bass) ch.setProperty (ids::muted, true, nullptr);
                auto eOwned = std::make_unique<AudioEngine>();
                auto& e = *eOwned;
                e.prepare (sampleRate, blockSize);
                e.updateFromProject (p.getState(), cache);
                e.play();
                return renderSeconds (e, 0.8);
            };
            auto clean = withLane (0.0f), dirty = withLane (1.0f);
            check (rms (dirty) > rms (clean) * 1.2f, "Automatizar la distorsión del 808 cambia el sonido");
        }
    }

    // --- Sonidos de los juegos de oído ------------------------------------------------------
    std::cout << "Juegos de oído\n";
    {
        engine.stop();
        renderSeconds (engine, 4.0);   // que terminen las colas de lo que sonaba antes
        auto tone = std::make_shared<juce::AudioBuffer<float>> (2, 22050);
        for (int i = 0; i < tone->getNumSamples(); ++i)
            for (int ch = 0; ch < 2; ++ch)
                tone->setSample (ch, i, 0.5f * std::sin (6.2831853f * 440.0f * (float) i / 44100.0f));
        engine.playBuffer (tone, 44100.0, 1.0f);
        auto heard = renderSeconds (engine, 0.3);
        check (heard.getMagnitude (0, 0, heard.getNumSamples()) > 0.3f, "El sonido del juego se oye (con el beat parado)");
        auto after = renderSeconds (engine, 0.5);
        check (after.getMagnitude (0, (int) (sampleRate * 0.3), (int) (sampleRate * 0.2)) < 1.0e-4f, "Y se para solo al terminar");
        engine.playBuffer (tone, 44100.0, 1.0f);
        engine.stopBuffer();
        check (renderSeconds (engine, 0.1).getMagnitude (0, 0, 4410) < 1.0e-4f, "stopBuffer lo corta");
    }

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
            auto eOwned = std::make_unique<AudioEngine>();
            auto& e = *eOwned;
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

            // Huecos de efectos: un plugin en el hueco 3 de la pista del bombo cambia su sonido
            // (el plugin de prueba es un sinte: como efecto, deja la pista en silencio)
            Project q;
            auto kick = q.findChannelByBuiltin ("builtin:kick");
            for (int s : { 0, 4, 8, 12 }) q.setStep (q.getCurrentPattern(), kick[ids::uid], s, true);
            for (auto c : q.channels()) if (c != kick) c.setProperty (ids::muted, true, nullptr);
            auto fxOwned = std::make_unique<AudioEngine>();
            auto& fx = *fxOwned;
            fx.prepare (sampleRate, blockSize);
            fx.updateFromProject (q.getState(), cache);
            fx.setMixerPlugin ((int) kick[ids::mixer], formats.createPluginInstance (*types[0], sampleRate, blockSize, error), 2);
            check (fx.getMixerPlugin ((int) kick[ids::mixer], 2) != nullptr && fx.getMixerPlugin ((int) kick[ids::mixer], 0) == nullptr,
                   "Un plugin puede ir en el hueco 3 de una pista");
            fx.play();
            auto withFx = renderSeconds (fx, 0.5);
            fx.setMixerPlugin ((int) kick[ids::mixer], nullptr, 2);
            fx.stop(); fx.setPlayPosition (0.0); fx.play();
            auto without = renderSeconds (fx, 0.5);
            check (withFx.getRMSLevel (0, 0, withFx.getNumSamples()) < 0.001f
                   && without.getRMSLevel (0, 0, without.getNumSamples()) > 0.01f, "El efecto del hueco 3 se aplica y se puede quitar");
        }
    }

    std::cout << (failures == 0 ? "\nTodas las pruebas han pasado.\n" : "\nHay pruebas que han fallado.\n");
    return failures == 0 ? 0 : 1;
}
