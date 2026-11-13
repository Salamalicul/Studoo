#include "MainComponent.h"
#include "ui/HelpTexts.h"

namespace studoo
{
namespace
{
    juce::File getStudooFolder (const juce::String& sub)
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("Studoo").getChildFile (sub);
        dir.createDirectory();
        return dir;
    }

    void showMessage (const juce::String& title, const juce::String& message)
    {
        juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                          .withIconType (juce::MessageBoxIconType::NoIcon)
                                          .withTitle (title)
                                          .withMessage (message)
                                          .withButton (txt ("Vale")),
                                      nullptr);
    }

    // ------------------------------------------------------------------------
    //  Exportación en segundo plano, con barra de progreso
    // ------------------------------------------------------------------------
    class ExportJob : public juce::ThreadWithProgressWindow
    {
    public:
        ExportJob (AudioEngine& e, juce::AudioDeviceManager& dm, juce::File targetFile, juce::File lameExe,
                   std::function<void (bool, const juce::String&)> onDone)
            : ThreadWithProgressWindow (txt ("Exportando tu canción..."), true, true),
              engine (e), deviceManager (dm), target (std::move (targetFile)), lame (std::move (lameExe)),
              done (std::move (onDone))
        {
            deviceManager.removeAudioCallback (&engine);
        }

        void run() override
        {
            const bool mp3 = lame.existsAsFile();
            const auto wav = mp3 ? juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("studoo_export.wav")
                                 : target;

            setStatusMessage (txt ("Generando el audio..."));
            ok = engine.renderSongToFile (wav, 44100.0, [this, mp3] (float p)
            {
                setProgress (p * (mp3 ? 0.8 : 1.0));
                return ! threadShouldExit();
            }, error);

            if (ok && mp3)
            {
                setStatusMessage (txt ("Convirtiendo a MP3..."));
                setProgress (0.85);
                target.deleteFile();
                juce::ChildProcess process;
                ok = process.start (juce::StringArray { lame.getFullPathName(), "-b", "320", "--quiet",
                                                        wav.getFullPathName(), target.getFullPathName() });
                if (ok)
                {
                    process.waitForProcessToFinish (600000);
                    ok = process.getExitCode() == 0 && target.existsAsFile();
                }
                if (! ok)
                    error = txt ("LAME no pudo convertir el archivo a MP3.");
                wav.deleteFile();
            }
        }

        void threadComplete (bool userPressedCancel) override
        {
            deviceManager.addAudioCallback (&engine);
            done (ok && ! userPressedCancel, userPressedCancel ? txt ("Exportación cancelada.") : error);
            delete this;
        }

    private:
        AudioEngine& engine;
        juce::AudioDeviceManager& deviceManager;
        juce::File target, lame;
        std::function<void (bool, const juce::String&)> done;
        bool ok = false;
        juce::String error;
    };
}

// ============================================================================
//  Construcción
// ============================================================================
MainComponent::MainComponent()
{
    setLookAndFeel (&lookAndFeel);
    juce::LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);

    juce::PropertiesFile::Options options;
    options.applicationName     = "Studoo";
   #if JUCE_LINUX
    options.folderName          = ".config/Studoo"; // carpeta oculta de ajustes, como el resto de programas
   #else
    options.folderName          = "Studoo";
   #endif
    options.filenameSuffix      = ".settings";
    options.osxLibrarySubFolder = "Application Support";
    settings = std::make_unique<juce::PropertiesFile> (options);

    pluginHost = std::make_unique<PluginHost> (*settings);
    ctx.reset (new AppContext { project, engine, samples, *pluginHost, *settings, {}, {}, {} });
    ctx->showView = [this] (View v) { showView (v); };
    ctx->showMixerPluginMenu = [this] (int track) { showMixerPluginMenu (track); };

    // --- Pantallas ----------------------------------------------------------------
    toolbar = std::make_unique<Toolbar> (*ctx);
    toolbar->onFileMenu = [this] { showFileMenu(); };
    toolbar->onAudioSettings = [this] { showAudioSettings(); };
    toolbar->onPlugins = [this] { pluginHost->showScanner (this); };
    toolbar->onRecord = [this] { toggleRecording(); };
    addAndMakeVisible (*toolbar);

    channelRack = std::make_unique<ChannelRack> (*ctx);
    pianoRoll = std::make_unique<PianoRoll> (*ctx);
    playlist = std::make_unique<Playlist> (*ctx);
    mixerView = std::make_unique<MixerView> (*ctx);
    automationView = std::make_unique<AutomationView> (*ctx);
    learnView = std::make_unique<LearnView>();
    learnView->onStartTutorial = [this] { startTutorial(); };
    learnView->onGoToView = [this] (View v) { showView (v); };
    learnView->setSettings (settings.get());

    views = { channelRack.get(), pianoRoll.get(), playlist.get(), mixerView.get(), automationView.get(), learnView.get() };
    for (auto* v : views)
        addChildComponent (v);

    const char* viewNames[] = { "Secuenciador (F6)", "Piano roll (F7)", "Playlist (F5)",
                                "Mezclador (F9)", "Automatización (F8)", "Aprende (F1)" };
    const char* viewTips[] = { "Secuenciador de patrones: crea ritmos paso a paso.",
                               "Piano roll: escribe melodías, acordes y 808 nota a nota.",
                               "Playlist: coloca los patrones en el tiempo para montar la canción.",
                               "Mezclador: volumen, panorama y efectos de cada pista.",
                               "Automatización: haz que un control se mueva solo a lo largo de la canción.",
                               "Aprende: explicaciones sencillas de producción musical y el tutorial." };
    for (int i = 0; i < (int) View::numViews; ++i)
    {
        auto* b = viewButtons.add (new juce::TextButton (txt (viewNames[i])));
        b->setTooltip (txt (viewTips[i]));
        b->setClickingTogglesState (false);
        b->getProperties().set ("tab", true);
        b->getProperties().set ("tabIcon", i);
        b->onClick = [this, i] { showView ((View) i); };
        makeNonFocusing (*b);
        addAndMakeVisible (b);
    }

    addChildComponent (tutorial);
    tutorial.onFinished = [this]
    {
        settings->setValue ("tutorialDone", true);
        settings->saveIfNeeded();
        grabKeyboardFocus();
    };

    // --- Audio -------------------------------------------------------------------------
    engine.onPluginWillBeDeleted = [] (juce::AudioPluginInstance* p) { PluginWindows::closeFor (p); };
    project.addChangeListener (this);
    engine.updateFromProject (project.getState(), samples);
    engine.setLiveChannel (project.getSelectedChannel());
    lastGeneration = project.getGeneration();

    auto savedAudio = settings->getXmlValue ("audioDevice");
    auto error = deviceManager.initialise (2, 2, savedAudio.get(), true);
    if (error.isNotEmpty())
        deviceManager.initialise (0, 2, nullptr, true);
    deviceManager.addAudioCallback (&engine);

    // Teclados MIDI: activamos todos los que estén conectados
    for (auto& midiInput : juce::MidiInput::getAvailableDevices())
        deviceManager.setMidiInputDeviceEnabled (midiInput.identifier, true);
    deviceManager.addMidiInputDeviceCallback ({}, &engine.getMidiCollector());
    deviceManager.addChangeListener (this);

    setWantsKeyboardFocus (true);
    setSize (1320, 820);
    showView (View::channelRack);
    updateTitle();
    startTimer (1000);

    juce::Component::SafePointer<MainComponent> safe (this);
    juce::Timer::callAfterDelay (700, [safe] { if (safe != nullptr) safe->checkAutosaveRecovery(); });

    // Buscar una versión nueva de Studoo unos segundos después de abrir
    juce::Timer::callAfterDelay (6000, [safe]
    {
        if (safe != nullptr && safe->settings->getBoolValue ("autoUpdate", true) && ! safe->tutorial.isRunning())
            safe->checkForUpdates (false);
    });
}

