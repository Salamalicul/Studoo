#include "Kits.h"
#include <array>
#include <cmath>

namespace studoo::kits
{
// ============================================================================
//  Lista de kits
// ============================================================================
namespace
{
    std::vector<KitInfo> makeKits()
    {
        std::vector<KitInfo> list;
        auto add = [&list] (KitInfo k) -> Character& { list.push_back (std::move (k)); return list.back().character; };

        // --- Trap -------------------------------------------------------------------
        {
            auto& c = add ({ "trap", "Trap", "Bombo con pegada, hi-hats brillantes y un 808 largo y grave.",
                             { { "kick", "Bombo" }, { "kick2", "Bombo corto" }, { "snare", "Caja" }, { "snare2", "Caja 2" },
                               { "clap", "Palmada" }, { "clap2", "Palmada doble" }, { "snap", "Chasquido" },
                               { "hihat", "Hi-hat" }, { "hihat2", "Hi-hat corto" }, { "openhat", "Hi-hat abierto" },
                               { "rim", "Rim" }, { "perc", "Perc" }, { "tomlo", "Tom grave" }, { "crash", "Platillo" } },
                             { true, 2.2f, 0.10f, 0.45f }, {} });
            c.kF0 = 180; c.kF1 = 50; c.kPitchDecay = 30; c.kAmpDecay = 7; c.kClick = 0.35; c.kDrive = 2.2; c.kLen = 0.55;
            c.sTone = 210; c.sToneDecay = 22; c.sNoiseDecay = 17; c.sHP = 700; c.sCentre = 4200; c.sNoise = 0.8; c.sToneAmt = 0.6; c.sLen = 0.3;
            c.hTune = 1.0; c.hDecay = 55; c.hOpenDecay = 7;
        }
        // --- Drill --------------------------------------------------------------------
        {
            auto& c = add ({ "drill", "Drill", "Sonido oscuro y agresivo: caja aguda, hi-hats rápidos y 808 con mucho deslizamiento.",
                             { { "kick", "Bombo" }, { "kick2", "Bombo corto" }, { "snare", "Caja" }, { "snare2", "Caja 2" },
                               { "rim", "Rim" }, { "hihat", "Hi-hat" }, { "hihat2", "Hi-hat corto" }, { "openhat", "Hi-hat abierto" },
                               { "perc", "Perc" }, { "clave", "Clave" }, { "crash", "Platillo" } },
                             { true, 2.6f, 0.16f, 0.6f }, {} });
            c.kF0 = 210; c.kF1 = 55; c.kPitchDecay = 36; c.kAmpDecay = 9; c.kClick = 0.5; c.kDrive = 3.0; c.kLen = 0.45;
            c.sTone = 260; c.sToneDecay = 26; c.sNoiseDecay = 21; c.sHP = 900; c.sCentre = 5200; c.sNoise = 0.9; c.sToneAmt = 0.55; c.sLen = 0.25;
            c.hTune = 1.1; c.hDecay = 70; c.hOpenDecay = 6; c.hNoise = 0.25;
            c.percF0 = 1500; c.percF1 = 1250; c.percDecay = 30;
        }
        // --- Boom Bap -------------------------------------------------------------------
        {
            auto& c = add ({ "boombap", "Boom Bap (hip-hop clásico)", "Batería redonda y polvorienta de los 90: bombo grueso y caja gorda.",
                             { { "kick", "Bombo" }, { "kick3", "Bombo largo" }, { "snare", "Caja" }, { "snare2", "Caja 2" },
                               { "rim", "Rim" }, { "snap", "Chasquido" }, { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" },
                               { "ride", "Ride" }, { "crash", "Platillo" }, { "shaker", "Shaker" }, { "tambourine", "Pandereta" } },
                             {}, {} });
            c.kF0 = 140; c.kF1 = 48; c.kPitchDecay = 22; c.kAmpDecay = 6; c.kClick = 0.25; c.kDrive = 1.8; c.kLen = 0.6;
            c.sTone = 175; c.sToneDecay = 16; c.sNoiseDecay = 11; c.sHP = 400; c.sCentre = 2600; c.sNoise = 0.85; c.sToneAmt = 0.75; c.sLen = 0.38;
            c.hTune = 0.85; c.hDecay = 40; c.hOpenDecay = 6; c.hNoise = 0.5;
            c.room = 0.12; c.roomSize = 0.35;
        }
        // --- Reggaetón ---------------------------------------------------------------------
        {
            auto& c = add ({ "reggaeton", "Reggaetón", "Todo lo necesario para el ritmo dembow: caja seca, timbales, bongó, conga y un 808 corto.",
                             { { "kick", "Bombo" }, { "kick2", "Bombo corto" }, { "snare", "Caja (dembow)" }, { "snare2", "Caja 2" },
                               { "clap", "Palmada" }, { "rim", "Rim" }, { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" },
                               { "tom", "Timbal" }, { "tomlo", "Timbal grave" }, { "perc", "Bongó" }, { "conga", "Conga" },
                               { "shaker", "Shaker" } },
                             { true, 1.0f, 0.05f, 0.25f }, {} });
            c.kF0 = 170; c.kF1 = 52; c.kPitchDecay = 30; c.kAmpDecay = 8; c.kClick = 0.4; c.kDrive = 2.5; c.kLen = 0.5;
            c.sTone = 230; c.sToneDecay = 24; c.sNoiseDecay = 19; c.sHP = 800; c.sCentre = 4800; c.sNoise = 0.85; c.sToneAmt = 0.6; c.sLen = 0.28;
            c.cHP = 1000; c.cCentre = 1500; c.cTail = 18; c.cGap = 0.010; c.cLen = 0.35;
            c.hDecay = 50; c.hOpenDecay = 8; c.hNoise = 0.35;
            c.percF0 = 470; c.percF1 = 390; c.percDecay = 16;
        }
        // --- House ------------------------------------------------------------------------------
        {
            auto& c = add ({ "house", "House / Electrónica", "Inspirado en las cajas de ritmos 909: bombo largo, palmada grande y hi-hats abiertos.",
                             { { "kick", "Bombo" }, { "kick3", "Bombo largo" }, { "clap", "Palmada" }, { "clap2", "Palmada doble" },
                               { "snare", "Caja" }, { "rim", "Rim" }, { "hihat", "Hi-hat" }, { "hihat2", "Hi-hat corto" },
                               { "openhat", "Hi-hat abierto" }, { "ride", "Ride" }, { "crash", "Platillo" }, { "shaker", "Shaker" },
                               { "cowbell", "Cencerro" }, { "tambourine", "Pandereta" } },
                             {}, {} });
            c.kF0 = 165; c.kF1 = 50; c.kPitchDecay = 26; c.kAmpDecay = 5; c.kClick = 0.55; c.kDrive = 2.0; c.kLen = 0.7;
            c.cHP = 800; c.cCentre = 1200; c.cTail = 9; c.cGap = 0.012; c.cLen = 0.55;
            c.hTune = 1.05; c.hDecay = 45; c.hOpenDecay = 5;
        }
        // --- Lo-fi ---------------------------------------------------------------------------------
        {
            auto& c = add ({ "lofi", "Lo-fi", "Sonidos suaves, apagados y con \"polvo\" de disco antiguo. Ideal para beats relajados.",
                             { { "kick", "Bombo" }, { "kick2", "Bombo corto" }, { "snare", "Caja" }, { "snare2", "Caja 2" },
                               { "rim", "Rim" }, { "snap", "Chasquido" }, { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" },
                               { "shaker", "Shaker" }, { "tambourine", "Pandereta" }, { "perc", "Perc" } },
                             {}, {} });
            c.kF0 = 120; c.kF1 = 45; c.kPitchDecay = 20; c.kAmpDecay = 7; c.kClick = 0.1; c.kDrive = 1.4; c.kLen = 0.5;
            c.sTone = 185; c.sToneDecay = 18; c.sNoiseDecay = 14; c.sHP = 400; c.sCentre = 2400; c.sNoise = 0.7; c.sToneAmt = 0.7; c.sLen = 0.35;
            c.hTune = 0.8; c.hDecay = 35; c.hOpenDecay = 7; c.hNoise = 0.5;
            c.lofi = true;
        }
        // --- Phonk ----------------------------------------------------------------------------------
        {
            auto& c = add ({ "phonk", "Phonk", "Bombos saturados, el famoso cencerro y un 808 muy distorsionado.",
                             { { "kick", "Bombo" }, { "kick2", "Bombo corto" }, { "snare", "Caja" }, { "clap", "Palmada" },
                               { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" }, { "cowbell", "Cencerro phonk" },
                               { "perc", "Perc" }, { "rim", "Rim" }, { "crash", "Platillo" } },
                             { true, 1.8f, 0.10f, 0.85f }, {} });
            c.kF0 = 190; c.kF1 = 48; c.kPitchDecay = 32; c.kAmpDecay = 6; c.kClick = 0.5; c.kDrive = 4.0; c.kLen = 0.5;
            c.sTone = 240; c.sToneDecay = 24; c.sNoiseDecay = 18; c.sCentre = 4500;
            c.hDecay = 60; c.extraDrive = 0.5; c.percTune = 1.08;
        }
        // --- Afrobeat ------------------------------------------------------------------------------
        {
            auto& c = add ({ "afrobeat", "Afrobeat", "Ritmos africanos: congas, bongós, toms, shaker y bloques de madera.",
                             { { "kick", "Bombo" }, { "snare", "Caja" }, { "rim", "Rim" }, { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" },
                               { "shaker", "Shaker" }, { "conga", "Conga" }, { "bongo", "Bongó" }, { "tomhi", "Tom agudo" },
                               { "tommid", "Tom medio" }, { "tomlo", "Tom grave" }, { "clave", "Clave" }, { "woodblock", "Bloque de madera" },
                               { "tambourine", "Pandereta" } },
                             {}, {} });
            c.kF0 = 150; c.kF1 = 52; c.kPitchDecay = 26; c.kAmpDecay = 8; c.kClick = 0.3; c.kDrive = 1.8;
            c.sTone = 250; c.sToneDecay = 30; c.sNoiseDecay = 22; c.sHP = 900; c.sCentre = 4500;
            c.hDecay = 60; c.hOpenDecay = 9; c.hNoise = 0.35; c.room = 0.1;
        }
        // --- Dancehall -------------------------------------------------------------------------------
        {
            auto& c = add ({ "dancehall", "Dancehall", "Batería caribeña con caja seca, rim y percusiones agudas.",
                             { { "kick", "Bombo" }, { "kick2", "Bombo corto" }, { "snare", "Caja" }, { "rim", "Rim" }, { "clap", "Palmada" },
                               { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" }, { "perc", "Perc" }, { "tomhi", "Timbal" },
                               { "cowbell", "Cencerro" }, { "shaker", "Shaker" } },
                             { true, 1.2f, 0.06f, 0.3f }, {} });
            c.kF0 = 160; c.kF1 = 50; c.kAmpDecay = 9; c.kClick = 0.4;
            c.sTone = 260; c.sToneDecay = 28; c.sNoiseDecay = 24; c.sHP = 1000; c.sCentre = 5000;
            c.hTune = 1.05; c.hDecay = 60; c.percF0 = 900; c.percF1 = 760; c.percDecay = 20;
        }
        // --- R&B ----------------------------------------------------------------------------------------
        {
            auto& c = add ({ "rnb", "R&B", "Suave y elegante: chasquidos, caja con sala, triángulo y un 808 redondo.",
                             { { "kick", "Bombo" }, { "kick3", "Bombo largo" }, { "snare", "Caja" }, { "snap", "Chasquido" },
                               { "clap", "Palmada" }, { "rim", "Rim" }, { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" },
                               { "shaker", "Shaker" }, { "triangle", "Triángulo" }, { "tambourine", "Pandereta" }, { "crash", "Platillo" } },
                             { true, 1.8f, 0.08f, 0.1f }, {} });
            c.kF0 = 130; c.kF1 = 46; c.kPitchDecay = 22; c.kAmpDecay = 7; c.kClick = 0.2; c.kDrive = 1.6;
            c.sTone = 200; c.sToneDecay = 20; c.sNoiseDecay = 16; c.sHP = 600; c.sCentre = 3500;
            c.hTune = 0.95; c.hDecay = 45; c.hNoise = 0.4; c.room = 0.3; c.roomSize = 0.6;
        }
        // --- Techno --------------------------------------------------------------------------------------
        {
            auto& c = add ({ "techno", "Techno", "Bombo largo y contundente, hi-hats metálicos y ride. Para la pista de baile.",
                             { { "kick", "Bombo" }, { "kick2", "Bombo corto" }, { "kick3", "Bombo largo" }, { "clap", "Palmada" },
                               { "rim", "Rim" }, { "hihat", "Hi-hat" }, { "hihat2", "Hi-hat corto" }, { "openhat", "Hi-hat abierto" },
                               { "ride", "Ride" }, { "crash", "Platillo" }, { "perc", "Perc" }, { "tommid", "Tom" } },
                             {}, {} });
            c.kF0 = 150; c.kF1 = 45; c.kPitchDecay = 24; c.kAmpDecay = 4.5; c.kClick = 0.6; c.kDrive = 3.0; c.kLen = 0.8;
            c.cHP = 850; c.cCentre = 1200; c.cTail = 10;
            c.hTune = 1.15; c.hDecay = 50; c.hOpenDecay = 4; c.hNoise = 0.25; c.room = 0.15;
        }
        // --- Jersey Club -----------------------------------------------------------------------------------
        {
            auto& c = add ({ "jersey", "Jersey Club", "Bombos rápidos con mucha pegada y palmadas brillantes.",
                             { { "kick", "Bombo" }, { "kick2", "Bombo corto" }, { "snare", "Caja" }, { "clap", "Palmada" },
                               { "clap2", "Palmada doble" }, { "snap", "Chasquido" }, { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" },
                               { "perc", "Perc" }, { "crash", "Platillo" } },
                             {}, {} });
            c.kF0 = 200; c.kF1 = 55; c.kPitchDecay = 34; c.kAmpDecay = 8; c.kClick = 0.55; c.kDrive = 2.6;
            c.sTone = 280; c.sToneDecay = 30; c.sNoiseDecay = 24; c.sHP = 1100; c.sCentre = 5500;
            c.cHP = 1100; c.cCentre = 1600; c.cTail = 16; c.hTune = 1.1; c.hDecay = 60;
        }
        // --- Dubstep / Bass ------------------------------------------------------------------------------------
        {
            auto& c = add ({ "dubstep", "Dubstep / Bass", "Bombo enorme, caja con mucha sala y un impacto para los drops.",
                             { { "kick", "Bombo" }, { "kick3", "Bombo largo" }, { "snare", "Caja" }, { "snare2", "Caja 2" },
                               { "clap", "Palmada" }, { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" }, { "crash", "Platillo" },
                               { "tomlo", "Tom grave" }, { "impact", "Impacto" } },
                             { true, 2.4f, 0.12f, 0.7f }, {} });
            c.kF0 = 160; c.kF1 = 42; c.kPitchDecay = 28; c.kAmpDecay = 5; c.kClick = 0.6; c.kDrive = 3.5; c.kLen = 0.7;
            c.sTone = 220; c.sToneDecay = 18; c.sNoiseDecay = 12; c.sHP = 600; c.sCentre = 3500; c.sLen = 0.45;
            c.room = 0.3; c.roomSize = 0.7;
        }
        // --- Pop -------------------------------------------------------------------------------------------------
        {
            auto& c = add ({ "pop", "Pop", "Batería limpia y moderna para canciones: caja con brillo, chasquidos y pandereta.",
                             { { "kick", "Bombo" }, { "kick2", "Bombo corto" }, { "snare", "Caja" }, { "clap", "Palmada" },
                               { "snap", "Chasquido" }, { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" }, { "shaker", "Shaker" },
                               { "tambourine", "Pandereta" }, { "ride", "Ride" }, { "crash", "Platillo" }, { "tommid", "Tom" } },
                             {}, {} });
            c.kF0 = 155; c.kF1 = 50; c.kPitchDecay = 26; c.kAmpDecay = 7; c.kClick = 0.35; c.kDrive = 1.9;
            c.sTone = 215; c.sToneDecay = 20; c.sNoiseDecay = 15; c.sCentre = 4000; c.room = 0.15;
        }
        // --- Synthwave / 80s ------------------------------------------------------------------------------------
        {
            auto& c = add ({ "synthwave", "Synthwave (años 80)", "La famosa caja enorme con reverb cortada de los 80 y toms electrónicos.",
                             { { "kick", "Bombo" }, { "snare", "Caja 80s" }, { "clap", "Palmada" }, { "hihat", "Hi-hat" },
                               { "openhat", "Hi-hat abierto" }, { "tomhi", "Tom agudo" }, { "tommid", "Tom medio" }, { "tomlo", "Tom grave" },
                               { "crash", "Platillo" }, { "cowbell", "Cencerro" } },
                             {}, {} });
            c.kF0 = 140; c.kF1 = 48; c.kPitchDecay = 24; c.kAmpDecay = 6; c.kClick = 0.3; c.kDrive = 1.8;
            c.sTone = 190; c.sToneDecay = 16; c.sNoiseDecay = 10; c.sHP = 500; c.sCentre = 3000;
            c.hTune = 0.95; c.hDecay = 45; c.room = 0.55; c.roomSize = 0.8; c.gated = true;
        }
        // --- Clásico 808 ------------------------------------------------------------------------------------------
        {
            auto& c = add ({ "classic808", "Clásico 808", "Inspirado en la legendaria caja de ritmos de 1980: bombo largo, claves, congas y cencerro.",
                             { { "kick", "Bombo" }, { "kick3", "Bombo largo" }, { "snare", "Caja" }, { "clap", "Palmada" }, { "rim", "Rim" },
                               { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" }, { "cowbell", "Cencerro" }, { "clave", "Clave" },
                               { "conga", "Conga" }, { "tomhi", "Tom agudo" }, { "tomlo", "Tom grave" }, { "shaker", "Maracas" },
                               { "crash", "Platillo" } },
                             { true, 2.0f, 0.06f, 0.2f }, {} });
            c.kF0 = 120; c.kF1 = 45; c.kPitchDecay = 14; c.kAmpDecay = 3.0; c.kClick = 0.15; c.kDrive = 1.3; c.kLen = 1.2;
            c.sTone = 240; c.sToneDecay = 22; c.sNoiseDecay = 20; c.sHP = 1200; c.sCentre = 6000;
            c.cHP = 900; c.cCentre = 1200; c.cTail = 12; c.hDecay = 55; c.hOpenDecay = 6; c.hNoise = 0.15;
        }
        // --- Clásico 909 ---------------------------------------------------------------------------------------------
        {
            auto& c = add ({ "classic909", "Clásico 909", "Inspirado en la caja de ritmos de la música house y techno: bombo con golpe y ride brillante.",
                             { { "kick", "Bombo" }, { "kick2", "Bombo corto" }, { "snare", "Caja" }, { "clap", "Palmada" }, { "rim", "Rim" },
                               { "hihat", "Hi-hat" }, { "openhat", "Hi-hat abierto" }, { "ride", "Ride" }, { "crash", "Platillo" },
                               { "tomhi", "Tom agudo" }, { "tommid", "Tom medio" }, { "tomlo", "Tom grave" } },
                             {}, {} });
            c.kF0 = 170; c.kF1 = 52; c.kPitchDecay = 28; c.kAmpDecay = 5; c.kClick = 0.6; c.kDrive = 2.0; c.kLen = 0.6;
            c.sTone = 230; c.sToneDecay = 25; c.sNoiseDecay = 18; c.sHP = 800; c.sCentre = 5000; c.sNoise = 0.9; c.sToneAmt = 0.5;
            c.cHP = 800; c.cCentre = 1300; c.cTail = 12; c.hTune = 1.05; c.hDecay = 48; c.hNoise = 0.2;
        }
        // --- Percusión latina ------------------------------------------------------------------------------------------
        {
            auto& c = add ({ "latin", "Percusión latina", "Congas, bongós, timbal, cencerro, claves, güiro, maracas y más para salsa, cumbia y bachata.",
                             { { "conga", "Conga" }, { "tomlo", "Tumbadora" }, { "bongo", "Bongó" }, { "tomhi", "Timbal" },
                               { "cowbell", "Cencerro" }, { "clave", "Claves" }, { "guiro", "Güiro" }, { "shaker", "Maracas" },
                               { "tambourine", "Pandereta" }, { "triangle", "Triángulo" }, { "woodblock", "Bloque de madera" } },
                             {}, {} });
            c.room = 0.12;
        }
        // --- Efectos (FX) ---------------------------------------------------------------------------------------------------
        {
            KitInfo fx { "fx", "Efectos (FX)", "Sonidos para transiciones y momentos especiales: subidas, impactos, caídas y más.",
                         { { "riser", "Subida (riser)" }, { "downlifter", "Bajada" }, { "impact", "Impacto" }, { "subdrop", "Caída de sub" },
                           { "reversecrash", "Platillo al revés" }, { "sweep", "Barrido" }, { "laser", "Láser" }, { "vinyl", "Ruido de vinilo" } },
                         {}, {} };
            fx.isEffects = true;
            add (std::move (fx)).room = 0.2;
        }

        return list;
    }
}

const std::vector<KitInfo>& all()
{
    static const std::vector<KitInfo> list = makeKits();
    return list;
}

const KitInfo* find (const juce::String& kitId)
{
    for (auto& k : all())
        if (kitId == k.id)
            return &k;
    return nullptr;
}

juce::String samplePath (const KitInfo& kit, const SoundInfo& sound)
{
    return juce::String ("builtin:") + kit.id + "/" + sound.id;
}

juce::String roleOf (const juce::String& path)
{
    if (! path.startsWith ("builtin:"))
        return {};
    auto rest = path.fromFirstOccurrenceOf ("builtin:", false, false);
    return rest.containsChar ('/') ? rest.fromFirstOccurrenceOf ("/", false, false) : rest;
}

// ============================================================================
//  Sonidos de sintetizador y tipos de 808
// ============================================================================
const std::vector<SynthPreset>& synthPresets()
{
    static const std::vector<SynthPreset> list = {
        { "Pad cálido",          "Fondo suave que entra poco a poco",        0, 0.8f,   1.0f,  0.8f,  1.5f,  2200.0f,  0.15f, 18.0f, 60 },
        { "Lead brillante",      "Melodía principal que destaca",             0, 0.005f, 0.2f,  0.8f,  0.2f,  8000.0f,  0.3f,  7.0f,  72 },
        { "Pluck",               "Notas cortas, como pellizcar una cuerda",  0, 0.002f, 0.25f, 0.0f,  0.25f, 3200.0f,  0.45f, 6.0f,  72 },
        { "Bajo sinte",          "Bajo grave con carácter",                   1, 0.003f, 0.3f,  0.7f,  0.1f,  900.0f,   0.35f, 3.0f,  36 },
        { "Sub bajo",            "Bajo muy grave y limpio",                   2, 0.005f, 0.3f,  0.9f,  0.15f, 18000.0f, 0.0f,  0.0f,  36 },
        { "Bajo ácido",          "Bajo chillón estilo acid house",            0, 0.002f, 0.25f, 0.2f,  0.1f,  700.0f,   0.85f, 0.0f,  36 },
        { "Piano eléctrico",     "Teclado suave para acordes",                3, 0.003f, 0.9f,  0.25f, 0.5f,  5000.0f,  0.1f,  4.0f,  60 },
        { "Campanas",            "Notas brillantes que se apagan despacio",   3, 0.001f, 1.6f,  0.0f,  1.2f,  9000.0f,  0.2f,  12.0f, 84 },
        { "Cuerdas",             "Como una orquesta de violines",             0, 0.35f,  0.8f,  0.85f, 0.9f,  3800.0f,  0.1f,  14.0f, 60 },
        { "Órgano",              "Sonido continuo de órgano",                 1, 0.01f,  0.1f,  1.0f,  0.08f, 6000.0f,  0.0f,  2.0f,  60 },
        { "Metales",             "Trompetas sintéticas para golpes",          0, 0.06f,  0.3f,  0.7f,  0.25f, 2600.0f,  0.35f, 9.0f,  60 },
        { "Arpegio",             "Notas muy cortas para patrones rápidos",    1, 0.001f, 0.12f, 0.0f,  0.1f,  4000.0f,  0.5f,  5.0f,  72 },
        { "Ambiente",            "Capa etérea para fondos",                   2, 1.5f,   2.0f,  0.9f,  2.5f,  3000.0f,  0.0f,  9.0f,  60 },
        { "Lead retro 8 bits",   "Como en los videojuegos antiguos",          1, 0.005f, 0.1f,  0.9f,  0.1f,  12000.0f, 0.1f,  0.0f,  72 },
    };
    return list;
}

const std::vector<Bass808Named>& bass808Presets()
{
    static const std::vector<Bass808Named> list = {
        { "808 clásico",            "Equilibrado, para cualquier estilo",       1.5f, 0.08f, 0.3f },
        { "808 largo",              "Suena mucho rato, ideal para trap lento",  3.0f, 0.10f, 0.25f },
        { "808 corto (punch)",      "Golpe corto y seco",                       0.6f, 0.03f, 0.35f },
        { "808 distorsionado",      "Fuerte y agresivo, se oye en el móvil",    2.0f, 0.08f, 0.8f },
        { "808 con mucho glide",    "Se desliza mucho entre notas (drill)",     2.2f, 0.25f, 0.4f },
        { "808 sub limpio",         "Muy grave y sin distorsión",               2.0f, 0.06f, 0.0f },
    };
    return list;
}

// ============================================================================
//  Síntesis de los sonidos
// ============================================================================
namespace
{
    constexpr double twoPi = juce::MathConstants<double>::twoPi;

    struct OnePoleHP
    {
        double a = 0.0, prevIn = 0.0, prevOut = 0.0;
        OnePoleHP (double cutoff, double sr) { set (cutoff, sr); }
        void set (double cutoff, double sr) { const double rc = 1.0 / (twoPi * cutoff); a = rc / (rc + 1.0 / sr); }
        double process (double x) { prevOut = a * (prevOut + x - prevIn); prevIn = x; return prevOut; }
    };

    struct OnePoleLP
    {
        double a = 0.0, state = 0.0;
        OnePoleLP (double cutoff, double sr) { set (cutoff, sr); }
        void set (double cutoff, double sr) { const double dt = 1.0 / sr, rc = 1.0 / (twoPi * cutoff); a = dt / (rc + dt); }
        double process (double x) { state += a * (x - state); return state; }
    };

    struct BandPass
    {
        juce::IIRFilter f;
        double sr;
        BandPass (double freq, double q, double sampleRate) : sr (sampleRate) { set (freq, q); }
        void set (double freq, double q) { f.setCoefficients (juce::IIRCoefficients::makeBandPass (sr, juce::jlimit (20.0, sr * 0.45, freq), q)); }
        double process (double x) { return f.processSingleSampleRaw ((float) x); }
    };

    struct Gen
    {
        double sr;
        juce::AudioBuffer<float> buffer;
        float* out = nullptr;
        int n = 0;
        juce::Random random { 4242 };

        Gen (double sampleRate, double seconds) : sr (sampleRate)
        {
            n = juce::jmax (1, (int) (seconds * sr));
            buffer.setSize (1, n);
            buffer.clear();
            out = buffer.getWritePointer (0);
        }

        double noise() { return random.nextDouble() * 2.0 - 1.0; }
        double t (int i) const { return i / sr; }
    };

    // Las 6 frecuencias "metálicas" de los platillos de la Roland 808
    constexpr double metalFreqs[] = { 205.3, 304.4, 369.6, 522.7, 540.0, 800.0 };

    double metal (std::array<double, 6>& phase, double tune, double sr)
    {
        double m = 0.0;
        for (size_t k = 0; k < 6; ++k)
        {
            phase[k] += metalFreqs[k] * tune / sr;
            if (phase[k] >= 1.0) phase[k] -= 1.0;
            m += phase[k] < 0.5 ? 1.0 : -1.0;
        }
        return m / 6.0;
    }

    // --- Batería ------------------------------------------------------------------
    void kick (Gen& g, double f0, double f1, double pitchDecay, double ampDecay, double click, double drive)
    {
        double phase = 0.0;
        const double norm = std::tanh (drive);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double f = f1 + (f0 - f1) * std::exp (-t * pitchDecay);
            phase += twoPi * f / g.sr;
            const double body = std::sin (phase) * std::exp (-t * ampDecay);
            const double c = g.noise() * std::exp (-t * 350.0) * click;
            g.out[i] = (float) (std::tanh ((body + c) * drive) / norm);
        }
    }

    void snare (Gen& g, double toneHz, double toneDecay, double noiseDecay, double hp, double centre, double noiseAmt, double toneAmt)
    {
        OnePoleHP h (hp, g.sr);
        BandPass l (centre, 0.55, g.sr);
        OnePoleLP soft1 (centre * 1.8, g.sr), soft2 (centre * 2.2, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double tone = (std::sin (twoPi * toneHz * t) + 0.5 * std::sin (twoPi * toneHz * 1.52 * t)) * std::exp (-t * toneDecay) * toneAmt;
            const double rattle = soft2.process (soft1.process (l.process (h.process (g.noise())))) * std::exp (-t * noiseDecay) * noiseAmt;
            g.out[i] = (float) std::tanh (tone * 1.3 + rattle * 2.2);
        }
    }

    void clap (Gen& g, double hp, double centre, double tail, double gap)
    {
        OnePoleHP h (hp, g.sr);
        BandPass b (centre, 0.7, g.sr);
        OnePoleLP l (6000.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            double env = 0.0;
            for (int k = 0; k < 3; ++k)
            {
                const double s = k * gap;
                if (t >= s && t < s + gap)
                    env = juce::jmax (env, std::exp (-(t - s) * 170.0));
            }
            if (t >= 3.0 * gap)
                env = juce::jmax (env, std::exp (-(t - 3.0 * gap) * tail) * 0.85);
            g.out[i] = (float) (l.process (b.process (h.process (g.noise()))) * env * 2.2);
        }
    }

    void hat (Gen& g, double tune, double decay, double noiseAmt)
    {
        std::array<double, 6> phase {};
        BandPass b (10000.0, 0.8, g.sr);
        OnePoleHP h1 (7000.0, g.sr), h2 (7000.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double x = metal (phase, tune, g.sr) + g.noise() * noiseAmt;
            const double env = juce::jmin (1.0, t / 0.0008) * std::exp (-t * decay);
            g.out[i] = (float) (h2.process (h1.process (b.process (x))) * env * 2.5);
        }
    }

    void cymbal (Gen& g, double tune, double decay, double bell)
    {
        std::array<double, 6> phase {};
        OnePoleHP h1 (4000.0, g.sr), h2 (4000.0, g.sr);
        OnePoleLP l (14000.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            double x = metal (phase, tune, g.sr) * 0.6 + g.noise() * 0.7;
            x = l.process (h2.process (h1.process (x)));
            // "Campana" del ride
            x += bell * (std::sin (twoPi * 2650.0 * t) + 0.6 * std::sin (twoPi * 4100.0 * t)) * std::exp (-t * 3.5) * 0.25;
            const double env = juce::jmin (1.0, t / 0.002) * std::exp (-t * decay);
            g.out[i] = (float) (x * env * 1.8);
        }
    }

    void tom (Gen& g, double f0, double f1, double decay)
    {
        double phase = 0.0;
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double f = f1 + (f0 - f1) * std::exp (-t * 14.0);
            phase += twoPi * f / g.sr;
            g.out[i] = (float) (std::tanh (std::sin (phase) * 1.4 + g.noise() * std::exp (-t * 300.0) * 0.2) * std::exp (-t * decay));
        }
    }

    void rim (Gen& g)
    {
        BandPass b (3200.0, 2.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double x = std::sin (twoPi * 1720.0 * t) * std::exp (-t * 110.0) * 0.6
                           + std::sin (twoPi * 460.0 * t) * std::exp (-t * 80.0) * 0.5
                           + b.process (g.noise()) * std::exp (-t * 220.0) * 1.2;
            g.out[i] = (float) std::tanh (x * 1.5);
        }
    }

    void cowbell (Gen& g, double tune)
    {
        BandPass b (900.0 * tune, 1.2, g.sr);
        double p1 = 0.0, p2 = 0.0;
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            p1 += 587.0 * tune / g.sr; if (p1 >= 1.0) p1 -= 1.0;
            p2 += 845.0 * tune / g.sr; if (p2 >= 1.0) p2 -= 1.0;
            const double x = (p1 < 0.5 ? 1.0 : -1.0) + (p2 < 0.5 ? 1.0 : -1.0);
            const double env = std::exp (-t * 40.0) * 0.4 + std::exp (-t * 9.0) * 0.6;
            g.out[i] = (float) (b.process (x) * env * 1.6);
        }
    }

    void shaker (Gen& g, double decay)
    {
        OnePoleHP h (5000.0, g.sr);
        OnePoleLP l (13000.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double env = t < 0.02 ? t / 0.02 : std::exp (-(t - 0.02) * decay);
            g.out[i] = (float) (l.process (h.process (g.noise())) * env * 1.2);
        }
    }

    void tambourine (Gen& g)
    {
        std::array<double, 6> phase {};
        OnePoleHP h (6000.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            // Varias sonajas que suenan casi a la vez
            double env = 0.0;
            for (double s : { 0.0, 0.012, 0.027 })
                if (t >= s)
                    env += std::exp (-(t - s) * 22.0) * (s == 0.0 ? 1.0 : 0.6);
            const double x = metal (phase, 2.3, g.sr) * 0.5 + g.noise() * 0.6;
            g.out[i] = (float) (h.process (x) * env * 1.3);
        }
    }

    void triangle (Gen& g)
    {
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double x = std::sin (twoPi * 4186.0 * t) + 0.45 * std::sin (twoPi * 6930.0 * t) + 0.25 * std::sin (twoPi * 9650.0 * t);
            g.out[i] = (float) (x * juce::jmin (1.0, t / 0.001) * std::exp (-t * 2.2) * 0.6);
        }
    }

    void snap (Gen& g)
    {
        BandPass b (2400.0, 1.6, g.sr);
        for (int i = 0; i < g.n; ++i)
            g.out[i] = (float) (b.process (g.noise()) * std::exp (-g.t (i) * 85.0) * 2.6);
    }

    void perc (Gen& g, double f0, double f1, double decay, double slap)
    {
        double phase = 0.0;
        BandPass b (2500.0, 1.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double f = f1 + (f0 - f1) * std::exp (-t * 45.0);
            phase += twoPi * f / g.sr;
            g.out[i] = (float) ((std::sin (phase) + 0.15 * std::sin (phase * 2.01)) * std::exp (-t * decay)
                                 + b.process (g.noise()) * std::exp (-t * 300.0) * slap);
        }
    }

    void clave (Gen& g, double freq)
    {
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            g.out[i] = (float) ((std::sin (twoPi * freq * t) + 0.2 * std::sin (twoPi * freq * 2.7 * t)) * std::exp (-t * 55.0));
        }
    }

    void woodblock (Gen& g, double tune)
    {
        BandPass b (1300.0 * tune, 3.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            const double x = (std::sin (twoPi * 1050.0 * tune * t) + 0.5 * std::sin (twoPi * 1680.0 * tune * t)) * std::exp (-t * 45.0)
                           + b.process (g.noise()) * std::exp (-t * 250.0) * 0.8;
            g.out[i] = (float) x;
        }
    }

    void guiro (Gen& g)
    {
        BandPass b (3500.0, 1.5, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            // 14 rasguños seguidos
            const double tick = std::fmod (t, 0.022);
            const double env = t < 0.31 ? std::exp (-tick * 280.0) : 0.0;
            g.out[i] = (float) (b.process (g.noise()) * env * 2.0);
        }
    }

    // --- Efectos (FX) ------------------------------------------------------------------
    void riser (Gen& g, bool up)
    {
        const double len = g.n / g.sr;
        BandPass b (300.0, 2.0, g.sr);
        double phase = 0.0;
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i), p = juce::jlimit (0.0, 1.0, t / len);
            const double pos = up ? p : 1.0 - p;
            if (i % 32 == 0)
                b.set (250.0 * std::pow (36.0, pos), 2.0);
            phase += twoPi * (110.0 * std::pow (8.0, pos)) / g.sr;
            const double amp = up ? p * p : (1.0 - p) * (1.0 - p);
            g.out[i] = (float) ((b.process (g.noise()) * 1.4 + std::sin (phase) * 0.25) * amp);
        }
    }

    void impact (Gen& g)
    {
        double phase = 0.0;
        OnePoleLP l (1500.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            phase += twoPi * (30.0 + 45.0 * std::exp (-t * 6.0)) / g.sr;
            const double boom = std::sin (phase) * std::exp (-t * 1.3);
            const double crash = l.process (g.noise()) * std::exp (-t * 4.0) * 0.8;
            g.out[i] = (float) std::tanh ((boom + crash) * 1.8);
        }
    }

    void subdrop (Gen& g)
    {
        double phase = 0.0;
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            phase += twoPi * (28.0 + 92.0 * std::exp (-t * 2.2)) / g.sr;
            g.out[i] = (float) std::tanh (std::sin (phase) * 1.5 * std::exp (-t * 1.4));
        }
    }

    void sweep (Gen& g)
    {
        const double len = g.n / g.sr;
        OnePoleHP h (200.0, g.sr);
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i), p = t / len;
            if (i % 32 == 0)
                h.set (200.0 * std::pow (40.0, p), g.sr);
            g.out[i] = (float) (h.process (g.noise()) * std::sin (juce::MathConstants<double>::pi * p) * 0.9);
        }
    }

    void laser (Gen& g)
    {
        double phase = 0.0;
        for (int i = 0; i < g.n; ++i)
        {
            const double t = g.t (i);
            phase += twoPi * (150.0 + 3400.0 * std::exp (-t * 14.0)) / g.sr;
            g.out[i] = (float) ((phase - std::floor (phase / twoPi) * twoPi < juce::MathConstants<double>::pi ? 0.6 : -0.6) * std::exp (-t * 6.0));
        }
    }

    void vinyl (Gen& g)
    {
        OnePoleLP l (3000.0, g.sr);
        juce::Random r (7);
        for (int i = 0; i < g.n; ++i)
        {
            double x = l.process (g.noise()) * 0.15;
            if (r.nextInt (1800) == 0)
                x += (r.nextDouble() - 0.5) * 1.2;
            g.out[i] = (float) x;
        }
    }

    // --- Procesado final ------------------------------------------------------------------
    void lofi (Gen& g)
    {
        OnePoleLP l1 (4200.0, g.sr), l2 (5200.0, g.sr);
        juce::Random dust (99);
        for (int i = 0; i < g.n; ++i)
        {
            double x = std::tanh (l2.process (l1.process (g.out[i])) * 1.4);
            if (dust.nextInt (6000) == 0)
                x += (dust.nextDouble() - 0.5) * 0.06;
            g.out[i] = (float) x;
        }
    }

    void finish (Gen& g, const Character& c)
    {
        if (c.extraDrive > 0.0)
        {
            const float k = (float) (1.0 + c.extraDrive * 4.0);
            for (int i = 0; i < g.n; ++i)
                g.out[i] = std::tanh (g.out[i] * k);
        }
        if (c.lofi)
            lofi (g);
        if (c.room > 0.0)
        {
            juce::Reverb reverb;
            reverb.setSampleRate (g.sr);
            juce::Reverb::Parameters p;
            p.roomSize = (float) c.roomSize;
            p.damping = 0.45f;
            p.wetLevel = (float) c.room;
            p.dryLevel = 1.0f;
            p.width = 0.0f;
            reverb.setParameters (p);
            reverb.processMono (g.out, g.n);
        }
        if (c.gated)
        {
            // Reverb "cortada" de los 80: suena grande y se apaga de golpe
            for (int i = 0; i < g.n; ++i)
            {
                const double t = g.t (i);
                g.out[i] *= (float) (t < 0.2 ? 1.0 : t < 0.26 ? (0.26 - t) / 0.06 : 0.0);
            }
        }

        const float peak = g.buffer.getMagnitude (0, g.n);
        if (peak > 0.0001f)
            g.buffer.applyGain (0.9f / peak);
        const int fade = juce::jmin (g.n, (int) (0.006 * g.sr));
        g.buffer.applyGainRamp (g.n - fade, fade, 1.0f, 0.0f);
    }
}

juce::AudioBuffer<float> render (const juce::String& kitId, const juce::String& sound, double sr)
{
    const auto* kit = find (kitId);
    if (kit == nullptr)
        return {};

    bool listed = false;
    for (auto& s : kit->sounds)
        listed = listed || sound == s.id;
    if (! listed)
        return {};

    const auto& c = kit->character;
    const double tail = c.room > 0.0 ? 0.3 + 0.9 * c.roomSize : 0.0;
    auto make = [sr, tail] (double seconds) { return Gen (sr, seconds + tail); };
    const double pt = c.percTune;

    auto done = [&c] (Gen& g) { finish (g, c); return g.buffer; };

    if (sound == "kick")   { auto g = make (c.kLen);       kick (g, c.kF0, c.kF1, c.kPitchDecay, c.kAmpDecay, c.kClick, c.kDrive); return done (g); }
    if (sound == "kick2")  { auto g = make (c.kLen * 0.7); kick (g, c.kF0 * 1.15, c.kF1 * 1.08, c.kPitchDecay * 1.3, c.kAmpDecay * 1.6, c.kClick * 1.4, c.kDrive * 1.25); return done (g); }
    if (sound == "kick3")  { auto g = make (c.kLen * 1.7); kick (g, c.kF0 * 0.9, c.kF1 * 0.92, c.kPitchDecay * 0.75, c.kAmpDecay * 0.55, c.kClick * 0.6, c.kDrive * 0.9); return done (g); }
    if (sound == "snare")  { auto g = make (c.sLen);       snare (g, c.sTone, c.sToneDecay, c.sNoiseDecay, c.sHP, c.sCentre, c.sNoise, c.sToneAmt); return done (g); }
    if (sound == "snare2") { auto g = make (c.sLen * 0.85); snare (g, c.sTone * 1.18, c.sToneDecay * 1.2, c.sNoiseDecay * 1.3, c.sHP * 1.2, c.sCentre * 1.25, c.sNoise, c.sToneAmt * 0.9); return done (g); }
    if (sound == "clap")   { auto g = make (c.cLen);       clap (g, c.cHP, c.cCentre, c.cTail, c.cGap); return done (g); }
    if (sound == "clap2")  { auto g = make (c.cLen * 1.2); clap (g, c.cHP, c.cCentre * 1.1, c.cTail * 0.75, c.cGap * 1.7); return done (g); }
    if (sound == "snap")   { auto g = make (0.15);         snap (g); return done (g); }
    if (sound == "rim")    { auto g = make (0.12);         rim (g); return done (g); }
    if (sound == "hihat")  { auto g = make (juce::jmin (1.0, 6.0 / c.hDecay)); hat (g, c.hTune, c.hDecay, c.hNoise); return done (g); }
    if (sound == "hihat2") { auto g = make (juce::jmin (1.0, 4.0 / c.hDecay)); hat (g, c.hTune * 1.06, c.hDecay * 1.7, c.hNoise); return done (g); }
    if (sound == "openhat") { auto g = make (juce::jmin (1.6, 5.0 / c.hOpenDecay)); hat (g, c.hTune, c.hOpenDecay, c.hNoise); return done (g); }
    if (sound == "ride")   { auto g = make (2.2);          cymbal (g, c.hTune * 1.4, 1.9, 1.0); return done (g); }
    if (sound == "crash")  { auto g = make (2.0);          cymbal (g, c.hTune * 1.25, 2.4, 0.0); return done (g); }
    if (sound == "tom")    { auto g = make (0.45);         tom (g, 320 * pt, 210 * pt, 9); return done (g); }
    if (sound == "tomhi")  { auto g = make (0.45);         tom (g, 290 * pt, 200 * pt, 10); return done (g); }
    if (sound == "tommid") { auto g = make (0.5);          tom (g, 210 * pt, 145 * pt, 9); return done (g); }
    if (sound == "tomlo")  { auto g = make (0.6);          tom (g, 150 * pt, 100 * pt, 8); return done (g); }
    if (sound == "perc")   { auto g = make (0.3);          perc (g, c.percF0 * pt, c.percF1 * pt, c.percDecay, 0.15); return done (g); }
    if (sound == "conga")  { auto g = make (0.4);          perc (g, 330 * pt, 290 * pt, 13, 0.5); return done (g); }
    if (sound == "bongo")  { auto g = make (0.3);          perc (g, 480 * pt, 410 * pt, 18, 0.45); return done (g); }
    if (sound == "clave")  { auto g = make (0.15);         clave (g, 2500 * pt); return done (g); }
    if (sound == "woodblock") { auto g = make (0.15);      woodblock (g, pt); return done (g); }
    if (sound == "cowbell") { auto g = make (0.45);        cowbell (g, pt); return done (g); }
    if (sound == "shaker") { auto g = make (0.18);         shaker (g, 32); return done (g); }
    if (sound == "tambourine") { auto g = make (0.35);     tambourine (g); return done (g); }
    if (sound == "triangle") { auto g = make (2.2);        triangle (g); return done (g); }
    if (sound == "guiro")  { auto g = make (0.4);          guiro (g); return done (g); }

    if (sound == "riser")        { auto g = make (4.0); riser (g, true); return done (g); }
    if (sound == "downlifter")   { auto g = make (3.0); riser (g, false); return done (g); }
    if (sound == "impact")       { auto g = make (2.8); impact (g); return done (g); }
    if (sound == "subdrop")      { auto g = make (2.0); subdrop (g); return done (g); }
    if (sound == "sweep")        { auto g = make (2.0); sweep (g); return done (g); }
    if (sound == "laser")        { auto g = make (0.5); laser (g); return done (g); }
    if (sound == "vinyl")        { auto g = make (4.0); vinyl (g); return done (g); }
    if (sound == "reversecrash")
    {
        auto g = make (2.2);
        cymbal (g, 1.25, 2.4, 0.0);
        g.buffer.reverse (0, g.n);
        return done (g);
    }

    return {};
}
}
