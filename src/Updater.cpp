#include "Updater.h"

namespace studoo::updater
{
namespace
{
    constexpr const char* apiUrl = "https://api.github.com/repos/Salamalicul/Studoo/releases/latest";
    constexpr const char* userAgent = "Studoo-Updater";

    juce::File currentExe()
    {
        return juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    }

   #if JUCE_LINUX || JUCE_BSD
    // En Linux usamos curl o wget (vienen con casi todas las distribuciones)
    bool hasCommand (const juce::String& name)
    {
        for (auto dir : juce::StringArray::fromTokens (juce::SystemStats::getEnvironmentVariable ("PATH", "/usr/bin:/bin"), ":", {}))
            if (juce::File (dir).getChildFile (name).existsAsFile())
                return true;
        return false;
    }

    bool runCommand (const juce::StringArray& args, juce::String* output, int timeoutMs, int& exitCode)
    {
        juce::ChildProcess process;
        if (! process.start (args, output != nullptr ? juce::ChildProcess::wantStdOut : 0))
            return false;
        if (output != nullptr)
            *output = process.readAllProcessOutput();
        if (! process.waitForProcessToFinish (timeoutMs))
        {
            process.kill();
            return false;
        }
        exitCode = (int) process.getExitCode();
        return true;
    }

    bool httpGet (const juce::String& url, const juce::File* toFile, juce::String* text, int timeoutSecs)
    {
        // Los archivos se piden a la API de GitHub como datos binarios
        const juce::String accept = toFile != nullptr ? "Accept: application/octet-stream" : "Accept: application/vnd.github+json";
        juce::StringArray args;
        if (hasCommand ("curl"))
        {
            args = { "curl", "-sSfL", "--max-time", juce::String (timeoutSecs), "-A", userAgent, "-H", accept };
            if (toFile != nullptr) args.addArray ({ "-o", toFile->getFullPathName() });
        }
        else if (hasCommand ("wget"))
        {
            args = { "wget", "-q", "--timeout=" + juce::String (timeoutSecs), "--user-agent=" + juce::String (userAgent),
                     "--header=" + accept,
                     "-O", toFile != nullptr ? toFile->getFullPathName() : juce::String ("-") };
        }
        else
        {
            return false;
        }
        args.add (url);

        int exitCode = -1;
        return runCommand (args, text, (timeoutSecs + 5) * 1000, exitCode) && exitCode == 0;
    }
   #else
    bool httpGet (const juce::String& url, const juce::File* toFile, juce::String* text, int timeoutSecs)
    {
        auto stream = juce::URL (url).createInputStream (
            juce::URL::InputStreamOptions (juce::URL::ParameterHandling::inAddress)
                .withExtraHeaders (juce::String ("User-Agent: ") + userAgent + "\r\n"
                                  + (toFile != nullptr ? "Accept: application/octet-stream" : "Accept: application/vnd.github+json"))
                .withConnectionTimeoutMs (timeoutSecs * 1000)
                .withNumRedirectsToFollow (8));
        if (stream == nullptr)
            return false;

        if (text != nullptr)
        {
            *text = stream->readEntireStreamAsString();
            return text->isNotEmpty();
        }

        toFile->deleteFile();
        juce::FileOutputStream out (*toFile);
        if (! out.openedOk())
            return false;
        out.writeFromInputStream (*stream, -1);
        out.flush();
        return toFile->getSize() > 0;
    }
   #endif

    // Saca la sección "Novedades de la versión X" de las notas de la release
    juce::String extractNotes (const juce::String& body)
    {
        auto lines = juce::StringArray::fromLines (body);
        juce::StringArray result;
        bool inside = false;
        for (auto line : lines)
        {
            if (line.startsWith ("###"))
            {
                if (inside) break;
                if (line.containsIgnoreCase ("Novedades")) { inside = true; continue; }
            }
            if (inside && line.trim().isNotEmpty())
            {
                // Quitamos los emojis (la fuente de la app no los tiene)
                juce::String clean;
                for (auto c : line)
                    if (c < 0x2190 || (c >= 0x2200 && c < 0x2300))
                        clean += juce::String::charToString (c);
                line = clean.replace ("  ", " ");
                result.add (line.replace ("**", "").replace ("`", "").replaceFirstOccurrenceOf ("- ", juce::String::fromUTF8 ("• ")).trimEnd());
            }
        }
        auto text = result.joinIntoString ("\n");
        if (text.length() > 900)
            text = text.substring (0, 900) + "...";
        return text;
    }