MainComponent::~MainComponent()
{
    PluginWindows::closeAll();
    project.removeChangeListener (this);
    deviceManager.removeChangeListener (this);
    deviceManager.removeMidiInputDeviceCallback ({}, &engine.getMidiCollector());
    deviceManager.removeAudioCallback (&engine);
    engine.stopRecording();

    if (auto xml = deviceManager.createStateXml())
        settings->setValue ("audioDevice", xml.get());
    settings->saveIfNeeded();

    tutorial.setVisible (false);
    juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
    setLookAndFeel (nullptr);
}

// ============================================================================
//  Cambios del proyecto y de la tarjeta de sonido
// ============================================================================
void MainComponent::changeListenerCallback (juce::ChangeBroadcaster* source)
{
    if (source == &deviceManager)
    {
        if (auto xml = deviceManager.createStateXml())
            settings->setValue ("audioDevice", xml.get());
        repaint (statusArea);
        return;
    }

    engine.updateFromProject (project.getState(), samples);
    engine.setLiveChannel (project.getSelectedChannel());

    if (project.getGeneration() != lastGeneration)
        afterProjectLoaded();

    restorePlugins();
    updateTitle();
}

void MainComponent::afterProjectLoaded()
{
    lastGeneration = project.getGeneration();
    pendingPlugins.clear();
    failedPlugins.clear();
    toolbar->bindToProject();
    mixerView->rebindAll();
}

// ============================================================================
//  Plugins
// ============================================================================
void MainComponent::storePluginStates()
{
    for (auto ch : project.channels())
        if (auto* p = engine.getChannelPlugin (ch[ids::uid]))
        {
            juce::MemoryBlock mb;
            p->getStateInformation (mb);
            ch.setProperty (ids::pluginState, mb.toBase64Encoding(), nullptr);
        }

    for (int t = 0; t < numMixerTracks; ++t)
        if (auto* p = engine.getMixerPlugin (t))
        {
            juce::MemoryBlock mb;
            p->getStateInformation (mb);
            project.getMixerTrack (t).setProperty (ids::pluginState, mb.toBase64Encoding(), nullptr);
        }
}

void MainComponent::restorePlugins()
{
    juce::Component::SafePointer<MainComponent> safe (this);

    auto load = [this, safe] (int key, const juce::String& desc, const juce::String& state,
                              std::function<void (std::unique_ptr<juce::AudioPluginInstance>)> install)
    {
        if (desc.isEmpty() || pendingPlugins.count (key) > 0 || failedPlugins.count (key) > 0)
            return;
        pendingPlugins.insert (key);
        const auto generation = project.getGeneration();

        pluginHost->createInstanceFromXml (desc, engine.getSampleRate(), engine.getBlockSize(),
            [safe, key, state, install, generation] (std::unique_ptr<juce::AudioPluginInstance> instance, const juce::String&)
        {
            if (safe == nullptr)
                return;
            safe->pendingPlugins.erase (key);
            if (instance == nullptr || generation != safe->project.getGeneration())
            {
                safe->failedPlugins.insert (key);
                return;
            }
            if (state.isNotEmpty())
            {
                juce::MemoryBlock mb;
                if (mb.fromBase64Encoding (state))
                    instance->setStateInformation (mb.getData(), (int) mb.getSize());
            }
            install (std::move (instance));
        });
    };

    for (auto ch : project.channels())
    {
        const int uid = ch[ids::uid];
        if (ch[ids::type].toString() == "plugin" && engine.getChannelPlugin (uid) == nullptr)
            load (uid, ch[ids::pluginDesc].toString(), ch[ids::pluginState].toString(),
                  [this, uid] (std::unique_ptr<juce::AudioPluginInstance> p)
                  {
                      engine.setChannelPlugin (uid, std::move (p));
                      channelRack->repaint();
                  });
    }

    for (int t = 0; t < numMixerTracks; ++t)
    {
        auto track = project.getMixerTrack (t);
        if (engine.getMixerPlugin (t) == nullptr)
            load (-1000 - t, track[ids::pluginDesc].toString(), track[ids::pluginState].toString(),
                  [this, t] (std::unique_ptr<juce::AudioPluginInstance> p)
                  {
                      engine.setMixerPlugin (t, std::move (p));
                      mixerView->rebindAll();
                  });
    }
}

