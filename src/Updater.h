#pragma once
// ============================================================================
//  Updater: busca versiones nuevas de Studoo en GitHub (Releases), las
//  descarga y las instala.
//   - Windows (instalador): ejecuta el instalador nuevo en modo silencioso.
//     Al terminar, el instalador vuelve a abrir Studoo.
//   - Linux (.deb): instala el paquete pidiendo la contraseña (pkexec).
//   - Linux (instalar.sh, en tu carpeta personal): descomprime y reinstala.
//   - Otros casos (zip, compilado a mano): abre la página de descargas.
//  Todas las funciones que usan internet tardan: llámalas desde un hilo aparte.
// ============================================================================
#include <juce_core/juce_core.h>

namespace studoo::updater
{
    inline constexpr const char* releasesPage = "https://github.com/Salamalicul/Studoo/releases/latest";

    struct ReleaseInfo
    {
        juce::String version;   // "1.5.0"
        juce::String notes;     // novedades de esa versión (texto sencillo)
        juce::String assetUrl;  // archivo a descargar para este ordenador (vacío si no hay)
        juce::String assetName;
    };

    // Cómo se instaló Studoo en este ordenador
    enum class InstallKind { windowsInstaller, linuxDeb, linuxUser, manual };
    InstallKind detectInstallKind();

    // true si 'remote' (por ejemplo "1.5.0") es más nueva que 'local'
    bool isNewer (const juce::String& remote, const juce::String& local);

    // Consulta la última versión publicada. Devuelve false si no hay internet.
    bool fetchLatest (ReleaseInfo& info, juce::String& error);

    // Descarga un archivo. Devuelve false si falla.
    bool download (const juce::String& url, const juce::File& destination, juce::String& error);

    // Linux: instala el archivo descargado y espera a que termine.
    bool installOnLinux (InstallKind, const juce::File& downloaded, juce::String& error);

    // Windows: lanza el instalador en modo silencioso (después hay que cerrar Studoo).
    bool launchWindowsInstaller (const juce::File& installer);

    // Vuelve a abrir Studoo (después de actualizar en Linux).
    void relaunch();
}
