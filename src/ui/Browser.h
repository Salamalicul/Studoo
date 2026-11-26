#pragma once
// ============================================================================
//  Browser: el navegador de sonidos (panel a la izquierda). Un clic escucha el
//  sonido; doble clic lo añade al secuenciador (o abre el proyecto).
// ============================================================================
#include "AppContext.h"
#include "Widgets.h"

namespace studoo
{
    class BrowserPanel : public juce::Component
    {
    public:
        explicit BrowserPanel (AppContext&);
        ~BrowserPanel() override;

        void paint (juce::Graphics&) override;
        void resized() override;
        void refresh();

        std::function<void (const juce::File&)> onOpenProject;
        std::function<void (const juce::String&)> onStatus;

        // Lo usan los elementos de la lista
        void preview (const juce::String& samplePath);
        void addSample (const juce::String& samplePath, const juce::String& name, const juce::String& kitId);
        void addSynthPreset (int index);
        void add808Preset (int index);
        AppContext& ctx;

    private:
        juce::TreeView tree;
        std::unique_ptr<juce::TreeViewItem> root;
        juce::TextButton addFolderButton;
        std::unique_ptr<juce::FileChooser> chooser;
    };
}
