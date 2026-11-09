#pragma once
// ============================================================================
//  PluginHost: busca, recuerda y abre plugins VST3.
//  La lista de plugins encontrados se guarda en los ajustes de Studoo.
// ============================================================================
#include <juce_audio_processors/juce_audio_processors.h>

namespace studoo
{
    class PluginHost : private juce::ChangeListener
    {
    public:
        explicit PluginHost (juce::PropertiesFile& settings);
        ~PluginHost() override;

        juce::AudioPluginFormatManager& getFormatManager() { return formatManager; }
        juce::KnownPluginList& getKnownPlugins()           { return knownPlugins; }

        juce::Array<juce::PluginDescription> getInstruments() const;
        juce::Array<juce::PluginDescription> getEffects() const;

        using Callback = std::function<void (std::unique_ptr<juce::AudioPluginInstance>, const juce::String& error)>;
        void createInstance (const juce::PluginDescription&, double sampleRate, int blockSize, Callback);
        void createInstanceFromXml (const juce::String& descriptionXml, double sampleRate, int blockSize, Callback);

        // Ventana para buscar plugins en el ordenador
        void showScanner (juce::Component* centreAround);

    private:
        void changeListenerCallback (juce::ChangeBroadcaster*) override;

        juce::PropertiesFile& settings;
        juce::AudioPluginFormatManager formatManager;
        juce::KnownPluginList knownPlugins;
    };

    // ------------------------------------------------------------------------
    //  Ventanas donde se muestran los controles de un plugin
    // ------------------------------------------------------------------------
    class PluginWindows
    {
    public:
        static void show (juce::AudioPluginInstance*, const juce::String& title);
        static void closeFor (juce::AudioProcessor*);
        static void closeAll();
    };
}
