# 🎛️ Studoo

**Studoo** es un programa para hacer música y beats (una *DAW*), inspirado en FL Studio.
Funciona en **Windows** y **Linux**, y está preparado para adaptarse a **Mac** más adelante.
Todo está en español y pensado para aprender mientras produces.

> Estado actual: **Fase 1**. Ventana en modo oscuro, Reproducir/Parar, BPM, metrónomo
> y secuenciador de batería de 16 pasos con 4 sonidos incluidos. Tiene tooltips y un botón de ayuda "?".

---

## 📥 Opción A (la más fácil): descargar la app ya hecha

GitHub compila Studoo automáticamente cada vez que se sube un cambio.

1. Entra en la página del repositorio en GitHub e inicia sesión.
2. Pulsa la pestaña **Actions** (arriba).
3. Haz clic en la ejecución más reciente que tenga un ✅ verde.
4. Baja hasta **Artifacts** y descarga:
   - **Studoo-Windows** si usas Windows.
   - **Studoo-Linux** si usas Linux.

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

## 🔧 Opción B: compilar Studoo tú mismo

*Compilar* significa convertir el código (texto) en un programa que se puede abrir.

### En Windows 10/11
1. Instala **Visual Studio 2022 Community** (gratis): https://visualstudio.microsoft.com/es/vs/community/
   Durante la instalación, marca **"Desarrollo para el escritorio con C++"**. Esa opción ya incluye CMake.
2. Instala **Git**: https://git-scm.com/download/win (puedes dejar todas las opciones por defecto).
3. Abre el menú Inicio y busca **"Developer PowerShell for VS 2022"**. Escribe:
   ```powershell
   git clone https://github.com/salamalicul/Studoo.git
   cd Studoo
   cmake -B build
   cmake --build build --config Release
   .\build\Studoo_artefacts\Release\Studoo.exe
   ```
   La primera vez tarda unos minutos, porque descarga JUCE y lo compila todo.

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
Después, ejecuta los mismos pasos que en Ubuntu, desde `git clone`.

### En Arch / Manjaro
```bash
sudo pacman -S --needed git cmake ninja gcc alsa-lib jack2 freetype2 fontconfig \
  libx11 libxcomposite libxcursor libxext libxinerama libxrandr libxrender mesa
```
(Si usas PipeWire, instala `pipewire-jack` en lugar de `jack2`.)

---

## 🔊 El sonido en Linux

Studoo puede usar **ALSA** o **JACK**. Pulsa el botón **Audio** de la app para elegir.

- **PipeWire** (Ubuntu 22.10+, Fedora y la mayoría de distribuciones modernas): funciona con ALSA y con JACK.
  Si una opción no suena, prueba la otra.
- Si con ALSA sale "dispositivo ocupado", elige el dispositivo **"default"** o **"pipewire"**.
- Con JACK clásico (sin PipeWire), arranca primero el servidor JACK, por ejemplo con QjackCtl.

---

## 🗂️ Cómo está organizado el código

```
Studoo/
├── CMakeLists.txt            ← "receta" para compilar en cualquier sistema
├── src/
│   ├── Main.cpp              ← arranque de la app y ventana principal
│   ├── MainComponent.*       ← une el audio con la interfaz
│   ├── audio/
│   │   ├── AudioEngine.*     ← el motor que produce el sonido y lleva el tiempo
│   │   └── DrumSounds.*      ← fabrica los sonidos de batería con matemáticas
│   └── ui/
│       ├── StudooLookAndFeel.*  ← colores y estilo (modo oscuro)
│       ├── TransportBar.*       ← barra de arriba (play, BPM, metrónomo...)
│       ├── StepSequencer.*      ← secuenciador de patrones
│       ├── HelpButton.*         ← botón "?" de ayuda
│       └── HelpTexts.h          ← todos los textos de ayuda
├── tests/EngineTest.cpp      ← pruebas automáticas del motor de audio
└── .github/workflows/build.yml  ← compilación automática en GitHub
```

## 🧱 Tecnología

- **C++17** y **JUCE 8** (https://juce.com), el conjunto de herramientas de audio más usado de la industria.
- **CMake** para compilar igual en Windows, Linux y Mac.
