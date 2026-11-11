#pragma once
// ============================================================================
//  Toolbar: la barra superior de Studoo.
// ============================================================================
#include "AppContext.h"
#include "HelpButton.h"
#include "Widgets.h"

namespace studoo
{
    class Toolbar : public juce::Component,
                    private juce::Timer,
                    private juce::ChangeListener
    {
    public:
        explicit Toolbar (AppContext&);
        ~Toolbar() override;

        std::function<void()> onFileMenu, onAudioSettings, onPlugins, onRecord;

        void togglePlayStop();
        void bindToProject();

        void paint (juce::Graphics&) override;
        void resized() override;

        juce::Component& getPlayButton()      { return playButton; }
        juce::Component& getBpmControl()      { return bpmSlider; }
        juce::Component& getModeButtons()     { return modeArea; }
        juce::Component& getRecordButton()    { return recordButton; }
        juce::Component& getFileButton()      { return fileButton; }

    private:
        void timerCallback() override;
        void changeListenerCallback (juce::ChangeBroadcaster*) override;
        void updateModeButtons();

        AppContext& ctx;

        IconButton fileButton   { IconButton::Icon::menu, Colours::accent };
        IconButton undoButton   { IconButton::Icon::undo, Colours::text };
        IconButton redoButton   { IconButton::Icon::redo, Colours::text };
        juce::Component modeArea;
        juce::TextButton patternModeButton, songModeButton;
        IconButton playButton   { IconButton::Icon::play, Colours::green };
        IconButton stopButton   { IconButton::Icon::stop, Colours::accent };
        IconButton recordButton { IconButton::Icon::record, juce::Colour (0xffff4040) };
        juce::Label bpmLabel;
        juce::Slider bpmSlider;
        juce::Label positionLabel;
        juce::TextButton metronomeButton;
        Knob volumeKnob { "Volumen", txt ("Volumen general (Master)") };
        juce::TextButton audioButton, pluginsButton;
        HelpButton helpButton;

        juce::Rectangle<int> logoArea, transportArea, meterArea;
        float meterL = 0.0f, meterR = 0.0f;
    };
}
