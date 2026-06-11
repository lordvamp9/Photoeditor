#include "ui/main_window.hpp"
#include "ui/theme/theme_engine.hpp"
#include "utils/logger.hpp"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("PhotoStudio");
    app.setOrganizationName("vamp9");
    app.setApplicationVersion("1.0.0");

    PhotoStudio::Utils::Logger::info("PhotoStudio Premium starting");
    PhotoStudio::UI::ThemeEngine::apply(app);

    PhotoStudio::UI::MainWindow window;
    window.show();

    return app.exec();
}
