#!/usr/bin/env python3
# ============================================================================
#  Crea packaging/android/Studoo.jucer (el proyecto de Projucer para Android).
#  Lee la versión de CMakeLists.txt y añade todos los archivos de src/, así
#  nunca se queda desactualizado.
#
#  Uso (desde la carpeta principal de Studoo):
#     python3 packaging/android/generate_jucer.py
#     Projucer --resave packaging/android/Studoo.jucer
#     cd packaging/android/Builds/Android && ./gradlew assembleRelease
#
#  JUCE tiene que estar en packaging/android/JUCE (una copia o un enlace).
# ============================================================================
import hashlib
import pathlib
import re
from xml.sax.saxutils import quoteattr

here = pathlib.Path(__file__).resolve().parent
root = here.parent.parent

cmake = (root / "CMakeLists.txt").read_text(encoding="utf-8")
version = re.search(r"project\(Studoo VERSION (\d+\.\d+\.\d+)", cmake).group(1)
major, minor, patch = (int(x) for x in version.split("."))
version_code = major * 10000 + minor * 100 + patch


def fid(path: str) -> str:
    """Identificador estable para cada archivo (Projucer necesita uno)."""
    return "f" + hashlib.md5(path.encode()).hexdigest()[:6]


def rel(p: pathlib.Path) -> str:
    return "../../" + p.relative_to(root).as_posix()


# Código: todo src/ menos las pruebas
sources = sorted(p for p in (root / "src").rglob("*") if p.suffix in (".cpp", ".h"))
files_xml = "\n".join(
    f'      <FILE id="{fid(rel(p))}" name="{p.name}" compile="{1 if p.suffix == ".cpp" else 0}" '
    f'resource="0" file="{rel(p)}"/>'
    for p in sources
)

# Recursos que van dentro de la app (logo y fuentes)
resources = [root / "resources/icon.png",
             root / "resources/fonts/Inter-Regular.otf",
             root / "resources/fonts/Inter-SemiBold.otf",
             root / "resources/fonts/InterDisplay-Black.otf"]
resources_xml = "\n".join(
    f'      <FILE id="{fid(rel(p))}" name="{p.name}" compile="0" resource="1" file="{rel(p)}"/>'
    for p in resources
)
icon_id = fid(rel(resources[0]))

modules = ["juce_audio_basics", "juce_audio_devices", "juce_audio_formats", "juce_audio_processors",
           "juce_audio_utils", "juce_core", "juce_data_structures", "juce_dsp", "juce_events",
           "juce_graphics", "juce_gui_basics", "juce_gui_extra"]
modules_xml = "\n".join(f'    <MODULE id="{m}" showAllCode="0" useLocalCopy="0" useGlobalPath="0"/>' for m in modules)
paths_xml = "\n".join(f'        <MODULEPATH id="{m}" path="JUCE/modules"/>' for m in modules)

defines = "JUCE_WEB_BROWSER=0\nJUCE_USE_CURL=0\nSTUDOO_ANDROID=1"

jucer = f"""<?xml version="1.0" encoding="UTF-8"?>
<!-- Archivo creado por generate_jucer.py: no lo edites a mano. -->
<JUCERPROJECT id="StdOo1" name="Studoo" projectType="guiapp" version="{version}"
              bundleIdentifier="com.studoo.app" companyName="Studoo"
              companyWebsite="https://github.com/Salamalicul/Studoo"
              useAppConfig="0" addUsingNamespaceToJuceHeader="0" jucerFormatVersion="1"
              cppLanguageStandard="17" defines={quoteattr(defines)}>
  <MAINGROUP id="StdMain" name="Studoo">
    <GROUP id="StdSrc" name="Source">
{files_xml}
    </GROUP>
    <GROUP id="StdRes" name="Resources">
{resources_xml}
    </GROUP>
  </MAINGROUP>
  <EXPORTFORMATS>
    <ANDROIDSTUDIO targetFolder="Builds/Android" androidMinimumSDK="24" androidTargetSDK="35"
                   androidVersionCode="{version_code}" androidScreenOrientation="landscape"
                   microphonePermissionNeeded="1" androidInternetNeeded="1" androidEnableContentSharing="1"
                   androidBluetoothNeeded="0" bigIcon="{icon_id}" smallIcon="{icon_id}"
                   androidKeyStore="${{user.home}}/.android/studoo.keystore" androidKeyStorePass="android"
                   androidKeyAlias="studoo" androidKeyAliasPass="android"
                   app_name="Studoo">
      <CONFIGURATIONS>
        <CONFIGURATION isDebug="1" name="Debug" headerPath="../../../../src"
                       androidArchitectures="arm64-v8a"/>
        <CONFIGURATION isDebug="0" name="Release" headerPath="../../../../src" optimisation="3"
                       androidArchitectures="arm64-v8a armeabi-v7a"/>
      </CONFIGURATIONS>
      <MODULEPATHS>
{paths_xml}
      </MODULEPATHS>
    </ANDROIDSTUDIO>
  </EXPORTFORMATS>
  <MODULES>
{modules_xml}
  </MODULES>
  <JUCEOPTIONS JUCE_USE_MP3AUDIOFORMAT="1" JUCE_PLUGINHOST_VST3="0" JUCE_PLUGINHOST_LV2="0"
               JUCE_PLUGINHOST_AU="0" JUCE_WEB_BROWSER="0" JUCE_USE_CURL="0"/>
</JUCERPROJECT>
"""

(here / "Studoo.jucer").write_text(jucer, encoding="utf-8")
print(f"Studoo.jucer creado (versión {version}, código {version_code}, {len(sources)} archivos)")
