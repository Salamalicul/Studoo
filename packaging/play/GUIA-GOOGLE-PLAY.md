# Guía: publicar Studoo en Google Play (paso a paso)

Sigue los pasos en orden. Los textos para copiar y pegar están en [`FICHA.md`](FICHA.md) y las imágenes en [`graficos/`](graficos/).

---

## 1. Crear la cuenta de desarrollador (una sola vez)

1. Entra en **https://play.google.com/console** con la cuenta de Google que quieras usar (mejor una solo para Studoo).
2. Elige **"Uso personal"** (cuenta personal).
3. Paga la cuota única de **25 $**.
4. **Verifica tu identidad** con tu DNI, tu dirección y un número de teléfono. Google puede tardar unos días en revisarlo.

## 2. Crear la app

1. En la Play Console: **Crear app**.
2. Nombre: `Studoo: haz música y beats` · Idioma: **Español (España)** · **App** · **Gratis**.
3. Acepta las declaraciones y pulsa **Crear app**.

## 3. Rellenar "Configura tu app"

En el **Panel**, completa cada apartado con lo que pone en `FICHA.md`:

- Política de privacidad
- Acceso a la app
- Anuncios (**No**)
- Clasificación de contenido
- Público objetivo
- Seguridad de los datos
- Categoría y datos de contacto
- **Ficha principal de Play Store**: nombre, descripciones, icono, gráfico de funciones y capturas.

## 4. Subir la app (el archivo .aab)

1. Descarga el archivo **`Studoo-vX.X.X-GooglePlay.aab`** de la última versión en  
   https://github.com/Salamalicul/Studoo/releases
2. En la Play Console: **Probar y publicar > Pruebas > Prueba cerrada > Crear canal** (llámalo "Probadores").
3. **Crear versión** y sube el `.aab`.
4. Cuando pregunte por la **firma de la app**, deja la opción recomendada: **"Google gestiona y protege la clave de firma de tu app"**. La llave que ya tienes pasa a ser tu **clave de subida**, y Google firma la app que reciben los usuarios.
5. Nombre de la versión: el número (por ejemplo `1.9.0`). Notas: copia las novedades.

## 5. La prueba cerrada (obligatoria para cuentas personales nuevas)

Google exige que **al menos 12 personas** prueben la app durante **14 días seguidos** antes de publicarla para todo el mundo.

1. En el canal "Probadores", añade una **lista de correos** con al menos 12 personas (amigos o familia con Android). Usa el correo de su cuenta de Google.
2. Comparte con ellos el **enlace de participación** que te da la Play Console. Tienen que pulsarlo, aceptar e instalar Studoo desde Google Play.
3. Pídeles que **no la desinstalen** y que la abran de vez en cuando durante los 14 días.
4. Pasados los 14 días: **Panel > Solicitar acceso a producción**. Responde las preguntas sobre la prueba y espera la revisión (unos días).

## 6. Cobrar: perfil de pagos y productos

1. **Monetizar > Configuración de monetización**: crea tu **perfil de pagos** con tus datos bancarios y fiscales. Google te paga cada mes. Se queda con un **15 %** de cada venta.
2. **Monetizar > Productos > Productos de compra única**: crea los **4 productos** de `FICHA.md` con **exactamente esos IDs** (`studoo_pro`, `donacion_pequena`, `donacion_mediana`, `donacion_grande`) y **actívalos**.  
   ⚠️ Esta opción solo aparece después de subir un `.aab` (paso 4).
3. **Clave de licencia** (recomendado, protege contra compras falsas):
   - **Monetizar > Configuración de monetización > Licencias**: copia la "clave pública RSA" (un texto largo).
   - En GitHub: **Settings > Secrets and variables > Actions > New repository secret**. Nombre: `PLAY_LICENSE_KEY`. Valor: la clave.
   - La siguiente versión ya comprobará que cada compra es de verdad.

## 7. Probar las compras sin pagar

1. **Configuración (de la cuenta) > Pruebas de licencias**: añade tu correo y el de quien quieras.
2. Con esas cuentas, al comprar Studoo Pro o donar, Google lo marca como **compra de prueba** y no cobra.

## 8. Actualizaciones

Cada vez que publiquemos una versión nueva de Studoo, GitHub crea el `.aab` nuevo. Súbelo en la Play Console: **Crear versión** en el canal de producción (o en el de pruebas). Google se encarga de actualizar la app en los móviles.

---

### Preguntas frecuentes

- **¿Puedo seguir publicando el APK en GitHub?** Sí, pero quien lo instale desde GitHub tendrá todo gratis. Cuando Studoo esté en Google Play, conviene dejar de publicar el APK, o publicarlo solo para probar.
- **¿Cuánto dinero da?** Depende de las descargas. Muchas apps pequeñas venden pocas licencias al principio. Ayuda mucho enseñar Studoo en TikTok, Instagram o YouTube (por ejemplo, haciendo un beat en 1 minuto).
- **Impuestos**: lo que ganes cuenta como ingreso. Pregunta a un gestor cómo declararlo según tu situación.
