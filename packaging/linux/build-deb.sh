#!/bin/bash
# ============================================================================
#  Crea el paquete .deb de Studoo (el "instalador" de Ubuntu, Linux Mint, Debian)
#  Uso: packaging/linux/build-deb.sh <ruta/al/ejecutable/Studoo> <versión> <carpeta_salida>
# ============================================================================
set -e
EXE="$1"; VERSION="$2"; OUT="$3"
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(mktemp -d)"
trap 'rm -rf "$ROOT"' EXIT

mkdir -p "$ROOT/DEBIAN" "$ROOT/opt/studoo" "$ROOT/usr/bin" "$ROOT/usr/share/applications"
install -m 755 "$EXE" "$ROOT/opt/studoo/Studoo"
ln -s /opt/studoo/Studoo "$ROOT/usr/bin/studoo"

# Iconos en varios tamaños
for SIZE in 48 64 128 256 512; do
    DIR="$ROOT/usr/share/icons/hicolor/${SIZE}x${SIZE}/apps"
    mkdir -p "$DIR"
    if command -v convert >/dev/null; then
        convert "$HERE/../../resources/icon.png" -resize ${SIZE}x${SIZE} "$DIR/studoo.png"
    else
        cp "$HERE/studoo.png" "$DIR/studoo.png"
    fi
done

cat > "$ROOT/usr/share/applications/studoo.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Studoo
GenericName=Estudio de música
Comment=Haz música y beats, en español
Exec=/opt/studoo/Studoo
Icon=studoo
Terminal=false
Categories=AudioVideo;Audio;Music;
Keywords=música;beats;daw;808;
StartupWMClass=Studoo
DESKTOP

SIZE_KB=$(du -sk "$ROOT" | cut -f1)
cat > "$ROOT/DEBIAN/control" <<CONTROL
Package: studoo
Version: $VERSION
Section: sound
Priority: optional
Architecture: amd64
Installed-Size: $SIZE_KB
Depends: libasound2 | libasound2t64, libfreetype6, libfontconfig1, libx11-6
Recommends: lame
Maintainer: Studoo <studoo@users.noreply.github.com>
Homepage: https://github.com/Salamalicul/Studoo
Description: Estudio para hacer música y beats, en español
 Studoo es una DAW (estación de trabajo de audio) para hacer música y beats:
 secuenciador de patrones, piano roll, playlist, mezclador con efectos,
 automatización, grabación, plugins VST3, kits de batería incluidos y un
 tutorial y apartado "Aprende" para aprender producción musical.
CONTROL

mkdir -p "$OUT"
dpkg-deb --build --root-owner-group "$ROOT" "$OUT/studoo_${VERSION}_amd64.deb"
echo "Paquete creado: $OUT/studoo_${VERSION}_amd64.deb"
