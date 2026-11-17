#pragma once
// ============================================================================
//  Pro: la versión gratis y "Studoo Pro" (pago único en Google Play), y las
//  donaciones.
//
//  Solo la versión de Google Play (STUDOO_PLAY=1) tiene cosas de pago. En el
//  ordenador y en la versión de GitHub, todo está desbloqueado.
//
//  Gratis: los primeros 4 niveles de Aprende, 5 kits, algunos sonidos de
//  sinte y 808, 3 bases de géneros y exportar canciones de hasta 1 minuto.
//  Pro: todo lo demás.
// ============================================================================
#include <juce_data_structures/juce_data_structures.h>
#include <functional>

namespace studoo::pro
{
    // --- ¿Hay cosas de pago en esta versión? -------------------------------------
    // Para probarlo en el ordenador: variable de entorno STUDOO_PRUEBA_TIENDA=1
    // (la "compra" se simula y no cobra nada).
    bool hasStore();

    // ¿Tiene Studoo Pro? (siempre sí fuera de Google Play)
    bool isUnlocked();

    // --- Qué es gratis ----------------------------------------------------------------
    constexpr double freeExportSeconds = 60.0;
    bool isKitFree (const juce::String& kitId);
    bool isSynthPresetFree (int index);
    bool is808PresetFree (int index);
    bool isGenreFree (const juce::String& genreId);
    bool isLearnLevelFree (int level);          // 0 = Nivel 1
    bool areGamesFree();
    bool areAchievementsFree();

    // --- Tienda (solo Google Play) --------------------------------------------------------
    inline constexpr const char* proProduct = "studoo_pro";
    inline constexpr const char* donationProducts[] = { "donacion_pequena", "donacion_mediana", "donacion_grande" };

    // Se llama al abrir Studoo: conecta con Google Play y recuerda si ya eras Pro
    void start (juce::PropertiesFile*);
    // Precio que muestra Google Play ("2,99 €"), o vacío si aún no se sabe
    juce::String price (const juce::String& productId);
    bool storeReady();
    void buy (const juce::String& productId);
    void refresh();   // vuelve a preguntar a Google Play (restaurar compra)

    // Avisos de la tienda: "pro_comprado", "donacion_ok", "pendiente",
    // "cancelado" o "error:texto". Llama a poll() de vez en cuando.
    using EventHandler = std::function<void (const juce::String& event)>;
    void setEventHandler (EventHandler);
    void poll();
}
