#pragma once
// ============================================================================
//  HelpTexts: los textos de ayuda de cada sección, todos juntos en un mismo
//  sitio para que sea fácil corregirlos o ampliarlos.
// ============================================================================

namespace studoo::help
{
    inline constexpr const char* transportTitle = "Barra de transporte";
    inline constexpr const char* transportText =
        "Es la barra de arriba. Controla la reproducción de tu música.\n\n"
        "- Reproducir (triángulo verde): empieza a sonar el patrón desde el principio. "
        "También puedes pulsar la BARRA ESPACIADORA.\n"
        "- Parar (cuadrado): detiene la música.\n"
        "- BPM: la velocidad de la canción (pulsaciones por minuto). Arrastra el número "
        "hacia la izquierda o la derecha para cambiarlo, o haz doble clic para volver a 140. "
        "Trap: 130-150. Hip-hop clásico: 85-95. Reggaetón: 90-100. House: 120-128.\n"
        "- Metrónomo: hace un \"clic\" en cada tiempo para que sepas dónde cae el ritmo. "
        "El primer clic de cada compás suena más agudo.\n"
        "- Volumen general: el volumen de todo lo que suena.\n"
        "- Audio: elige tu tarjeta de sonido o auriculares. En Linux puedes elegir ALSA "
        "o JACK (si usas PipeWire, ambos funcionan).";

    inline constexpr const char* sequencerTitle = "Secuenciador de patrones";
    inline constexpr const char* sequencerText =
        "Aquí se crean los ritmos de batería.\n\n"
        "Cada FILA es un sonido (bombo, caja, hi-hat, palmada) y cada CUADRADO es un paso. "
        "Un patrón tiene 16 pasos, que forman 1 compás. Cada grupo de 4 pasos es un tiempo "
        "(por eso están pintados de colores alternos).\n\n"
        "Cuando reproduces, una luz recorre los pasos de izquierda a derecha. Si un paso "
        "está encendido, ese sonido suena en ese momento.\n\n"
        "Cómo se usa:\n"
        "- Clic izquierdo en un cuadrado: lo enciende o lo apaga.\n"
        "- Mantén pulsado y arrastra: pintas (o borras) varios pasos seguidos.\n"
        "- Clic derecho: borra un paso.\n"
        "- Clic en el nombre del sonido: lo escuchas una vez.\n"
        "- Luz verde a la izquierda: silencia o activa ese sonido (mute).\n"
        "- Rueda naranja: volumen de ese sonido.\n\n"
        "Truco para empezar: bombo en los pasos 1 y 9, caja en los pasos 5 y 13, "
        "y hi-hat en todos los pasos impares. ¡Ya tienes un beat!";
}