void MainComponent::showMixerPluginMenu (int track)
{
    auto* current = engine.getMixerPlugin (track);
    const auto effects = pluginHost->getEffects();

    juce::PopupMenu menu;
    if (current != nullptr)
    {
        menu.addItem (1, txt ("Abrir ") + current->getName());
        menu.addItem (2, txt ("Quitar plugin"));
        menu.addSeparator();
    }
    juce::PopupMenu list;
    for (int i = 0; i < effects.size(); ++i)
        list.addItem (100 + i, effects[i].name + "  (" + effects[i].manufacturerName + ")");
    if (effects.isEmpty())
        list.addItem (-1, txt ("No hay efectos: pulsa \"Plugins\" arriba para buscarlos"), false);
    menu.addSubMenu (current != nullptr ? txt ("Cambiar por") : txt ("Añadir efecto VST3"), list);

    menu.showMenuAsync (juce::PopupMenu::Options(), [this, track, effects] (int result)
    {
        auto trackTree = project.getMixerTrack (track);
        if (result == 1)
        {
            PluginWindows::show (engine.getMixerPlugin (track), trackTree[ids::name].toString());
        }
        else if (result == 2)
        {
            engine.setMixerPlugin (track, nullptr);
            trackTree.setProperty (ids::pluginDesc, "", nullptr);
            trackTree.setProperty (ids::pluginState, "", nullptr);
            mixerView->rebindAll();
        }
        else if (result >= 100 && result - 100 < effects.size())
        {
            const auto desc = effects[result - 100];
            juce::Component::SafePointer<MainComponent> safe (this);
            pluginHost->createInstance (desc, engine.getSampleRate(), engine.getBlockSize(),
                [safe, track, desc] (std::unique_ptr<juce::AudioPluginInstance> instance, const juce::String& error)
            {
                if (safe == nullptr)
                    return;
                if (instance == nullptr)
                {
                    showMessage ("Plugin", txt ("No se pudo abrir el plugin: ") + error);
                    return;
                }
                auto trackTree = safe->project.getMixerTrack (track);
                if (auto xml = desc.createXml())
                    trackTree.setProperty (ids::pluginDesc, xml->toString (juce::XmlElement::TextFormat().singleLine()), nullptr);
                trackTree.setProperty (ids::pluginState, "", nullptr);
                auto* raw = instance.get();
                safe->engine.setMixerPlugin (track, std::move (instance));
                safe->mixerView->rebindAll();
                PluginWindows::show (raw, desc.name);
            });
        }
    });
}

// ============================================================================
//  Archivo
// ============================================================================
void MainComponent::showFileMenu()
{
    juce::PopupMenu menu;
    menu.addItem (1, txt ("Nuevo proyecto"), true, false);
    menu.addItem (2, txt ("Abrir proyecto..."));
    menu.addItem (3, txt ("Guardar"));
    menu.addItem (4, txt ("Guardar como..."));
    menu.addSeparator();
    menu.addItem (5, txt ("Exportar a WAV..."));
    menu.addItem (6, txt ("Exportar a MP3..."));
    menu.addSeparator();
    menu.addItem (7, txt ("Tutorial: tu primer beat"));
    menu.addItem (8, txt ("Atajos de teclado"));
    menu.addItem (9, txt ("Abrir la carpeta de proyectos"));
    menu.addSeparator();
    menu.addItem (11, txt ("Buscar actualizaciones"));
    menu.addItem (12, txt ("Buscar actualizaciones al abrir Studoo"), true, settings->getBoolValue ("autoUpdate", true));
    menu.addSeparator();
    menu.addItem (10, txt ("Salir"));

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&toolbar->getFileButton()), [this] (int r)
    {
        switch (r)
        {
            case 1:  newProject(); break;
            case 2:  openProject(); break;
            case 3:  saveProject(); break;
            case 4:  saveProjectAs(); break;
            case 5:  exportSong (false); break;
            case 6:  exportSong (true); break;
            case 7:  startTutorial(); break;
            case 8:  showMessage (txt (help::shortcutsTitle), txt (help::shortcutsText)); break;
            case 9:  getStudooFolder ("Proyectos").startAsProcess(); break;
            case 10: juce::JUCEApplication::getInstance()->systemRequestedQuit(); break;
            case 11: checkForUpdates (true); break;
            case 12:
                settings->setValue ("autoUpdate", ! settings->getBoolValue ("autoUpdate", true));
                settings->saveIfNeeded();
                break;
            default: break;
        }
    });
}

void MainComponent::checkUnsavedThen (std::function<void()> action)
{
    if (! project.hasUnsavedChanges())
    {
        action();
        return;
    }

    juce::Component::SafePointer<MainComponent> safe (this);
    juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                      .withIconType (juce::MessageBoxIconType::NoIcon)
                                      .withTitle (txt ("Cambios sin guardar"))
                                      .withMessage (txt ("¿Quieres guardar los cambios de \"") + project.getDisplayName() + "\"?")
                                      .withButton ("Guardar")
                                      .withButton (txt ("No guardar"))
                                      .withButton ("Cancelar"),
                                  [safe, action] (int result)
    {
        if (safe == nullptr)
            return;
        if (result == 1)      safe->saveProject (action);
        else if (result == 2) action();
    });
}

void MainComponent::newProject()
{
    checkUnsavedThen ([this]
    {
        engine.stop();
        engine.clearAllPlugins();
        PluginWindows::closeAll();
        project.resetToDefault();
        project.setFile ({});
        updateTitle();
        setStatus (txt ("Proyecto nuevo"));
    });
}

void MainComponent::openProject()
{
    checkUnsavedThen ([this]
    {
        chooser = std::make_unique<juce::FileChooser> (txt ("Abrir proyecto de Studoo"), getStudooFolder ("Proyectos"), "*.studoo");
        chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                              [this] (const juce::FileChooser& fc)
        {
            auto f = fc.getResult();
            if (! f.existsAsFile())
                return;
            engine.stop();
            engine.clearAllPlugins();
            PluginWindows::closeAll();
            juce::String error;
            if (project.loadFromFile (f, error))
                setStatus (txt ("Proyecto abierto: ") + f.getFileName());
            else
                showMessage (txt ("No se pudo abrir"), error);
            updateTitle();
        });
    });
}

void MainComponent::saveProject (std::function<void()> then)
{
    if (project.getFile() == juce::File())
    {
        saveProjectAs (then);
        return;
    }

    storePluginStates();
    if (project.saveToFile (project.getFile()))
    {
        setStatus (txt ("Guardado: ") + project.getFile().getFullPathName());
        updateTitle();
        if (then)
            then();
    }
    else
    {
        showMessage (txt ("No se pudo guardar"), txt ("No se puede escribir en ") + project.getFile().getFullPathName());
    }
}