    juce::String assetSuffix (InstallKind kind)
    {
        switch (kind)
        {
            case InstallKind::windowsInstaller: return "-Instalador-Windows.exe";
            case InstallKind::linuxDeb:         return "-Linux.deb";
            case InstallKind::linuxUser:        return "-Linux.tar.gz";
            case InstallKind::manual:           return {};
        }
        return {};
    }
}

InstallKind detectInstallKind()
{
    const auto exe = currentExe();
   #if JUCE_WINDOWS
    if (exe.getSiblingFile ("unins000.exe").existsAsFile())
        return InstallKind::windowsInstaller;
   #elif JUCE_LINUX || JUCE_BSD
    const auto path = exe.getFullPathName();
    if (path.startsWith ("/opt/studoo/"))
        return InstallKind::linuxDeb;
    if (path.contains ("/.local/share/studoo/"))
        return InstallKind::linuxUser;
   #endif
    juce::ignoreUnused (exe);
    return InstallKind::manual;
}

bool isNewer (const juce::String& remote, const juce::String& local)
{
    auto a = juce::StringArray::fromTokens (remote.trimCharactersAtStart ("vV"), ".", {});
    auto b = juce::StringArray::fromTokens (local.trimCharactersAtStart ("vV"), ".", {});
    for (int i = 0; i < juce::jmax (a.size(), b.size()); ++i)
    {
        const int x = a[i].getIntValue(), y = b[i].getIntValue();
        if (x != y)
            return x > y;
    }
    return false;
}

bool fetchLatest (ReleaseInfo& info, juce::String& error)
{
    juce::String text;
    if (! httpGet (apiUrl, nullptr, &text, 15) || text.isEmpty())
    {
        error = juce::String::fromUTF8 ("No se pudo conectar con GitHub. ¿Tienes internet?");
        return false;
    }

    const auto json = juce::JSON::parse (text);
    const auto tag = json["tag_name"].toString();
    if (tag.isEmpty())
    {
        error = juce::String::fromUTF8 ("GitHub no ha devuelto la información de la última versión.");
        return false;
    }

    info.version = tag.trimCharactersAtStart ("vV");
    info.notes = extractNotes (json["body"].toString());

    const auto suffix = assetSuffix (detectInstallKind());
    if (suffix.isNotEmpty())
        if (auto* assets = json["assets"].getArray())
            for (auto& asset : *assets)
                if (asset["name"].toString().endsWith (suffix))
                {
                    info.assetName = asset["name"].toString();
                    info.assetUrl = asset["url"].toString(); // API: sirve el archivo con "Accept: application/octet-stream"
                }
    return true;
}

bool download (const juce::String& url, const juce::File& destination, juce::String& error)
{
    destination.getParentDirectory().createDirectory();
    destination.deleteFile();
    if (httpGet (url, &destination, nullptr, 600) && destination.getSize() > 100000)
        return true;
    error = juce::String::fromUTF8 ("No se pudo descargar la actualización.");
    return false;
}

bool installOnLinux (InstallKind kind, const juce::File& downloaded, juce::String& error)
{
   #if JUCE_LINUX || JUCE_BSD
    int exitCode = -1;
    if (kind == InstallKind::linuxDeb)
    {
        if (! hasCommand ("pkexec"))
        {
            error = juce::String::fromUTF8 ("Falta \"pkexec\" para pedir la contraseña. Instala el archivo .deb a mano.");
            return false;
        }
        // pkexec muestra la ventana del sistema que pide la contraseña
        if (runCommand ({ "pkexec", "apt-get", "install", "-y", "--allow-downgrades", downloaded.getFullPathName() },
                        nullptr, 15 * 60 * 1000, exitCode) && exitCode == 0)
            return true;
        error = juce::String::fromUTF8 ("No se pudo instalar (¿cancelaste la contraseña?).");
        return false;
    }

    if (kind == InstallKind::linuxUser)
    {
        auto dir = downloaded.getParentDirectory().getChildFile ("extraido");
        dir.deleteRecursively();
        dir.createDirectory();
        if (runCommand ({ "tar", "-xzf", downloaded.getFullPathName(), "-C", dir.getFullPathName() }, nullptr, 120000, exitCode)
            && exitCode == 0)
        {
            auto script = dir.getChildFile ("Studoo-Linux").getChildFile ("instalar.sh");

            // Studoo está abierto: no se puede sobrescribir, pero sí apartarlo.
            // instalar.sh copiará el nuevo en su sitio.
            auto installed = juce::File::getSpecialLocation (juce::File::userHomeDirectory).getChildFile (".local/share/studoo/Studoo");
            auto old = installed.getSiblingFile ("Studoo.antiguo");
            old.deleteFile();
            if (installed.existsAsFile())
                installed.moveFileTo (old);

            if (script.existsAsFile()
                && runCommand ({ "bash", script.getFullPathName() }, nullptr, 120000, exitCode) && exitCode == 0)
            {
                old.deleteFile();
                return true;
            }
            if (! installed.existsAsFile())
                old.moveFileTo (installed); // si algo falla, dejamos la versión de antes
        }
        error = juce::String::fromUTF8 ("No se pudo instalar la actualización.");
        return false;
    }
   #endif
    juce::ignoreUnused (kind, downloaded);
    error = "Tipo de instalación no compatible.";
    return false;
}

bool launchWindowsInstaller (const juce::File& installer)
{
    // /SILENT: sin preguntas, solo una barra de progreso. Al terminar, el
    // instalador vuelve a abrir Studoo (ver studoo.iss).
    return installer.startAsProcess ("/SILENT /SUPPRESSMSGBOXES /NORESTART /CLOSEAPPLICATIONS /SP-");
}

void relaunch()
{
   #if JUCE_LINUX || JUCE_BSD
    // Se abre un momento después, cuando esta copia ya se ha cerrado
    static juce::ChildProcess process;
    juce::String exe = "/usr/bin/studoo";
    if (detectInstallKind() == InstallKind::linuxUser)
        exe = juce::File::getSpecialLocation (juce::File::userHomeDirectory)
                  .getChildFile (".local/share/studoo/Studoo").getFullPathName();
    process.start (juce::StringArray { "/bin/sh", "-c", "sleep 1; exec \"$0\"", exe }, 0);
   #else
    currentExe().startAsProcess();
   #endif
}
}
