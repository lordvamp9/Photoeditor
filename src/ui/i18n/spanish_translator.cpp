#include "ui/i18n/spanish_translator.hpp"

namespace PhotoStudio::UI {

SpanishTranslator::SpanishTranslator(QObject* parent) : QTranslator(parent)
{
    m_map = {
        // ----- Menus -----
        {"&File", "&Archivo"},
        {"&New...", "&Nuevo..."},
        {"&Open...", "&Abrir..."},
        {"Import as &Layer...", "Importar como &capa..."},
        {"&Save Project", "&Guardar proyecto"},
        {"Save Project &As...", "Guardar proyecto &como..."},
        {"&Export As...", "&Exportar como..."},
        {"E&xit", "&Salir"},
        {"&Edit", "&Edicion"},
        {"&Undo", "&Deshacer"},
        {"&Redo", "&Rehacer"},
        {"Select &All", "Seleccionar &todo"},
        {"&Deselect", "&Deseleccionar"},
        {"&Invert Selection", "&Invertir seleccion"},
        {"&Preferences...", "&Preferencias..."},
        {"&Image", "&Imagen"},
        {"Image &Size...", "&Tamano de imagen..."},
        {"&Canvas Size...", "Tamano de &lienzo..."},
        {"Rotate 90 CW", "Rotar 90 horario"},
        {"Rotate 90 CCW", "Rotar 90 antihorario"},
        {"Flip Horizontal", "Voltear horizontal"},
        {"Flip Vertical", "Voltear vertical"},
        {"&Flatten Image", "&Aplanar imagen"},
        {"&Layer", "&Capa"},
        {"Add &Text Layer...", "Anadir capa de &texto..."},
        {"Filte&r", "Filt&ro"},
        {"&View", "&Ver"},
        {"Zoom &In", "&Acercar"},
        {"Zoom &Out", "A&lejar"},
        {"&Fit on Screen", "&Ajustar a pantalla"},
        {"&Actual Pixels", "Pixeles &reales"},
        {"&Help", "A&yuda"},
        {"&About PhotoStudio", "&Acerca de PhotoStudio"},
        {"About PhotoStudio", "Acerca de PhotoStudio"},

        // ----- Docks / panels -----
        {"Tools", "Herramientas"},
        {"Layers", "Capas"},
        {"History", "Historial"},
        {"Tool Options", "Opciones de herramienta"},
        {"Color", "Color"},
        {"Select a tool to edit its options", "Selecciona una herramienta para editar sus opciones"},

        // ----- Dialogs -----
        {"Open", "Abrir"},
        {"Could not open file:\n%1", "No se pudo abrir el archivo:\n%1"},
        {"Import as Layer", "Importar como capa"},
        {"Import Layer", "Importar capa"},
        {"Project saved", "Proyecto guardado"},
        {"Save Project As", "Guardar proyecto como"},
        {"Export As", "Exportar como"},
        {"Exported %1", "Exportado %1"},
        {"Export", "Exportar"},
        {"Export failed.", "La exportacion fallo."},
        {"Image Size", "Tamano de imagen"},
        {"Canvas Size", "Tamano de lienzo"},
        {"Rotate 90", "Rotar 90"},
        {"Flatten Image", "Aplanar imagen"},
        {"Text Layer", "Capa de texto"},
        {"Add Text Layer", "Anadir capa de texto"},
        {"Text:", "Texto:"},
        {"No document open\nFile - New  or  File - Open",
         "Ningun documento abierto\nArchivo - Nuevo  o  Archivo - Abrir"},

        // ----- Text dialog -----
        {"Text", "Texto"},
        {"Type your text here...", "Escribe tu texto aqui..."},
        {"Font:", "Fuente:"},
        {"Size:", "Tamano:"},
        {"Style:", "Estilo:"},
        {"Bold", "Negrita"},
        {"Italic", "Cursiva"},
        {"Underline", "Subrayado"},
        {"Color:", "Color:"},
        {"Text Color", "Color del texto"},

        // ----- Settings dialog -----
        {"Preferences", "Preferencias"},
        {"Language:", "Idioma:"},
        {"English", "Ingles"},
        {"Spanish", "Espanol"},
        {"Language changes take effect after restarting PhotoStudio.",
         "El cambio de idioma se aplica al reiniciar PhotoStudio."},
        {"Restart required", "Reinicio necesario"},
        {"Please restart PhotoStudio to apply the new language.",
         "Reinicia PhotoStudio para aplicar el nuevo idioma."},

        // ----- Tools (names from the core) -----
        {"Move", "Mover"},
        {"Rectangular Marquee", "Marco rectangular"},
        {"Elliptical Marquee", "Marco eliptico"},
        {"Lasso", "Lazo"},
        {"Magic Wand", "Varita magica"},
        {"Brush", "Pincel"},
        {"Eraser", "Borrador"},
        {"Clone Stamp", "Tampon de clonar"},
        {"Paint Bucket", "Bote de pintura"},
        {"Gradient", "Degradado"},
        {"Color Picker", "Cuentagotas"},
        {"Eyedropper", "Cuentagotas"},

        // ----- Tool options -----
        {"Size", "Tamano"},
        {"Hardness", "Dureza"},
        {"Opacity", "Opacidad"},
        {"Flow", "Flujo"},
        {"Spacing", "Espaciado"},
        {"Angle", "Angulo"},
        {"Smoothing", "Suavizado"},
        {"Pressure controls size", "La presion controla el tamano"},
        {"Pressure controls opacity", "La presion controla la opacidad"},
        {"Feather", "Difuminar"},
        {"Tolerance", "Tolerancia"},

        // ----- Brush tips -----
        {"Soft Round", "Redondo suave"},
        {"Hard Round", "Redondo duro"},
        {"Airbrush", "Aerografo"},
        {"Calligraphy", "Caligrafia"},
        {"Chalk", "Tiza"},
        {"Scatter", "Dispersion"},
        {"Pencil", "Lapiz"},

        // ----- Filter categories -----
        {"Adjustments", "Ajustes"},
        {"Blur", "Desenfoque"},
        {"Detail", "Detalle"},
        {"Artistic", "Artisticos"},
        {"Distortion", "Distorsion"},
        {"Render", "Render"},
    };
}

QString SpanishTranslator::translate(const char*, const char* sourceText, const char*,
                                     int) const
{
    if (!sourceText)
        return {};
    const auto it = m_map.constFind(QString::fromUtf8(sourceText));
    return it != m_map.constEnd() ? it.value() : QString();
}

} // namespace PhotoStudio::UI