void MainComponent::saveProjectAs (std::function<void()> then)
{
    auto initial = project.getFile() != juce::File() ? project.getFile()
                                                     : getStudooFolder ("Proyectos").getChildFile ("Mi beat.studoo");
    chooser = std::make_unique<juce::FileChooser> (txt ("Guardar proyecto"), initial, "*.studoo");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this, then] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult();
        if (f == juce::File())
            return;
        project.setFile (f.withFileExtension (".studoo"));
        saveProject (then);
    });
}

juce::File MainComponent::findLame()
{
    const auto exeDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile).getParentDirectory();
   #if JUCE_WINDOWS
    const juce::String name = "lame.exe";
    const auto separator = ";";
   #else
    const juce::String name = "lame";
    const auto separator = ":";
   #endif

    juce::Array<juce::File> candidates { exeDir.getChildFile (name), juce::File ("/usr/bin/lame"),
                                         juce::File ("/usr/local/bin/lame"), juce::File ("/opt/homebrew/bin/lame") };
    for (auto& dir : juce::StringArray::fromTokens (juce::SystemStats::getEnvironmentVariable ("PATH", {}), separator, ""))
        if (dir.isNotEmpty() && juce::File::isAbsolutePath (dir))
            candidates.add (juce::File (dir).getChildFile (name));

    for (auto& f : candidates)
        if (f.existsAsFile())
            return f;
    return {};
}

void MainComponent::exportSong (bool mp3)
{
    if (project.playlist().getNumChildren() == 0)
    {
        showMessage (txt ("La playlist está vacía"),
                     txt ("Se exporta la canción de la PLAYLIST. Coloca primero tus patrones en la playlist (F5)."));
        return;
    }

    juce::File lame;
    if (mp3)
    {
        lame = findLame();
        if (! lame.existsAsFile())
        {
           #if JUCE_WINDOWS
            const auto how = txt ("1. Descarga \"LAME for Windows\" (por ejemplo, desde rarewares.org, sección \"LAME Bundles\").\n"
                                  "2. Descomprime el archivo.\n"
                                  "3. Copia \"lame.exe\" en la misma carpeta que Studoo.exe.\n"
                                  "4. Vuelve a intentarlo.");
           #else
            const auto how = txt ("Abre una terminal y escribe:\n\n"
                                  "  Ubuntu/Debian/Mint:  sudo apt install lame\n"
                                  "  Fedora:  sudo dnf install lame\n"
                                  "  Arch/Manjaro:  sudo pacman -S lame\n\n"
                                  "Luego vuelve a intentarlo.");
           #endif
            showMessage (txt ("Falta LAME (el conversor a MP3)"),
                         txt ("Para exportar a MP3, Studoo usa el programa gratuito LAME, que no está instalado.\n\n") + how
                             + txt ("\n\nMientras tanto, puedes exportar a WAV."));
            return;
        }
    }

    const auto ext = mp3 ? juce::String (".mp3") : juce::String (".wav");
    chooser = std::make_unique<juce::FileChooser> (mp3 ? txt ("Exportar a MP3") : txt ("Exportar a WAV"),
                                                   getStudooFolder ("Exportados").getChildFile (project.getDisplayName() + ext),
                                                   "*" + ext);
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles
                              | juce::FileBrowserComponent::warnAboutOverwriting,
                          [this, lame, ext] (const juce::FileChooser& fc)
    {
        auto f = fc.getResult();
        if (f == juce::File())
            return;
        f = f.withFileExtension (ext);
        engine.stop();

        juce::Component::SafePointer<MainComponent> safe (this);
        auto* job = new ExportJob (engine, deviceManager, f, lame, [safe, f] (bool ok, const juce::String& error)
        {
            if (ok)
            {
                showMessage (txt ("¡Canción exportada!"), txt ("Tu canción se ha guardado en:\n") + f.getFullPathName());
                if (safe != nullptr) safe->setStatus (txt ("Exportado: ") + f.getFileName());
            }
            else
            {
                showMessage (txt ("No se pudo exportar"), error);
            }
        });
        job->launchThread();
    });
}

// ============================================================================
//  Grabación
// ============================================================================
void MainComponent::toggleRecording()
{
    if (engine.isRecording())
    {
        auto result = engine.stopRecording();
        engine.stop();
        if (! result.valid)
        {
            setStatus (txt ("No se grabó nada (¿estaba sonando en modo CANCIÓN?)."));
            return;
        }

        project.beginTransaction();
        int count = 1;
        for (auto ch : project.channels())
            if (ch[ids::type].toString() == "audio")
                ++count;
        auto ch = project.addChannel (ChannelType::audio, txt ("Grabación ") + juce::String (count), 7);
        ch.setProperty (ids::sample, result.file.getFullPathName(), project.um());
        project.getMixerTrack (7).setProperty (ids::name, "Voz", project.um());

        // Buscamos una pista de la playlist libre en ese momento
        const double start = result.startSteps, end = result.startSteps + result.lengthSteps;
        int freeTrack = numPlaylistTracks - 1;
        for (int t = 0; t < numPlaylistTracks; ++t)
        {
            bool used = false;
            for (auto clip : project.playlist())
                if ((int) clip[ids::track] == t && (double) clip[ids::start] < end
                    && (double) clip[ids::start] + (double) clip[ids::length] > start)
                    used = true;
            if (! used) { freeTrack = t; break; }
        }
        project.addClip ("audio", ch[ids::uid], freeTrack, start, juce::jmax (1.0, result.lengthSteps));
        setStatus (txt ("Grabación añadida a la playlist."));
        showView (View::playlist);
        return;
    }

    auto folder = project.getFile() != juce::File() ? project.getFile().getParentDirectory().getChildFile ("Grabaciones")
                                                    : getStudooFolder ("Grabaciones");
    folder.createDirectory();
    auto file = folder.getNonexistentChildFile ("Grabacion_" + juce::Time::getCurrentTime().formatted ("%Y%m%d_%H%M%S"), ".wav");

    juce::String error;
    project.getState().setProperty (ids::playMode, "song", nullptr);
    if (! engine.startRecording (file, error))
    {
        showMessage (txt ("No se puede grabar"), error);
        return;
    }
    if (! engine.isPlaying())
        engine.play();
    setStatus (txt ("GRABANDO... Pulsa R o el círculo rojo para terminar."));
}

