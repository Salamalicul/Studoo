# 🎛️ Studoo

**Studoo** es un programa para hacer música y beats (una *DAW*), inspirado en FL Studio.
Funciona en **Windows** y **Linux** y está preparado para adaptarse a **Mac** más adelante.
Todo está en español y pensado para aprender mientras produces.

## ✨ Qué incluye

| Función | Dónde está |
|---|---|
| Secuenciador de patrones (16/32/64 pasos), varios patrones | Pestaña **Secuenciador** (F6) |
| Piano roll para melodías, acordes (con ayudante de acordes) y 808 | Pestaña **Piano roll** (F7) |
| Playlist para montar la canción con patrones y grabaciones | Pestaña **Playlist** (F5) |
| Tus samples (WAV, MP3, AIFF, FLAC, OGG) y kits de batería (carpeta entera) | Secuenciador > **+ Añadir**, o arrastrando archivos |
| Instrumentos: kit de batería, **sampler**, **sintetizador** y **808** con glide | Secuenciador > **+ Añadir** |
| Mezclador: volumen, panorama, mute/solo, **EQ, compresor, delay y reverb**, medidores | Pestaña **Mezclador** (F9) |
| Automatización (volumen, panorama, efectos, filtro) | Pestaña **Automatización** (F8) |
| Grabación de voz o audio con el micrófono | Botón rojo / tecla **R** |
| Plugins **VST3** (instrumentos y efectos) | Botón **Plugins** + **+ Añadir** / mezclador |
| Teclados y controladores **MIDI** (se activan solos) | Botón **Audio** |
| Guardar/abrir proyectos (`.studoo`) y **autoguardado** cada minuto | Menú Archivo (☰) |
| Deshacer / rehacer (**Ctrl+Z / Ctrl+Y**) | Flechas de la barra superior |
| Exportar a **WAV** y **MP3** | Menú Archivo (☰) |
| BPM, metrónomo, modo PATRÓN/CANCIÓN y atajos de teclado | Barra superior |
| Tutorial "Tu primer beat", botón **?** en cada sección, tooltips y apartado **Aprende** | Pestaña **Aprende** (F1) |

---

## 📥 Opción A (la más fácil): descargar la app ya hecha

GitHub compila Studoo automáticamente cada vez que se sube un cambio.

1. Entra en la página del repositorio en GitHub e inicia sesión.
2. Pulsa la pestaña **Actions** (arriba).
3. Haz clic en la ejecución más reciente que tenga un ✅ verde.
4. Baja hasta **Artifacts** y descarga **Studoo-Windows** o **Studoo-Linux**.

### Windows
1. Descomprime el `.zip` descargado (clic derecho → *Extraer todo*).
2. Haz doble clic en **Studoo.exe**.
3. Si Windows muestra *"Windows protegió su PC"*, pulsa **Más información** → **Ejecutar de todas formas**.
   Sale porque la app es nueva y no está "firmada". Es normal en programas caseros.

### Linux (Ubuntu y otras distribuciones)
1. Descomprime el `.zip` descargado. Dentro hay un archivo `Studoo-Linux.tar.gz`.
2. Abre una terminal en esa carpeta y escribe:
   ```bash
   tar -xzf Studoo-Linux.tar.gz
   ./Studoo-Linux/Studoo
   ```

---

## 🎧 Exportar a MP3

Para crear MP3, Studoo usa el programa gratuito **LAME**:
- **Windows:** descarga "LAME for Windows" (por ejemplo, desde rarewares.org, sección *LAME Bundles*) y copia `lame.exe` en la misma carpeta que `Studoo.exe`.
- **Ubuntu/Debian/Mint:** `sudo apt install lame` · **Fedora:** `sudo dnf install lame` · **Arch:** `sudo pacman -S lame`

Exportar a WAV no necesita nada extra.

## 🔌 Plugins VST3

1. Instala el plugin: en Windows suele ir en `C:\Program Files\Common Files\VST3`; en Linux, en `~/.vst3`.
2. En Studoo pulsa **Plugins** → *Options...* → *Scan for new or updated VST3 plug-ins*.
3. Los instrumentos aparecen en **+ Añadir → Plugin VST3**, y los efectos en el botón **+ Plugin** de cada pista del mezclador.

## 🔊 El sonido en Linux

Studoo puede usar **ALSA** o **JACK**. Pulsa el botón **Audio** de la app para elegir.

- **PipeWire** (Ubuntu 22.10+, Fedora y la mayoría de distribuciones modernas): funciona con ALSA y con JACK.
  Si una opción no suena, prueba la otra.
