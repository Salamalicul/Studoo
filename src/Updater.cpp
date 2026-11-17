// En Android necesitamos las ayudas de JUCE para llamar a Java (JNI).
// Tienen que activarse antes de incluir JUCE.
#define JUCE_CORE_INCLUDE_JNI_HELPERS 1
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
            // Solo HTTPS (también en las redirecciones)
            args = { "curl", "-sSfL", "--proto", "=https", "--proto-redir", "=https",
                     "--max-time", juce::String (timeoutSecs), "-A", userAgent, "-H", accept };
            if (toFile != nullptr) args.addArray ({ "-o", toFile->getFullPathName() });
        }
        else if (hasCommand ("wget"))
        {
            args = { "wget", "-q", "--https-only", "--timeout=" + juce::String (timeoutSecs), "--user-agent=" + juce::String (userAgent),
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
            case InstallKind::androidApk:       return "-Android.apk";
            case InstallKind::manual:           return {};
        }
        return {};
    }
}

InstallKind detectInstallKind()
{
    const auto exe = currentExe();
   #if JUCE_ANDROID
    juce::ignoreUnused (exe);
    return InstallKind::androidApk;
   #elif JUCE_WINDOWS
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
    if (tag.isEmpty() && json["message"].toString() == "Not Found")
    {
        error = juce::String::fromUTF8 ("No se encuentra Studoo en GitHub. Puede que el proyecto sea privado: "
                                        "para que las actualizaciones funcionen, tiene que ser público.");
        return false;
    }
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
            {
                // Seguridad: solo aceptamos archivos de la release oficial de Studoo,
                // por HTTPS, y con un nombre de archivo normal (sin carpetas)
                const auto name = asset["name"].toString();
                const auto url = asset["url"].toString(); // API: sirve el archivo con "Accept: application/octet-stream"
                const auto digest = asset["digest"].toString();
                if (name.endsWith (suffix) && isSafeAssetName (name) && isOfficialAssetUrl (url)
                    && digest.startsWith ("sha256:"))
                {
                    info.assetName = name;
                    info.assetUrl = url;
                    info.sha256 = digest.fromFirstOccurrenceOf ("sha256:", false, false).toLowerCase();
                }
            }
    return true;
}

bool isOfficialAssetUrl (const juce::String& url)
{
    return url.startsWith ("https://api.github.com/repos/Salamalicul/Studoo/releases/assets/")
        && url.fromLastOccurrenceOf ("/", false, false).containsOnly ("0123456789");
}

bool isSafeAssetName (const juce::String& name)
{
    return name.startsWith ("Studoo-") && name.length() < 100
        && name.containsOnly ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_")
        && ! name.contains ("..");
}

juce::File downloadFolder()
{
    // Carpeta privada del usuario (no la carpeta temporal compartida por todos)
   #if JUCE_ANDROID
    return juce::File::getSpecialLocation (juce::File::tempDirectory).getChildFile ("StudooActualizacion");
   #else
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("Studoo").getChildFile ("Actualizacion");
   #endif
}

bool download (const juce::String& url, const juce::String& sha256, const juce::File& destination, juce::String& error)
{
    if (! isOfficialAssetUrl (url) || ! isSafeAssetName (destination.getFileName()))
    {
        error = juce::String::fromUTF8 ("La actualización no viene de la página oficial de Studoo. No se ha descargado.");
        return false;
    }
    destination.getParentDirectory().createDirectory();
    destination.deleteFile();
    if (! httpGet (url, &destination, nullptr, 600) || destination.getSize() <= 100000)
    {
        error = juce::String::fromUTF8 ("No se pudo descargar la actualización.");
        return false;
    }

    // Seguridad: el archivo tiene que ser exactamente el que se publicó en GitHub
    if (sha256.length() != 64 || juce::SHA256 (destination).toHexString().toLowerCase() != sha256)
    {
        destination.deleteFile();
        error = juce::String::fromUTF8 ("La descarga no coincide con la versión oficial de Studoo (puede que se haya "
                                        "estropeado por el camino). Por seguridad no se ha instalado. Inténtalo más tarde.");
        return false;
    }
    return true;
}

bool installOnLinux (InstallKind kind, const juce::File& downloaded, juce::String& error)
{
   #if JUCE_LINUX || JUCE_BSD
    int exitCode = -1;
    if (kind == InstallKind::linuxDeb)
    {
        if (! juce::File ("/usr/bin/pkexec").existsAsFile() || ! juce::File ("/usr/bin/apt-get").existsAsFile())
        {
            error = juce::String::fromUTF8 ("Falta \"pkexec\" para pedir la contraseña. Instala el archivo .deb a mano.");
            return false;
        }
        // pkexec muestra la ventana del sistema que pide la contraseña
        // Rutas completas: así no se puede colar otro programa con el mismo nombre
        if (runCommand ({ "/usr/bin/pkexec", "/usr/bin/apt-get", "install", "-y", "--allow-downgrades", downloaded.getFullPathName() },
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

int launchAndroidInstaller (const juce::File& apk)
{
   #if JUCE_ANDROID
    // Llamamos a StudooUpdater.installApk (packaging/android/java) desde C++.
    // La clase es de la app, así que se carga con el "class loader" de la app.
    auto* env = juce::getEnv();
    auto context = juce::getAppContext();
    if (env == nullptr || context.get() == nullptr)
        return -1;

    auto clear = [env] { if (env->ExceptionCheck()) { env->ExceptionClear(); return true; } return false; };

    jclass contextClass = env->GetObjectClass (context.get());
    jmethodID getClassLoader = env->GetMethodID (contextClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject loader = env->CallObjectMethod (context.get(), getClassLoader);
    env->DeleteLocalRef (contextClass);
    if (clear() || loader == nullptr)
        return -1;

    jclass loaderClass = env->FindClass ("java/lang/ClassLoader");
    jmethodID loadClass = env->GetMethodID (loaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    jstring className = env->NewStringUTF ("com.studoo.app.StudooUpdater");
    auto updaterClass = (jclass) env->CallObjectMethod (loader, loadClass, className);
    env->DeleteLocalRef (className);
    env->DeleteLocalRef (loaderClass);
    env->DeleteLocalRef (loader);
    if (clear() || updaterClass == nullptr)
        return -1;

    jmethodID install = env->GetStaticMethodID (updaterClass, "installApk", "(Landroid/content/Context;Ljava/lang/String;)I");
    if (clear() || install == nullptr)
    {
        env->DeleteLocalRef (updaterClass);
        return -1;
    }
    jstring path = env->NewStringUTF (apk.getFullPathName().toRawUTF8());
    const jint result = env->CallStaticIntMethod (updaterClass, install, context.get(), path);
    env->DeleteLocalRef (path);
    env->DeleteLocalRef (updaterClass);
    if (clear())
        return -1;
    return (int) result;
   #else
    juce::ignoreUnused (apk);
    return -1;
   #endif
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
