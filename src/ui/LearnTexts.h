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
//    [reto:id]          -> reto que Studoo comprueba solo (ver Challenges.cpp)
//    [base:genero|Texto] -> botón que carga la base de un género en un patrón
//                          nuevo (ver model/GenreBeats.cpp) y abre el secuenciador
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
        "Nivel 8 · Géneros musicales",
        "Nivel 9 · Producción avanzada",
    };

    inline constexpr Lesson lessons[] = {
    // =========================================================================
    { "Nivel 1 · Primeros pasos", "Cómo aprender con Studoo", "3 min",
      "como empezar a producir musica desde cero",
R"TXT(Bienvenido a **Aprende**. Aquí tienes todo lo necesario para pasar de no saber nada a terminar tus propias canciones, en orden y sin prisas.

[anim:ruta|La ruta: 9 niveles, de lo más básico a la producción avanzada]

## Cómo funciona este apartado
1. Las lecciones están ordenadas en **9 niveles**. Hazlas en orden la primera vez. El nivel **Géneros musicales** explica cada estilo y trae una base para cargar y cambiar.
2. Cada lección tiene explicaciones, **ilustraciones que se mueven** y pasos para hacerlo en Studoo.
3. Muchas lecciones tienen **retos**: Studoo comprueba solo si los has hecho. Y al final hay un **test** de 3 preguntas.
4. En la pestaña **Juegos** entrenas el oído, en **Logros** ves tu racha y tus medallas, y en **Diccionario** buscas cualquier palabra.
5. Pulsa **Marcar como aprendida** cuando la termines. Verás tu progreso en la lista de lecciones.
6. Si quieres verlo explicado en vídeo, pulsa **Ver vídeos en YouTube**: se abre una búsqueda en español sobre ese tema.

> Practica un poco **cada día** para mantener tu **racha** (la llamita de arriba). ¡Hay medallas por 3, 7 y 30 días seguidos!

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

? Cambia de sitio un golpe de bombo (por ejemplo, del paso 9 al 11) y escucha cómo cambia la sensación. Prueba 3 variaciones distintas.

[reto:primer_beat]
[reto:silencio])TXT" },

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

? Pon el metrónomo, elige 90 BPM y da palmas siguiendo el clic. Luego prueba a 140. ¿Notas lo distinto que se siente?

[reto:bpm])TXT" },

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

? Elige "32 pasos" y haz un ritmo de 2 compases donde el segundo compás sea un poco distinto al primero (por ejemplo, un golpe de bombo extra al final).

[reto:largo_64])TXT" },

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

? Haz un ritmo con el kit Trap y luego pulsa "Usar este kit" con el kit Boom Bap. Escucha cómo el mismo ritmo cambia de estilo.

[reto:caja_2_4])TXT" },

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

? Haz un patrón de trap de 2 compases (32 pasos) con un roll de hi-hat solo al final del segundo compás.

[reto:roll])TXT" },

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

? Haz el dembow y después crea un segundo patrón quitando el bombo durante un compás. Eso es un "drop": cuando el bombo vuelve, la energía sube.

[reto:dembow])TXT" },

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

? Haz este ritmo con el kit House y luego prueba con el kit Clásico 909, el sonido original del house de los años 80 y 90.

[reto:bombo_negras]
[reto:contratiempo])TXT" },

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

## El swing en Studoo
Arriba en el **Secuenciador** hay un desplegable **Swing**. Empieza con **Swing 20%** o **30%** y escucha cómo los hi-hats empiezan a "bailar". Afecta a todo el proyecto (también a las notas del piano roll).

> El boom bap y el lo-fi suelen llevar entre 20% y 40% de swing. El trap y el techno casi siempre van rectos (0%).

? Haz un boom bap y compáralo con tu ritmo de trap. Fíjate en que en el boom bap la caja suena dos veces por compás y en el trap solo una.

[reto:hihat_corcheas])TXT" },

    { "Nivel 2 · Ritmo y batería", "Dar vida a la batería", "6 min",
      "como hacer que la bateria suene humana variaciones fills",
R"TXT(Un ritmo que se repite exactamente igual durante 3 minutos cansa. Estos trucos le dan vida.

[anim:velocidad|Golpes con distinta fuerza (velocidad): fuerte, suave, medio...]

## Trucos
1. **Variaciones**: duplica el patrón (menú "Patrón..." → Duplicar) y cambia 2 o 3 golpes. Alterna los dos en la playlist.
2. **Fills**: al final de cada 4 u 8 compases, añade algo especial (un roll, toms, quitar el bombo).
3. **Silencios**: quitar la batería durante 1 compás antes del estribillo da mucha fuerza a la vuelta.
4. **Velocity (fuerza de cada nota)**: debajo del Piano roll está el carril **VELOCITY**, con una barra por nota. Arrástrala hacia abajo y esa nota suena más suave. Alterna hi-hats fuertes y suaves.
5. **Swing**: con el desplegable **Swing** del Secuenciador los pasos pares se retrasan un poquito y el ritmo suena más humano.
6. **Capas**: pon una palmada a la vez que la caja para que suene más gorda.
7. **Percusiones**: un shaker o una conga de fondo dan movimiento sin molestar.

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

## La ayuda de Studoo
En el Piano roll, pulsa **Herramientas → Escala** y elige, por ejemplo, **Menor**. Luego, en **Nota base de la escala**, elige **La**. Las filas que no son de la escala se ven **más oscuras** y las notas nuevas se **pegan** solas a la escala: así es casi imposible desafinar.

[reto:escala]

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

? Haz la progresión La menor → Fa → Do → Sol con el sintetizador "Pad" (+ Añadir → Sintetizador). Ponle tu ritmo de batería debajo.

[reto:acorde])TXT" },

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

? Copia esta melodía en el canal Sintetizador: La, Do, Mi, Re (una nota por tiempo). Luego cambia la última nota por Sol y escucha la diferencia.

[reto:notas_melodia])TXT" },

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

? Haz un 808 que siga tu bombo de trap usando La1 y Sol1, con un glide de La1 a Do2 al final del patrón.

[reto:808_notas])TXT" },

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
- **Tono**: sube o baja el sample en semitonos (+12 = una octava más agudo).
- **Inicio** y **Final**: recortan el sample. También puedes arrastrar las marcas **verde** (inicio) y **roja** (final) de la forma de onda.
- **Al revés**: el sample suena del final al principio. Un platillo al revés es la entrada perfecta para un estribillo.
- **Trozos**: corta el sample en partes iguales (chops). Lo vemos en el Nivel 9.

! Usa solo samples gratis o "royalty free" (libres de derechos) si vas a publicar tu música. Usar trozos de canciones de otros sin permiso puede darte problemas.

? Busca en internet "free drum samples" o "samples gratis royalty free", descarga un pack y arrastra un par de sonidos al secuenciador.

[reto:sample_kit])TXT" },

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

? Toca la misma nota con las 4 ondas y describe con tus palabras cómo suena cada una.

[reto:onda])TXT" },

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

? Con el mismo sintetizador, haz primero un pluck y luego un pad cambiando solo las 4 ruedas ADSR.

[reto:adsr_pad])TXT" },

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