// ============================================================================
//  Vistas, audio, tutorial
// ============================================================================
void MainComponent::showView (View v)
{
    currentView = v;
    for (int i = 0; i < (int) View::numViews; ++i)
    {
        views[(size_t) i]->setVisible (i == (int) v);
        viewButtons[i]->setToggleState (i == (int) v, juce::dontSendNotification);
    }
    grabKeyboardFocus();
}

void MainComponent::showAudioSettings()
{
    auto* selector = new juce::AudioDeviceSelectorComponent (deviceManager, 0, 2, 2, 2, true, false, true, false);
    selector->setSize (600, 520);

    juce::DialogWindow::LaunchOptions o;
    o.content.setOwned (selector);
    o.dialogTitle = txt ("Ajustes de audio y MIDI");
    o.dialogBackgroundColour = Colours::panel;
    o.escapeKeyTriggersCloseButton = true;
    o.useNativeTitleBar = true;
    o.resizable = true;
    o.componentToCentreAround = this;
    o.launchAsync();
}

void MainComponent::startTutorial()
{
    engine.stop();
    auto kick = project.findChannelByBuiltin ("builtin:kick");
    auto snare = project.findChannelByBuiltin ("builtin:snare");
    auto hat = project.findChannelByBuiltin ("builtin:hihat");
    auto bass = project.findChannelByType (ChannelType::bass808);

    if (! kick.isValid() || ! snare.isValid() || ! hat.isValid() || ! bass.isValid())
    {
        showMessage ("Tutorial", txt ("El tutorial usa el kit incluido (Bombo, Caja, Hi-hat y 808). "
                                      "Crea un proyecto nuevo (Archivo > Nuevo proyecto) y vuelve a empezarlo."));
        return;
    }

    // Empezamos en un patrón vacío (si el actual ya está vacío, lo usamos)
    project.beginTransaction();
    auto pattern = project.getCurrentPattern();
    if (pattern.getNumChildren() > 0)
        pattern = project.addPattern();
    pattern.setProperty (ids::name, txt ("Mi primer beat"), project.um());
    project.setCurrentPattern (pattern[ids::uid]);
    project.getState().setProperty (ids::playMode, "pattern", nullptr);

    const int kickUid = kick[ids::uid], snareUid = snare[ids::uid], hatUid = hat[ids::uid], bassUid = bass[ids::uid];
    auto hasSteps = [this] (int uid, std::initializer_list<int> steps)
    {
        auto p = project.getCurrentPattern();
        for (int s : steps)
            if (! project.getStep (p, uid, s))
                return false;
        return true;
    };
    auto countSteps = [this] (int uid)
    {
        int n = 0;
        auto p = project.getCurrentPattern();
        for (int s = 0; s < Project::getPatternLength (p); ++s)
            n += project.getStep (p, uid, s) ? 1 : 0;
        return n;
    };
    auto rowOf = [this] (int uid) { return project.getChannelIndex (uid); };
    auto startBpm = std::make_shared<double> (0.0);
    auto startClips = std::make_shared<int> (0);

    std::vector<TutorialStep> steps;
    steps.push_back ({ txt ("¡Bienvenido a Studoo!"),
                       txt ("Vamos a hacer juntos tu primer beat, paso a paso. Tardarás unos 5 minutos.\n\n"
                            "Harás un ritmo de batería, una línea de 808 y montarás una pequeña canción.\n\n"
                            "Puedes salir cuando quieras y repetir el tutorial desde Archivo o desde Aprende."),
                       [this] { showView (View::channelRack); }, nullptr, nullptr });
    steps.push_back ({ txt ("La barra de arriba"),
                       txt ("Aquí están los controles principales: archivo, deshacer, reproducir, grabar, BPM (velocidad), "
                            "metrónomo y volumen.\n\nTRUCO: deja el ratón quieto encima de cualquier botón y aparecerá una "
                            "explicación. Y cada sección tiene un botón \"?\" naranja con su ayuda."),
                       nullptr, [this] { return toolbar.get(); }, nullptr });
    steps.push_back ({ txt ("El secuenciador"),
                       txt ("Esta cuadrícula es el secuenciador. Cada FILA es un sonido y cada CUADRADO es un paso. "
                            "16 pasos = 1 compás.\n\nCuando reproduces, una luz recorre los pasos de izquierda a derecha y "
                            "suenan los que estén encendidos."),
                       [this] { showView (View::channelRack); }, [this] { return &channelRack->getGridComponent(); }, nullptr });
    steps.push_back ({ txt ("1. El bombo"),
                       txt ("El bombo es el golpe grave (\"pum\") que marca el pulso.\n\nHaz clic en los pasos 1, 5, 9 y 13 "
                            "de la fila \"Bombo\" (la primera)."),
                       nullptr, [this] { return &channelRack->getGridComponent(); },
                       [hasSteps, kickUid] { return hasSteps (kickUid, { 0, 4, 8, 12 }); } });
    steps.push_back ({ txt ("2. La caja"),
                       txt ("La caja es el golpe seco (\"pa\"). Va en los tiempos 2 y 4.\n\nHaz clic en los pasos 5 y 13 de la "
                            "fila \"Caja\"."),
                       nullptr, [this] { return &channelRack->getGridComponent(); },
                       [hasSteps, snareUid] { return hasSteps (snareUid, { 4, 12 }); } });
    steps.push_back ({ txt ("3. Los hi-hats"),
                       txt ("El hi-hat es el \"tss\" metálico que da movimiento.\n\nEnciende al menos 8 pasos de la fila "
                            "\"Hi-hat\". TRUCO: mantén pulsado el ratón y arrastra para pintar varios seguidos."),
                       nullptr, [this] { return &channelRack->getGridComponent(); },
                       [countSteps, hatUid] { return countSteps (hatUid) >= 8; } });
    steps.push_back ({ txt ("4. ¡Dale al Play!"),
                       txt ("Pulsa el triángulo verde (o la BARRA ESPACIADORA) para escuchar tu ritmo.\n\n"
                            "Si no oyes nada, revisa el volumen del ordenador o pulsa \"Audio\" para elegir tus altavoces."),
                       nullptr, [this] { return &toolbar->getPlayButton(); }, [this] { return engine.isPlaying(); } });
    steps.push_back ({ txt ("5. La velocidad (BPM)"),
                       txt ("El BPM es la velocidad. Arrastra el número del BPM hacia la izquierda o la derecha y escucha "
                            "cómo cambia.\n\nTrap: 130-150. Reggaetón: 90-100. Hip-hop: 85-95."),
                       [this, startBpm] { *startBpm = project.getState()[ids::bpm]; },
                       [this] { return &toolbar->getBpmControl(); },
                       [this, startBpm] { return ! juce::approximatelyEqual ((double) project.getState()[ids::bpm], *startBpm); } });
    steps.push_back ({ txt ("6. El 808 en el piano roll"),
                       txt ("Ahora el bajo 808. Las notas se escriben en el PIANO ROLL: más arriba = más agudo, más a la "
                            "derecha = más tarde.\n\nHaz clic para añadir al menos 3 notas, cerca de Do2 y Do3 (las notas "
                            "graves). Arrastra el borde derecho de una nota para alargarla."),
                       [this, bassUid] { project.setSelectedChannel (bassUid); showView (View::pianoRoll); },
                       [this] { return &pianoRoll->getGridComponent(); },
                       [this, bassUid]
                       {
                           int n = 0;
                           for (auto item : project.getCurrentPattern())
                               n += (item.hasType (ids::NOTE) && (int) item[ids::channel] == bassUid) ? 1 : 0;
                           return n >= 3;
                       } });
    steps.push_back ({ txt ("7. La playlist"),
                       txt ("La playlist es donde montas la canción completa colocando patrones en el tiempo.\n\n"
                            "Arriba está elegido tu patrón \"Mi primer beat\". Haz clic en 2 huecos vacíos de la Pista 1 "
                            "para colocarlo varias veces."),
                       [this, startClips] { engine.stop(); showView (View::playlist); *startClips = project.playlist().getNumChildren(); },
                       [this] { return &playlist->getGridComponent(); },
                       [this, startClips] { return project.playlist().getNumChildren() >= *startClips + 2; } });
    steps.push_back ({ txt ("8. Modo CANCIÓN"),
                       txt ("Para escuchar la playlist, pulsa CANCIÓN (o la tecla L). Luego dale a Play.\n\n"
                            "En modo PATRÓN solo se repite el patrón que estás editando."),
                       nullptr, [this] { return &toolbar->getModeButtons(); },
                       [this] { return project.getState()[ids::playMode].toString() == "song"; } });
    steps.push_back ({ txt ("9. El mezclador"),
                       txt ("Aquí ajustas el volumen de cada sonido y le añades efectos: EQ, compresor, delay y reverb.\n\n"
                            "Prueba a subir la REVERB de la pista \"Caja\" mientras suena la canción."),
                       [this] { showView (View::mixer); }, [this] { return (juce::Component*) mixerView.get(); }, nullptr });
    steps.push_back ({ txt ("10. Guarda y exporta"),
                       txt ("Abre el menú Archivo (las tres rayas) para guardar tu proyecto (Ctrl+S) y para exportar tu "
                            "canción a WAV o MP3 y escucharla en el móvil.\n\nStudoo también guarda una copia automática "
                            "cada minuto."),
                       nullptr, [this] { return &toolbar->getFileButton(); }, nullptr });
    steps.push_back ({ txt ("¡Enhorabuena! 🎉"),
                       txt ("¡Has hecho tu primer beat!\n\nEn \"Aprende\" tienes explicaciones de BPM, 808, acordes, "
                            "mezcla, efectos y estructura de canciones.\n\nRecuerda: botón \"?\" en cada sección y "
                            "mensajes de ayuda al pasar el ratón."),
                       [this] { engine.stop(); showView (View::learn); }, nullptr, nullptr });

    juce::ignoreUnused (rowOf);
    tutorial.setBounds (getLocalBounds());
    tutorial.start (std::move (steps));
}

