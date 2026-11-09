#pragma once
// ============================================================================
//  InstrumentPanel: los ajustes del canal seleccionado (sampler, sinte, 808...)
// ============================================================================
#include "AppContext.h"
#include "HelpButton.h"
#include "Widgets.h"

namespace studoo
{
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

        AppContext& ctx;
        juce::ValueTree channel;
        HelpButton helpButton;
        juce::OwnedArray<juce::Component> controls;
        juce::Label infoLabel;
        juce::String title, typeName;
    };
}
