# PhotoStudio Premium

Editor profesional de fotos de escritorio, escrito en **C++20** con **Qt 6** y **OpenCV**, con estetica **Frutiger Aero / Aeroglass** (tema oscuro con acentos cian, glassmorphism e iconografia SVG, sin emojis).

Publicado bajo la marca **vamp9**.

---

## Caracteristicas

### Motor de imagen
- Sistema de capas completo: opacidad, visibilidad, mascaras, offsets, renombrado, duplicado, reordenado, fusionar hacia abajo y aplanar.
- **23 modos de mezcla** (Normal, Multiply, Screen, Overlay, Soft/Hard/Vivid/Linear/Pin Light, Color Dodge/Burn, Difference, Exclusion, Hue, Saturation, Color, Luminosity...) con composicion Porter-Duff de alpha recto.
- Selecciones con mascara de 8 bits: rectangulo, elipse, lazo y varita magica, con feather, y combinacion (Shift = sumar, Alt = restar, Shift+Alt = intersectar).
- Historial de deshacer/rehacer multinivel (48 estados) con panel de historial navegable.

### Herramientas
Mover, marco rectangular, marco eliptico, lazo, varita magica, pincel (tamano/dureza/opacidad con suavizado radial), borrador, tampon de clonar (Alt+clic fija el origen), bote de pintura, degradado lineal y cuentagotas.

### Filtros (33 integrados)
- **Ajustes**: Brillo/Contraste, Niveles, Curvas (spline Catmull-Rom), Exposicion, Tono/Saturacion, Equilibrio de color, Intensidad (vibrance), Invertir, Desaturar, Sepia, Posterizar, Umbral.
- **Desenfoque**: Gaussiano, De caja, De movimiento, De superficie (bilateral).
- **Detalle**: Enfocar, Mascara de enfoque, Paso alto, Claridad, Reducir ruido (NL-means).
- **Artisticos**: Pintura al oleo, Acuarela, Boceto a lapiz, Comic, Pixelar, Vineta, Anadir ruido.
- **Distorsion**: Encoger/Inflar, Remolino, Onda, Esferizar.
- **Render**: Nubes fractales, Mapa de degradado.

Todos los filtros tienen **vista previa en vivo** con sliders y **respetan la seleccion activa**.

### Entrada / salida
- Importa PNG, JPEG, WebP, TIFF y BMP (como documento o como capa).
- Exporta PNG, JPEG (calidad), WebP (calidad), TIFF y BMP.
- Formato de proyecto propio **`.psproj`** que conserva capas, mascaras, modos de mezcla, opacidad y offsets.

### Interfaz
- Tema Frutiger Aero-Aeroglass completo via QSS (paleta `#0F1419` / `#00B8FF`).
- Paneles acoplables: Capas, Historial, Opciones de herramienta y Color.
- Lienzo con zoom (rueda), ajuste a pantalla, pixeles reales, paneo (espacio o boton central), tablero de transparencia y hormigas marchantes animadas en la seleccion.
- Capas de texto con seleccion de fuente.

---

## Compilacion

Requisitos: **CMake 3.20+**, un compilador C++20 (MSVC 2022 / GCC 11+ / Clang 15+), **Qt 6** (Core, Gui, Widgets, Svg) y **OpenCV 4** (core, imgproc, imgcodecs, photo).

### Linux (Ubuntu/Debian)

```bash
sudo apt install cmake ninja-build qt6-base-dev libqt6svg6-dev libopencv-dev
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/src/PhotoStudio
```

### Windows (vcpkg)

```powershell
git clone https://github.com/Microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat
$env:VCPKG_ROOT = (Resolve-Path .\vcpkg)
cmake --preset windows-msvc
cmake --build --preset windows-msvc
```

### macOS

```bash
brew install cmake ninja qt opencv
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Mas detalle en [docs/BUILDING.md](docs/BUILDING.md).

### Tests

```bash
cmake -B build -DPHOTOSTUDIO_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

---

## Estructura

```
src/
  core/         Capas, documento, modos de mezcla, historial, seleccion
  filters/      33 filtros CPU organizados por categoria + registro
  tools/        Pincel, borrador, clonar, selecciones, relleno, degradado...
  io/           Importacion/exportacion de imagenes y formato .psproj
  math/         Color (HSL/luminancia), interpolacion, splines
  performance/  Thread pool
  ui/           Qt6: ventana principal, canvas, paneles, dialogos, tema
shaders/        GLSL 4.60 para el pipeline GPU (composicion, blur, curvas...)
assets/         Tema QSS + iconos SVG
tests/          Tests unitarios (GoogleTest)
```

Arquitectura completa en [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

---

## Licencia

**Vamp9 Non-Commercial Attribution License 1.0** - ver [LICENSE](LICENSE).

En resumen: uso, modificacion y distribucion libres para fines no comerciales manteniendo la atribucion a **vamp9**. Queda prohibida la comercializacion, salvo que se realice bajo la marca **vamp9** con autorizacion expresa de su titular.

Copyright (c) 2026 **vamp9**
