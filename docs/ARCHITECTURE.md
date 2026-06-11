# Arquitectura de PhotoStudio Premium

## Vision general

El proyecto se divide en dos capas estrictas:

1. **`photostudio_core`** (biblioteca estatica, sin Qt): motor de imagen completo.
   Depende solo de OpenCV y la STL. Es la parte testeada por unit tests.
2. **`PhotoStudio`** (ejecutable Qt6): interfaz, dialogos y tema. Traduce eventos
   de usuario a llamadas al core.

```
Usuario -> Qt UI (MainWindow/CanvasWidget) -> ToolContext -> Core::Document
                                                            -> Filters::*
Core::Document::renderComposite() -> cv::Mat RGBA -> QImage -> QPainter
```

## Namespaces

| Namespace | Responsabilidad |
|---|---|
| `PhotoStudio::Core` | `Document`, `Layer`, modos de mezcla, historial, seleccion |
| `PhotoStudio::Filters` | `FilterBase`, `FilterRegistry`, 33 filtros por categoria |
| `PhotoStudio::Tools` | `ToolBase`, `ToolContext`, herramientas interactivas |
| `PhotoStudio::IO` | Importacion/exportacion y formato `.psproj` |
| `PhotoStudio::Math` | Color (luminancia, HSL no separable), interpolacion |
| `PhotoStudio::Performance` | `ThreadPool` |
| `PhotoStudio::UI` | Qt: ventana, canvas, paneles, dialogos, tema |

## Modelo de imagen

- Pixeles: `cv::Mat` `CV_8UC4`, **RGBA con alpha recto** (no premultiplicado).
- Mascaras y selecciones: `CV_8UC1` (0-255, los valores intermedios suavizan).
- Cada `Layer` tiene un `offset` entero respecto al documento, lo que permite
  mover capas sin re-muestrear.

## Composicion

`Document::renderComposite()` recorre las capas de abajo arriba y llama a
`compositeOver()` (`core/blend_modes.cpp`):

- Mezcla "over" de Porter-Duff con el termino de modo de mezcla:
  `Co = as*(1-ab)*Cs + as*ab*B(Cb,Cs) + (1-as)*ab*Cb`
- Modos separables por canal y no separables (Hue/Saturation/Color/Luminosity)
  segun la especificacion PDF/Photoshop.
- Paralelizado por filas con `cv::parallel_for_`.
- El resultado se cachea (`m_compositeCache`) y se invalida con `markDirty()`.

## Historial

Snapshots completos (clonado profundo de capas + seleccion) con un maximo de 48
estados. Cada operacion de usuario termina llamando a `Document::addState()`
a traves de `ToolContext::commitHistory`. `undo()/redo()/gotoState()` restauran
clones, de modo que los estados del historial son inmutables.

## Filtros

`FilterBase::apply(cv::Mat& rgba, ParamMap)` muta la imagen en sitio. Cada filtro
declara sus parametros (`FilterParam{key, label, min, max, default, step}`) y el
`FilterDialog` genera los sliders automaticamente, con vista previa en vivo
(restaurando un backup al cancelar). `applyFilterToActiveLayer()` limita el
efecto a la seleccion activa mediante fusion ponderada por la mascara.

## Herramientas

Las herramientas son objetos sin estado de Qt que reciben coordenadas de
documento + modificadores y operan sobre `ToolContext.document`. El canvas
traduce coordenadas de widget a documento (zoom/pan) y consulta
`previewPolygon()` para dibujar overlays (marquesinas, linea de degradado).

## Formato de proyecto (.psproj)

```
"PSPJ" | u32 version | u32 jsonLen | JSON (doc + metadatos de capas)
por capa: u64 len | PNG(pixeles RGBA) | u64 len | PNG(mascara, opcional)
```

PNG como contenedor de blobs da compresion sin dependencias extra.

## Shaders

`shaders/*.glsl` contiene los kernels GLSL 4.60 del pipeline GPU planificado
(composicion, blur separable, curvas via LUT, liquify por textura de
desplazamiento). El renderizado actual es CPU (QPainter + OpenCV); los shaders
documentan el siguiente paso de aceleracion.
