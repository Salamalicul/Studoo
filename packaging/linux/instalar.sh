#!/bin/bash
# ============================================================================
#  Instalador de Studoo para Linux
#  Copia Studoo a tu carpeta personal y lo añade al menú de aplicaciones.
#  No necesita contraseña (no toca nada del sistema).
#  Uso:  ./Studoo-Linux/instalar.sh
# ============================================================================
set -e
ORIGEN="$(cd "$(dirname "$0")" && pwd)"
DESTINO="$HOME/.local/share/studoo"

echo "Instalando Studoo..."
mkdir -p "$DESTINO" "$HOME/.local/bin" "$HOME/.local/share/applications"
cp "$ORIGEN/Studoo" "$DESTINO/Studoo"
chmod +x "$DESTINO/Studoo"
cp "$ORIGEN/studoo.png" "$DESTINO/studoo.png"
cp "$ORIGEN/desinstalar.sh" "$DESTINO/desinstalar.sh"
chmod +x "$DESTINO/desinstalar.sh"

# Comando "studoo" para abrirlo desde la terminal
ln -sf "$DESTINO/Studoo" "$HOME/.local/bin/studoo"

# Acceso directo en el menú de aplicaciones
cat > "$HOME/.local/share/applications/studoo.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Studoo
GenericName=Estudio de música
Comment=Haz música y beats, en español
Exec="$DESTINO/Studoo"
Icon=$DESTINO/studoo.png
Terminal=false
Categories=AudioVideo;Audio;Music;
Keywords=música;beats;daw;808;
DESKTOP
chmod +x "$HOME/.local/share/applications/studoo.desktop"
update-desktop-database "$HOME/.local/share/applications" >/dev/null 2>&1 || true

# Acceso directo también en el escritorio, si existe
for ESCRITORIO in "$HOME/Escritorio" "$HOME/Desktop"; do
    if [ -d "$ESCRITORIO" ]; then
        cp "$HOME/.local/share/applications/studoo.desktop" "$ESCRITORIO/Studoo.desktop"
        chmod +x "$ESCRITORIO/Studoo.desktop"
        gio set "$ESCRITORIO/Studoo.desktop" metadata::trusted true >/dev/null 2>&1 || true
    fi
done

echo ""
echo "¡Studoo instalado!"
echo "  - Búscalo en el menú de aplicaciones como \"Studoo\"."
echo "  - O escribe:  studoo   en una terminal (puede que tengas que abrir una terminal nueva)."
echo "  - Para desinstalarlo:  ~/.local/share/studoo/desinstalar.sh"
