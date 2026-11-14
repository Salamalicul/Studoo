#pragma once
// ============================================================================
//  Version.h: el número de versión de Studoo ("1.6.0").
//  En Windows y Linux lo pone CMake (STUDOO_VERSION). En Android lo pone
//  Projucer (JUCE_APP_VERSION) y aquí lo convertimos en texto.
// ============================================================================
#ifndef STUDOO_VERSION
 #define STUDOO_VERSION_TEXT2(x) #x
 #define STUDOO_VERSION_TEXT(x) STUDOO_VERSION_TEXT2(x)
 #define STUDOO_VERSION STUDOO_VERSION_TEXT (JUCE_APP_VERSION)
#endif
