#pragma once
// ============================================================================
//  HelpTexts: los textos de ayuda del botón "?" de cada sección.
//  Están todos aquí para que sea fácil corregirlos o ampliarlos.
// ============================================================================

namespace studoo::help
{
    inline constexpr const char* toolbarTitle = "Barra superior";
    inline constexpr const char* toolbarText =
        "Controla la reproducción y los ajustes generales.\n\n"
        "- Archivo (tres rayas): nuevo, abrir, guardar y exportar a WAV o MP3.\n"
        "- Deshacer / Rehacer (flechas): también con Ctrl+Z y Ctrl+Y.\n"
        "- PATRÓN / CANCIÓN: en PATRÓN se repite el patrón que estás editando. "
        "En CANCIÓN suena la playlist completa, de principio a fin (atajo: L).\n"
        "- Reproducir / Parar: también con la BARRA ESPACIADORA.\n"
        "- Grabar (círculo rojo): graba tu voz o un instrumento con el micrófono (atajo: R).\n"
        "- BPM: velocidad de la canción. Arrastra el número o haz doble clic para volver a 140.\n"
        "- Posición: en qué compás y tiempo estás.\n"
        "- Metrónomo: un \"clic\" en cada tiempo (Ctrl+M).\n"
        "- Volumen: volumen general (el mismo que el fader Master del mezclador).\n"
        "- Audio: elige tarjeta de sonido, micrófono y teclado MIDI.\n"
        "- Plugins: busca los plugins VST3 instalados en tu ordenador.";

    inline constexpr const char* rackTitle = "Secuenciador de patrones";
    inline constexpr const char* rackText =
        "Aquí se crean los ritmos. Cada FILA es un canal (un instrumento) y cada CUADRADO un paso. "
        "16 pasos = 1 compás; cada grupo de 4 pasos es un tiempo.\n\n"
        "Pasos:\n"
        "- Clic: encender/apagar. Arrastra para pintar varios. Clic derecho: borrar.\n\n"
        "Cada canal tiene:\n"
        "- Luz verde: silenciar (mute).\n"
        "- Nombre: clic para seleccionarlo y escucharlo. Clic derecho: más opciones "
        "(piano roll, renombrar, cambiar sample, eliminar).\n"
        "- Rueda VOL: volumen. Rueda PAN: izquierda/derecha.\n"
        "- Número: a qué pista del mezclador va su sonido.\n\n"
        "Arriba puedes cambiar de patrón, crear patrones nuevos, elegir la duración (16, 32 o 64 pasos) "
        "y añadir canales: samples, kits de batería, sintetizador, 808 o plugins VST3.\n\n"
        "SONIDOS INCLUIDOS: en \"+ Añadir\" tienes 18 kits de batería (Trap, Drill, Reggaetón, Phonk, Afrobeat, House, Lo-fi...), "
        "efectos FX, 14 sonidos de sintetizador ya preparados y 6 tipos de 808. "
        "\"Usar este kit\" cambia los sonidos de tu batería por los del kit SIN perder el ritmo que ya has escrito.\n\n"
        "TRUCO: arrastra archivos WAV o MP3 desde tu explorador de archivos hasta aquí para añadirlos como canales.";

    inline constexpr const char* instrumentTitle = "Ajustes del instrumento";
    inline constexpr const char* instrumentText =
        "Aquí cambias cómo suena el canal seleccionado.\n\n"
        "SAMPLER: reproduce un archivo de audio. \"One-shot\" significa que suena entero aunque la nota sea corta "
        "(ideal para batería). \"Nota base\" es la nota en la que el sample suena a su tono original.\n\n"
        "SINTETIZADOR: crea sonido desde cero.\n"
        "- Onda: el \"color\" del sonido (sierra = brillante, cuadrada = hueca, seno = suave, triángulo = dulce).\n"
        "- Ataque, Caída, Sostenido, Relajación (ADSR): cómo empieza, baja, se mantiene y termina el sonido.\n"
        "- Filtro: quita agudos (más bajo = sonido más oscuro). Resonancia: realza la zona del filtro.\n"
        "- Desafinar: separa los dos osciladores para un sonido más ancho.\n\n"
        "808: bajo típico del trap. Caída = cuánto dura; Glide = cuánto tarda en deslizarse de una nota a otra "
        "cuando las notas se solapan; Distorsión = más fuerte y agresivo.\n\n"
        "PLUGIN: pulsa \"Abrir plugin\" para ver sus controles.";

