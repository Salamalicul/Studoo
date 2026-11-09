#pragma once
// ============================================================================
//  MixerView: el mezclador, con una tira vertical por cada pista.
// ============================================================================
#include "AppContext.h"
#include "HelpButton.h"
#include "Widgets.h"

namespace studoo
{
    class MixerStrip : public juce::Component
    {
    public:
        MixerStrip (AppContext&, int trackIndex);

        void bind();
        void updateMeter();
        void paint (juce::Graphics&) override;
        void resized() override;
        void mouseDoubleClick (const juce::MouseEvent&) override;

    private:
        Knob* makeKnob (const juce::String& label, const juce::String& tip);

        AppContext& ctx;
        const int index;
        juce::OwnedArray<Knob> knobs;
        Knob *eqLow, *eqMid, *eqHigh, *compThreshold, *compRatio, *delayMix, *delayTime, *delayFeedback,
             *reverbMix, *reverbSize, *pan;
        juce::ToggleButton compOn;
        juce::TextButton muteButton, soloButton, pluginButton;
        juce::Slider fader;
        float meterL = 0.0f, meterR = 0.0f;
        juce::Rectangle<int> meterArea, nameArea;
    };

    class MixerView : public juce::Component,
                      private juce::ChangeListener,
                      private juce::Timer
    {
    public:
        explicit MixerView (AppContext&);
        ~MixerView() override;

        void paint (juce::Graphics&) override;
        void resized() override;
        void rebindAll() { for (auto* s : strips) s->bind(); }

    private:
        void changeListenerCallback (juce::ChangeBroadcaster*) override;
        void timerCallback() override;

        AppContext& ctx;
        HelpButton helpButton;
        juce::Label infoLabel;
        juce::Viewport viewport;
        juce::Component stripHolder;
        juce::OwnedArray<MixerStrip> strips;
        juce::Rectangle<int> headerArea;
        juce::uint32 lastGeneration = 0;
    };
}