void MainComponent::updateTitle()
{
    if (auto* window = findParentComponentOfClass<juce::DocumentWindow>())
        window->setName ("Studoo - " + project.getDisplayName() + (project.hasUnsavedChanges() ? " *" : ""));
}

// ============================================================================
//  Actualizaciones automáticas
// ============================================================================
void MainComponent::checkForUpdates (bool userAsked)
{
    if (updateBusy)
    {
        if (userAsked)
            setStatus (txt ("Ya se está buscando o instalando una actualización..."));
        return;
    }

    updateBusy = true;
    if (userAsked)
        setStatus (txt ("Buscando actualizaciones..."));

    juce::Component::SafePointer<MainComponent> safe (this);
    juce::Thread::launch ([safe, userAsked]
    {
        updater::ReleaseInfo info;
        juce::String error;
        const bool ok = updater::fetchLatest (info, error);

        juce::MessageManager::callAsync ([safe, userAsked, ok, info, error]
        {
            if (safe == nullptr)
                return;
            safe->updateBusy = false;
            if (userAsked)
                safe->setStatus ({});

            if (! ok)
            {
                if (userAsked)
                    showMessage (txt ("Buscar actualizaciones"), error);
                return;
            }
            if (! updater::isNewer (info.version, STUDOO_VERSION))
            {
                if (userAsked)
                    showMessage (txt ("Studoo está al día"),
                                 txt ("Ya tienes la última versión de Studoo (") + juce::String (STUDOO_VERSION) + ").");
                return;
            }
            if (! userAsked && safe->settings->getValue ("skipVersion") == info.version)
                return;
            safe->offerUpdate (info);
        });
    });
}

