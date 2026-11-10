#!/bin/bash
# Quita Studoo del ordenador (tus proyectos en Documentos/Studoo NO se borran)
rm -f "$HOME/.local/bin/studoo" "$HOME/.local/share/applications/studoo.desktop"
rm -f "$HOME/Escritorio/Studoo.desktop" "$HOME/Desktop/Studoo.desktop"
rm -rf "$HOME/.local/share/studoo"
update-desktop-database "$HOME/.local/share/applications" >/dev/null 2>&1 || true
echo "Studoo se ha desinstalado. Tus proyectos siguen en Documentos/Studoo."
