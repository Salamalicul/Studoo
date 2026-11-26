// ============================================================================
//  Pruebas automáticas de "Aprende":
//    - cada reto NO se da por hecho en un proyecto nuevo, y SÍ cuando haces
//      lo que pide
//    - todos los sonidos de los juegos de oído suenan (no salen en silencio)
//    - el progreso: lecciones, tests, retos, racha y logros
// ============================================================================
#include <juce_audio_utils/juce_audio_utils.h>
#include "audio/Kits.h"
#include "model/GenreBeats.h"
#include "ui/Challenges.h"
#include "ui/AudioEditor.h"
#include "ui/EarGames.h"
#include "ui/LearnProgress.h"
#include "Pro.h"

#include <iostream>

using namespace studoo;

static int failures = 0;

static juce::String u (const char* utf8) { return juce::String::fromUTF8 (utf8); }

static void check (bool condition, const char* what) ;
static void check (bool condition, const juce::String& what)
{
    std::cout << (condition ? "  OK    " : "  FALLO ") << what << "\n";
    if (! condition)
        ++failures;
}

namespace
{
    juce::ValueTree channelWithSample (Project& p, const juce::String& sample)
    {
        return p.channels().getChildWithProperty (ids::sample, sample);
    }

    void steps (Project& p, const juce::ValueTree& ch, std::initializer_list<int> list)
    {
        for (int s : list)
            p.setStep (p.getCurrentPattern(), (int) ch[ids::uid], s, true);
    }

    // Lo que haría el usuario para superar cada reto
    void doChallenge (Project& p, const juce::String& id)
    {
        auto kick = channelWithSample (p, "builtin:kick");
        auto snare = channelWithSample (p, "builtin:snare");
        auto hat = channelWithSample (p, "builtin:hihat");
        auto synth = p.findChannelByType (ChannelType::synth);
        auto bass = p.findChannelByType (ChannelType::bass808);
        auto pattern = p.getCurrentPattern();
        auto track1 = p.getMixerTrack (1);

        if (id == "primer_beat")         steps (p, kick, { 0, 4, 8, 12 });
        else if (id == "bpm")            p.getState().setProperty (ids::bpm, 120.0, nullptr);
        else if (id == "largo_64")       pattern.setProperty (ids::length, 64, nullptr);
        else if (id == "caja_2_4")       steps (p, snare, { 4, 12 });
        else if (id == "bombo_negras")   steps (p, kick, { 0, 4, 8, 12 });
        else if (id == "contratiempo")   steps (p, hat, { 2, 6, 10, 14 });
        else if (id == "hihat_corcheas") steps (p, hat, { 0, 2, 4, 6, 8, 10, 12, 14 });
        else if (id == "roll")           steps (p, hat, { 12, 13, 14 });
        else if (id == "dembow")         steps (p, snare, { 3, 6, 11, 14 });
        else if (id == "silencio")       kick.setProperty (ids::muted, true, nullptr);
        else if (id == "sample_kit")     p.addKitSound (kits::all()[0], kits::all()[0].sounds[0]);
        else if (id == "notas_melodia")  for (int n : { 60, 62, 64, 65 }) p.addNote (pattern, synth[ids::uid], (n - 60) * 2.0, 1.0, n);
        else if (id == "acorde")         for (int n : { 60, 64, 67 }) p.addNote (pattern, synth[ids::uid], 0.0, 4.0, n);
        else if (id == "808_notas")      { p.addNote (pattern, bass[ids::uid], 0.0, 4.0, 36); p.addNote (pattern, bass[ids::uid], 4.0, 4.0, 36); p.addNote (pattern, bass[ids::uid], 8.0, 4.0, 39); }
        else if (id == "onda")           synth.setProperty (ids::wave, 1, nullptr);
        else if (id == "adsr_pad")       synth.setProperty (ids::attack, 0.5f, nullptr);
        else if (id == "filtro")         synth.setProperty (ids::cutoff, 1000.0f, nullptr);
        else if (id == "volumen_mezcla") track1.setProperty (ids::volume, 0.5f, nullptr);
        else if (id == "panorama")       kick.setProperty (ids::pan, 0.5f, nullptr);
        else if (id == "eq")             track1.setProperty (ids::eqLow, 3.0f, nullptr);
        else if (id == "compresor")      track1.setProperty (ids::compOn, true, nullptr);
        else if (id == "reverb")         track1.setProperty (ids::reverbMix, 0.3f, nullptr);
        else if (id == "delay")          track1.setProperty (ids::delayMix, 0.3f, nullptr);
        else if (id == "dos_patrones")   p.addPattern();
        else if (id == "playlist")       for (int i = 1; i < 4; ++i) p.addClip ("pattern", pattern[ids::uid], 0, i * 16.0, 16.0);
        else if (id == "automatizacion")
        {
            AutomationTarget t;
            t.kind = AutomationTarget::Kind::mixer;
            t.index = 1;
            t.param = AutomationTarget::Param::volume;
            auto lane = p.addAutomationLane (t);
            lane.getChild (1).setProperty (ids::value, 0.1f, nullptr);
        }
        else if (id == "grabacion")      p.addChannel (ChannelType::audio, "Voz", 7);
        else if (id == "base_genero")    genres::load (p, p.addPattern ("Base House"), "house");
        else if (id == "swing")          p.getState().setProperty (ids::swing, 0.3, nullptr);
        else if (id == "velocity")       p.addNote (pattern, synth[ids::uid], 0.0, 1.0, 60, 0.3f);
        else if (id == "sidechain")      p.getMixerTrack (5).setProperty (ids::sidechain, 0.6, nullptr);
        else if (id == "efectos_creativos") p.getMixerTrack (6).setProperty (ids::chorusMix, 0.5, nullptr);
        else if (id == "autotune")       p.getMixerTrack (7).setProperty (ids::tuneAmount, 0.5, nullptr);
        else if (id == "escala")         p.getState().setProperty (ids::scaleType, 2, nullptr);
        else if (id == "envio")
        {
            p.getMixerTrack (3).setProperty (ids::sendTo, 16, nullptr);
            p.getMixerTrack (3).setProperty (ids::sendLevel, 0.5, nullptr);
        }
        else if (id == "editar_grabacion")
        {
            auto voice = p.addChannel (ChannelType::audio, "Voz", 7);
            p.addClip ("audio", voice[ids::uid], 1, 0.0, 16.0, 2.0);
        }
        else if (id == "chops")
        {
            kick.setProperty (ids::slices, 8, nullptr);
            for (int n = 0; n < 3; ++n)
                p.addNote (pattern, kick[ids::uid], n * 4.0, 1.0, 60 + n, 0.8f);
        }
        else                             std::cout << "  (reto sin prueba: " << id << ")\n";
    }

