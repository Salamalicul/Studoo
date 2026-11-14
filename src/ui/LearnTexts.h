#pragma once
// ============================================================================
//  LearnTexts: las lecciones del apartado "Aprende", ordenadas por niveles.
//
//  Formato del texto de cada lección (una cosa por línea):
//    ## Título          -> subtítulo
//    1. Texto           -> paso numerado (con su círculo)
//    - Texto            -> punto de una lista
//    > Texto            -> cuadro "Truco" (azul)
//    ! Texto            -> cuadro "Ojo" (naranja)
//    ? Texto            -> cuadro "Practica" (verde)
//    [anim:id|Pie]      -> ilustración animada (ver Illustrations.cpp)
//    [ir:vista|Texto]   -> botón que abre una parte de Studoo
//                          (secuenciador, pianoroll, playlist, mezclador, automatizacion)
//    **texto**          -> negrita, dentro de cualquier línea
//    Línea vacía        -> separa párrafos
// ============================================================================

namespace studoo::learn
{
    struct Lesson
    {
        const char* level;    // nivel al que pertenece
        const char* title;
        const char* minutes;  // tiempo aproximado de lectura y práctica
        const char* video;    // qué buscar en YouTube para ver vídeos del tema
        const char* body;
    };

    inline constexpr const char* levels[] = {
        "Nivel 1 · Primeros pasos",
        "Nivel 2 · Ritmo y batería",
        "Nivel 3 · Notas y melodía",
        "Nivel 4 · Diseño de sonido",
        "Nivel 5 · Mezcla",
        "Nivel 6 · De beat a canción",
        "Nivel 7 · Seguir mejorando",
    };

