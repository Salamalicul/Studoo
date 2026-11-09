#pragma once
// ============================================================================
//  AppContext: las piezas que comparten todas las pantallas de Studoo.
//  Así cada vista puede leer el proyecto, hacer sonar notas, etc.
// ============================================================================
#include <juce_gui_basics/juce_gui_basics.h>
#include "model/Project.h"
#include "audio/AudioEngine.h"
#include "audio/PluginHost.h"
#include "audio/SampleCache.h"

namespace studoo
{
    enum class View { channelRack = 0, pianoRoll, playlist, mixer, automation, learn, numViews };

    struct AppContext
    {
        Project& project;
        AudioEngine& engine;
        SampleCache& samples;
        PluginHost& plugins;
        juce::PropertiesFile& settings;

        std::function<void (View)> showView;
        std::function<void (int channelUid)> addPluginChannel;  // abre el menú de plugins
        std::function<void (int track)> showMixerPluginMenu;
    };
}
