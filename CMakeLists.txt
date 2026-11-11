# ============================================================================
#  Studoo - "receta" de compilación (CMake)
#  CMake lee este archivo y prepara todo lo necesario para compilar la app
#  en Windows, Linux (y en el futuro Mac).
# ============================================================================
cmake_minimum_required(VERSION 3.22)

# En Windows: incluir las librerías de C++ dentro del .exe, así no hace falta
# instalar nada extra para abrir la app.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(Studoo VERSION 1.3.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release)
endif()

option(STUDOO_BUILD_TESTS "Compilar también las pruebas automáticas" OFF)

# ----------------------------------------------------------------------------
#  JUCE: se descarga automáticamente la primera vez (no tienes que instalarlo).
#  Si ya lo tienes descargado, puedes indicar su carpeta con:
#     cmake -B build -DSTUDOO_JUCE_DIR=/ruta/a/JUCE
# ----------------------------------------------------------------------------
set(STUDOO_JUCE_DIR "" CACHE PATH "Carpeta local de JUCE (opcional)")

if(STUDOO_JUCE_DIR)
    add_subdirectory(${STUDOO_JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        URL https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.9.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

# Código del motor de audio (lo comparten la app y las pruebas)
set(STUDOO_ENGINE_SOURCES
    src/model/Project.cpp
    src/audio/AudioEngine.cpp
    src/audio/DrumSounds.cpp
    src/audio/Instruments.cpp
    src/audio/Kits.cpp
    src/audio/MixerTrack.cpp
    src/audio/PluginHost.cpp
    src/audio/SampleCache.cpp
    src/audio/Snapshot.cpp)

# ----------------------------------------------------------------------------
#  La aplicación
# ----------------------------------------------------------------------------
juce_add_gui_app(Studoo
    PRODUCT_NAME "Studoo"
    ICON_BIG resources/icon.png
    COMPANY_NAME "Studoo"
    VERSION ${PROJECT_VERSION})

target_sources(Studoo PRIVATE
    src/Main.cpp
    src/MainComponent.cpp
    src/ui/StudooLookAndFeel.cpp
    src/ui/Widgets.cpp
    src/ui/HelpButton.cpp
    src/ui/Toolbar.cpp
    src/ui/ChannelRack.cpp
    src/ui/InstrumentPanel.cpp
    src/ui/PianoRoll.cpp
    src/ui/Playlist.cpp
    src/ui/MixerView.cpp
    src/ui/AutomationView.cpp
    src/ui/LearnView.cpp
    src/ui/Tutorial.cpp
    ${STUDOO_ENGINE_SOURCES})

target_include_directories(Studoo PRIVATE src)

# Imágenes que van dentro de la app (el logo)
juce_add_binary_data(StudooData SOURCES
    resources/icon.png
    resources/fonts/Inter-Regular.otf
    resources/fonts/Inter-SemiBold.otf
    resources/fonts/InterDisplay-Black.otf)

set(STUDOO_DEFINITIONS
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_MP3AUDIOFORMAT=1
    JUCE_PLUGINHOST_VST3=1
    JUCE_PLUGINHOST_LV2=0
    JUCE_PLUGINHOST_AU=0
    STUDOO_VERSION="${PROJECT_VERSION}")

# Sistemas de audio de Linux (ALSA y JACK; PipeWire funciona a través de ambos)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND STUDOO_DEFINITIONS JUCE_ALSA=1 JUCE_JACK=1)
endif()

target_compile_definitions(Studoo PRIVATE
    ${STUDOO_DEFINITIONS}
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Studoo,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Studoo,JUCE_VERSION>")

target_link_libraries(Studoo
    PRIVATE
        StudooData
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# Los textos en español (con tildes y ñ) están en UTF-8
if(MSVC)
    target_compile_options(Studoo PRIVATE /utf-8)
endif()

# ----------------------------------------------------------------------------
#  Pruebas automáticas del motor de audio (opcional)
# ----------------------------------------------------------------------------
if(STUDOO_BUILD_TESTS)
    juce_add_console_app(StudooEngineTest PRODUCT_NAME "StudooEngineTest")
    target_sources(StudooEngineTest PRIVATE tests/EngineTest.cpp ${STUDOO_ENGINE_SOURCES})
    target_include_directories(StudooEngineTest PRIVATE src)
    target_compile_definitions(StudooEngineTest PRIVATE ${STUDOO_DEFINITIONS})
    target_link_libraries(StudooEngineTest
        PRIVATE juce::juce_audio_utils juce::juce_audio_processors juce::juce_dsp juce::juce_gui_extra
        PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
    if(MSVC)
        target_compile_options(StudooEngineTest PRIVATE /utf-8)
    endif()
endif()