? Pon un acorde largo con un sintetizador de sierra y mueve la rueda Filtro mientras suena.

[reto:filtro])TXT" },

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

? Pon a cero todos los faders y vuelve a mezclar tu beat siguiendo el orden de arriba.

[reto:volumen_mezcla])TXT" },

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

? Pon el hi-hat un 30 % a la derecha y una percusión un 30 % a la izquierda. Escucha con auriculares.

[reto:panorama])TXT" },

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

? Baja los graves del hi-hat y de la melodía y escucha cómo el 808 se oye más limpio.

[reto:eq])TXT" },

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

? Activa el compresor en la pista de tu 808 y baja el umbral hasta que notes que todas las notas suenan igual de fuertes.

[reto:compresor])TXT" },

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

? Pon reverb en el clap con Tamaño grande y Mezcla al 20 %. Luego quítala y compara.

[reto:reverb])TXT" },

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

? Pon un delay de 0,75 con repeticiones medias en tu melodía o en un pluck y escúchalo.

[reto:delay])TXT" },

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

? Monta una canción de al menos 1 minuto: Intro 4 - Estribillo 8 - Estrofa 8 - Estribillo 8 - Outro 4.

[reto:dos_patrones]
[reto:playlist])TXT" },

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

? Haz un fade out: automatiza el volumen del Master para que baje a cero en los últimos 4 compases.

[reto:automatizacion])TXT" },

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

## Editar la grabación
En la playlist la grabación se ve con su **forma de onda** (los picos son las partes fuertes).
- **Quitar el principio**: arrastra el **borde izquierdo** del bloque hacia la derecha. Para quitar el final, el borde derecho.
- **Cortar en dos**: clic derecho (o mantén pulsado) donde quieras cortar → **Cortar aquí**. Luego puedes mover o borrar cada trozo.
- **Duplicar**: pone una copia justo detrás (genial para repetir un coro).
- **Volumen**: sube o baja solo ese trozo.
- **Fundidos**: el sonido entra o se va poco a poco, sin cortes bruscos.
- **Seguir el tempo**: si luego cambias el BPM, la grabación se estira o se encoge para ir a tiempo, **sin cambiar el tono**.
- **Tono**: sube o baja la voz unos semitonos sin cambiar su velocidad (por ejemplo, para hacer coros más agudos).

? Graba 8 compases hablando o cantando encima de tu beat, quita el silencio del principio y ponle un poco de reverb y compresor.

[reto:editar_grabacion]

