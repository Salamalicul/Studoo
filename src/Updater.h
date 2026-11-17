#pragma once
// ============================================================================
//  Updater: busca versiones nuevas de Studoo en GitHub (Releases), las
//  descarga y las instala.
//   - Windows (instalador): ejecuta el instalador nuevo en modo silencioso.
//     Al terminar, el instalador vuelve a abrir Studoo.
//   - Linux (.deb): instala el paquete pidiendo la contraseña (pkexec).
//   - Linux (instalar.sh, en tu carpeta personal): descomprime y reinstala.
//   - Android: descarga el APK y abre el instalador de Android (hay que confirmar).
//   - Otros casos (zip, compilado a mano): abre la página de descargas.
//  Todas las funciones que usan internet tardan: llámalas desde un hilo aparte.
// ============================================================================
#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>

namespace studoo::updater
{
    inline constexpr const char* releasesPage = "https://github.com/Salamalicul/Studoo/releases/latest";

    struct ReleaseInfo
    {
        juce::String version;   // "1.5.0"
        juce::String notes;     // novedades de esa versión (texto sencillo)
        juce::String assetUrl;  // archivo a descargar para este ordenador (vacío si no hay)
        juce::String assetName;
        juce::String sha256;    // huella del archivo que da GitHub (para comprobar la descarga)
    };

    // Cómo se instaló Studoo en este ordenador
    enum class InstallKind { windowsInstaller, linuxDeb, linuxUser, androidApk, manual };
    InstallKind detectInstallKind();

    // true si 'remote' (por ejemplo "1.5.0") es más nueva que 'local'
    bool isNewer (const juce::String& remote, const juce::String& local);

    // Consulta la última versión publicada. Devuelve false si no hay internet.
    bool fetchLatest (ReleaseInfo& info, juce::String& error);

    // Seguridad: solo se descargan archivos de la release oficial de Studoo en
    // GitHub, por HTTPS y con un nombre normal (sin carpetas ni "..").
    bool isOfficialAssetUrl (const juce::String& url);
    bool isSafeAssetName (const juce::String& name);

    // Carpeta privada del usuario donde se guardan las descargas
    juce::File downloadFolder();

    // Descarga un archivo y comprueba que su huella SHA-256 es la que da GitHub.
    // Devuelve false si falla o si el archivo no es exactamente el publicado.
    bool download (const juce::String& url, const juce::String& sha256, const juce::File& destination, juce::String& error);

    // Linux: instala el archivo descargado y espera a que termine.
    bool installOnLinux (InstallKind, const juce::File& downloaded, juce::String& error);

    // Windows: lanza el instalador en modo silencioso (después hay que cerrar Studoo).
    bool launchWindowsInstaller (const juce::File& installer);

    // Android: abre el instalador del sistema con el APK descargado.
    // Devuelve 0 si se abrió, 1 si antes hay que dar permiso para instalar apps
    // (se abre la pantalla de ajustes) y -1 si hubo un error.
    int launchAndroidInstaller (const juce::File& apk);

    // Vuelve a abrir Studoo (después de actualizar en Linux).
    void relaunch();
}
