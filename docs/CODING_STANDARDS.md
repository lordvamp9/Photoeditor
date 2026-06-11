# Estandares de codigo

- **C++20**, sin extensiones de compilador.
- Formato con `.clang-format` (LLVM base, 4 espacios, 110 columnas). Ejecutar
  `clang-format -i` antes de cada commit.
- Analisis estatico con `.clang-tidy` (bugprone, performance, modernize,
  readability).

## Convenciones

- Namespaces: `PhotoStudio::Core`, `::Filters`, `::Tools`, `::IO`, `::Math`,
  `::UI`, `::Utils`, `::Performance`.
- Tipos: `PascalCase`. Funciones y variables: `camelCase`. Miembros: prefijo
  `m_`. Constantes: prefijo `k`.
- Alias numericos de `core/types.hpp` (`u8`, `u32`, `i32`, `f32`, `f64`).
- Punteros inteligentes: `Layer_Ptr = std::shared_ptr<Layer>`,
  `Document_Ptr = std::unique_ptr<Document>`. Nada de `new`/`delete` manuales
  fuera de la propiedad de Qt (parent-child).
- El core **no** incluye cabeceras de Qt; la UI **no** implementa logica de
  imagen (solo orquesta llamadas al core).
- Imagenes siempre `CV_8UC4` RGBA de alpha recto; mascaras `CV_8UC1`.
- Los comentarios documentan restricciones no obvias, no parafrasean el codigo.

## Tests

Todo cambio en `core/`, `filters/` o `io/` debe acompanarse de un unit test en
`tests/unit/`. Los tests no deben depender de Qt ni de archivos del repositorio
(usar `std::filesystem::temp_directory_path()` para artefactos temporales).