    inline constexpr Lesson lessons[] = {
    // =========================================================================
    { "Nivel 1 · Primeros pasos", "Cómo aprender con Studoo", "3 min",
      "como empezar a producir musica desde cero",
R"TXT(Bienvenido a **Aprende**. Aquí tienes todo lo necesario para pasar de no saber nada a terminar tus propias canciones, en orden y sin prisas.

[anim:ruta|La ruta: 7 niveles, de lo más básico a seguir mejorando]

## Cómo funciona este apartado
1. Las lecciones están ordenadas en **7 niveles**. Hazlas en orden la primera vez.
2. Cada lección tiene explicaciones, **ilustraciones que se mueven** y pasos para hacerlo en Studoo.
3. Al final de cada lección hay un cuadro verde **Practica**: hazlo de verdad, es lo que más enseña.
4. Pulsa **Marcar como aprendida** cuando la termines. Verás tu progreso en la lista de la izquierda.
5. Si quieres verlo explicado en vídeo, pulsa **Ver vídeos en YouTube**: se abre una búsqueda en español sobre ese tema.

> Es mejor practicar 20 minutos cada día que 4 horas un solo día. La música se aprende con las manos y con los oídos.

## Si tienes prisa
Haz la lección **Tu primer beat en 5 minutos** y el **tutorial guiado** (botón de arriba a la derecha). Luego vuelve aquí cuando quieras entender el porqué de las cosas.

? Pulsa "Siguiente lección" abajo y empieza el Nivel 1.)TXT" },

    { "Nivel 1 · Primeros pasos", "Qué es una DAW", "3 min",
      "que es una DAW explicacion para principiantes",
R"TXT(DAW significa **Digital Audio Workstation** (estación de trabajo de audio digital). Es el programa donde se hace música con el ordenador. Studoo es una DAW.

[anim:flujo|El camino de tu música: sonidos → patrones → mezcla → canción]

## Las partes de Studoo
- **Secuenciador** (F6): donde haces los ritmos encendiendo cuadraditos.
- **Piano roll** (F7): donde escribes melodías, acordes y el 808 nota a nota.
- **Playlist** (F5): donde colocas los patrones uno detrás de otro para formar la canción.
- **Mezclador** (F9): donde ajustas el volumen y los efectos de cada sonido.
- **Automatización** (F8): donde haces que un control se mueva solo.

## La idea más importante
Una canción se hace con **patrones**: trocitos de música de 1 a 4 compases (por ejemplo, la batería del estribillo). Primero creas patrones y luego los colocas en la playlist.

> Pasa el ratón por encima de cualquier botón de Studoo y te dirá qué hace. Y en cada sección hay un botón "?" con su explicación.

? Pulsa F6, F7, F5 y F9 y fíjate en cada sección. No toques nada todavía: solo mira dónde está cada cosa.)TXT" },

    { "Nivel 1 · Primeros pasos", "Tu primer beat en 5 minutos", "5 min",
      "como hacer un beat de trap facil paso a paso",
R"TXT(Vamos a hacer un ritmo sencillo pero completo. Cada cuadrado del secuenciador es un **paso**; si está encendido, suena.

[anim:beat_basico|El ritmo que vas a hacer: bombo, caja y hi-hat]

## Paso a paso
1. Abre el **Secuenciador** (F6).
2. En la fila **Bombo**, enciende los pasos **1** y **9** (haz clic en ellos).
3. En la fila **Caja**, enciende los pasos **5** y **13**.
4. En la fila **Hi-hat**, enciende todos los pasos impares: 1, 3, 5, 7, 9, 11, 13 y 15.
5. Pulsa la **barra espaciadora** para escucharlo. Púlsala otra vez para parar.
6. Prueba a mover el **BPM** (arriba): arrástralo hacia arriba y hacia abajo y escucha cómo cambia la velocidad.

[ir:secuenciador|Abrir el secuenciador]

> Puedes arrastrar el ratón sobre varios pasos para encenderlos de una vez. Clic derecho los apaga.

## ¿Por qué suena bien?
El bombo y la caja se turnan: "pum - pa - pum - pa". Ese vaivén es la base de casi toda la música moderna. El hi-hat rellena el espacio y marca la velocidad.

? Cambia de sitio un golpe de bombo (por ejemplo, del paso 9 al 11) y escucha cómo cambia la sensación. Prueba 3 variaciones distintas.)TXT" },

    { "Nivel 1 · Primeros pasos", "BPM y tempo", "4 min",
      "que es el BPM en la musica explicado",
R"TXT(El **BPM** (beats por minuto) es la **velocidad** de la canción: cuántos pulsos hay en un minuto. Es el ritmo al que mueves la cabeza.

[anim:pulso|Cada luz es un pulso. Los pulsos se cuentan de 4 en 4]

## Velocidades típicas por estilo
- **R&B lento**: 60-80 BPM
- **Boom bap / hip-hop clásico**: 85-95 BPM
- **Reggaetón**: 88-100 BPM
- **Pop**: 100-125 BPM
- **House**: 120-128 BPM
- **Trap**: 130-160 BPM (se siente "a medio tiempo", como si fuera la mitad)
- **Drill**: 140-145 BPM
- **Drum & bass**: 170-175 BPM

## En Studoo
1. Arrastra el número **BPM** de la barra superior hacia arriba o abajo.
2. Doble clic sobre él para volver a 140.
3. Activa el **Metrónomo** para oír un "clic" en cada pulso.

> Para saber el BPM de una canción que te gusta, busca en internet "nombre de la canción BPM".

? Pon el metrónomo, elige 90 BPM y da palmas siguiendo el clic. Luego prueba a 140. ¿Notas lo distinto que se siente?)TXT" },

    { "Nivel 1 · Primeros pasos", "Compases, tiempos y pasos", "4 min",
      "compases y tiempos musica explicacion facil",
R"TXT(La música se cuenta en **compases**. Casi toda la música moderna usa compases de **4 tiempos**: "1, 2, 3, 4 - 1, 2, 3, 4...".

Cada tiempo se divide en **4 pasos** (semicorcheas). Por eso un patrón de 16 pasos dura **1 compás**: 4 tiempos × 4 pasos = 16 pasos.

[anim:pasos|16 pasos = 4 tiempos = 1 compás. La luz recorre los pasos al ritmo del BPM]

## Cómo lo ves en el secuenciador
- Los pasos **1, 5, 9 y 13** son el comienzo de cada tiempo (sus números están más claros).
- Los pasos están pintados en **grupos de 4** con dos tonos distintos para que no te pierdas.
- Arriba puedes elegir patrones de **16, 32 o 64 pasos** (1, 2 o 4 compases).

## Cómo se cuenta
Los músicos cuentan los pasos así: "**1** y-e-a **2** y-e-a **3** y-e-a **4** y-e-a". El número es el tiempo y "y-e-a" son los pasos de en medio.

? Elige "32 pasos" y haz un ritmo de 2 compases donde el segundo compás sea un poco distinto al primero (por ejemplo, un golpe de bombo extra al final).)TXT" },

    // =========================================================================
    { "Nivel 2 · Ritmo y batería", "Las partes de la batería", "5 min",
      "partes de la bateria en la produccion musical kick snare hihat",
R"TXT(Cada sonido de la batería tiene un trabajo. Si entiendes el trabajo de cada uno, sabrás dónde ponerlo.

[anim:beat_basico|Bombo y caja se turnan; el hi-hat rellena]

## Los sonidos principales
- **Bombo** (kick): el golpe grave, el "pum". Da fuerza y marca el pulso.
- **Caja** (snare): el golpe seco, el "pa". Casi siempre en los tiempos 2 y 4 (pasos 5 y 13).
- **Palmada** (clap): parecida a la caja. Se pone con ella o en su lugar.
- **Hi-hat cerrado**: el "ts" corto y metálico. Marca el ritmo rápido.
- **Hi-hat abierto**: un "tssss" más largo. Da aire, normalmente entre golpes.
- **Toms y percusiones**: congas, shakers, cencerros... dan color y estilo.
- **Platillo** (crash): un golpe fuerte que marca el principio de una sección.

## Kits incluidos
Studoo trae **18 kits** (Trap, Drill, Reggaetón, House, Lo-fi, Afrobeat...). En el secuenciador pulsa **+ Añadir → Sonidos incluidos → Kits de batería**.

> "Usar este kit" cambia los sonidos de tu batería por los de otro estilo **sin borrar tu ritmo**. Es una forma rapidísima de probar ideas.

[ir:secuenciador|Abrir el secuenciador]

? Haz un ritmo con el kit Trap y luego pulsa "Usar este kit" con el kit Boom Bap. Escucha cómo el mismo ritmo cambia de estilo.)TXT" },

    { "Nivel 2 · Ritmo y batería", "Ritmo de trap paso a paso", "8 min",
      "como hacer un beat de trap hi hats rolls tutorial",
R"TXT(El trap se toca rápido (130-160 BPM) pero se **siente a medio tiempo**: la caja suena solo una vez por compás, en el paso 9.

[anim:beat_trap|Bombo, clap en el 9, hi-hats con rolls y el 808 siguiendo al bombo]

## Paso a paso (a 140 BPM)
1. **Bombo**: pasos 1 y 11. Puedes añadir uno en el 8 para más movimiento.
2. **Clap o caja**: solo en el paso **9**.
3. **Hi-hats**: enciende todos los pasos impares (1, 3, 5...).
4. **Rolls**: elige un patrón de 32 pasos y enciende **todos** los pasos seguidos en algún trozo (por ejemplo, los últimos 4). Eso hace la típica ráfaga "trrrr".
5. **808**: escribe en el piano roll notas largas que empiecen donde está el bombo (lo verás en el Nivel 3).

## Lo que hace que suene a trap
- Mucho espacio: pocos golpes de bombo, pero muy fuertes.
- Los hi-hats son los protagonistas: cambia su ritmo cada pocos compases.
- El 808 es el bajo y lleva la melodía grave.

> No pongas rolls en todos los compases. Si todo es especial, nada es especial: úsalos al final de cada 2 o 4 compases.

? Haz un patrón de trap de 2 compases (32 pasos) con un roll de hi-hat solo al final del segundo compás.)TXT" },

    { "Nivel 2 · Ritmo y batería", "Ritmo de reggaetón (dembow)", "6 min",
      "como hacer ritmo de reggaeton dembow tutorial",
R"TXT(El reggaetón se basa en un ritmo llamado **dembow**. Es muy reconocible: "pum - ta - pum - ta" con la caja un poquito antes y después de lo normal.

[anim:beat_reggaeton|El dembow: bombo en cada tiempo y caja en los pasos 4, 7, 12 y 15]

## Paso a paso (a 95 BPM)
1. **Bombo**: pasos 1, 5, 9 y 13 (uno en cada tiempo).
2. **Caja**: pasos **4, 7, 12 y 15**.
3. **Hi-hat**: pasos impares, o solo en los tiempos si quieres algo más limpio.
4. Añade una **percusión** (shaker o güiro del kit Reggaetón o Latina) en todos los pasos impares con el volumen bajo.

## Lo que hace que suene a reggaetón
- El patrón de la caja es lo que hace el "dembow". Es la parte más importante.
- El bombo constante da el movimiento para bailar.
- Las percusiones latinas rellenan y dan sabor.

> Usa el kit **Reggaetón** de "+ Añadir → Sonidos incluidos" y ya tendrás los sonidos adecuados.

? Haz el dembow y después crea un segundo patrón quitando el bombo durante un compás. Eso es un "drop": cuando el bombo vuelve, la energía sube.)TXT" },

    { "Nivel 2 · Ritmo y batería", "Ritmo de house", "6 min",
      "como hacer un beat de house four on the floor",
R"TXT(El house (120-128 BPM) usa el ritmo **four on the floor**: el bombo suena en **cada tiempo**, sin parar. Es el ritmo de la música de discoteca.

[anim:beat_house|Bombo en cada tiempo, clap en 2 y 4, hi-hat abierto entre bombos]

## Paso a paso (a 124 BPM)
1. **Bombo**: pasos 1, 5, 9 y 13.
2. **Clap**: pasos 5 y 13.
3. **Hi-hat abierto**: pasos **3, 7, 11 y 15**, justo entre los bombos ("pum-tss-pum-tss").
4. **Hi-hat cerrado**: todos los pasos impares, con el volumen más bajo.

## Lo que hace que suene a house
- El bombo nunca para: es un pulso hipnótico.
- El hi-hat abierto entre bombos da el "rebote".
- Las canciones cambian poco a poco: se añade o quita un sonido cada 8 compases.

? Haz este ritmo con el kit House y luego prueba con el kit Clásico 909, el sonido original del house de los años 80 y 90.)TXT" },

    { "Nivel 2 · Ritmo y batería", "Boom bap y el swing", "6 min",
      "como hacer un beat boom bap tutorial",
R"TXT(El boom bap (85-95 BPM) es el sonido del hip-hop clásico: bombo ("boom") y caja ("bap") muy marcados, con un aire relajado.

[anim:beat_boombap|Boom bap: bombo en 1, 8 y 11; caja en 5 y 13]

## Paso a paso (a 90 BPM)
1. **Bombo**: pasos 1, 8 y 11.
2. **Caja**: pasos 5 y 13.
3. **Hi-hat**: pasos impares.
4. Usa el kit **Boom Bap** o **Lo-fi** para un sonido más "de vinilo".

## El swing
El **swing** es retrasar un poquito los golpes que caen entre pulsos. Así el ritmo deja de sonar como un robot y suena más humano y con más "groove".

[anim:swing|Arriba: recto (robot). Abajo: con swing (los pasos pares se retrasan)]

> En Studoo puedes conseguir un efecto parecido con la velocidad de las notas (lección siguiente) y poniendo el hi-hat solo en algunos pasos.

? Haz un boom bap y compáralo con tu ritmo de trap. Fíjate en que en el boom bap la caja suena dos veces por compás y en el trap solo una.)TXT" },

    { "Nivel 2 · Ritmo y batería", "Dar vida a la batería", "6 min",
      "como hacer que la bateria suene humana variaciones fills",
R"TXT(Un ritmo que se repite exactamente igual durante 3 minutos cansa. Estos trucos le dan vida.

[anim:velocidad|Golpes con distinta fuerza (velocidad): fuerte, suave, medio...]

## Trucos
1. **Variaciones**: duplica el patrón (menú "Patrón..." → Duplicar) y cambia 2 o 3 golpes. Alterna los dos en la playlist.
2. **Fills**: al final de cada 4 u 8 compases, añade algo especial (un roll, toms, quitar el bombo).
3. **Silencios**: quitar la batería durante 1 compás antes del estribillo da mucha fuerza a la vuelta.
4. **Velocidad**: en el piano roll, las notas con menos velocidad suenan más suaves. Alterna hi-hats fuertes y suaves.
5. **Capas**: pon una palmada a la vez que la caja para que suene más gorda.
6. **Percusiones**: un shaker o una conga de fondo dan movimiento sin molestar.

> Cuantos menos sonidos, más fuerte suena cada uno. Antes de añadir algo, prueba a quitar algo.

? Crea 3 patrones de batería: normal, con variación y con fill. Colócalos en la playlist así: normal, variación, normal, fill.)TXT" },

    // =========================================================================
    { "Nivel 3 · Notas y melodía", "Las notas y el teclado", "5 min",
      "notas musicales en el piano para principiantes",
R"TXT(En español las notas se llaman **Do, Re, Mi, Fa, Sol, La, Si**. Después de Si vuelve a empezar Do, pero más agudo: eso es una **octava**.

[anim:teclado|Las teclas blancas son Do, Re, Mi, Fa, Sol, La, Si. Las negras son los sostenidos (#)]

## Las teclas
- **Blancas**: las 7 notas "naturales".
- **Negras**: las notas de en medio. Se llaman sostenidos (#): Do#, Re#, Fa#, Sol#, La#.
- La distancia de una tecla a la de al lado (blanca o negra) es un **semitono**.
- 12 semitonos = 1 octava.

## Cómo encontrar Do
Do es la tecla blanca que está justo **a la izquierda del grupo de 2 teclas negras**. En el piano roll de Studoo verás escrito "Do3", "Do4"... El número es la octava.

> Para el 808 y los bajos usa notas entre **Do1 y Do3**. Para melodías, entre **Do4 y Do6**.

[ir:pianoroll|Abrir el piano roll]

? Abre el piano roll, haz clic en las teclas del piano de la izquierda y busca todos los Do. Luego toca Do-Re-Mi-Fa-Sol-La-Si-Do.)TXT" },

    { "Nivel 3 · Notas y melodía", "Escalas: mayor y menor", "6 min",
      "escalas mayores y menores explicacion facil productores",
R"TXT(Una **escala** es un grupo de notas que suenan bien juntas. Si usas solo las notas de una escala, es muy difícil que algo suene mal.

[anim:escalas|Do mayor (alegre) y La menor (triste) usan las mismas teclas blancas]

## Las dos escalas más usadas
- **Mayor**: suena alegre, brillante. Do mayor = todas las teclas blancas empezando en Do.
- **Menor**: suena triste, oscura, misteriosa. La menor = todas las teclas blancas empezando en La.

## El truco más fácil
Usa **solo teclas blancas** y haz que tu melodía empiece y termine en **La**: sonará a menor (perfecto para trap, drill o reggaetón). Si empieza y termina en **Do**, sonará a mayor.

## Escalas muy usadas en música urbana
- **Do menor**: Do, Re, Re#, Fa, Sol, Sol#, La#.
- **La menor**: La, Si, Do, Re, Mi, Fa, Sol.
- **Fa# menor**: Fa#, Sol#, La, Si, Do#, Re, Mi.

> La "receta" de la escala menor es: tono, semitono, tono, tono, semitono, tono, tono (un tono = 2 semitonos). Con ella puedes construir la escala menor desde cualquier nota.

? Escribe una melodía de 8 notas usando solo teclas blancas que empiece y termine en La. Después muévela entera a otras notas y escucha qué pasa.)TXT" },

    { "Nivel 3 · Notas y melodía", "Acordes y progresiones", "8 min",
      "acordes y progresiones de acordes para productores",
R"TXT(Un **acorde** son 3 o más notas a la vez. Una **progresión** es una serie de acordes que se repite (por ejemplo, 4 acordes de 1 compás cada uno).

[anim:acordes|Una progresión clásica: La menor → Fa mayor → Do mayor → Sol mayor]

## Cómo se construye un acorde
- **Mayor** (alegre): la nota base + 4 semitonos + 3 semitonos. Ejemplo: Do-Mi-Sol.
- **Menor** (triste): la nota base + 3 semitonos + 4 semitonos. Ejemplo: La-Do-Mi.
- Truco: en las teclas blancas, coge una tecla, **salta una** y coge la siguiente, salta otra y coge la siguiente.

## Progresiones que funcionan siempre
- **La menor → Fa → Do → Sol**: la de miles de canciones pop y urbanas.
- **La menor → Re menor → Mi menor**: oscura, perfecta para trap.
- **Do → Sol → La menor → Fa**: alegre, de pop.

## En Studoo
1. Abre el **piano roll**.
2. Arriba, en **Acorde**, elige "Menor" o "Mayor".
3. Haz clic en la nota base: Studoo pone el acorde completo.
4. Alarga los acordes para que cada uno dure 1 compás (16 pasos).

[ir:pianoroll|Abrir el piano roll]

? Haz la progresión La menor → Fa → Do → Sol con el sintetizador "Pad" (+ Añadir → Sintetizador). Ponle tu ritmo de batería debajo.)TXT" },

    { "Nivel 3 · Notas y melodía", "Usar el piano roll", "5 min",
      "como usar el piano roll tutorial",
R"TXT(El piano roll muestra las notas como barras. Es donde escribes melodías, acordes y bajos.

[anim:pianoroll|Más arriba = más agudo. Más a la derecha = más tarde. Más larga = suena más tiempo]

## Cómo se usa
1. Elige arriba el **canal** que quieres editar (por ejemplo, "Sintetizador" o "808").
2. **Clic** en un hueco vacío: añade una nota.
3. **Arrastra** una nota: muévela.
4. **Arrastra su borde derecho**: alárgala o acórtala.
5. **Clic derecho**: bórrala.
6. **Ajuste**: elige a qué cuadrícula se pegan las notas (1 paso, 1/2 paso, 1 tiempo...).

> Las notas grises de fondo son las de otros canales. Te ayudan a hacer que la melodía encaje con el 808 o con los acordes.

[ir:pianoroll|Abrir el piano roll]

? Copia esta melodía en el canal Sintetizador: La, Do, Mi, Re (una nota por tiempo). Luego cambia la última nota por Sol y escucha la diferencia.)TXT" },

    { "Nivel 3 · Notas y melodía", "Cómo hacer una melodía pegadiza", "8 min",
      "como hacer melodias pegadizas produccion musical",
R"TXT(Una melodía pegadiza es **sencilla, repetida y con una pequeña sorpresa**. No hace falta saber mucha teoría.

[anim:melodia|Pregunta (compás 1) y respuesta (compás 2): la segunda mitad contesta a la primera]

## La fórmula pregunta-respuesta
1. **Pregunta**: escribe una frase corta de 3 a 5 notas en el primer compás.
2. **Repite** la misma frase en el segundo compás, pero **cambia el final**.
3. Repite las dos frases otra vez. ¡Ya tienes una melodía de 4 compases!

## Consejos
- Usa **pocas notas** de la escala (3 o 4 distintas bastan).
- **Deja silencios**: los huecos hacen que la melodía respire.
- Repite el **mismo ritmo** aunque cambies las notas.
- Las notas largas al final de cada frase dan sensación de "punto final".
- Canta o tararea tu idea antes de escribirla: si se puede cantar, es pegadiza.

> Si no se te ocurre nada, coge las notas de tu primer acorde y cambia su orden.

? Escribe una melodía pregunta-respuesta de 2 compases en La menor usando solo La, Do, Mi y Sol.)TXT" },

    { "Nivel 3 · Notas y melodía", "El 808 paso a paso", "8 min",
      "como hacer un 808 con glide tutorial",
R"TXT(El **808** viene de una caja de ritmos de 1980. Su bombo largo y grave se convirtió en el **bajo** del hip-hop, el trap y el drill.

[anim:808|Un 808: golpe inicial, cola grave que se apaga y glide (deslizamiento) entre dos notas]

## Cómo escribir un 808
1. Añade un 808: **+ Añadir → Sonidos incluidos → 808** (hay 6 tipos).
2. Abre el **piano roll** y elige el canal 808.
3. Pon las notas **donde está el bombo**, para que golpeen juntos.
4. Usa notas graves: entre **Do1 y Do3**.
5. Usa la **nota base** de tu canción (si estás en La menor, La).
6. Haz notas **largas** y cambia de nota de vez en cuando (por ejemplo, cuando cambia el acorde).

## El glide
Si dos notas del 808 se **solapan**, el sonido se desliza de una a otra ("uuuh"). Es el famoso **slide** del trap. Controla cuánto tarda con la rueda **Glide** del panel del instrumento.

! Si el bombo y el 808 suenan a la vez y se hace una "bola", baja un poco el volumen de uno de los dos o quita el bombo donde el 808 ya golpea fuerte.

? Haz un 808 que siga tu bombo de trap usando La1 y Sol1, con un glide de La1 a Do2 al final del patrón.)TXT" },

    // =========================================================================
    { "Nivel 4 · Diseño de sonido", "Samples y sampler", "5 min",
      "que es un sample y como usar samples",
R"TXT(Un **sample** es un trozo de audio grabado: un golpe de batería, una voz, un acorde de guitarra... Un **sampler** es un instrumento que reproduce samples.

[anim:sampler|El sampler cambia el tono del sample: más arriba = más agudo y más rápido]

## Cómo usarlos en Studoo
1. **Sonidos incluidos**: más de 200 en "+ Añadir → Sonidos incluidos".
2. **Tus samples**: "+ Añadir → Sample", o arrastra archivos WAV o MP3 desde tu explorador al secuenciador.
3. **Kit de carpeta**: "+ Añadir → Kit de batería (carpeta)" carga todos los sonidos de una carpeta.

## Ajustes del sampler
- **One-shot**: el sample suena entero aunque la nota sea corta. Actívalo para la batería.
- **Nota base**: la nota en la que el sample suena con su tono original.

! Usa solo samples gratis o "royalty free" (libres de derechos) si vas a publicar tu música. Usar trozos de canciones de otros sin permiso puede darte problemas.

? Busca en internet "free drum samples" o "samples gratis royalty free", descarga un pack y arrastra un par de sonidos al secuenciador.)TXT" },

    { "Nivel 4 · Diseño de sonido", "Las ondas del sintetizador", "6 min",
      "tipos de ondas sintetizador seno cuadrada sierra",
R"TXT(Un **sintetizador** crea el sonido desde cero con **osciladores**, que producen ondas. La forma de la onda decide el "color" del sonido.

[anim:ondas|Las 4 ondas básicas y cómo suenan]

## Las 4 ondas
- **Seno**: suave y pura, casi sin brillo. Ideal para sub-bajos.
- **Triángulo**: dulce, como una flauta.
- **Cuadrada**: hueca, como de videojuego antiguo.
- **Sierra**: brillante y potente. La más usada para pads, leads y bajos.

## En Studoo
1. Añade un sintetizador: **+ Añadir → Sintetizador**. Hay 14 sonidos ya preparados.
2. En el panel de abajo, cambia la **Onda** y escucha la diferencia.
3. **Desafinar** separa un poco los dos osciladores: el sonido se vuelve más ancho, como un coro.

> Empieza siempre desde un sonido preparado y cambia una cosa cada vez. Así aprendes qué hace cada control.

? Toca la misma nota con las 4 ondas y describe con tus palabras cómo suena cada una.)TXT" },

    { "Nivel 4 · Diseño de sonido", "La envolvente ADSR", "6 min",
      "envolvente ADSR explicacion sintetizador",
R"TXT(La **envolvente ADSR** controla cómo cambia el volumen de cada nota desde que empieza hasta que se apaga.

[anim:adsr|Ataque, Caída, Sostenido y Relajación. El punto recorre la nota]

## Las 4 partes
- **Ataque (A)**: lo que tarda en llegar al máximo. Corto = golpea. Largo = entra suave.
- **Caída (D)**: lo que tarda en bajar desde el máximo hasta el sostenido.
- **Sostenido (S)**: el volumen mientras mantienes la nota (es un nivel, no un tiempo).
- **Relajación (R)**: lo que tarda en apagarse cuando sueltas la nota.

## Recetas
- **Pluck** (sonido de cuerda pulsada): ataque 0, caída corta, sostenido 0, relajación corta.
- **Pad** (colchón de fondo): ataque largo, sostenido alto, relajación larga.
- **Lead** (melodía principal): ataque corto, sostenido alto, relajación media.

? Con el mismo sintetizador, haz primero un pluck y luego un pad cambiando solo las 4 ruedas ADSR.)TXT" },

    { "Nivel 4 · Diseño de sonido", "Filtro y resonancia", "5 min",
      "filtro paso bajo cutoff resonancia explicacion",
R"TXT(El **filtro** quita frecuencias. El de Studoo es **paso bajo**: deja pasar los graves y quita los agudos a partir de un punto (el **corte**).

[anim:filtro|El filtro se abre y se cierra: más abierto = más brillante]

## Controles
- **Filtro (corte)**: más bajo = sonido más oscuro y apagado. Más alto = más brillante.
- **Resonancia**: realza justo la zona del corte. Con mucha, suena "chillón" o "líquido".

## Ideas
- Un pad con el filtro bajo queda de fondo y no molesta a la voz.
- **Automatiza** el filtro para que se abra poco a poco antes del estribillo: crea mucha tensión.

> Cuando algo "molesta" en la mezcla pero no sabes por qué, prueba a bajar su filtro.

? Pon un acorde largo con un sintetizador de sierra y mueve la rueda Filtro mientras suena.)TXT" },

    { "Nivel 4 · Diseño de sonido", "Plugins VST3", "5 min",
      "mejores plugins VST3 gratis",
R"TXT(Los **plugins** son instrumentos o efectos de otros fabricantes que se añaden a Studoo. **VST3** es el formato más común.

## Cómo instalarlos
1. Descarga e instala el plugin. Normalmente va a **C:\Program Files\Common Files\VST3** (Windows) o **~/.vst3** (Linux).
2. En Studoo pulsa **Plugins → Options... → Scan for new or updated VST3 plug-ins**.
3. **Instrumentos**: secuenciador → **+ Añadir → Plugin VST3**.
4. **Efectos**: mezclador → botón **+ Plugin** de una pista.

## Plugins gratuitos muy buenos
- **Vital**: sintetizador moderno muy potente.
- **Surge XT**: sintetizador libre con cientos de sonidos.
- **Dexed**: sintetizador de estilo años 80.
- **Valhalla Supermassive**: reverbs y delays enormes.

! Descarga plugins solo desde la web oficial de cada fabricante. Nunca uses versiones "crackeadas": suelen traer virus.

? Instala un sintetizador gratuito, escanéalo y úsalo para tocar tu progresión de acordes.)TXT" },

    // =========================================================================
    { "Nivel 5 · Mezcla", "Mezclar: el volumen", "6 min",
      "como mezclar un beat volumen gain staging principiantes",
R"TXT(**Mezclar** es ajustar cada sonido para que todos se escuchen bien juntos. El 80 % de una buena mezcla es simplemente **el volumen**.

[anim:volumenes|Una mezcla típica de trap: el bombo y el 808 mandan, y el Master no llega al rojo]

## El orden para mezclar
1. Pon todos los faders del mezclador **abajo**.
2. Sube primero lo más importante: **bombo y 808**.
3. Añade la **caja o clap** hasta que se oiga clara.
4. Luego la **melodía o los acordes**, y después los **hi-hats** y percusiones.
5. Si hay voz, debe entenderse siempre por encima del beat.

## El Master
El Master es la suma de todo. Sus luces **no deben llegar al rojo**. Si llegan, baja los faders de las pistas (no el Master).

> Escucha tu mezcla a **volumen bajo**: si a volumen bajo se entiende todo, la mezcla está bien.

[ir:mezclador|Abrir el mezclador]

? Pon a cero todos los faders y vuelve a mezclar tu beat siguiendo el orden de arriba.)TXT" },

    { "Nivel 5 · Mezcla", "Panorama: izquierda y derecha", "4 min",
      "panorama en la mezcla paneo explicacion",
R"TXT(El **panorama** (pan) coloca cada sonido a la izquierda, al centro o a la derecha. Así cada uno tiene su sitio y la mezcla suena más ancha.

[anim:panorama|Cada sonido en su sitio: lo importante en el centro y el resto a los lados]

## Dónde va cada cosa
- **Centro**: bombo, caja, 808, voz principal. Son la base.
- **Un poco a los lados**: hi-hats, percusiones, guitarras, coros.
- **Muy a los lados**: efectos y detalles.

## En Studoo
- Cada canal del secuenciador tiene su rueda **PAN**.
- Cada pista del mezclador también tiene **PAN**.
- Doble clic en la rueda la devuelve al centro.

> Si mueves un sonido a la izquierda, intenta mover otro parecido a la derecha para que la mezcla no se "caiga" hacia un lado.

? Pon el hi-hat un 30 % a la derecha y una percusión un 30 % a la izquierda. Escucha con auriculares.)TXT" },

    { "Nivel 5 · Mezcla", "El ecualizador (EQ)", "6 min",
      "ecualizacion para principiantes graves medios agudos",
R"TXT(El **EQ** sube o baja zonas de frecuencia: graves, medios y agudos. Sirve para que cada sonido tenga su propio espacio y no se pisen.

[anim:eq|Dónde vive cada instrumento: graves a la izquierda, agudos a la derecha]

## Las 3 zonas
- **Graves** (menos de 250 Hz): peso y potencia. Bombo y 808.
- **Medios** (250-4000 Hz): cuerpo. Voces, cajas, sintetizadores, guitarras.
- **Agudos** (más de 4000 Hz): brillo y aire. Hi-hats, platillos, la "s" de la voz.

## Reglas de oro
1. Es mejor **quitar** lo que sobra que subir lo que falta.
2. Quita graves a lo que no los necesita (hi-hats, voces, pads): dejarás sitio al 808.
3. Si dos sonidos chocan, baja en uno la zona donde destaca el otro.

> En el mezclador de Studoo cada pista tiene EQ de **Graves, Medios y Agudos**. Gira hacia la izquierda para quitar.

? Baja los graves del hi-hat y de la melodía y escucha cómo el 808 se oye más limpio.)TXT" },

    { "Nivel 5 · Mezcla", "El compresor", "6 min",
      "que es un compresor de audio explicacion facil",
R"TXT(El **compresor** baja automáticamente las partes más fuertes. Así el sonido queda más igualado, más "pegado" y más presente.

[anim:compresor|Arriba: sin compresor. Abajo: con compresor (los picos que pasan de la línea se bajan)]

## Controles
- **Umbral**: a partir de qué volumen empieza a actuar. Más bajo = comprime más.
- **Ratio**: cuánto baja lo que pasa del umbral. 2:1 = suave, 4:1 = normal, 10:1 = muy fuerte.

## Dónde usarlo
- **Voces**: para que se entiendan siempre, aunque cantes más fuerte o más flojo.
- **808 y bajo**: para que suenen igual de fuerte en todas las notas.
- **Batería**: para que suene con más pegada.

! Si comprimes demasiado, el sonido pierde vida y suena "aplastado". Empieza con ratio 3:1 y ve bajando el umbral poco a poco.

? Activa el compresor en la pista de tu 808 y baja el umbral hasta que notes que todas las notas suenan igual de fuertes.)TXT" },

    { "Nivel 5 · Mezcla", "Reverb: el espacio", "5 min",
      "como usar reverb en la mezcla",
R"TXT(La **reverb** imita los rebotes del sonido en una sala. Hace que algo suene lejos, en una habitación o en una catedral.

[anim:reverb|El golpe original y su "cola" de rebotes: sala pequeña y sala grande]

## Controles
- **Mezcla**: cuánta reverb se oye.
- **Tamaño**: habitación pequeña o espacio enorme.

## Consejos
- **Poca** reverb en voces, claps y melodías: les da profundidad.
- **Nada** (o casi nada) en el bombo y el 808: se vuelven confusos.
- Un sonido con mucha reverb parece más **lejano**; uno sin reverb, más **cerca**.

> Pon la reverb a tu gusto y luego bájala un poco. Casi siempre se pone demasiada.

? Pon reverb en el clap con Tamaño grande y Mezcla al 20 %. Luego quítala y compara.)TXT" },

    { "Nivel 5 · Mezcla", "Delay: el eco", "5 min",
      "como usar delay en voces y melodias",
R"TXT(El **delay** repite el sonido como un eco. En Studoo va sincronizado con el BPM y rebota de izquierda a derecha (ping-pong).

[anim:delay|Cada repetición suena más baja y en el otro lado]

## Controles
- **Mezcla**: el volumen del eco.
- **Tiempo**: cuánto tarda cada repetición, en tiempos: 0,25 = semicorchea, 0,5 = corchea, 0,75 = corchea con puntillo, 1 = negra.
- **Repeticiones** (feedback): cuántas veces se repite. Más alto = más ecos.

## Ideas
- Delay de **0,75** en una melodía: le da un rebote muy pegadizo.
- Delay al final de las frases de la voz: rellena los huecos.
- Con muchas repeticiones y un poco de reverb consigues ambientes de ensueño.

? Pon un delay de 0,75 con repeticiones medias en tu melodía o en un pluck y escúchalo.)TXT" },

    // =========================================================================
    { "Nivel 6 · De beat a canción", "Estructura de una canción", "8 min",
      "estructura de una cancion intro estribillo verso",
R"TXT(Una canción se arma con **secciones** que se repiten. Tener una estructura clara hace que tu beat se convierta en una canción de verdad.

[anim:estructura|Una estructura típica: intro, estribillo, estrofa, estribillo, estrofa, estribillo y final]

## Las secciones
- **Intro** (4-8 compases): presenta la canción, con menos instrumentos.
- **Estrofa** (8-16 compases): donde se cuenta la historia. Con menos elementos para dejar sitio a la voz.
- **Pre-estribillo**: sube la tensión justo antes del estribillo.
- **Estribillo** (8 compases): la parte pegadiza, con todo sonando.
- **Puente**: una parte diferente para romper la repetición.
- **Outro**: el final.

## Cómo hacerlo en Studoo
1. Crea un patrón para cada sección (menú **Patrón... → Nuevo**). Ponles nombre: "Intro", "Estribillo"...
2. Haz el estribillo completo y luego **duplícalo** y quita cosas para hacer la estrofa y la intro.
3. Ve a la **Playlist** y coloca los patrones en orden.
4. Pulsa **CANCIÓN** (o la tecla L) para escuchar la canción entera.

[ir:playlist|Abrir la playlist]

? Monta una canción de al menos 1 minuto: Intro 4 - Estribillo 8 - Estrofa 8 - Estribillo 8 - Outro 4.)TXT" },

    { "Nivel 6 · De beat a canción", "Automatización", "5 min",
      "automatizacion en la produccion musical ejemplos",
R"TXT(**Automatizar** es hacer que un control se mueva solo a lo largo de la canción. Hace que la música evolucione y no suene siempre igual.

[anim:automatizacion|La línea dibuja el valor y la rueda se mueve sola siguiéndola]

## Ejemplos
- **Fade out**: el volumen del Master baja poco a poco al final.
- **Filtro que se abre**: el sintetizador va ganando brillo antes del estribillo.
- **Más reverb** en la última palabra de una frase.

## En Studoo
1. Abre **Automatización** (F8) y pulsa **+ Nueva**.
2. Elige qué control quieres mover.
3. Haz clic para añadir puntos y arrástralos. Arriba = máximo, abajo = mínimo.
4. Clic derecho en un punto lo borra.

! La automatización solo actúa en modo **CANCIÓN**.

[ir:automatizacion|Abrir la automatización]

? Haz un fade out: automatiza el volumen del Master para que baje a cero en los últimos 4 compases.)TXT" },

    { "Nivel 6 · De beat a canción", "Grabar tu voz", "8 min",
      "como grabar voz en casa con microfono consejos",
R"TXT(Puedes grabar tu voz (o una guitarra) con un micrófono y ponerla encima del beat.

[anim:grabar|El camino del sonido: micrófono → ordenador → Studoo, escuchando con auriculares]

## Paso a paso
1. Conecta el micrófono (o usa el del ordenador o los auriculares).
2. Pulsa **Audio** y activa tu micrófono en **Canales de entrada**.
3. Ponte **auriculares**, para que el micrófono no grabe el beat.
4. Haz clic en la regla de la **playlist** donde quieras empezar.
5. Pulsa el **círculo rojo** (o R). Studoo pasa a modo CANCIÓN y graba.
6. Pulsa otra vez para parar. La grabación aparece en la playlist.

## Consejos para que suene mejor
- Graba en una habitación con cosas blandas (cortinas, sofá, ropa): hay menos eco.
- Ponte a un palmo del micrófono.
- Si las luces del mezclador llegan arriba del todo, aléjate o baja la ganancia.
- Graba varias tomas y quédate con la mejor.

? Graba 8 compases hablando o cantando encima de tu beat y ponle un poco de reverb y compresor.)TXT" },

    { "Nivel 6 · De beat a canción", "Teclados MIDI", "4 min",
      "como conectar un teclado MIDI al ordenador",
R"TXT(**MIDI** es el "idioma" que usan los teclados y controladores para mandar notas al ordenador. No manda sonido: solo **qué nota** tocas y **con qué fuerza**.

## Cómo usarlo
1. Conecta el teclado por **USB** antes de abrir Studoo.
2. Pulsa **Audio** y comprueba que el teclado está activado en **Entradas MIDI**.
3. Haz clic en el nombre de un canal del secuenciador para seleccionarlo.
4. Toca: sonará ese instrumento.

> Un teclado de 25 teclas es suficiente para empezar y cabe en cualquier mesa.

? Toca tu progresión de acordes con el teclado para aprenderla "con los dedos".)TXT" },

    { "Nivel 6 · De beat a canción", "Exportar y compartir", "4 min",
      "exportar cancion WAV MP3 para subir a internet",
R"TXT(**Exportar** convierte tu proyecto en un archivo de audio que puedes escuchar en el móvil o subir a internet.

## Formatos
- **WAV**: máxima calidad, archivos grandes. Para guardar o mandar a quien vaya a mezclar o masterizar.
- **MP3**: más pequeño, para compartir con amigos. Studoo necesita el programa gratuito LAME (te explica cómo instalarlo).

## Paso a paso
1. Comprueba que tu canción está montada en la **playlist**.
2. Menú ☰ → **Exportar a WAV** o **Exportar a MP3**.
3. Elige dónde guardarlo y espera a que termine la barra.

> Antes de compartir, escucha tu canción en el móvil, en el coche y en unos auriculares baratos. Si suena bien en todos, está lista.

? Exporta tu canción en MP3 y escúchala en tu móvil.)TXT" },

    { "Nivel 6 · De beat a canción", "Guardar y no perder nada", "3 min",
      "consejos para organizar proyectos de musica",
R"TXT(- Guarda con **Ctrl+S**. Los proyectos de Studoo terminan en **.studoo**.
- **Ctrl+Z** deshace y **Ctrl+Y** rehace casi cualquier cambio. ¡Experimenta sin miedo!
- Studoo hace una **copia automática cada minuto**. Si se cierra de golpe, al abrirlo te ofrece recuperar tu trabajo.
- Los samples **no se copian** dentro del proyecto: guárdalos siempre en la misma carpeta (por ejemplo, "Documentos/Studoo/Samples").

## Organízate
1. Usa nombres claros: "Trap oscuro 140 v2" mejor que "proyecto final final 3".
2. Cuando una idea te guste, haz **Guardar como** con otro nombre antes de cambiarla mucho.
3. Haz una carpeta por canción con el proyecto, los samples y lo exportado.

? Crea una carpeta para tu próxima canción y guarda allí tu proyecto con un buen nombre.)TXT" },

    // =========================================================================
    { "Nivel 7 · Seguir mejorando", "Una rutina para mejorar rápido", "5 min",
      "como mejorar produciendo musica rutina de practica",
R"TXT(Los productores que mejoran rápido no son los que más saben, sino los que **más canciones terminan**. Esta rutina te ayuda a avanzar cada semana.

[anim:rutina|Una semana de práctica: poco tiempo cada día, con un objetivo distinto]

## La rutina
1. **Lunes**: un ritmo nuevo de un estilo que no hayas probado.
2. **Martes**: melodía y acordes (piano roll).
3. **Miércoles**: diseño de sonido (cambia un sintetizador desde cero).
4. **Jueves**: mezcla (vuelve a mezclar algo que ya hiciste).
5. **Viernes**: **estudia una canción** que te guste (ver lección siguiente).
6. **Fin de semana**: **termina** y exporta una canción, aunque no sea perfecta.

## Reglas
- Ponte un **límite de tiempo** (por ejemplo, 1 beat en 30 minutos). Las prisas obligan a decidir.
- Termina lo que empiezas: se aprende más de 10 canciones terminadas que de 100 bucles a medias.
- Guarda tus beats antiguos: dentro de unos meses verás lo mucho que has mejorado.

? Esta semana, haz un beat al día durante 20 minutos y expórtalos todos.)TXT" },

    { "Nivel 7 · Seguir mejorando", "Aprender de tus canciones favoritas", "6 min",
      "como analizar canciones para aprender produccion",
R"TXT(Escuchar con atención canciones que te gustan es una de las mejores escuelas. Se llama **usar referencias**.

## Cómo analizar una canción
1. Busca su **BPM** y su **tonalidad** (escribe en internet "nombre de la canción BPM key").
2. Cuenta los compases de cada sección: ¿cuánto dura la intro? ¿Y el estribillo?
3. Fíjate en la batería: ¿dónde suena la caja? ¿Los hi-hats hacen rolls?
4. Escucha qué sonidos entran y salen en cada sección.
5. Apunta todo en una libreta.

## Practicar recreando
Intenta **recrear la batería y la estructura** de una canción en Studoo para practicar. Es como copiar un dibujo para aprender a dibujar.

! Recrear para aprender está genial, pero **no publiques** copias de canciones de otros ni uses sus trozos sin permiso. Coge las ideas (estructura, energía, tipo de sonidos) y haz tu propia música.

> Compara tu mezcla con una canción profesional a un volumen parecido. Te dirá enseguida si te faltan graves o te sobra brillo.

? Elige una canción que te guste y apunta su BPM, su estructura (secciones y compases) y dónde suena la caja.)TXT" },

    { "Nivel 7 · Seguir mejorando", "Entrena tu oído", "5 min",
      "entrenamiento auditivo para productores ejercicios",
R"TXT(Tu herramienta más importante son tus **oídos**. Se pueden entrenar como un músculo.

## Ejercicios
1. **Adivina el BPM**: pon una canción, da palmas y prueba en Studoo hasta que el metrónomo coincida.
2. **Saca melodías**: intenta tocar en el piano roll la melodía de una canción que conozcas, nota a nota.
3. **¿Mayor o menor?**: escucha canciones y decide si suenan alegres (mayor) o tristes (menor).
4. **¿Qué EQ he tocado?**: pide a alguien que suba o baje graves, medios o agudos y adivínalo.
5. **Instrumento a instrumento**: escucha una canción 5 veces fijándote cada vez en una cosa distinta (bombo, bajo, voz, hi-hat, efectos).

## Cuida tus oídos
- Produce a **volumen moderado**. Los oídos se cansan y luego todo te parece igual.
- Descansa **5 minutos cada hora**.
- Si notas pitidos, para durante un buen rato.

? Saca la melodía de una canción infantil sencilla (por ejemplo, el "Cumpleaños feliz") en el piano roll.)TXT" },

    { "Nivel 7 · Seguir mejorando", "Terminar canciones", "5 min",
      "como terminar canciones y salir del bucle de 8 compases",
R"TXT(Es muy normal quedarse atascado con un bucle de 8 compases que suena bien pero nunca se convierte en canción. Estos trucos te ayudan a terminar.

## El método "de todo a menos"
1. Haz el **estribillo** primero, con todo lo que quieras.
2. Cópialo en la playlist hasta llenar unos 2-3 minutos.
3. Ahora **quita** cosas en cada sección: en la intro quita la batería, en la estrofa quita la melodía principal...
4. Añade **transiciones**: un platillo al empezar el estribillo, un silencio antes, un riser (Efectos FX) al final de la estrofa.

## Más trucos
- **Plazo**: decide que la canción estará terminada el domingo, pase lo que pase.
- **El 90 % vale**: nadie va a notar el último 10 % de perfección. Termina y empieza otra.
- **Pide opinión**: enséñala a alguien de confianza y pregúntale en qué momento se aburrió.

> Los efectos FX incluidos (subida, impacto, caída de sub, platillo al revés...) están hechos justo para las transiciones.

? Coge el mejor bucle que tengas guardado y conviértelo hoy en una canción de 2 minutos con el método de arriba.)TXT" },

    { "Nivel 7 · Seguir mejorando", "Tus siguientes pasos", "4 min",
      "teoria musical para productores curso gratis",
R"TXT(¡Has llegado al final de la ruta! Ya conoces lo necesario para hacer canciones completas. A partir de aquí, todo es practicar y profundizar.

[anim:ruta|Has recorrido los 7 niveles. Ahora: practicar, practicar y practicar]

## Qué aprender después
- **Más teoría**: los grados de la escala, acordes de séptima e inversiones. Hacen tus progresiones más ricas.
- **Diseño de sonido**: aprende a usar un sintetizador gratuito como Vital o Surge XT a fondo.
- **Mezcla avanzada**: sidechain, saturación, bus de batería...
- **Masterización**: el último paso para que tu canción suene fuerte y pareja en todas partes.

## Dónde seguir aprendiendo
- Usa el botón **Ver vídeos en YouTube** de cada lección: hay muchísimos tutoriales gratuitos en español.
- Busca comunidades de productores (foros, Discord, Reddit) donde compartir tus canciones y pedir opinión.
- Vuelve a hacer las lecciones de vez en cuando: entenderás cosas nuevas cada vez.

> El mejor productor no es el que tiene más plugins, es el que más ha practicado. ¡A por ello!

? Repasa la lista de la izquierda. ¿Hay alguna lección sin marcar? Hazla, y después empieza tu próxima canción.)TXT" },
    };
}
