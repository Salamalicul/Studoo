#pragma once
// ============================================================================
//  MainComponent: todo lo que hay dentro de la ventana principal.
//  Une el motor de audio con la tarjeta de sonido y con la interfaz.
// ============================================================================
#include <juce_audio_utils/juce_audio_utils.h>

#include "audio/AudioEngine.h"
#include "ui/StudooLookAndFeel.h"
#include "ui/TransportBar.h"
#include "ui/StepSequencer.h"

namespace studoo
{
    class MainComponent : public juce::Component,
                          private juce::ChangeListener
    {
    public:
        MainComponent();
        ~MainComponent() override;

        void paint (juce::Graphics&) override;
        void resized() override;
        bool keyPressed (const juce::KeyPress&) override;
        void parentHierarchyChanged() override;

    private:
        void initialiseAudio();
        void showAudioSettings();
        void saveAudioSettings();
        void changeListenerCallback (juce::ChangeBroadcaster*) override;

        StudooLookAndFeel lookAndFeel;
        juce::TooltipWindow tooltipWindow { this, 600 }; // aparece tras 0,6 s

        AudioEngine engine;
        juce::AudioDeviceManager deviceManager;
        juce::AudioSourcePlayer audioPlayer;
        std::unique_ptr<juce::PropertiesFile> settings;

        TransportBar transportBar { engine };
        StepSequencer sequencer { engine };

        juce::Rectangle<int> statusArea;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
    };
}