    float peak (const AudioEngine::SharedBuffer& b)
    {
        return b == nullptr ? 0.0f : b->getMagnitude (0, b->getNumSamples());
    }
}

static void check (bool condition, const char* what) { check (condition, u (what)); }

int main()
{
    // Simulamos la tienda de Google Play (versión gratis) para probar Studoo Pro
   #if JUCE_WINDOWS
    _putenv_s ("STUDOO_PRUEBA_TIENDA", "1");
   #else
    setenv ("STUDOO_PRUEBA_TIENDA", "1", 1);
   #endif
    juce::ScopedJuceInitialiser_GUI init;

    // --- Retos ---------------------------------------------------------------------
    std::cout << "Retos (" << learn::challenges().size() << ")\n";
    for (auto& c : learn::challenges())
    {
        Project fresh;
        check (! c.check (fresh), u ("Proyecto nuevo: \"") + c.id + "\" todavía no está hecho");
        Project p;
        doChallenge (p, c.id);
        check (c.check (p), u ("Al hacerlo, \"") + c.id + "\" se detecta");
    }

    // --- Editor de audio ------------------------------------------------------------------
    std::cout << "Editor de audio\n";
    {
        using Op = AudioEditorPanel::Op;
        auto make = []
        {
            juce::AudioBuffer<float> b (1, 1000);
            for (int i = 0; i < 1000; ++i) b.setSample (0, i, 0.5f);
            return b;
        };
        auto b = make();
        AudioEditorPanel::apply (b, Op::silence, 100, 200);
        check (b.getMagnitude (0, 100, 100) == 0.0f && b.getSample (0, 99) == 0.5f && b.getSample (0, 200) == 0.5f,
               u ("Silenciar solo el trozo elegido"));
        b = make();
        AudioEditorPanel::apply (b, Op::crop, 100, 300);
        check (b.getNumSamples() == 200, u ("Recortar: se queda solo el trozo"));
        b = make();
        AudioEditorPanel::apply (b, Op::remove, 100, 300);
        check (b.getNumSamples() == 800, u ("Borrar un trozo: lo de después se junta"));
        b = make();
        AudioEditorPanel::apply (b, Op::normalise, 0, 1000);
        check (std::abs (b.getMagnitude (0, 0, 1000) - 0.966f) < 0.01f, u ("Normalizar: sube al máximo sin pasarse"));
        b = make();
        AudioEditorPanel::apply (b, Op::fadeIn, 0, 1000);
        check (b.getSample (0, 0) < 0.01f && b.getSample (0, 999) > 0.45f, u ("Fundido de entrada"));
        b = make();
        for (int k = 0; k < 10; ++k) AudioEditorPanel::apply (b, Op::louder, 0, 1000);
        check (b.getMagnitude (0, 0, 1000) <= 1.0f, u ("Subir mucho el volumen nunca pasa del máximo"));
        b = make();
        AudioEditorPanel::apply (b, Op::remove, -50, 5000);   // fuera de los límites
        check (b.getNumSamples() == 1000, u ("Un trozo imposible no rompe nada"));
    }

    // --- Bases con swing y sidechain ------------------------------------------------------
    std::cout << "Bases con swing y sidechain\n";
    {
        Project p;
        genres::load (p, p.addPattern ("Base House"), "house");
        int withSidechain = 0;
        for (int t = 1; t < numMixerTracks; ++t)
            if ((float) p.getMixerTrack (t).getProperty (ids::sidechain, 0.0f) > 0.3f)
            {
                ++withSidechain;
                std::cout << "    pista " << t << " escucha a la " << (int) p.getMixerTrack (t)[ids::sidechainSource] << "\n";
            }
        check (withSidechain >= 1, u ("La base de House trae sidechain"));
        Project q;
        genres::load (q, q.addPattern ("Base Lo-fi"), "lofi");
        check ((double) q.getState()[ids::swing] > 0.2, u ("La base de Lo-fi trae swing"));
    }

    // --- Sonidos de los juegos ------------------------------------------------------------
    std::cout << "Sonidos de los juegos de oído\n";
    {
        auto engineOwned = std::make_unique<AudioEngine>();
        auto& engine = *engineOwned;
        learn::Progress progress;
        EarGames games (engine, progress);

        int drums = 0, silent = 0;
        for (auto& kit : kits::all())
            for (auto& s : kit.sounds)
            {
                ++drums;
                if (peak (games.renderDrum (kit.id, s.id)) < 0.01f)
                {
                    ++silent;
                    std::cout << "    en silencio: " << kit.id << "/" << s.id << "\n";
                }
            }
        check (silent == 0, juce::String (drums) + u (" sonidos de batería suenan"));

        auto notes = games.renderNotes ({ { 60 }, { 67 } }, 0.75);
        check (peak (notes) > 0.05f && peak (notes) <= 1.0f, "Las dos notas de \"¿Sube o baja?\" suenan sin saturar");
        auto chord = games.renderNotes ({ { 60 }, { 64 }, { 67 }, { 60, 64, 67 } }, 0.42);
        check (peak (chord) > 0.05f && peak (chord) <= 1.0f, "El acorde de \"¿Mayor o menor?\" suena sin saturar");

        for (auto& g : genres::all())
        {
            auto beat = games.renderGenre (g.id);
            const bool ok = beat != nullptr && beat->getNumSamples() > 44100 && peak (beat) > 0.05f && peak (beat) <= 1.0f
                            && beat->getRMSLevel (0, 0, beat->getNumSamples()) > 0.01f;
            check (ok, u ("La base de ") + genres::nameOf (g.id) + u (" suena en \"¿Qué género es?\""));
        }
    }

    // --- Progreso, racha y logros ----------------------------------------------------------
    std::cout << "Progreso y logros\n";
    {
        auto file = juce::File::createTempFile (".settings");
        juce::PropertiesFile::Options o;
        juce::PropertiesFile settings (file, o);
        learn::Progress progress;
        progress.setSettings (&settings);
        juce::StringArray unlocked;
        progress.onUnlocked = [&] (const learn::Achievement& a) { unlocked.add (a.id); };

        check (progress.currentStreak() == 0, "Sin practicar: racha 0");
        progress.setLessonDone (u ("Qué es una DAW"), true);
        check (progress.isLessonDone (u ("Qué es una DAW")) && progress.numLessonsDone() == 1, "Lección marcada como aprendida");
        check (progress.currentStreak() == 1 && progress.practicedToday(), "Racha de 1 día al practicar hoy");
        check (unlocked.contains ("primer_paso"), "Logro \"Primer paso\" al aprender una lección");

        progress.recordQuiz ("BPM y tempo", 2, 3);
        progress.recordQuiz ("BPM y tempo", 1, 3);
        check (progress.bestQuizScore ("BPM y tempo") == 2, "Se guarda la mejor nota del test");
        progress.recordQuiz ("BPM y tempo", 3, 3);
        check (progress.numPerfectQuizzes() == 1 && unlocked.contains ("test_perfecto"), "Test perfecto y su logro");

        progress.setChallengeDone ("bpm");
        progress.setChallengeDone ("bpm");
        check (progress.numChallengesDone() == 1 && unlocked.contains ("primer_reto"), "Reto contado una sola vez y su logro");

        progress.recordGameStreak ("altura", 5);
        progress.recordGameStreak ("altura", 3);
        check (progress.bestGameStreak ("altura") == 5 && unlocked.contains ("oido_fino"), "Récord del juego y logro \"Oído fino\"");

        // Racha: ayer practicó y llevaba 6 días -> hoy son 7
        settings.setValue ("learnLastDay", (juce::Time::getCurrentTime() - juce::RelativeTime::days (1)).formatted ("%Y-%m-%d"));
        settings.setValue ("learnStreak", 6);
        progress.setLessonDone ("BPM y tempo", true);
        check (progress.currentStreak() == 7 && unlocked.contains ("racha_7"), "La racha sube a 7 días y da su logro");

        // Si pasa un día sin practicar, la racha se pierde
        settings.setValue ("learnLastDay", (juce::Time::getCurrentTime() - juce::RelativeTime::days (2)).formatted ("%Y-%m-%d"));
        check (progress.currentStreak() == 0, "Un día sin practicar: la racha vuelve a 0");
        check (progress.bestStreak() == 7, "La mejor racha se recuerda");

        const int before = unlocked.size();
        progress.setLessonDone (u ("Qué es una DAW"), true);
        check (unlocked.size() == before, "Los logros no se repiten");
        file.deleteFile();
    }

    // --- Studoo Pro: qué es gratis y qué es de pago --------------------------------------
    std::cout << "Studoo Pro (tienda simulada)\n";
    {
        auto file = juce::File::createTempFile (".settings");
        juce::PropertiesFile::Options o;
        juce::PropertiesFile settings (file, o);
        pro::start (&settings);
        juce::StringArray events;
        pro::setEventHandler ([&] (const juce::String& e) { events.add (e); });

        check (pro::hasStore() && ! pro::isUnlocked(), "Versión gratis: Pro no está desbloqueado");
        check (pro::isKitFree ("trap") && pro::isKitFree ("lofi") && ! pro::isKitFree ("drill") && ! pro::isKitFree ("fx"),
               "5 kits gratis; el resto y los FX son Pro");
        check (pro::isSynthPresetFree (0) && ! pro::isSynthPresetFree (6) && pro::is808PresetFree (1) && ! pro::is808PresetFree (2),
               "Algunos sintes y 808 gratis, el resto Pro");
        check (pro::isGenreFree ("house") && ! pro::isGenreFree ("drill"), "3 bases gratis; el resto Pro");
        check (pro::isLearnLevelFree (3) && ! pro::isLearnLevelFree (4) && ! pro::areGamesFree() && ! pro::areAchievementsFree(),
               "Aprende: niveles 1-4 gratis; 5-9, juegos y logros Pro");
        int freeKits = 0;
        for (auto& k : kits::all())
            freeKits += pro::isKitFree (k.id) ? 1 : 0;
        check (freeKits == 5, "Hay exactamente 5 kits gratis");

        pro::buy (pro::donationProducts[0]);
        check (! pro::isUnlocked() && events.contains ("donacion_ok"), "Donar no desbloquea Pro, pero da las gracias");
        pro::buy (pro::proProduct);
        check (pro::isUnlocked() && events.contains ("pro_comprado"), "Comprar Pro lo desbloquea");
        check (pro::isKitFree ("drill") && pro::isGenreFree ("drill") && pro::isLearnLevelFree (8) && pro::areGamesFree(),
               "Con Pro, todo está desbloqueado");
        check (settings.getBoolValue ("proUnlocked", false), "Pro se recuerda (para usarlo sin internet)");
        pro::setEventHandler (nullptr);
        file.deleteFile();
    }

    std::cout << (failures == 0 ? "\nTodas las pruebas de Aprende han pasado.\n" : "\nHay pruebas que han fallado.\n");
    return failures == 0 ? 0 : 1;
}