[reto:grabacion])TXT" },

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

    { "Nivel 7 · Seguir mejorando", "Trucos del piano roll", "7 min",
      "trucos piano roll seleccionar copiar arpegiador",
R"TXT(Los productores trabajan **rápido** gracias a unos pocos trucos del piano roll. Con ellos harás en segundos lo que antes te llevaba minutos.

[anim:pianoroll|Selecciona varias notas y muévelas o cópialas todas a la vez]

## Seleccionar varias notas
- Pulsa **Seleccionar** (o mantén **Ctrl**) y arrastra un rectángulo sobre las notas. Se marcan con un borde blanco.
- Arrastra una de ellas: **se mueven todas juntas**.
- **Ctrl+C** copia, **Ctrl+V** pega, **Ctrl+B** las duplica justo detrás y **Supr** las borra.
- Las **flechas ↑ ↓** suben o bajan su tono (con **Mayús**, una octava entera).

## Herramientas
- **Arpegiador**: convierte cada acorde en notas sueltas (subiendo, bajando, sube y baja o al azar), al ritmo del **Ajuste**. Pon acordes largos, elige Ajuste 1 paso y prueba.
- **Escala**: oscurece las notas que no son de la escala y pega las nuevas a ella.
- **Exportar MIDI**: guarda las notas del patrón en un archivo .mid para usarlas en otro programa. **Importar MIDI** hace lo contrario: trae melodías de otros sitios.

## Grabar tocando
Con un **teclado MIDI** (o las teclas del piano de la izquierda), pulsa **● Grabar notas**: el patrón suena y todo lo que toques se escribe solo. Después pulsa **Cuantizar** para colocarlo en su sitio.

> Haz un acorde, duplícalo 3 veces con Ctrl+B y sube o baja cada copia con las flechas: acabas de hacer una progresión en 10 segundos.

[ir:pianoroll|Abrir el piano roll]

? Escribe 4 acordes largos en el Piano eléctrico y pásales el **arpegiador** "Sube y baja" con Ajuste 1/2 paso.)TXT" },

    { "Nivel 7 · Seguir mejorando", "Tus siguientes pasos", "4 min",
      "teoria musical para productores curso gratis",
R"TXT(¡Has terminado la ruta principal! Ya conoces lo necesario para hacer canciones completas. A partir de aquí, todo es practicar y profundizar.

[anim:ruta|Ya tienes lo básico. En los niveles 8 y 9: géneros y producción avanzada]

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

    // =========================================================================
    { "Nivel 8 · Géneros musicales", "Los géneros y cómo usar las bases", "4 min",
      "generos de musica urbana y electronica explicados",
R"TXT(Cada género musical es como una **receta**: tiene su tempo, su forma de tocar la batería, su tipo de bajo y sus sonidos. Si conoces la receta, puedes cocinarla tú mismo... y luego inventarte tu propio plato.

## Qué hace diferente a cada género
- **El tempo (BPM)**: el reggaetón va a unos 95, el house a 124 y el trap a 140.
- **El ritmo de la batería**: dónde caen el bombo y la caja. Es lo que más se reconoce.
- **El bajo**: un 808 largo y grave (trap), un bajo a contratiempo (house), un sub suave (lo-fi)...
- **Los acordes y sonidos**: piano, campanas, cuerdas, pads...

## Cómo funcionan las bases de este nivel
1. En cada lección hay un botón **Cargar la base**. Al pulsarlo, Studoo crea un **patrón nuevo** con esa base (no se borra nada de lo que tenías).
2. Studoo cambia el **tempo** al típico del género y añade los **canales** que hagan falta: la batería de su kit, el bajo y los acordes.
3. Se abre el **Secuenciador**. Pulsa **Espacio** (o el botón ▶) para escucharla.
4. Después, sigue los pasos de **Experimenta** de cada lección: quita y pon pasos, cambia notas, sube o baja el tempo... Así entenderás para qué sirve cada parte.

> También puedes cargar las bases sin pasar por aquí: en el Secuenciador, botón **Beat de ejemplo**.

! Las bases son un punto de partida para aprender. Cuando hagas tus canciones, cámbialas tanto como quieras: así serán tuyas.

? Elige el género que más te guste de la lista de la izquierda y carga su base.

[reto:base_genero])TXT" },

    // =========================================================================
    { "Nivel 8 · Géneros musicales", "Trap", "8 min",
      "como hacer un beat de trap desde cero",
R"TXT(El trap nació en **Atlanta (Estados Unidos)** a principios de los 2000 y hoy está en todas partes: rap, música urbana en español, pop... Se reconoce por sus **hi-hats rapidísimos** y un **808** muy grave que hace temblar los altavoces.

[anim:beat_trap|Trap a medio tiempo: la palmada cae en el tiempo 3]

## La receta
- **Tempo**: entre 130 y 150 BPM, pero se siente **a medio tiempo** (como si fuera la mitad de rápido).
- **Palmada o caja**: solo una vez por compás, en el **tiempo 3** (paso 9).
- **Hi-hats**: en corcheas (uno sí, uno no) y con **rolls** ("trrrr") de vez en cuando.
- **808**: largo y grave, toca casi siempre a la vez que el bombo.
- **Melodía**: oscura, en escala **menor**, con campanas, pianos o flautas.

[base:trap|Cargar la base de Trap]

## Experimenta con la base
1. Escucha el **compás 2 y el 4**: los hi-hats hacen un roll al final. Quita esos pasos y escucha la diferencia.
2. Haz tu propio roll: enciende 3 o 4 pasos seguidos de hi-hat en otro sitio.
3. Abre el **Piano roll**, elige el canal **808** y alarga o mueve sus notas. El 808 es la "segunda melodía" del trap.
4. Prueba a cambiar el tempo a 150: suena más agresivo. A 130 suena más relajado.
5. En el canal **Campanas trap**, cambia alguna nota del acorde por otra de la escala.

> Mueve el bombo a otros pasos: en trap el bombo es libre, lo importante es que el 808 lo siga.

? Crea un segundo patrón con la misma base y cambia solo los hi-hats y el 808. Ya tienes una estrofa y un estribillo.)TXT" },

    // =========================================================================
    { "Nivel 8 · Géneros musicales", "Reggaetón", "8 min",
      "como hacer un beat de reggaeton dembow",
R"TXT(El reggaetón nació en **Puerto Rico y Panamá** en los años 90. Su ritmo, el **dembow**, viene del dancehall de Jamaica. Hoy es uno de los géneros más escuchados del mundo.

[anim:beat_reggaeton|El dembow: bombo en cada tiempo y la caja que "contesta"]

## La receta
- **Tempo**: entre 88 y 100 BPM. La base va a 95.
- **Bombo**: en cada tiempo (pasos 1, 5, 9 y 13).
- **Caja**: el famoso patrón **3 + 3 + 2** (pasos 4, 7, 12 y 15). Es lo que hace "pum-ch-pum-ch".
- **Percusión**: shaker, bongós, timbales... dan el sabor latino.
- **Bajo**: un 808 corto que sigue el ritmo del dembow.
- **Acordes**: muchas veces 4 acordes que se repiten, como **La menor – Fa – Do – Sol**.

[base:reggaeton|Cargar la base de Reggaetón]

## Experimenta con la base
1. **Silencia** (botón verde) todo menos el **Bombo** y la **Caja**: ese es el dembow puro.
2. Mueve un paso de la caja una posición a la derecha. ¿Notas que ya no suena a reggaetón? Así de importante es su sitio.
3. Escucha el **Bongó**: solo toca al final de algunos compases. Es un **relleno** para que no sea siempre igual.
4. Abre el **Piano roll** con el canal **Pluck reggaetón** y cambia el ritmo de los acordes.
5. Prueba el tempo a 90 (más romántico) o a 100 (más para bailar).

> En el reggaetón actual se suele quitar la batería unos compases antes del estribillo y volver con todo. ¡Pruébalo en la Playlist!

? Cambia el sonido de los acordes por **Piano eléctrico** o **Cuerdas** (menú + Añadir → Sintetizador) y copia las mismas notas.)TXT" },

    // =========================================================================
    { "Nivel 8 · Géneros musicales", "House", "8 min",
      "como hacer house music desde cero",
R"TXT(El house nació en **Chicago** a mediados de los años 80, en clubs como **The Warehouse** (de ahí su nombre). Es música para bailar: alegre, constante y con mucho groove. De él salen el deep house, el tech house, el afro house y muchos más.

[anim:beat_house|Four on the floor: el bombo en cada tiempo]

## La receta
- **Tempo**: entre 118 y 128 BPM. La base va a 124.
- **Bombo "four on the floor"**: en **cada tiempo**, siempre (pasos 1, 5, 9 y 13).
- **Palmada**: en los tiempos **2 y 4** (pasos 5 y 13).
- **Hi-hat abierto a contratiempo**: justo **entre** los bombos (pasos 3, 7, 11 y 15). Es el famoso "pum-tss-pum-tss".
- **Shaker o hi-hat cerrado** rellenando para dar movimiento.
- **Bajo**: también a contratiempo, rebotando entre los bombos.
- **Acordes**: piano u órgano tocando acordes cortos y rítmicos (**stabs**). La base usa La menor 7 – Re menor 7 – Fa 7 – Mi menor 7.

[base:house|Cargar la base de House]

## Experimenta con la base
1. Silencia todo menos el **Bombo**: así de simple es la base del house.
2. Activa el **Hi-hat abierto**. ¿Notas que de repente dan ganas de moverse? Es el contratiempo.
3. Abre el **Piano roll** con el canal **Piano house** y haz las notas más cortas o más largas. Cambia mucho el estilo.
4. En el **Bajo house**, prueba a poner notas en otros sitios: siempre que no caigan encima del bombo, sonará bien.
5. Sube el tempo a 128 (más fiestero) o bájalo a 120 (más deep house).

> En house las canciones son largas y cambian poco a poco: cada 8 o 16 compases entra o sale un elemento (el shaker, el piano, el bajo...).

? Haz 3 patrones: uno solo con batería, otro con batería y bajo, y otro con todo. Colócalos en la Playlist uno detrás de otro y escucha cómo la canción "crece".)TXT" },

    // =========================================================================
    { "Nivel 8 · Géneros musicales", "Boom bap (hip-hop clásico)", "7 min",
      "como hacer un beat boom bap",
R"TXT(El boom bap es el sonido del **hip-hop de Nueva York** de finales de los 80 y los 90. Su nombre imita el sonido de la batería: **"boom"** (bombo) y **"bap"** (caja). Muchos raperos en español siguen usándolo.

[anim:beat_boombap|Boom (bombo) y bap (caja en los tiempos 2 y 4)]

## La receta
- **Tempo**: entre 85 y 95 BPM.
- **Caja**: fuerte y gorda en los tiempos **2 y 4** (pasos 5 y 13).
- **Bombo**: libre, con algún golpe extra antes de la caja.
- **Hi-hats**: en corcheas, a menudo con **swing** (un poco "cojos", que dan groove).
- **Acordes**: samples de discos antiguos de jazz o soul. La base imita ese sonido con un **piano eléctrico** y acordes de séptima.
- **Bajo**: sencillo, siguiendo al bombo.

[base:boombap|Cargar la base de Boom bap]

## Experimenta con la base
1. Silencia el **Piano jazz** y escucha solo la batería: es lo que más importa en boom bap.
2. Pon y quita golpes de bombo entre los pasos 9 y 12. Cada cambio da otro "rebote".
3. Haz que los hi-hats no toquen todos igual: quita alguno para que respiren.
4. En el Piano roll, haz los acordes más cortos para que suenen "cortados" como un sample.
5. Baja el tempo a 85 para un rap más tranquilo.

> Si tienes samples de vinilo (de forma legal), cárgalos en un canal sampler: es la esencia del boom bap.

? Escribe un verso de rap de 8 líneas encima de tu base y grábalo con el micrófono (lección **Grabar tu voz**).)TXT" },

    // =========================================================================
    { "Nivel 8 · Géneros musicales", "Lo-fi", "7 min",
      "como hacer musica lofi hip hop",
R"TXT(El **lo-fi hip-hop** es la música tranquila que se usa para estudiar o relajarse. "Lo-fi" significa **baja fidelidad**: suena un poco "viejo", suave y cálido, como un casete. Le influyeron mucho productores como **J Dilla** y **Nujabes**.

[anim:beat_lofi|Lo-fi: batería sencilla y relajada a 80 BPM]

## La receta
- **Tempo**: lento, entre 70 y 90 BPM.
- **Batería suave**: bombo y caja sin mucha pegada, hi-hats discretos.
- **Acordes de jazz**: acordes de **séptima** (maj7, m7) con piano eléctrico o guitarra. Largos y suaves.
- **Bajo**: un sub suave, que casi no se oye pero se siente.
- **Ambiente**: ruido de vinilo, lluvia, sonidos de la calle...
- **Nada demasiado brillante**: los agudos se cortan con un filtro.

[base:lofi|Cargar la base de Lo-fi]

## Experimenta con la base
1. Selecciona el canal **Piano lo-fi** y baja su **filtro (cutoff)**: suena más apagado y "lo-fi".
2. Sube un poco la **Reverb** del piano en el Mezclador para que suene lejano.
3. Abre el Piano roll y cambia una nota de cada acorde, un semitono arriba o abajo. Algunos cambios suenan "jazz" y otros no: entrena tu oído.
4. Baja el volumen de la batería: en lo-fi los acordes son los protagonistas.
5. Prueba el tempo a 72 para algo aún más tranquilo.

> Añade una melodía muy sencilla de 3 o 4 notas con el preset **Campanas**. Menos es más.

? Haz una base lo-fi de 4 compases que podrías escuchar mientras estudias. Si no te cansa después de escucharla 5 minutos seguidos, ¡lo has conseguido!)TXT" },

    // =========================================================================
    { "Nivel 8 · Géneros musicales", "Drill", "8 min",
      "como hacer un beat de drill",
R"TXT(El drill nació en **Chicago** hacia 2011 y después se transformó en **Londres** (UK drill), que es el sonido que más se escucha hoy, también en España y Latinoamérica. Es oscuro, agresivo y muy rítmico.

[anim:beat_drill|Drill: caja desplazada y hi-hats que "saltan"]

## La receta
- **Tempo**: entre 138 y 145 BPM, a medio tiempo (como el trap).
- **Caja**: en el **tiempo 3** y con golpes extra **desplazados** (por ejemplo, el paso 15). Eso le da su balanceo raro.
- **Hi-hats**: patrones que "saltan" (grupos de 3) en vez de ir siempre iguales.
- **808 con glide**: las notas se **deslizan** de una a otra. Es lo más característico del drill.
- **Melodía**: oscura, en **menor**, con cuerdas, pianos o coros.

[base:drill|Cargar la base de Drill]

## Experimenta con la base
1. Escucha solo la **Caja** y los **Hi-hats**: así se reconoce un drill.
2. Abre el **Piano roll** con el **808 drill** y pon dos notas muy seguidas con distinta altura: escucharás el **deslizamiento**.
3. En el panel del 808, sube o baja el **Glide** para que deslice más o menos.
4. Mueve el golpe extra de la caja (paso 15) a otro sitio y escucha cómo cambia el balanceo.
5. Silencia las **Cuerdas drill** y escribe tu propia melodía oscura en escala menor.

! El 808 del drill puede tapar a todo lo demás. Si suena "embarrado", baja un poco su volumen o el del bombo.

? Haz dos versiones del 808: una que siga al bombo y otra con más notas y deslizamientos. ¿Cuál te gusta más?)TXT" },

    // =========================================================================
    { "Nivel 8 · Géneros musicales", "Techno", "7 min",
      "como hacer techno desde cero",
R"TXT(El techno nació en **Detroit** a mediados de los 80, más o menos a la vez que el house. Es más **mecánico, oscuro y repetitivo**: busca que la gente entre en trance bailando durante horas. Berlín es hoy su gran capital.

[anim:beat_techno|Techno: bombo constante y hi-hats que corren]

## La receta
- **Tempo**: entre 125 y 140 BPM. La base va a 130.
- **Bombo**: en cada tiempo, **fuerte y largo**. Es el corazón de todo.
- **Hi-hats y percusiones metálicas**: rápidos, a veces en todos los pasos.
- **Bajo "rolling"**: notas cortas en los huecos entre bombos, que hacen que la música "ruede".
- **Pocos acordes**: a veces ninguno. Mejor un pad o un sonido raro que cambia poco a poco.
- **Automatización**: el filtro se abre y se cierra para crear tensión.

[base:techno|Cargar la base de Techno]

## Experimenta con la base
1. Silencia el **Pad techno**: muchos temas de techno solo tienen batería y bajo.
2. Selecciona el **Bajo techno** (un bajo ácido) y mueve su **cutoff** y **resonancia** mientras suena. ¡Así suena el acid techno!
3. Graba ese movimiento con la **Automatización** para que se mueva solo.
4. Quita y pon el **Hi-hat corto** cada 8 compases en la Playlist para crear subidas y bajadas.
5. Sube el tempo a 135 o 140 para un techno más duro.

> En techno, los cambios son pequeños y lentos. Un solo sonido que entra después de 16 compases ya es un gran momento.

? Haz un bucle de techno de 4 compases y automatiza el filtro del bajo para que se abra poco a poco durante 16 compases.)TXT" },

    // =========================================================================
    { "Nivel 8 · Géneros musicales", "Afrobeats", "7 min",
      "como hacer un beat de afrobeats",
R"TXT(Los **afrobeats** vienen de **Nigeria y Ghana** y se han hecho famosos en todo el mundo en los últimos años. Mezclan ritmos africanos con pop, dancehall y hip-hop. Suenan alegres, con mucho movimiento y muy bailables.

[anim:beat_afro|Afrobeats: percusiones que se cruzan]

## La receta
- **Tempo**: entre 95 y 115 BPM. La base va a 105.
- **Percusión**: shakers, congas y rims que se **cruzan** entre sí. Es lo más importante.
- **Bombo**: no cae en todos los tiempos; tiene un ritmo que "baila".
- **Rim o caja**: un patrón parecido al dembow (grupos de 3 + 3 + 2).
- **Acordes**: alegres, en escala **mayor**, con plucks, guitarras o pianos.
- **Bajo**: redondo, con pocas notas.

[base:afrobeat|Cargar la base de Afrobeats]

## Experimenta con la base
1. Silencia todo menos **Shaker**, **Conga** y **Rim**: escucha cómo se mezclan las percusiones.
2. Añade un golpe de conga en otro paso. En afrobeats las percusiones "conversan" entre ellas.
3. Abre el Piano roll con el **Pluck afro** y prueba ritmos distintos para los acordes.
4. Compara esta base con la de reggaetón: ¿qué se parece y qué no?
5. Usa el kit **Percusión latina** para añadir más sonidos (bongó, claves, güiro...).

> El secreto de los afrobeats es el **espacio**: no llenes todos los pasos, deja huecos para que respire.

? Haz que cada compás de percusión tenga un pequeño cambio respecto al anterior, como si fueran percusionistas de verdad.)TXT" },

    // =========================================================================
    { "Nivel 8 · Géneros musicales", "Phonk", "6 min",
      "como hacer phonk drift phonk",
R"TXT(El **phonk** viene del rap de **Memphis** de los años 90, con su sonido sucio de casete. Hacia 2020 nació el **drift phonk**, que se hizo viral en TikTok y en vídeos de coches. Se reconoce por el famoso **cencerro** (cowbell) que hace la melodía.

[anim:beat_phonk|Phonk: el cencerro hace la melodía]

## La receta
- **Tempo**: entre 120 y 160 BPM. La base va a 130.
- **Cencerro**: toca un patrón rítmico (y a veces melódico) encima de todo.
- **808 distorsionado**: muy saturado, para que suene fuerte y agresivo.
- **Batería**: bombo y caja potentes, hi-hats sencillos.
- **Voces**: samples de voces de rap antiguo, a menudo con efectos.

[base:phonk|Cargar la base de Phonk]

## Experimenta con la base
1. Silencia el **Cencerro phonk**: ¿a que ya no suena a phonk? Es la clave del género.
2. Cambia los pasos del cencerro para hacer tu propio patrón.
3. Abre el **Piano roll** con el canal del cencerro y pon notas de distinta altura: ¡el cencerro también puede tocar melodías!
4. En el panel del 808, sube la **distorsión (drive)** para un sonido aún más agresivo.
5. Sube el tempo a 150 para un drift phonk más rápido.

! El 808 distorsionado suena muy fuerte. Vigila que el medidor del Master no llegue al rojo.

? Crea una melodía de cencerro de 2 compases que se te quede en la cabeza.)TXT" },

    // =========================================================================
    { "Nivel 8 · Géneros musicales", "Pop", "7 min",
      "como producir una cancion pop",
R"TXT(El **pop** (de "popular") es la música pensada para gustar a mucha gente: canciones con **estribillos pegadizos**, estructuras claras y un sonido limpio. Toma ideas de todos los géneros: urbano, electrónica, rock...

[anim:beat_pop|Pop: una batería clara que deja sitio a la voz]

## La receta
- **Tempo**: entre 100 y 130 BPM. La base va a 110.
- **Batería sencilla**: palmada o caja en los tiempos **2 y 4**, bombo marcando el ritmo.
- **Los 4 acordes mágicos**: **Do – Sol – La menor – Fa**. Esta progresión está en cientos de canciones famosas.
- **Bajo**: sigue la nota principal de cada acorde.
- **La voz** es la protagonista: todo lo demás le deja sitio.
- **Estructura clara**: estrofa, pre-estribillo, estribillo...

[base:pop|Cargar la base de Pop]

## Experimenta con la base
1. Escucha los acordes del **Piano pop**: seguro que te recuerdan a alguna canción.
2. Cambia el orden de los acordes en el Piano roll (por ejemplo, La menor – Fa – Do – Sol). Suena más triste, ¿verdad?
3. Canta o tararea una melodía encima. Con estos acordes casi todo suena bien.
4. Añade la **Pandereta** en los estribillos para que tengan más energía.
5. Haz una versión **sin batería** para la intro y otra con todo para el estribillo.

> Truco de pop: en el estribillo, sube el volumen, añade un instrumento más y usa las notas más agudas de la canción.

? Usa estos 4 acordes para hacer una canción completa con la lección **Estructura de una canción**.)TXT" },

    // =========================================================================
    { "Nivel 8 · Géneros musicales", "R&B", "6 min",
      "como hacer un beat de rnb",
R"TXT(El **R&B** (rhythm and blues) es música **suave, sensual y con mucha emoción**. El R&B moderno mezcla acordes de jazz y soul con baterías de hip-hop y trap. Es perfecto para canciones románticas.

[anim:beat_rnb|R&B: lento, con chasquidos y hi-hats con rolls suaves]

## La receta
- **Tempo**: lento, entre 60 y 90 BPM. La base va a 75.
- **Chasquidos de dedos (snaps)** o caja suave en los tiempos **2 y 4**.
- **Hi-hats**: con pequeños rolls, pero más suaves que en el trap.
- **Acordes ricos**: acordes de **séptima** y novena, con pads, pianos eléctricos o guitarras.
- **808 limpio**: redondo y grave, sin distorsión.
- **Voces**: muchas capas de coros y armonías.

[base:rnb|Cargar la base de R&B]

## Experimenta con la base
1. Escucha el **Pad R&B**: acordes largos que lo envuelven todo.
2. Cambia el pad por **Piano eléctrico**: R&B más clásico.
3. Abre el Piano roll con el **808 R&B** y añade notas que "caminen" entre acorde y acorde.
4. Sube la **Reverb** del chasquido para que suene en una sala grande.
5. Baja el tempo a 68 para una balada.

> En R&B la voz hace muchos adornos (melismas). Deja huecos en la base para que la voz pueda lucirse.

? Graba una melodía tarareada encima de tu base y luego intenta tocarla en el Piano roll con el preset **Lead brillante**.)TXT" },

    // =========================================================================
    { "Nivel 9 · Producción avanzada", "Sidechain: el bombeo", "7 min",
      "que es el sidechain explicado facil",
R"TXT(¿Has notado que en el house y en la música electrónica todo parece "respirar" al ritmo del bombo? Eso es el **sidechain**: cada vez que suena el bombo, otros sonidos (el bajo, los pads) bajan un momento su volumen y luego vuelven a subir.

[anim:sidechain|Cada vez que golpea el bombo, el pad se "agacha" y vuelve a subir]

## Para qué sirve
- **Hacer sitio al bombo**: el bombo y el bajo comparten los graves. Si no se apartan, se tapan y suena "embarrado".
- **Dar movimiento**: el famoso efecto de bombeo hace que la música se mueva sola, aunque los acordes sean largos.

## Cómo se hace normalmente
1. Se pone un **compresor** en la pista del bajo o del pad.
2. Se le dice que "escuche" al bombo (eso es la cadena lateral, *sidechain*).
3. Cada golpe de bombo hace que el compresor baje el volumen del bajo un instante.

## Cómo hacerlo en Studoo
Cada pista del **Mezclador** tiene su sección **SIDECHAIN**:
1. Asegúrate de que el bombo va a su propia pista (en el Secuenciador, la columna "Pista").
2. En el Mezclador, busca la pista del **bajo** o del **pad**.
3. En **SIDECHAIN**, elige en el desplegable la pista del **bombo** (la que "manda").
4. Sube la rueda **Bajada**: cuanto más alta, más se agacha el sonido con cada bombo. Empieza por un **50%**.

[base:house|Cargar la base de House (ya trae el sidechain puesto)]

## Otros trucos
- **Sidechain "natural"**: pon el bajo **entre** los bombos, nunca encima. Así nunca chocan.
- **Bombeo con automatización**: una línea de volumen en forma de **V** en cada tiempo da un efecto parecido.

[ir:mezclador|Ir al Mezclador]

? Carga la base de House, ve al Mezclador y baja a 0 la **Bajada** del Bajo house. Luego súbela al 100%. ¿Oyes cómo "respira"?

[reto:sidechain])TXT" },

    // =========================================================================
    { "Nivel 9 · Producción avanzada", "Chops: trocear un sample", "7 min",
      "como hacer chops de samples",
R"TXT(Hacer **chops** (trocear) es cortar un sample en trocitos y volver a tocarlos en otro orden. Así se hizo el hip-hop clásico: de una frase de soul salían melodías nuevas. Hoy se hace con voces, acordes, guitarras...

[anim:sampler|Cada trozo del sample es una nota distinta del piano roll]

## Trocear en Studoo
1. Carga un sample largo (un acorde, una frase de voz o un loop) en un canal **Sampler**.
2. Abajo, en el panel del sonido, sube **Trozos**: por ejemplo **8 trozos**. En la **forma de onda** verás las líneas de cada trozo.
3. Ve al **Piano roll** con ese canal. La **Nota base** toca el trozo 1, la nota de encima el trozo 2, la siguiente el 3...
4. Pon notas de distintas alturas y escucha: cada una suena un trozo diferente.
5. Desactiva **One-shot** si quieres que cada trozo se corte al acabar la nota.

## Ideas para tus chops
- **Reordena**: toca los trozos en otro orden y crea una melodía nueva.
- **Repite**: el mismo trozo 2 o 3 veces seguidas da un efecto "tartamudo" muy usado.
- **Al revés**: activa **Al revés** para que los trozos suenen del final al principio.
- **Tono**: con **Tono** puedes subir o bajar todos los trozos (sin cambiar el orden).

> Si el sample tiene un silencio al principio, arrastra la **marca verde** de la forma de onda (o sube el **Inicio**) antes de trocear: así los trozos caen justo en los golpes.

! Para publicar, usa samples libres de derechos o graba los tuyos (tu voz, una guitarra, un objeto de casa).

? Graba tu voz diciendo una frase, ponla en 8 trozos y haz un ritmo de 1 compás con los trozos desordenados.

[reto:chops])TXT" },

    { "Nivel 9 · Producción avanzada", "Efectos creativos: filtro, distorsión y chorus", "7 min",
      "efectos creativos filtro distorsion chorus produccion",
R"TXT(Además de EQ, compresor, delay y reverb, cada pista del **Mezclador** tiene tres efectos para **cambiar el carácter** del sonido.

[anim:filtro|El filtro deja pasar unas frecuencias y quita otras]

## Filtro
La rueda **Filtro** tiene dos lados:
- **Hacia la izquierda**: quita agudos. El sonido se vuelve apagado, como si viniera de otra habitación.
- **Hacia la derecha**: quita graves. El sonido se vuelve fino, como de radio o de teléfono.
- **En el centro**: no hace nada.

> El truco más usado en la electrónica: **automatiza el Filtro** de la batería antes del drop (de "apagado" a normal). La subida se nota muchísimo. En Automatización está como "Filtro".

## Sucio (distorsión)
La rueda **Distor.** satura el sonido: más fuerte, más agresivo y con más armónicos. Va genial en 808, bajos, guitarras y voces con efecto. Un poco (10-30%) da calor; mucho la rompe a propósito.

## Chorus
El **Chorus** copia el sonido varias veces con pequeñas diferencias: suena **más ancho**, como si tocaran varios a la vez. Perfecto para pads, acordes y coros. Evítalo en el bombo y en el bajo (se debilitan).

## LFO del sintetizador
En el panel del **Sintetizador** hay dos ruedas más: **LFO vel.** y **LFO cant.** El LFO mueve el filtro del sinte arriba y abajo solo: es el "wobble" del dubstep. Prueba LFO vel. a 2 Hz y LFO cant. al 60%.

> Todo esto se puede **automatizar**: en Automatización encontrarás el filtro, la resonancia y el LFO del sinte, la distorsión del 808, el tono del sampler y los envíos.

[ir:mezclador|Ir al Mezclador]

? Pon un pad con acordes, súbele el **Chorus** al 60% y compara. Después automatiza su **Filtro** de -100% a 0% durante 4 compases.

[reto:efectos_creativos])TXT" },

    { "Nivel 9 · Producción avanzada", "Envíos y buses", "7 min",
      "que son los envios y buses en la mezcla",
R"TXT(Hasta ahora cada pista tenía sus propios efectos. Los productores usan además **envíos**: mandar una **copia** del sonido de varias pistas a una pista especial.

[anim:volumenes|Varias pistas mandan una copia a la misma pista de efectos]

## Pista de efectos (reverb compartida)
En vez de poner una reverb en cada pista, se crea **una sola pista de reverb** y las demás le mandan un poco de sonido. Así todo suena en la **misma sala** y la mezcla queda más unida.
1. En el **Mezclador**, elige una pista libre del final (por ejemplo, la **16**). Renómbrala "Reverb" con doble clic.
2. En esa pista, sube la **Mezcla** de la reverb al **100%** y el **Tamaño** a tu gusto.
3. En la pista de la voz o de los acordes, en **ENVÍO**, elige **→ 16. Reverb** y gira la rueda **Envío**.
4. Cuanto más envío, más reverb tiene ese sonido.

## Bus (agrupar pistas)
Un **bus** es una pista que junta varias. Por ejemplo, toda la batería:
1. Renombra una pista del final como "Batería".
2. En el bombo, la caja y los hi-hats: **ENVÍO → Batería**, rueda al 100% y activa **Solo envío**.
3. Ahora el fader de "Batería" sube o baja toda la batería a la vez, y puedes ponerle un compresor a todo junto.

! Solo se puede enviar a pistas con un **número mayor** (para que el sonido no dé vueltas sin fin). Por eso las pistas de efectos y los buses se ponen al **final**.

> Studoo tiene **16 pistas** en el mezclador: sobra sitio para tus sonidos, pistas de efectos y buses.

[ir:mezclador|Ir al Mezclador]

? Crea una pista "Reverb" en la 16 y mándale un poco de la voz y de los acordes. Compara con el envío a 0.

[reto:envio])TXT" },

    // =========================================================================
    { "Nivel 9 · Producción avanzada", "Herramientas de estudio profesional", "9 min",
      "autotune half time stems como usar",
R"TXT(Studoo tiene las herramientas que usan los estudios profesionales. Aquí tienes para qué sirve cada una y dónde está.

[anim:volumenes|Cada pista del mezclador tiene su botón FX+ con los efectos avanzados]

## Autotune y afinador (Mezclador → FX+)
- **Autotune**: lleva tu voz a la nota correcta de la escala. Un **20-40%** corrige sin que se note; al **100%** sale el efecto "robot" del trap.
- Elige la **nota base** y la **escala** de tu canción (por ejemplo, La · Menor). Si no lo sabes, deja "Todas las notas".
- El **afinador** muestra qué nota cantas (o tocas) y si vas alta o baja: verde = afinado.

## Efectos de ritmo (FX+ → Ritmo)
- **Half-time**: la pista suena a mitad de velocidad (y una octava más grave). Muy usado en drops de trap y dubstep.
- **Tartamudeo**: repite el primer trocito de cada tiempo ("ta-ta-ta").
- **Puerta rítmica**: corta el sonido a ritmo (un pad que hace "tá-tá-tá").
- **Parada de cinta**: el sonido se frena al final de cada compás, como un disco que se para.

## Más cosas de estudio
- **Puerta de ruido**: corta el ruido de fondo entre frase y frase.
- **EQ paramétrico**: 4 bandas para quitar o subir frecuencias exactas (por ejemplo, un pitido).
- **Maximizar**: en el Master, al final, para que la canción suene fuerte sin romperse.
- **Exportar por pistas (stems)**: en el menú ☰. Un archivo por pista para mandarlo a quien mezcle tu canción.
- **Pads** (en el secuenciador): toca tu batería con los dedos y grábala.
- **Navegador** (F4): escucha y añade sonidos con doble clic.
- **Controles MIDI**: clic derecho en cualquier rueda → "Conectar a un control MIDI" y mueve una rueda de tu teclado.

> Autotune + un poco de reverb + delay corto = la cadena básica de la voz del trap y el reggaetón.

[ir:mezclador|Ir al Mezclador]

? Graba una frase cantada, ponle Autotune al 60% con la escala de tu canción y prueba el efecto Half-time en la batería durante 1 compás.

[reto:autotune])TXT" },

    { "Nivel 9 · Producción avanzada", "Mezclar voces", "8 min",
      "como mezclar voces paso a paso",
R"TXT(La voz es lo más importante de una canción: si no se entiende o suena "pegada encima" del beat, todo pierde. Mezclar voces tiene unos pasos que casi siempre se repiten.

[anim:volumenes|La voz principal tiene que destacar por encima del beat]

## La cadena de la voz (en orden)
1. **Graba bien**: cerca del micro (un palmo), con auriculares y en una habitación sin eco. Lo que se graba mal no se arregla del todo.
2. **EQ: quita graves**. La voz no necesita los graves más profundos. Baja los **Graves** de su pista: se quita el ruido de golpes y "bum".
3. **Compresor**: actívalo (botón **On**). Hace que las palabras flojas y fuertes suenen parecidas y la voz se entienda siempre.
4. **EQ: un poco de brillo**. Sube un poco los **Agudos** para que tenga presencia (sin pasarte: las "s" pican).
5. **Reverb y delay suaves**: un poco de espacio para que no suene seca. Mucho menos de lo que crees.
6. **Volumen**: súbela hasta que se entienda cada palabra, pero que siga pareciendo parte de la canción.

## El panorama de las voces
- **Voz principal**: siempre en el **centro**.
- **Coros y dobles**: un poco a los lados (por ejemplo, uno a la izquierda y otro a la derecha).

! No pongas mucha reverb a la voz principal: suena lejana y se entiende peor. Para efectos grandes, mejor en coros o en palabras sueltas.

[ir:mezclador|Ir al Mezclador]

? Graba una frase hablada, haz los pasos 2 a 6 en su pista y compara con el botón de **mute** de los efectos... ¡o escúchala antes y después!)TXT" },

    // =========================================================================
    { "Nivel 9 · Producción avanzada", "Masterizar", "7 min",
      "que es masterizar una cancion para principiantes",
R"TXT(**Masterizar** es el último paso antes de publicar: la canción ya está mezclada y ahora se le da el toque final para que suene **fuerte, pareja y bien en todas partes** (móvil, coche, auriculares, altavoz).

[anim:master|El master: la mezcla entra, sale más pareja y con más volumen, sin pasar del rojo]

## Qué se hace al masterizar
- **EQ general**: pequeños ajustes de graves y agudos a toda la canción.
- **Compresión suave**: une todo y lo hace sonar más compacto.
- **Limitador**: sube el volumen al máximo posible **sin pasar del rojo** (sin distorsionar). En Studoo, la pista **Master** trae el **Limitador** encendido.
- **Comprobar en varios sitios**: escuchar en el móvil, en auriculares y en un altavoz.

## Masterizar en Studoo
Todo lo que pongas en la pista **Master** del Mezclador afecta a la canción entera.
1. Termina la mezcla primero: el master no arregla una mala mezcla.
2. En el **Master**, usa la **EQ** con cuidado (como mucho, 2 o 3 puntos arriba o abajo).
3. Activa el **compresor** del Master con el umbral alto (que actúe poco).
4. Mira el medidor del Master: el pico más fuerte debe quedar **cerca del máximo, pero sin llegar al rojo**.
5. Exporta en **WAV** y escúchala en varios aparatos.

> Compara tu canción con una canción de referencia del mismo género, al mismo volumen. Te dirá si te sobran graves o te faltan agudos.

! Subir todo "a tope" no hace que suene mejor: lo aplasta y cansa al oído. Las plataformas como Spotify bajan el volumen de las canciones demasiado fuertes.

? Exporta tu última canción, escúchala en el móvil y en auriculares, y apunta una cosa que cambiarías en el Master.)TXT" },

    // =========================================================================
    { "Nivel 9 · Producción avanzada", "Teoría: grados y acordes de séptima", "9 min",
      "grados de la escala y acordes de septima explicacion",
R"TXT(Ya sabes qué es un acorde y una escala. Ahora un truco de los músicos: en vez de nombrar los acordes por su nota (Do, Sol...), se nombran por su **número dentro de la escala**. Así una progresión sirve en cualquier tono.

[anim:acordes|Cada acorde de la escala tiene su número (grado)]

## Los grados
En **Do mayor**, cada nota de la escala es la base de un acorde:
- **I** Do mayor · **II** Re menor · **III** Mi menor · **IV** Fa mayor
- **V** Sol mayor · **VI** La menor · **VII** Si disminuido

Fíjate: **I, IV y V** son mayores (alegres) y **II, III y VI** son menores (tristes).

## Progresiones famosas en grados
- **I – V – VI – IV**: la del pop (Do – Sol – La menor – Fa).
- **VI – IV – I – V**: la misma, empezando en el VI. Suena más emotiva (La menor – Fa – Do – Sol).
- **II – V – I**: la del jazz (Re menor – Sol – Do).
- **I – VI – IV – V**: la de los años 50.

## Acordes de séptima
Un acorde de séptima es un acorde básico de 3 notas **más una cuarta nota**: la que está 2 notas por encima de la última.
- **Do maj7** = Do – Mi – Sol – **Si**: suave y elegante (lo-fi, R&B).
- **La m7** = La – Do – Mi – **Sol**: relajado y nocturno.
- **Sol 7** = Sol – Si – Re – **Fa**: con tensión, "pide" ir a Do.

[ir:pianoroll|Probar acordes en el Piano roll]

> En el Piano roll, el desplegable de acordes tiene **Séptima**: con un clic pones las 4 notas.

? Escribe la progresión **II – V – I** en Do (Re menor 7 – Sol 7 – Do maj7) con el preset **Piano eléctrico**. ¡Acabas de tocar jazz!)TXT" },

    // =========================================================================
    { "Nivel 9 · Producción avanzada", "Analizar una canción paso a paso", "8 min",
      "como analizar una cancion para producir",
R"TXT(Los mejores productores aprendieron **copiando para aprender**: escuchando canciones que les gustaban y descubriendo cómo estaban hechas. Aquí tienes un método paso a paso.

[anim:estructura|Apunta cuántos compases dura cada parte]

## El método
1. **Elige una canción** que te guste y que sea de un estilo parecido al tuyo.
2. **Averigua el tempo**: busca "BPM de [canción]" en internet, o pon el metrónomo de Studoo y cambia el BPM hasta que los clics vayan a la vez que la canción.
3. **Cuenta la estructura**: con el metrónomo, cuenta cuántos compases dura cada parte (intro, estrofa, estribillo...). Apúntalo.
4. **Lista los instrumentos**: escucha varias veces, fijándote cada vez en una sola cosa: la batería, el bajo, los acordes, la melodía, los efectos.
5. **Recrea el ritmo**: copia el bombo, la caja y los hi-hats en el Secuenciador de Studoo.
6. **Busca la tonalidad**: tararea la nota que parece "casa" (donde la canción descansa) y búscala en el Piano roll.
7. **Haz tu versión**: cambia el ritmo, los acordes o los sonidos. Así aprendes y además creas algo tuyo.

## Qué apuntar (plantilla)
- Tempo: ___ BPM · Tono: ___
- Estructura: Intro ___ · Estrofa ___ · Estribillo ___ ...
- Batería: ¿qué estilo? ¿dónde cae la caja?
- ¿Qué entra y qué sale en cada parte?

! Recrear canciones para aprender está genial. Publicar una copia como si fuera tuya, no: haz siempre tu propia versión.

? Analiza una canción con este método y monta su estructura en la Playlist de Studoo con patrones vacíos (uno por parte).)TXT" },

    // =========================================================================
    { "Nivel 9 · Producción avanzada", "Más efectos: flanger, phaser, vocoder y compañía", "8 min",
      "que es flanger phaser vocoder transient shaper multiband",
R"TXT(En el **Mezclador**, el botón **FX+** de cada pista tiene a la derecha una columna con más efectos. Aquí tienes qué hace cada uno y cuándo usarlo.

[anim:volumenes|Cada pista del mezclador tiene su FX+ con todos estos efectos]

## Flanger y phaser
- **Flanger**: un eco cortísimo que se mueve. Suena "a avión" o a chorro. Queda genial en hi-hats, guitarras y en las subidas antes de un drop.
- **Phaser**: el sonido "gira", como si pasara por un tubo. Muy usado en pads, pianos eléctricos y guitarras.
- **Velocidad**: lo rápido que se mueven los dos. Despacio (0,2 Hz) = suave; rápido (3 Hz) = nervioso.

## Ancho estéreo
- A la **izquierda** el sonido se junta en el centro (mono). Úsalo en el **bajo** y el **bombo**: así suenan firmes en cualquier altavoz.
- A la **derecha** se abre hacia los lados. Para pads, coros y efectos.

## Transitorios (Golpe y Cola)
- **Golpe**: el principio de cada sonido. Súbelo y la batería "pega" más; bájalo y suena más suave.
- **Cola**: lo que suena después del golpe. Bájala para una batería seca y apretada; súbela para que suene más grande.

## Compresor multibanda
Aprieta por separado los **graves**, los **medios** y los **agudos**. En el **Master**, un 20-40% hace que la canción suene más igualada y fuerte.

## Vocoder: la voz de robot
1. Graba tu voz en una pista del mezclador (por ejemplo, la 7).
2. Pon un **sinte** tocando acordes largos en otra pista.
3. En el FX+ de la pista del **sinte**, en VOCODER, elige la pista de la **voz**.
4. Baja el volumen de la pista de la voz: ¡el sinte "habla" con tus palabras!

## Controlador de picos
Hace que el volumen de una pista mueva un control de otra. Ejemplos:
- El **bombo** baja el **volumen** del bajo (como un sidechain).
- La **caja** abre el **filtro** de un pad a ritmo.
- La **voz** sube la **reverb** de los acordes cuando cantas.

> Además, cada pista tiene ahora **10 huecos** para plugins VST3.

[ir:mezclador|Ir al Mezclador]

? Pon flanger a tus hi-hats y ancho al 0% en el 808. Luego prueba el vocoder con tu voz y unos acordes.)TXT" },

    // =========================================================================
    { "Nivel 9 · Producción avanzada", "Trabajar rápido como los profesionales", "9 min",
      "trucos para producir mas rapido flujo de trabajo",
R"TXT(Studoo tiene muchas herramientas para que pases menos tiempo con el ratón y más haciendo música.

[anim:estructura|Los marcadores te dicen dónde va cada parte de la canción]

## Empezar con una plantilla
**Archivo → Nuevo desde una plantilla**: eliges un estilo (trap, reggaetón, house, lo-fi...) y la canción ya viene montada con su base, la estructura (intro, verso, estribillo...), las pistas con nombre y una mezcla inicial. Solo te queda cambiarla a tu gusto.

## Playlist
- **Marcadores**: clic derecho en la regla (los números) → Añadir marcador. Clic en un marcador = ir allí.
- **Grupos**: clic en el nombre de una pista → Crear un grupo. Pulsa el triángulo del grupo para **plegarlo** y tener más sitio.
- **Hacer único**: clic derecho en un bloque. Crea una copia del patrón para cambiarlo sin tocar los otros bloques.
- **Convertir a audio**: clic derecho en un bloque. Lo graba como un trozo de audio (para cortarlo, darle la vuelta...).
- **Fundido cruzado**: clic derecho en un trozo de audio. Une dos trozos sin cortes bruscos.
- **Directo**: botón de arriba. Lanzas tus patrones con botones mientras suena, como un DJ.

## Piano roll
- **Notas fantasma**: ves en color las notas de los otros canales. Doble clic en una = editar ese canal.
- **Slide**: activa el botón y las notas nuevas se **deslizan** desde la anterior (el 808 del trap).
- **Carril de abajo**: pulsa su nombre para cambiar la **fuerza**, el **panorama**, la **afinación fina** o la **modulación** de cada nota.
- **Herramientas → Generar una idea**: Studoo crea una melodía, un bajo, acordes o un arpegio en tu escala. ¿No te gusta? Ctrl+Z y otra vez.
- **Humanizar**: mueve un poquito cada nota para que suene menos "a máquina".
- **Unir** y **partir** notas.

## Más trucos
- **Capas**: clic derecho en un canal → Capas. Ese canal hace sonar otros a la vez (un piano + unas cuerdas con las mismas notas).
- **Recuperar lo que tocaste**: Studoo se acuerda de lo que tocaste en el teclado aunque no le dieras a grabar. Menú → Recuperar lo que tocaste.
- **Visualizador**: menú → Visualizador. Crea un **vídeo** con animaciones y tu canción para subirlo a redes.

[ir:playlist|Ir a la Playlist]

? Empieza una canción con la plantilla de tu estilo favorito, añade un marcador "Drop" y genera una melodía con Herramientas → Generar una idea.)TXT" },
    };
}