    inline constexpr const char* pianoRollTitle = "Piano roll";
    inline constexpr const char* pianoRollText =
        "Aquí escribes melodías, acordes y líneas de 808 nota a nota.\n\n"
        "- Vertical: la altura de la nota (a la izquierda tienes un piano; haz clic en sus teclas para escucharlas).\n"
        "- Horizontal: el tiempo. Las líneas fuertes marcan los tiempos.\n\n"
        "Cómo se usa:\n"
        "- Clic en un hueco vacío: añade una nota.\n"
        "- Arrastra una nota: muévela. Arrastra su borde derecho: cámbiale la duración.\n"
        "- Clic derecho en una nota: bórrala.\n"
        "- \"Ajuste\": a qué cuadrícula se pegan las notas.\n"
        "- \"Acorde\": añade un acorde completo con un solo clic (mayor = alegre, menor = triste).\n\n"
        "Arriba eliges qué canal (instrumento) estás editando. Las notas son del patrón actual.\n"
        "Para el 808: haz notas largas, y si solapas dos notas el tono se \"desliza\" (glide).";

    inline constexpr const char* playlistTitle = "Playlist (la canción)";
    inline constexpr const char* playlistText =
        "Aquí construyes la canción completa colocando patrones a lo largo del tiempo.\n\n"
        "- Cada cuadrito de la regla es un compás.\n"
        "- Elige arriba qué patrón quieres colocar y haz clic en un hueco vacío.\n"
        "- Arrastra un bloque para moverlo. Arrastra su borde derecho para alargarlo (el patrón se repite).\n"
        "- Clic derecho en un bloque: borrarlo. Doble clic: abrir ese patrón en el secuenciador.\n"
        "- Clic en la regla de arriba: mueve el cursor de reproducción.\n\n"
        "IMPORTANTE: para escuchar la playlist pulsa CANCIÓN en la barra superior (o la tecla L).\n\n"
        "Las grabaciones de voz aparecen aquí como bloques verdes.\n\n"
        "Idea de estructura: Intro (4 compases) - Estribillo (8) - Estrofa (16) - Estribillo (8) - Final (4).";

    inline constexpr const char* mixerTitle = "Mezclador";
    inline constexpr const char* mixerText =
        "El mezclador junta el sonido de todos los canales. Cada canal envía su sonido a una pista "
        "(lo eliges en el secuenciador). Todas las pistas acaban en el MASTER, que es lo que oyes.\n\n"
        "En cada pista, de arriba a abajo:\n"
        "- EQ: sube o baja graves, medios y agudos.\n"
        "- Compresor: iguala el volumen (lo fuerte baja). Umbral = a partir de dónde actúa; "
        "Ratio = cuánto comprime.\n"
        "- Delay: eco sincronizado con el BPM. Tiempo en tiempos de compás (0,75 = corchea con puntillo).\n"
        "- Reverb: sensación de sala o espacio.\n"
        "- Plugin: añade un efecto VST3.\n"
        "- PAN: izquierda/derecha. M = silenciar. S = escuchar solo esa pista.\n"
        "- Fader: el volumen. La barra de color muestra el nivel. Si llega arriba del todo, baja el volumen.";

    inline constexpr const char* automationTitle = "Automatización";
    inline constexpr const char* automationText =
        "La automatización mueve un control solo, a lo largo de la canción. Por ejemplo: que la reverb "
        "aumente en el estribillo, que el volumen baje al final (fade out) o que el filtro del "
        "sintetizador se abra poco a poco.\n\n"
        "- Pulsa \"Nueva\" y elige qué control quieres automatizar.\n"
        "- La línea naranja es el valor: arriba = máximo, abajo = mínimo.\n"
        "- Clic en un hueco: añade un punto. Arrastra un punto para moverlo. Clic derecho: borrarlo.\n\n"
        "IMPORTANTE: la automatización solo actúa en modo CANCIÓN.";

    inline constexpr const char* learnTitle = "Aprende";
    inline constexpr const char* learnText =
        "Aquí tienes explicaciones sencillas de los conceptos básicos de producción musical. "
        "Elige un tema en la lista de la izquierda.\n\n"
        "También puedes repetir el tutorial \"Tu primer beat\" desde el menú Archivo.";

    inline constexpr const char* shortcutsTitle = "Atajos de teclado";
    inline constexpr const char* shortcutsText =
        "Espacio: reproducir / parar\n"
        "L: cambiar entre PATRÓN y CANCIÓN\n"
        "R: grabar\n"
        "Ctrl+M: metrónomo\n\n"
        "Ctrl+Z: deshacer\n"
        "Ctrl+Y (o Ctrl+Mayús+Z): rehacer\n\n"
        "Ctrl+N: proyecto nuevo\n"
        "Ctrl+O: abrir proyecto\n"
        "Ctrl+S: guardar\n"
        "Ctrl+Mayús+S: guardar como\n"
        "Ctrl+E: exportar a WAV\n\n"
        "F5: playlist\n"
        "F6: secuenciador\n"
        "F7: piano roll\n"
        "F8: automatización\n"
        "F9: mezclador\n"
        "F1: aprende";
}
