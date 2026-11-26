#pragma once
// ============================================================================
//  InstrumentPanel: los ajustes del canal seleccionado (sampler, sinte, 808...)
// ============================================================================
#include "AppContext.h"
#include "HelpButton.h"
#include "Widgets.h"

namespace studoo
{
    // Forma de onda del sample: arrastra las marcas para elegir dónde empieza y dónde acaba.
    // Muestra también los trozos (chops) y si suena al revés.
    class SampleWaveform : public juce::Component,
                           public juce::SettableTooltipClient,
                           private juce::ValueTree::Listener
    {
    public:
        SampleWaveform (AppContext&, juce::ValueTree channel);
        ~SampleWaveform() override;

        void paint (juce::Graphics&) override;
        void mouseDown (const juce::MouseEvent&) override;
        void mouseDrag (const juce::MouseEvent&) override;

    private:
        void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&) override;
        void rebuildPeaks();
        juce::Rectangle<float> waveArea() const;

        AppContext& ctx;
        juce::ValueTree channel;
        juce::String peaksPath;
        std::vector<float> peaks;   // 0..1, unos cientos de valores
        bool draggingStart = true;
    };

    class InstrumentPanel : public juce::Component
    {
    public:
        explicit InstrumentPanel (AppContext&);

        void setChannel (juce::ValueTree channel);
        void refresh();
        std::function<void (juce::ValueTree)> onLoadSample;

        void paint (juce::Graphics&) override;
        void resized() override;

    private:
        Knob* addKnob (const juce::String& label, const juce::String& tooltip, const juce::Identifier& id,
                       double min, double max, double def, double skewMid = 0.0);

        void showMultisampleMenu (juce::Component& from);
        void loadMultisample (const juce::Array<juce::File>&);
        std::unique_ptr<juce::FileChooser> chooser;

        AppContext& ctx;
        juce::ValueTree channel;
        HelpButton helpButton;
        juce::OwnedArray<juce::Component> controls;
        juce::Label infoLabel;
        juce::String title, typeName;
    };
}
