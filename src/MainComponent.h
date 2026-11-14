#pragma once
// ============================================================================
//  MainComponent: todo lo que hay dentro de la ventana principal.
//  Une el proyecto, el motor de audio, la tarjeta de sonido y las pantallas.
// ============================================================================
#include <juce_audio_utils/juce_audio_utils.h>
#include <set>

#include "model/Project.h"
#include "audio/AudioEngine.h"
#include "audio/PluginHost.h"
#include "audio/SampleCache.h"
#include "ui/AppContext.h"
#include "ui/AutomationView.h"
#include "ui/ChannelRack.h"
#include "ui/LearnView.h"
#include "ui/MixerView.h"
#include "ui/PianoRoll.h"
#include "ui/Playlist.h"
#include "ui/StudooLookAndFeel.h"
#include "ui/Toolbar.h"
#include "ui/Tutorial.h"
#include "Updater.h"

namespace studoo
{
    class MainComponent : public juce::Component,
                          private juce::ChangeListener,
                          private juce::Timer
    {
    public:
        MainComponent();
        ~MainComponent() override;

        void paint (juce::Graphics&) override;
        void resized() override;
        bool keyPressed (const juce::KeyPress&) override;
        void parentHierarchyChanged() override;

        // Al cerrar la ventana: pregunta si quieres guardar y luego llama a 'quit'
        void requestQuit (std::function<void()> quit);

    private:
        // --- Archivo -----------------------------------------------------------
        void showFileMenu();
        void newProject();
        void openProject();
        void saveProject (std::function<void()> then = {});
        void saveProjectAs (std::function<void()> then = {});
        void checkUnsavedThen (std::function<void()> action);
        void exportSong (bool mp3);
        static juce::File findLame();
        void afterProjectLoaded();
        void storePluginStates();
        void restorePlugins();

        // --- Otros ----------------------------------------------------------------
        void showView (View);
        void showAudioSettings();
        void toggleRecording();
        void showMixerPluginMenu (int track);
        void startTutorial();
        void checkAutosaveRecovery();
        void writeAutosave();
        void updateTitle();
        void setStatus (const juce::String&);
        juce::File getAutosaveFile() const;

        // --- Tablets (Android) ------------------------------------------------------
        void shareFile (const juce::File&);
        void importProject();
        LongPressRightClick longPress;          // mantener pulsado = clic derecho
        juce::ScopedMessageBox shareBox;

        // --- Actualizaciones automáticas ----------------------------------------
        void checkForUpdates (bool userAsked);
        void offerUpdate (const updater::ReleaseInfo&);
        void downloadAndInstall (const updater::ReleaseInfo&);

        void changeListenerCallback (juce::ChangeBroadcaster*) override;
        void timerCallback() override;

        StudooLookAndFeel lookAndFeel;
        juce::TooltipWindow tooltipWindow { this, 900 };

        std::unique_ptr<juce::PropertiesFile> settings;
        Project project;
        SampleCache samples;
        AudioEngine engine;
        juce::AudioDeviceManager deviceManager;
        std::unique_ptr<PluginHost> pluginHost;
        std::unique_ptr<AppContext> ctx;

        std::unique_ptr<Toolbar> toolbar;
        juce::OwnedArray<juce::TextButton> viewButtons;
        std::unique_ptr<ChannelRack> channelRack;
        std::unique_ptr<PianoRoll> pianoRoll;
        std::unique_ptr<Playlist> playlist;
        std::unique_ptr<MixerView> mixerView;
        std::unique_ptr<AutomationView> automationView;
        std::unique_ptr<LearnView> learnView;
        std::array<juce::Component*, (size_t) View::numViews> views {};
        View currentView = View::channelRack;
        TutorialOverlay tutorial;

        std::unique_ptr<juce::FileChooser> chooser;
        std::set<int> pendingPlugins, failedPlugins;
        juce::uint32 lastAutosaveVersion = 0, lastGeneration = 0;
        int autosaveCounter = 0;
        juce::String statusMessage;
        int statusTimeout = 0;
        bool updateBusy = false;
        juce::Rectangle<int> statusArea, viewBarArea;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
    };
}
