# ============================================================================
#  Studoo - "receta" de compilación (CMake)
#  CMake lee este archivo y prepara todo lo necesario para compilar la app
#  en Windows, Linux (y en el futuro Mac).
# ============================================================================
cmake_minimum_required(VERSION 3.22)

# En Windows: incluir las librerías de C++ dentro del .exe, así no hace falta
# instalar nada extra para abrir la app.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(Studoo VERSION 0.1.0 LANGUAGES C CXX)

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
    src/audio/AudioEngine.cpp
    src/audio/DrumSounds.cpp)

# ----------------------------------------------------------------------------
#  La aplicación
# ----------------------------------------------------------------------------
juce_add_gui_app(Studoo
    PRODUCT_NAME "Studoo"
    COMPANY_NAME "Studoo"
    VERSION ${PROJECT_VERSION})

target_sources(Studoo PRIVATE
    src/Main.cpp
    src/MainComponent.cpp
    src/ui/StudooLookAndFeel.cpp
    src/ui/TransportBar.cpp
    src/ui/StepSequencer.cpp
    src/ui/HelpButton.cpp
    ${STUDOO_ENGINE_SOURCES})

target_include_directories(Studoo PRIVATE src)

set(STUDOO_DEFINITIONS
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_USE_MP3AUDIOFORMAT=1
    JUCE_ALSA=1
    JUCE_JACK=1
    JUCE_DISPLAY_SPLASH_SCREEN=0
    STUDOO_VERSION="${PROJECT_VERSION}")

target_compile_definitions(Studoo PRIVATE
    ${STUDOO_DEFINITIONS}
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:Studoo,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:Studoo,JUCE_VERSION>")

target_link_libraries(Studoo
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_devices
        juce::juce_audio_formats
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
        PRIVATE juce::juce_audio_formats juce::juce_audio_basics
        PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
    if(MSVC)
        target_compile_options(StudooEngineTest PRIVATE /utf-8)
    endif()
endif()