- Si con ALSA sale "dispositivo ocupado", elige el dispositivo **"default"** o **"pipewire"**.
- Con JACK clásico (sin PipeWire), arranca primero el servidor JACK, por ejemplo con QjackCtl.

---

## 🔧 Opción B: compilar Studoo tú mismo

*Compilar* significa convertir el código (texto) en un programa que se puede abrir.

### En Windows 10/11
1. Instala **Visual Studio 2022 Community** (gratis): https://visualstudio.microsoft.com/es/vs/community/
   Durante la instalación, marca **"Desarrollo para el escritorio con C++"** (ya incluye CMake).
2. Instala **Git**: https://git-scm.com/download/win
3. Abre **"Developer PowerShell for VS 2022"** desde el menú Inicio y escribe:
   ```powershell
   git clone https://github.com/salamalicul/Studoo.git
   cd Studoo
   cmake -B build
   cmake --build build --config Release
   .\build\Studoo_artefacts\Release\Studoo.exe
   ```

### En Ubuntu / Debian / Linux Mint
```bash
sudo apt update
sudo apt install -y git cmake ninja-build g++ libasound2-dev libjack-jackd2-dev \
  libfreetype-dev libfontconfig1-dev libx11-dev libxcomposite-dev libxcursor-dev \
  libxext-dev libxinerama-dev libxrandr-dev libxrender-dev libgl1-mesa-dev

git clone https://github.com/salamalicul/Studoo.git
cd Studoo
cmake -B build -G Ninja
cmake --build build
./build/Studoo_artefacts/Release/Studoo
```

### En Fedora
```bash
sudo dnf install -y git cmake ninja-build gcc-c++ alsa-lib-devel pipewire-jack-audio-connection-kit-devel \
  freetype-devel fontconfig-devel libX11-devel libXcomposite-devel libXcursor-devel \
  libXext-devel libXinerama-devel libXrandr-devel libXrender-devel mesa-libGL-devel
```
Después, los mismos pasos que en Ubuntu desde `git clone`.

### En Arch / Manjaro
```bash
sudo pacman -S --needed git cmake ninja gcc alsa-lib jack2 freetype2 fontconfig \
  libx11 libxcomposite libxcursor libxext libxinerama libxrandr libxrender mesa
```
(Con PipeWire, instala `pipewire-jack` en lugar de `jack2`.)

### Pruebas automáticas
```bash
cmake -B build -G Ninja -DSTUDOO_BUILD_TESTS=ON
cmake --build build
./build/StudooEngineTest_artefacts/Release/StudooEngineTest
```

---

## 🗂️ Cómo está organizado el código

```
src/
├── Main.cpp, MainComponent.*   ← arranque, ventana principal, archivo, exportar, grabar, atajos
├── model/                      ← los datos de la canción (guardar, abrir, deshacer)
│   ├── Ids.h                   ← nombres de todos los datos
│   └── Project.*               ← canales, patrones, playlist, mezclador, automatización
├── audio/                      ← todo lo que suena
│   ├── AudioEngine.*           ← el motor: tiempo, notas, mezcla, grabación, exportación
│   ├── Snapshot.*              ← "foto" del proyecto para el hilo de audio
│   ├── Instruments.*           ← sampler, sintetizador, 808, plugins, metrónomo
│   ├── MixerTrack.*            ← EQ, compresor, delay, reverb, volumen y panorama
│   ├── SampleCache.*           ← carga de WAV/MP3...
│   ├── DrumSounds.*            ← sonidos de batería incluidos (generados con matemáticas)
│   └── PluginHost.*            ← búsqueda y ventanas de plugins VST3
└── ui/                         ← las pantallas
    ├── Toolbar.*               ← barra superior
    ├── ChannelRack.*, InstrumentPanel.*  ← secuenciador y ajustes del instrumento
    ├── PianoRoll.*, Playlist.*, MixerView.*, AutomationView.*, LearnView.*
    ├── Tutorial.*              ← tutorial guiado
    ├── HelpTexts.h, LearnTexts.h ← todos los textos de ayuda y de "Aprende"
    └── StudooLookAndFeel.*, Widgets.*, HelpButton.* ← estilo y piezas comunes
```

## 🧱 Tecnología

- **C++17** y **JUCE 8** (https://juce.com), el conjunto de herramientas de audio más usado de la industria.
- **CMake** para compilar igual en Windows, Linux y Mac.
