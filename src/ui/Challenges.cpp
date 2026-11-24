#include "Challenges.h"
#include "audio/Kits.h"
#include <map>
#include <set>

namespace studoo::learn
{
namespace
{
    // ¿De qué tipo es este canal de batería? Mira el sonido incluido y, si es
    // un sample tuyo, el nombre del canal.
    bool isRole (const juce::ValueTree& ch, std::initializer_list<const char*> roles, std::initializer_list<const char*> names)
    {
        const auto role = kits::roleOf (ch[ids::sample].toString());
        for (auto r : roles)
            if (role.startsWith (r))
                return true;
        const auto name = ch[ids::name].toString().toLowerCase();
        for (auto n : names)
            if (name.contains (juce::String::fromUTF8 (n)))
                return true;
        return false;
    }

    bool isKick (const juce::ValueTree& ch)  { return isRole (ch, { "kick" }, { "bombo", "kick" }); }
    bool isSnare (const juce::ValueTree& ch) { return isRole (ch, { "snare", "clap", "snap", "rim" }, { "caja", "palmada", "clap", "snare", "chasquido" }); }
    bool isHat (const juce::ValueTree& ch)   { return isRole (ch, { "hihat", "openhat", "shaker" }, { "hi-hat", "hihat", "hat", "shaker" }); }

    juce::String stepsOf (Project& p, const juce::ValueTree& ch)
    {
        auto pattern = p.getCurrentPattern();
        for (auto item : pattern)
            if (item.hasType (ids::STEPSET) && (int) item[ids::channel] == (int) ch[ids::uid])
                return item[ids::data].toString();
        return {};
    }

    bool hasSteps (const juce::String& data, std::initializer_list<int> steps)
    {
        for (int s : steps)
            if (s >= data.length() || data[s] != '1')
                return false;
        return true;
    }

    // ¿Algún canal de este tipo tiene estos pasos en el patrón que estás viendo?
    bool anyChannel (Project& p, std::function<bool (const juce::ValueTree&)> kind, std::initializer_list<int> steps)
    {
        for (auto ch : p.channels())
            if (kind (ch) && hasSteps (stepsOf (p, ch), steps))
                return true;
        return false;
    }

    juce::Array<juce::ValueTree> notesOf (Project& p, const juce::ValueTree& ch)
    {
        juce::Array<juce::ValueTree> notes;
        for (auto pattern : p.patterns())
            for (auto item : pattern)
                if (item.hasType (ids::NOTE) && (int) item[ids::channel] == (int) ch[ids::uid])
                    notes.add (item);
        return notes;
    }

    bool anyOfType (Project& p, const char* type, std::function<bool (const juce::ValueTree&)> test)
    {
        for (auto ch : p.channels())
            if (ch[ids::type].toString() == type && test (ch))
                return true;
        return false;
    }

    bool anyMixer (Project& p, std::function<bool (const juce::ValueTree&)> test)
    {
        for (int t = 1; t < numMixerTracks; ++t)
            if (test (p.getMixerTrack (t)))
                return true;
        return false;
    }

