#pragma once
// ============================================================================
//  LearnTexts: los temas del apartado "Aprende".
// ============================================================================

namespace studoo::learn
{
    struct Topic { const char* title; const char* body; };

    inline constexpr Topic topics[] = {
        { "Qué es una DAW",
          "DAW significa \"Digital Audio Workstation\" (estación de trabajo de audio digital). Es el programa "
          "donde se hace música en el ordenador: FL Studio, Ableton, Logic... y ahora Studoo.\n\n"
          "En una DAW tienes:\n"
          "- Instrumentos (que suenan cuando les das notas).\n"
          "- Un secuenciador (donde decides CUÁNDO suena cada cosa).\n"
          "- Un mezclador (donde ajustas volúmenes y efectos).\n"
          "- Una forma de exportar tu canción para escucharla en cualquier sitio." },

        { "BPM y tempo",
          "El BPM (\"beats per minute\", pulsaciones por minuto) es la VELOCIDAD de la canción. "
          "Es como el ritmo al que das palmas o mueves la cabeza.\n\n"
          "Velocidades típicas:\n"
          "- Hip-hop clásico / boom bap: 85-95 BPM\n"
          "- Reggaetón: 88-100 BPM\n"
          "- R&B: 60-80 BPM\n"
          "- House: 120-128 BPM\n"
          "- Trap: 130-160 BPM (a menudo se siente \"a medio tiempo\", como si fuera la mitad)\n"
          "- Drill: 140-145 BPM\n"
          "- Drum & bass: 170-175 BPM\n\n"
          "En Studoo cambias el BPM en la barra de arriba." },

        { "Compases, tiempos y pasos",
          "La música se cuenta en COMPASES. Casi toda la música moderna usa compases de 4 TIEMPOS: "
          "\"1, 2, 3, 4 - 1, 2, 3, 4...\".\n\n"
          "Cada tiempo se divide en 4 PASOS (semicorcheas). Por eso un patrón de 16 pasos es 1 compás:\n"
          "4 tiempos x 4 pasos = 16 pasos.\n\n"
          "En el secuenciador, los pasos 1, 5, 9 y 13 son el comienzo de cada tiempo. "
          "Por eso están pintados en grupos de 4 colores." },

        { "Las partes de la batería",
          "- BOMBO (kick): el golpe grave, el \"pum\". Marca el pulso. Suele ir en los tiempos 1 y 3, o donde quieras "
          "dar fuerza.\n"
          "- CAJA (snare): el golpe seco, el \"pa\". Casi siempre en los tiempos 2 y 4 (pasos 5 y 13).\n"
          "- PALMADA (clap): parecida a la caja, se pone junto a ella o en su lugar.\n"
          "- HI-HAT (charles): el \"tss\" metálico. Marca el ritmo rápido: cada 2 pasos, cada paso, o con "
          "\"rolls\" (ráfagas) en el trap.\n\n"
          "Beat básico: bombo en 1 y 9, caja en 5 y 13, hi-hat en todos los impares." },

        { "Qué es un 808",
          "El 808 viene de la caja de ritmos Roland TR-808 (1980). Su bombo largo y grave se convirtió en el BAJO "
          "del hip-hop y el trap.\n\n"
          "Un 808 es un bajo que:\n"
          "- Empieza con un golpe (como un bombo).\n"
          "- Mantiene una nota grave que se va apagando.\n"
          "- Puede deslizarse entre notas (\"glide\" o \"slide\").\n\n"
          "Consejos:\n"
          "- Escribe el 808 en el piano roll, siguiendo el ritmo del bombo.\n"
          "- Usa notas graves (entre Do1 y Do3).\n"
          "- Si el bombo y el 808 chocan, quita el bombo donde suena el 808, o baja un poco el volumen de uno." },

        { "Samples y sampler",
          "Un SAMPLE es un trozo de audio grabado: un golpe de batería, una voz, un acorde de piano...\n\n"
          "Un SAMPLER es un instrumento que reproduce samples. Si le das una nota más aguda, el sample suena más "
          "agudo (y más rápido); si es más grave, suena más grave.\n\n"
          "En Studoo:\n"
          "- Studoo trae más de 200 sonidos incluidos: \"+ Añadir\" > Kits de batería (18 estilos), 808, Sintetizador y Efectos (FX).\n"
          "- Añade tus propios samples desde \"Añadir\" > \"Sample\", o arrastrando archivos WAV o MP3.\n"
          "- \"Kit de batería\" carga todos los samples de una carpeta a la vez.\n\n"
          "Usa solo samples gratuitos o con licencia (\"royalty free\") si vas a publicar tu música." },

        { "Sintetizador y ADSR",
          "Un sintetizador crea sonido desde cero con OSCILADORES (ondas).\n\n"
          "Tipos de onda:\n"
          "- Sierra: brillante y potente (pads, leads).\n"
          "- Cuadrada: hueca, como de videojuego.\n"
          "- Seno: suave y pura (sub-bajos).\n"
          "- Triángulo: dulce, entre seno y cuadrada.\n\n"
          "La envolvente ADSR controla cómo cambia el volumen de cada nota:\n"
          "- Ataque (A): lo que tarda en llegar al máximo. Corto = golpe; largo = entra suave.\n"
          "- Caída (D): lo que tarda en bajar hasta el sostenido.\n"
          "- Sostenido (S): el volumen mientras mantienes la nota.\n"
          "- Relajación (R): lo que tarda en apagarse al soltar la nota.\n\n"
          "El FILTRO quita agudos: bájalo para un sonido más oscuro y suave." },

        { "Notas, escalas y acordes",
          "Las notas en español: Do, Re, Mi, Fa, Sol, La, Si. Entre algunas hay \"sostenidos\" (#), las teclas negras.\n\n"
          "Una ESCALA es un grupo de notas que suenan bien juntas. Truco fácil: usa solo las TECLAS BLANCAS y "
          "tendrás Do mayor (alegre) o La menor (triste/oscura).\n\n"
          "Escala de Do menor (muy usada en trap): Do, Re, Re#, Fa, Sol, Sol#, La#.\n\n"
          "Un ACORDE son 3 o más notas a la vez:\n"
          "- Mayor (alegre): nota + 4 semitonos + 3 semitonos (Do-Mi-Sol).\n"
          "- Menor (triste): nota + 3 semitonos + 4 semitonos (La-Do-Mi).\n\n"
          "En el piano roll puedes elegir \"Acorde\" y Studoo añade el acorde completo con un clic." },

        { "El piano roll",
          "El piano roll muestra las notas como barras:\n"
          "- Más arriba = más agudo.\n"
          "- Más a la derecha = más tarde.\n"
          "- Más larga = suena más tiempo.\n\n"
          "Se usa para melodías, acordes y líneas de bajo/808. Cada canal tiene sus propias notas dentro de cada "
          "patrón." },

        { "Estructura de una canción",
          "Una canción se arma con secciones que se repiten:\n"
          "- INTRO: presenta la canción (4-8 compases), a menudo con menos instrumentos.\n"
          "- ESTROFA (verso): donde se cuenta la historia (16 compases es típico en rap).\n"
          "- PRE-ESTRIBILLO: sube la tensión antes del estribillo.\n"
          "- ESTRIBILLO (hook/coro): la parte pegadiza que se repite (8 compases).\n"
          "- PUENTE: una parte diferente para romper la repetición.\n"
          "- OUTRO: el final.\n\n"
          "Ejemplo: Intro 4 - Estribillo 8 - Estrofa 16 - Estribillo 8 - Estrofa 16 - Estribillo 8 - Outro 4.\n\n"
          "Truco: haz un patrón para cada sección (por ejemplo \"Estribillo\" con todo, \"Estrofa\" con menos "
          "elementos) y colócalos en la playlist." },

        { "Mezcla: volumen y panorama",
          "MEZCLAR es ajustar cada instrumento para que todos se escuchen bien juntos.\n\n"
          "1. VOLUMEN: empieza con el bombo y el 808 (lo más importante en hip-hop/trap) y luego añade lo demás.\n"
          "2. PANORAMA (pan): coloca sonidos a la izquierda o la derecha. Bombo, caja, 808 y voz casi siempre en el "
          "centro. Hi-hats, percusiones y coros un poco a los lados.\n"
          "3. Deja \"aire\": el Master no debe llegar al rojo. Si se satura, baja los faders.\n\n"
          "Consejo: escucha tu mezcla a volumen bajo; si se entiende todo, vas bien." },

        { "EQ (ecualizador)",
          "El EQ sube o baja zonas de frecuencia:\n"
          "- GRAVES (por debajo de 200 Hz): peso y potencia. Bombo y 808.\n"
          "- MEDIOS (alrededor de 1000 Hz): cuerpo. Voces, cajas, sintetizadores.\n"
          "- AGUDOS (por encima de 5000 Hz): brillo y aire. Hi-hats, voces.\n\n"
          "Consejos:\n"
          "- Es mejor QUITAR lo que sobra que subir lo que falta.\n"
          "- Quita graves a lo que no los necesita (hi-hats, voces) para dejar sitio al 808." },

        { "Compresor",
          "El compresor reduce la diferencia entre las partes fuertes y suaves: baja lo fuerte para que todo suene "
          "más igualado y \"pegado\".\n\n"
          "- UMBRAL: a partir de qué volumen empieza a actuar. Más bajo = comprime más.\n"
          "- RATIO: cuánto comprime. 2:1 = suave; 4:1 = normal; 10:1 = muy fuerte.\n\n"
          "Úsalo en voces (para que se entiendan siempre), en el 808 y en el Master con cuidado." },

        { "Reverb",
          "La reverb simula el eco de una sala: el sonido rebota en las paredes. Hace que algo suene lejano o en un "
          "espacio grande.\n\n"
          "- MEZCLA: cuánta reverb se oye.\n"
          "- TAMAÑO: habitación pequeña o catedral.\n\n"
          "Consejos: un poco en voces, claps y melodías. Nada (o casi nada) en el bombo y el 808, porque se "
          "vuelven confusos." },

        { "Delay",
          "El delay es un ECO que repite el sonido. En Studoo está sincronizado con el BPM y rebota entre izquierda y "
          "derecha (ping-pong).\n\n"
          "- MEZCLA: volumen del eco.\n"
          "- TIEMPO: cuánto tarda cada repetición (en tiempos: 0,25 = semicorchea, 0,5 = corchea, "
          "0,75 = corchea con puntillo, 1 = negra).\n"
          "- FEEDBACK: cuántas repeticiones (más alto = más ecos).\n\n"
          "Muy usado en voces (al final de las frases) y en melodías." },

        { "Automatización",
          "Automatizar es hacer que un control se mueva solo a lo largo de la canción. Ejemplos:\n"
          "- Subir la reverb en el estribillo.\n"
          "- Bajar el volumen del Master al final (fade out).\n"
          "- Abrir el filtro de un sintetizador poco a poco para crear tensión.\n\n"
          "En Studoo: pestaña \"Automatización\" > \"Nueva\". Recuerda que solo actúa en modo CANCIÓN." },

        { "Grabar voz",
          "Para grabar tu voz:\n"
          "1. Conecta un micrófono (o usa el del ordenador o los auriculares).\n"
          "2. Pulsa \"Audio\" y activa tu micrófono en \"Canales de entrada\".\n"
          "3. Usa AURICULARES para que el micrófono no grabe el beat.\n"
          "4. Coloca el cursor donde quieras empezar (clic en la regla de la playlist).\n"
          "5. Pulsa el círculo rojo (o R). Studoo cambia a modo CANCIÓN y empieza a grabar.\n"
          "6. Pulsa de nuevo para terminar. La grabación aparece en la playlist como un bloque verde.\n\n"
          "Consejo: no grites muy cerca del micrófono; si la barra del mezclador llega arriba, aléjate un poco." },

        { "Teclados MIDI",
          "MIDI es un \"idioma\" que usan los teclados y controladores para enviar notas al ordenador. No envía sonido, "
          "solo QUÉ nota tocas y con qué fuerza.\n\n"
          "En Studoo:\n"
          "1. Conecta tu teclado por USB antes de abrir Studoo.\n"
          "2. Pulsa \"Audio\" y comprueba que tu teclado está activado en \"Entradas MIDI\".\n"
          "3. Selecciona un canal (clic en su nombre) y toca: sonará ese instrumento." },

        { "Plugins VST3",
          "Los plugins son instrumentos o efectos de otros fabricantes que se añaden a la DAW. VST3 es el formato "
          "más común.\n\n"
          "1. Instala el plugin en tu ordenador (suele ir a C:\\Program Files\\Common Files\\VST3 en Windows o "
          "~/.vst3 en Linux).\n"
          "2. En Studoo pulsa \"Plugins\" > \"Options...\" > \"Scan for new or updated VST3 plug-ins\".\n"
          "3. Instrumentos: secuenciador > \"Añadir\" > \"Plugin VST3\".\n"
          "4. Efectos: mezclador > botón \"Plugin\" de una pista.\n\n"
          "Hay muchos plugins gratuitos de calidad: Vital (sintetizador), Surge XT, Dexed, Valhalla Supermassive (reverb)..." },

        { "Exportar tu canción",
          "Exportar convierte tu proyecto en un archivo de audio que puedes escuchar en el móvil o subir a internet.\n\n"
          "- WAV: máxima calidad, archivos grandes. Ideal para guardar o enviar a otra persona para que lo mezcle.\n"
          "- MP3: más pequeño, para compartir. Necesita el programa gratuito LAME (Studoo te explica cómo "
          "instalarlo si no lo encuentra).\n\n"
          "Se exporta la playlist completa (modo CANCIÓN). Asegúrate de que tu canción está montada en la playlist." },

        { "Guardar, autoguardado y deshacer",
          "- Guarda con Ctrl+S. Los proyectos de Studoo terminan en \".studoo\".\n"
          "- Los samples NO se copian dentro del proyecto: guárdalos en una carpeta fija (por ejemplo, una carpeta "
          "\"Samples\" junto a tu proyecto).\n"
          "- Studoo hace una copia automática cada minuto. Si se cierra de golpe, al volver a abrirlo te ofrecerá "
          "recuperarla.\n"
          "- Ctrl+Z deshace y Ctrl+Y rehace casi cualquier cambio." },
    };
}