void MainComponent::offerUpdate (const updater::ReleaseInfo& info)
{
    const auto kind = updater::detectInstallKind();
    const bool automatic = kind != updater::InstallKind::manual && info.assetUrl.isNotEmpty();

    juce::String message = txt ("Hay una versión nueva: Studoo ") + info.version
                         + txt (" (tú tienes la ") + juce::String (STUDOO_VERSION) + ").";
    if (info.notes.isNotEmpty())
        message << "\n\n" << txt ("Novedades:") << "\n" << info.notes;
    message << "\n\n";
    if (! automatic)
        message << txt ("Se abrirá la página de descargas para que bajes la versión nueva.");
    else if (kind == updater::InstallKind::windowsInstaller)
        message << txt ("Studoo la descargará, se cerrará un momento para instalarla y se volverá a abrir solo. "
                        "Tus proyectos no se tocan.");
    else if (kind == updater::InstallKind::linuxDeb)
        message << txt ("Studoo la descargará y la instalará. Te pedirá tu contraseña. Tus proyectos no se tocan.");
    else
        message << txt ("Studoo la descargará y la instalará. Tus proyectos no se tocan.");

    juce::Component::SafePointer<MainComponent> safe (this);
    juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                      .withIconType (juce::MessageBoxIconType::NoIcon)
                                      .withTitle (txt ("¡Studoo se puede actualizar!"))
                                      .withMessage (message)
                                      .withButton (automatic ? txt ("Actualizar ahora") : txt ("Ir a descargas"))
                                      .withButton (txt ("Saltar esta versión"))
                                      .withButton (txt ("Más tarde")),
                                  [safe, info, automatic] (int result)
    {
        if (safe == nullptr)
            return;
        if (result == 1)
        {
            if (automatic) safe->downloadAndInstall (info);
            else           juce::URL (updater::releasesPage).launchInDefaultBrowser();
        }
        else if (result == 2)
        {
            safe->settings->setValue ("skipVersion", info.version);
            safe->settings->saveIfNeeded();
        }
    });
}

void MainComponent::downloadAndInstall (const updater::ReleaseInfo& info)
{
    if (updateBusy)
        return;
    updateBusy = true;
    setStatus (txt ("Descargando Studoo ") + info.version + txt ("... puedes seguir usando la app."));
    statusTimeout = 100000;

    const auto kind = updater::detectInstallKind();
    juce::Component::SafePointer<MainComponent> safe (this);
    juce::Thread::launch ([safe, info, kind]
    {
        auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("StudooActualizacion");
        dir.deleteRecursively();
        dir.createDirectory();
        const auto file = dir.getChildFile (info.assetName);

        juce::String error;
        bool ok = updater::download (info.assetUrl, file, error);
        if (ok && kind != updater::InstallKind::windowsInstaller)
        {
            juce::MessageManager::callAsync ([safe]
            {
                if (safe != nullptr)
                {
                    safe->setStatus (txt ("Instalando la actualización..."));
                    safe->statusTimeout = 100000;
                }
            });
            ok = updater::installOnLinux (kind, file, error);
        }

        juce::MessageManager::callAsync ([safe, info, kind, file, ok, error]
        {
            if (safe == nullptr)
                return;
            safe->updateBusy = false;
            safe->setStatus ({});
            safe->statusTimeout = 0;

            if (! ok)
            {
                showMessage (txt ("No se pudo actualizar"),
                             error + txt ("\n\nPuedes descargar la versión nueva a mano desde la página de descargas "
                                          "(menú Archivo → Buscar actualizaciones → Ir a descargas)."));
                return;
            }

            if (kind == updater::InstallKind::windowsInstaller)
            {
                // Pregunta si hay que guardar, lanza el instalador y cierra Studoo
                safe->requestQuit ([file]
                {
                    if (updater::launchWindowsInstaller (file))
                        juce::JUCEApplication::getInstance()->quit();
                    else
                        showMessage (txt ("No se pudo actualizar"), txt ("No se pudo abrir el instalador."));
                });
                return;
            }

            juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                              .withIconType (juce::MessageBoxIconType::NoIcon)
                                              .withTitle (txt ("¡Studoo ") + info.version + txt (" instalado!"))
                                              .withMessage (txt ("Reinicia Studoo para usar la versión nueva."))
                                              .withButton (txt ("Reiniciar ahora"))
                                              .withButton (txt ("Más tarde")),
                                          [safe] (int result)
            {
                if (safe != nullptr && result == 1)
                    safe->requestQuit ([]
                    {
                        updater::relaunch();
                        juce::JUCEApplication::getInstance()->quit();
                    });
            });
        });
    });
}

void MainComponent::setStatus (const juce::String& text)
{
    statusMessage = text;
    statusTimeout = 6;
    repaint (statusArea);
}

// ============================================================================
//  Autoguardado
// ============================================================================
juce::File MainComponent::getAutosaveFile() const
{
    return settings->getFile().getSiblingFile ("Autoguardado.studoo");
}

void MainComponent::writeAutosave()
{
    if (! project.hasUnsavedChanges() || project.getVersion() == lastAutosaveVersion)
        return;

    storePluginStates();
    lastAutosaveVersion = project.getVersion();
    getAutosaveFile().replaceWithText (project.toXmlString());
    settings->setValue ("autosaveProjectPath", project.getFile().getFullPathName());
    settings->saveIfNeeded();
}

void MainComponent::checkAutosaveRecovery()
{
    auto autosave = getAutosaveFile();
    if (autosave.existsAsFile())
    {
        juce::Component::SafePointer<MainComponent> safe (this);
        juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                          .withIconType (juce::MessageBoxIconType::NoIcon)
                                          .withTitle (txt ("Recuperar trabajo"))
                                          .withMessage (txt ("Studoo no se cerró correctamente la última vez, pero hay una copia "
                                                             "automática de tu trabajo.\n\n¿Quieres recuperarla?"))
                                          .withButton ("Recuperar")
                                          .withButton (txt ("No, empezar de cero")),
                                      [safe, autosave] (int result)
        {
            if (safe == nullptr)
                return;
            if (result == 1)
            {
                juce::String error;
                if (safe->project.loadFromXmlString (autosave.loadFileAsString(), error))
                {
                    const auto path = safe->settings->getValue ("autosaveProjectPath");
                    safe->project.setFile (path.isNotEmpty() ? juce::File (path) : juce::File());
                    safe->setStatus (txt ("Trabajo recuperado. Guárdalo con Ctrl+S."));
                    safe->updateTitle();
                }
            }
            autosave.deleteFile();
        });
        return;
    }

    if (! settings->getBoolValue ("tutorialDone", false))
        startTutorial();
}

void MainComponent::requestQuit (std::function<void()> quit)
{
    if (tutorial.isRunning())
        tutorial.stop();

    juce::Component::SafePointer<MainComponent> safe (this);
    checkUnsavedThen ([safe, quit]
    {
        if (safe != nullptr)
            safe->getAutosaveFile().deleteFile();
        quit();
    });
}

