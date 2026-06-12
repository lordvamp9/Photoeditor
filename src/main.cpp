#include "ui/dialogs/settings_dialog.hpp"
#include "ui/i18n/spanish_translator.hpp"
#include "ui/main_window.hpp"
#include "ui/theme/theme_engine.hpp"
#include "utils/logger.hpp"

#include <QApplication>
#include <QIcon>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("PhotoStudio");
    app.setOrganizationName("vamp9");
    app.setApplicationVersion("1.1.0");
    app.setWindowIcon(QIcon(":/icon.png"));

    // UI language (English by default; Spanish optional via Preferences).
    if (PhotoStudio::UI::SettingsDialog::savedLanguage() == "es")
        app.installTranslator(new PhotoStudio::UI::SpanishTranslator(&app));

    PhotoStudio::Utils::Logger::info("PhotoStudio Premium starting");
    PhotoStudio::UI::ThemeEngine::apply(app);

    PhotoStudio::UI::MainWindow window;
    window.show();

    return app.exec();
}
