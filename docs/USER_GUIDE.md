# Guia de usuario - PhotoStudio Premium

## Primeros pasos

Al abrir la aplicacion se crea un documento de 1280x800. Usa **File > New** para
elegir tamano, DPI y fondo (blanco o transparente), o **File > Open** para abrir
una imagen (PNG, JPEG, WebP, TIFF, BMP) o un proyecto `.psproj`.

## Navegacion del lienzo

| Accion | Atajo |
|---|---|
| Zoom | Rueda del raton |
| Paneo | Boton central, o Espacio + arrastrar |
| Ajustar a pantalla | Ctrl+0 |
| Pixeles reales (100%) | Ctrl+1 |

## Herramientas

- **Mover (V)**: arrastra la capa activa.
- **Marcos / Lazo / Varita**: crean selecciones. Shift suma, Alt resta,
  Shift+Alt intersecta. "Feather" suaviza el borde; la varita tiene tolerancia.
- **Pincel**: tamano, dureza y opacidad en el panel Tool Options. Pinta con el
  color frontal del panel Color.
- **Borrador**: borra alpha con el mismo motor del pincel.
- **Tampon de clonar**: Alt+clic define el origen; despues pinta normalmente.
- **Bote de pintura**: relleno por tolerancia de color.
- **Degradado**: arrastra una linea; rellena del color frontal al de fondo.
- **Cuentagotas**: toma el color compuesto bajo el cursor.

Las herramientas de pintura y relleno **respetan la seleccion activa**.

## Capas

El panel Layers muestra la pila (la superior primero):

- Casilla = visibilidad. Doble clic en el nombre lo renombra.
- Slider de opacidad y desplegable de modo de mezcla (23 modos).
- Botones: `+` nueva, `=` duplicar, `^`/`v` reordenar, `M` fusionar hacia abajo,
  `-` eliminar.
- **Layer > Add Text Layer** crea una capa de texto con la fuente que elijas.

## Filtros

El menu **Filter** se organiza por categorias. Cada filtro abre un dialogo con
sliders y **vista previa en vivo** sobre la capa activa; Cancelar restaura la
imagen original. Si hay una seleccion activa, el filtro solo afecta a esa zona.

## Historial

**Edit > Undo/Redo** (Ctrl+Z / Ctrl+Y) o el panel History, que permite saltar a
cualquier estado anterior con un clic (hasta 48 estados).

## Guardar y exportar

- **File > Save Project** guarda un `.psproj` con todas las capas.
- **File > Export As** aplana la imagen y la guarda como PNG, JPEG, WebP, TIFF
  o BMP, con control de calidad para los formatos con perdida.