void MainComponent::timerCallback()
{
    if (++autosaveCounter >= 60)
    {
        autosaveCounter = 0;
        writeAutosave();
    }

    if (statusTimeout > 0 && --statusTimeout == 0 && ! engine.isRecording())
    {
        statusMessage.clear();
        repaint (statusArea);
    }
}

// ============================================================================
//  Dibujo, diseño y teclado
// ============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // Fondo con un brillo naranja muy suave arriba a la izquierda
    g.fillAll (Colours::background);
    g.setGradientFill (juce::ColourGradient (Colours::accent.withAlpha (0.06f), 0.0f, 0.0f,
                                             juce::Colours::transparentBlack, (float) getWidth() * 0.6f, (float) getHeight() * 0.7f, true));
    g.fillAll();

    // Barra de pestañas
    g.setColour (Colours::panelDark.withAlpha (0.7f));
    g.fillRoundedRectangle (viewBarArea.toFloat().expanded (4.0f, 3.0f), 10.0f);

    // Barra de estado
    g.setGradientFill (juce::ColourGradient (Colours::header, 0.0f, (float) statusArea.getY(),
                                             Colours::panelDark, 0.0f, (float) statusArea.getBottom(), false));
    g.fillRect (statusArea);
    g.setColour (Colours::outline.withAlpha (0.7f));
    g.drawHorizontalLine (statusArea.getY(), 0.0f, (float) getWidth());
    g.setFont (Fonts::regular (12.5f));

    auto area = statusArea.reduced (12, 0);
    if (auto* device = deviceManager.getCurrentAudioDevice())
    {
        // Cápsula con un punto verde: el audio funciona
        const auto text = deviceManager.getCurrentAudioDeviceType() + "  ·  " + device->getName() + "  ·  "
                          + juce::String ((int) device->getCurrentSampleRate()) + " Hz";
        const float w = juce::GlyphArrangement::getStringWidth (Fonts::regular (12.0f), text) + 34.0f;
        auto pill = area.removeFromRight ((int) w + 2).toFloat().reduced (0.0f, 4.0f);
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        g.fillRoundedRectangle (pill, pill.getHeight() * 0.5f);
        auto dot = pill.removeFromLeft (pill.getHeight()).withSizeKeepingCentre (7.0f, 7.0f).translated (4.0f, 0.0f);
        Draw::glow (g, dot, Colours::green, 4.0f, 3.5f);
        g.setColour (Colours::green);
        g.fillEllipse (dot);
        g.setColour (Colours::textDim);
        g.setFont (Fonts::regular (12.0f));
        g.drawText (text, pill.withTrimmedLeft (4.0f), juce::Justification::centredLeft);
        g.setFont (Fonts::regular (12.5f));
    }
    else
    {
        g.setColour (Colours::accent);
        g.drawText (txt ("Sin audio: pulsa \"Audio\""), area.removeFromRight (300), juce::Justification::centredRight);
    }

    g.setColour (statusMessage.isNotEmpty() ? Colours::accent : Colours::textDim);
    g.drawText (statusMessage.isNotEmpty() ? statusMessage
                                           : txt ("Espacio: play/stop  ·  Ctrl+Z: deshacer  ·  Ctrl+S: guardar  ·  "
                                                  "Pasa el ratón por encima de un control para ver qué hace"),
                area, juce::Justification::centredLeft);
}

void MainComponent::resized()
{
    auto area = getLocalBounds();
    toolbar->setBounds (area.removeFromTop (58));
    statusArea = area.removeFromBottom (26);

    viewBarArea = area.removeFromTop (40).reduced (10, 6);
    {
        auto bar = viewBarArea;
        const int w = juce::jmin (175, bar.getWidth() / (int) View::numViews);
        for (auto* b : viewButtons)
        {
            b->setBounds (bar.removeFromLeft (w - 4));
            bar.removeFromLeft (4);
        }
    }

    auto content = area.reduced (10, 4).withTrimmedBottom (4);
    for (auto* v : views)
        v->setBounds (content);

    tutorial.setBounds (getLocalBounds());
}

bool MainComponent::keyPressed (const juce::KeyPress& key)
{
    const auto mods = key.getModifiers();
    const int code = key.getKeyCode();
    const auto upper = juce::CharacterFunctions::toUpperCase ((juce::juce_wchar) code);
    const bool cmd = mods.isCommandDown();

    if (key == juce::KeyPress::spaceKey)               { toolbar->togglePlayStop(); return true; }
    if (cmd && upper == 'Z' && mods.isShiftDown())     { project.getUndo().redo(); return true; }
    if (cmd && upper == 'Z')                           { project.getUndo().undo(); return true; }
    if (cmd && upper == 'Y')                           { project.getUndo().redo(); return true; }
    if (cmd && upper == 'S' && mods.isShiftDown())     { saveProjectAs(); return true; }
    if (cmd && upper == 'S')                           { saveProject(); return true; }
    if (cmd && upper == 'O')                           { openProject(); return true; }
    if (cmd && upper == 'N')                           { newProject(); return true; }
    if (cmd && upper == 'E')                           { exportSong (false); return true; }
    if (cmd && upper == 'M')
    {
        auto& s = project.getState();
        s.setProperty (ids::metronome, ! (bool) s[ids::metronome], nullptr);
        return true;
    }
    if (! cmd && ! mods.isAltDown() && upper == 'L')
    {
        auto& s = project.getState();
        s.setProperty (ids::playMode, s[ids::playMode].toString() == "song" ? "pattern" : "song", nullptr);
        return true;
    }
    if (! cmd && ! mods.isAltDown() && upper == 'R')   { toggleRecording(); return true; }

    if (code == juce::KeyPress::F5Key) { showView (View::playlist); return true; }
    if (code == juce::KeyPress::F6Key) { showView (View::channelRack); return true; }
    if (code == juce::KeyPress::F7Key) { showView (View::pianoRoll); return true; }
    if (code == juce::KeyPress::F8Key) { showView (View::automation); return true; }
    if (code == juce::KeyPress::F9Key) { showView (View::mixer); return true; }
    if (code == juce::KeyPress::F1Key) { showView (View::learn); return true; }
    return false;
}

void MainComponent::parentHierarchyChanged()
{
    updateTitle();
    if (isShowing())
        grabKeyboardFocus();
}
}