    float f (const juce::ValueTree& t, const juce::Identifier& id, float def) { return (float) t.getProperty (id, def); }
}

const std::vector<Challenge>& challenges()
{
    static const std::vector<Challenge> list = {
        { "primer_beat", "Tu primer ritmo",
          "Enciende al menos 4 pasos en cualquier canal del Secuenciador.",
          "Haz clic (o toca) en los cuadrados grises de la derecha de cada canal.",
          View::channelRack,
          [] (Project& p) {
              for (auto ch : p.channels())
                  if (stepsOf (p, ch).retainCharacters ("1").length() >= 4)
                      return true;
              return false; } },

        { "bpm", "Cambia la velocidad",
          "Pon el tempo (BPM) a un número distinto de 140.",
          "Arrastra arriba o abajo el número naranja de BPM de la barra superior.",
          View::channelRack,
          [] (Project& p) { return std::abs ((double) p.getState().getProperty (ids::bpm, 140.0) - 140.0) > 0.5; } },

        { "largo_64", "Un patrón de 4 compases",
          "Haz que el patrón que estás viendo dure 64 pasos (4 compases).",
          "En el Secuenciador, abre el desplegable \"16 pasos (1 compás)\" y elige 64 pasos.",
          View::channelRack,
          [] (Project& p) { return Project::getPatternLength (p.getCurrentPattern()) >= 64; } },

        { "caja_2_4", "La caja en el 2 y el 4",
          "Pon la caja (o la palmada) en los pasos 5 y 13.",
          "Son los tiempos 2 y 4: cuenta 1-2-3-4 y la caja cae en el 2 y en el 4.",
          View::channelRack,
          [] (Project& p) { return anyChannel (p, isSnare, { 4, 12 }); } },

        { "bombo_negras", "Four on the floor",
          "Pon el bombo en los pasos 1, 5, 9 y 13 (en cada tiempo).",
          "El bombo cae al principio de cada grupo de 4 pasos.",
          View::channelRack,
          [] (Project& p) { return anyChannel (p, isKick, { 0, 4, 8, 12 }); } },

        { "contratiempo", "El hi-hat a contratiempo",
          "Pon un hi-hat en los pasos 3, 7, 11 y 15 (justo entre los bombos).",
          "Es el \"tss\" del house: pum-TSS-pum-TSS.",
          View::channelRack,
          [] (Project& p) { return anyChannel (p, isHat, { 2, 6, 10, 14 }); } },

        { "hihat_corcheas", "Hi-hats en corcheas",
          "Pon el hi-hat en todos los pasos impares: 1, 3, 5, 7, 9, 11, 13 y 15.",
          "Uno sí, uno no, de principio a fin.",
          View::channelRack,
          [] (Project& p) { return anyChannel (p, isHat, { 0, 2, 4, 6, 8, 10, 12, 14 }); } },

        { "roll", "Haz un roll de hi-hat",
          "Enciende 3 o más pasos SEGUIDOS en un hi-hat (el \"trrrr\" del trap).",
          "Por ejemplo, los pasos 13, 14, 15 y 16.",
          View::channelRack,
          [] (Project& p) {
              for (auto ch : p.channels())
                  if (isHat (ch) && stepsOf (p, ch).contains ("111"))
                      return true;
              return false; } },

        { "dembow", "El dembow",
          "Pon la caja en los pasos 4, 7, 12 y 15: el ritmo del reggaetón.",
          "Es el patrón 3 + 3 + 2. Mira la ilustración de la lección.",
          View::channelRack,
          [] (Project& p) { return anyChannel (p, isSnare, { 3, 6, 11, 14 }); } },

        { "silencio", "Silencia un canal",
          "Silencia (mute) cualquier canal con su botón verde.",
          "Es el círculo verde a la izquierda del nombre del canal. Gris = silenciado.",
          View::channelRack,
          [] (Project& p) {
              for (auto ch : p.channels())
                  if ((bool) ch[ids::muted])
                      return true;
              return false; } },

        { "sample_kit", "Añade un sonido de un kit",
          "Añade al Secuenciador un sonido de cualquier kit incluido.",
          "Pulsa \"+ Añadir\" y elige un sonido suelto de un kit (por ejemplo, Trap > Hi-hat abierto).",
          View::channelRack,
          [] (Project& p) {
              for (auto ch : p.channels())
                  if (ch[ids::sample].toString().startsWith ("builtin:") && ch[ids::sample].toString().containsChar ('/'))
                      return true;
              return false; } },

        { "notas_melodia", "Escribe una melodía",
          "En el Piano roll, pon al menos 4 notas en un sintetizador, con 3 alturas distintas.",
          "Elige un canal de sinte arriba del Piano roll y haz clic en la cuadrícula.",
          View::pianoRoll,
          [] (Project& p) {
              return anyOfType (p, "synth", [&p] (const juce::ValueTree& ch) {
                  auto notes = notesOf (p, ch);
                  std::set<int> pitches;
                  for (auto& n : notes) pitches.insert ((int) n[ids::pitch]);
                  return notes.size() >= 4 && pitches.size() >= 3; }); } },

        { "acorde", "Tu primer acorde",
          "Pon 3 notas que empiecen a la vez en el mismo canal (un acorde).",
          "En el Piano roll, cambia \"Nota sola\" por \"Acorde mayor\" y haz un solo clic.",
          View::pianoRoll,
          [] (Project& p) {
              for (auto ch : p.channels())
              {
                  std::map<double, int> starts;
                  for (auto& n : notesOf (p, ch))
                      if (++starts[(double) n[ids::start]] >= 3)
                          return true;
              }
              return false; } },

        { "808_notas", "Una línea de 808",
          "Escribe al menos 3 notas en un 808, con 2 alturas distintas.",
          "En el Piano roll, elige el canal 808 arriba y pon notas largas.",
          View::pianoRoll,
          [] (Project& p) {
              return anyOfType (p, "808", [&p] (const juce::ValueTree& ch) {
                  auto notes = notesOf (p, ch);
                  std::set<int> pitches;
                  for (auto& n : notes) pitches.insert ((int) n[ids::pitch]);
                  return notes.size() >= 3 && pitches.size() >= 2; }); } },

        { "onda", "Cambia la onda",
          "En un sintetizador, cambia la onda a cuadrada, seno o triángulo.",
          "Selecciona el canal del sinte: abajo verás el desplegable de la onda.",
          View::channelRack,
          [] (Project& p) { return anyOfType (p, "synth", [] (const juce::ValueTree& ch) { return (int) ch[ids::wave] != 0; }); } },

        { "adsr_pad", "Un sonido que entra despacio",
          "Sube el Ataque de un sintetizador a 0,3 segundos o más.",
          "Selecciona el sinte y gira la rueda \"Ataque\" hacia la derecha.",
          View::channelRack,
          [] (Project& p) { return anyOfType (p, "synth", [] (const juce::ValueTree& ch) { return f (ch, ids::attack, 0.0f) >= 0.3f; }); } },

        { "filtro", "Oscurece el sonido",
          "Baja el Filtro de un sintetizador a 1500 Hz o menos.",
          "Selecciona el sinte y gira la rueda \"Filtro\" hacia la izquierda.",
          View::channelRack,
          [] (Project& p) { return anyOfType (p, "synth", [] (const juce::ValueTree& ch) { return f (ch, ids::cutoff, 5000.0f) <= 1500.0f; }); } },

        { "volumen_mezcla", "Ajusta un volumen",
          "En el Mezclador, mueve el volumen de cualquier pista (no el Master).",
          "Arrastra arriba o abajo el fader (la barra vertical) de una pista.",
          View::mixer,
          [] (Project& p) { return anyMixer (p, [] (const juce::ValueTree& t) { return std::abs (f (t, ids::volume, 0.8f) - 0.8f) > 0.05f; }); } },

        { "panorama", "Mueve un sonido a un lado",
          "Gira el PAN de un canal o de una pista hacia la izquierda o la derecha.",
          "Gira la rueda PAN: el sonido se irá hacia un lado.",
          View::mixer,
          [] (Project& p) {
              for (auto ch : p.channels())
                  if (std::abs (f (ch, ids::pan, 0.0f)) >= 0.2f) return true;
              return anyMixer (p, [] (const juce::ValueTree& t) { return std::abs (f (t, ids::pan, 0.0f)) >= 0.2f; }); } },

        { "eq", "Usa el ecualizador",
          "Sube o baja los Graves, Medios o Agudos de una pista del Mezclador.",
          "Gira cualquiera de las tres ruedas de EQ de una pista.",
          View::mixer,
          [] (Project& p) {
              return anyMixer (p, [] (const juce::ValueTree& t) {
                  return std::abs (f (t, ids::eqLow, 0.0f)) >= 1.0f || std::abs (f (t, ids::eqMid, 0.0f)) >= 1.0f
                      || std::abs (f (t, ids::eqHigh, 0.0f)) >= 1.0f; }); } },

        { "compresor", "Enciende un compresor",
          "Activa el compresor de una pista del Mezclador.",
          "Pulsa el botón \"On\" del compresor en cualquier pista.",
          View::mixer,
          [] (Project& p) { return anyMixer (p, [] (const juce::ValueTree& t) { return (bool) t[ids::compOn]; }); } },

        { "reverb", "Añade espacio",
          "Sube la Reverb (Mezcla) de una pista.",
          "En el Mezclador, gira la rueda \"Mezcla\" de la reverb.",
          View::mixer,
          [] (Project& p) { return anyMixer (p, [] (const juce::ValueTree& t) { return f (t, ids::reverbMix, 0.0f) >= 0.1f; }); } },

        { "delay", "Añade un eco",
          "Sube el Delay (Mezcla) de una pista.",
          "En el Mezclador, gira la rueda \"Mezcla\" del delay.",
          View::mixer,
          [] (Project& p) { return anyMixer (p, [] (const juce::ValueTree& t) { return f (t, ids::delayMix, 0.0f) >= 0.1f; }); } },

        { "dos_patrones", "Un segundo patrón",
          "Crea un segundo patrón (por ejemplo, para el estribillo).",
          "En el Secuenciador, pulsa \"Patrón...\" y elige \"Nuevo patrón\" o \"Duplicar este patrón\".",
          View::channelRack,
          [] (Project& p) { return p.patterns().getNumChildren() >= 2; } },

        { "playlist", "Monta una canción",
          "Coloca al menos 4 bloques en la Playlist.",
          "En la Playlist, haz clic en la cuadrícula para colocar el patrón elegido arriba.",
          View::playlist,
          [] (Project& p) { return p.playlist().getNumChildren() >= 4; } },

        { "automatizacion", "Haz que algo se mueva solo",
          "Crea una automatización y mueve un punto para que la línea suba o baje.",
          "En Automatización, pulsa \"+ Nueva\", elige un control y arrastra un punto.",
          View::automation,
          [] (Project& p) {
              for (auto lane : p.automation())
              {
                  std::set<int> values;
                  for (auto pt : lane) values.insert (juce::roundToInt ((float) pt[ids::value] * 100.0f));
                  if (values.size() >= 2) return true;
              }
              return false; } },

        { "grabacion", "Graba algo",
          "Graba tu voz o un instrumento con el botón rojo.",
          "Pulsa el círculo rojo (o la tecla R), habla o canta, y vuelve a pulsarlo.",
          View::playlist,
          [] (Project& p) { return anyOfType (p, "audio", [] (const juce::ValueTree&) { return true; }); } },

        { "base_genero", "Carga una base",
          "Carga la base de cualquier género (desde una lección o con \"Beat de ejemplo\").",
          "Cualquier lección del nivel Géneros musicales tiene el botón \"Cargar la base\".",
          View::channelRack,
          [] (Project& p) {
              for (auto pattern : p.patterns())
                  if (pattern[ids::name].toString().startsWith ("Base "))
                      return true;
              return false; } },

        { "swing", "Ponle swing",
          "Elige un Swing de 20% o más en el Secuenciador.",
          "Es el desplegable \"Swing 0%\" de arriba, al lado de la duración del patrón.",
          View::channelRack,
          [] (Project& p) { return (double) p.getState().getProperty (ids::swing, 0.0) >= 0.19; } },

        { "velocity", "Notas suaves",
          "En el carril VELOCITY del Piano roll, baja al menos una nota por debajo de la mitad.",
          "Debajo de las notas hay una barra por nota: arrástrala hacia abajo.",
          View::pianoRoll,
          [] (Project& p) {
              for (auto pattern : p.patterns())
                  for (auto item : pattern)
                      if (item.hasType (ids::NOTE) && (float) item.getProperty (ids::velocity, 0.8f) < 0.5f)
                          return true;
              return false; } },

        { "sidechain", "Sidechain de verdad",
          "En el Mezclador, sube la Bajada del SIDECHAIN de una pista al 30% o más.",
          "Elige en el desplegable la pista del bombo y gira la rueda \"Bajada\".",
          View::mixer,
          [] (Project& p) {
              for (int t = 1; t < numMixerTracks; ++t)
                  if ((float) p.getMixerTrack (t).getProperty (ids::sidechain, 0.0f) >= 0.3f)
                      return true;
              return false; } },

        { "chops", "Trocea un sample",
          "Pon un sampler en 4 trozos o más y toca al menos 3 trozos distintos en el Piano roll.",
          "Sube \"Trozos\" en el panel del sonido y pon notas de distintas alturas.",
          View::pianoRoll,
          [] (Project& p) {
              return anyOfType (p, "sampler", [&p] (const juce::ValueTree& ch) {
                  if ((int) ch.getProperty (ids::slices, 1) < 4)
                      return false;
                  std::set<int> pitches;
                  for (auto& n : notesOf (p, ch)) pitches.insert ((int) n[ids::pitch]);
                  return pitches.size() >= 3; }); } },

        { "envio", "Una pista de efectos",
          "En el Mezclador, manda (ENVÍO) una pista a otra con la rueda Envío al 20% o más.",
          "En la sección ENVÍO elige la pista de destino en el desplegable y gira la rueda.",
          View::mixer,
          [] (Project& p) {
              for (int t = 1; t < numMixerTracks; ++t)
              {
                  auto tr = p.getMixerTrack (t);
                  const int to = tr.getProperty (ids::sendTo, 0);
                  if (to > t && to < numMixerTracks && (float) tr.getProperty (ids::sendLevel, 0.0f) >= 0.2f)
                      return true;
              }
              return false; } },

        { "editar_grabacion", "Edita tu grabación",
          "En la Playlist, corta una grabación en dos o quítale el principio arrastrando su borde izquierdo.",
          "Clic derecho (o mantener pulsado) sobre la grabación → Cortar aquí.",
          View::playlist,
          [] (Project& p) {
              int audioClips = 0;
              for (auto clip : p.playlist())
                  if (clip[ids::kind].toString() == "audio")
                  {
                      ++audioClips;
                      if ((double) clip[ids::offset] > 0.0)
                          return true;
                  }
              return audioClips >= 2; } },

        { "efectos_creativos", "Efectos creativos",
          "En el Mezclador, usa el Filtro, el Sucio (Distor.) o el Chorus de una pista (al 20% o más).",
          "Son las tres ruedas de la sección \"FILTRO · SUCIO · CHORUS\".",
          View::mixer,
          [] (Project& p) {
              for (int t = 0; t < numMixerTracks; ++t)
              {
                  auto tr = p.getMixerTrack (t);
                  if (std::abs ((float) tr.getProperty (ids::trackFilter, 0.0f)) >= 0.2f
                      || (float) tr.getProperty (ids::distortion, 0.0f) >= 0.2f
                      || (float) tr.getProperty (ids::chorusMix, 0.0f) >= 0.2f)
                      return true;
              }
              return false; } },
    };
    return list;
}

const Challenge* findChallenge (const juce::String& id)
{
    for (auto& c : challenges())
        if (id == c.id)
            return &c;
    return nullptr;
}
}
