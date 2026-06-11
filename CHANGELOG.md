# Changelog

## [1.0.0] - 2026-06-11

Primera version publica, bajo la marca vamp9.

### Motor
- Sistema de capas con opacidad, visibilidad, mascaras, offsets y 23 modos de mezcla.
- Composicion Porter-Duff con alpha recto, paralelizada con cv::parallel_for_.
- Historial de deshacer/rehacer de 48 estados con panel navegable.
- Selecciones de 8 bits (rectangulo, elipse, lazo, varita magica) con feather y operaciones de combinacion.

### Filtros
- 33 filtros en 6 categorias (Ajustes, Desenfoque, Detalle, Artisticos, Distorsion, Render).
- Dialogo generico con vista previa en vivo y restauracion al cancelar.
- Aplicacion restringida a la seleccion activa con fusion suavizada.

### Herramientas
- Mover, marcos de seleccion, lazo, varita, pincel, borrador, tampon de clonar, bote de pintura, degradado y cuentagotas.

### E/S
- Importacion PNG/JPEG/WebP/TIFF/BMP; exportacion con control de calidad.
- Formato de proyecto .psproj (capas + mascaras + metadatos).

### UI
- Tema Frutiger Aero-Aeroglass (QSS), iconos SVG, paneles acoplables, lienzo con zoom/pan y hormigas marchantes.
- Capas de texto con seleccion de fuente.

### Infraestructura
- CMake 3.20+, presets, vcpkg manifest, conanfile, clang-format/tidy.
- Tests unitarios con GoogleTest y CI en GitHub Actions (Ubuntu).
