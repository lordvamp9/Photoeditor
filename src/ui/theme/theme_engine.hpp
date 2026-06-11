#pragma once

#include <QApplication>

namespace PhotoStudio::UI {

// Applies the Frutiger Aero / Aeroglass dark theme: Fusion style, dark palette
// and the QSS sheet bundled in the application resources.
class ThemeEngine {
public:
    static void apply(QApplication& app);
};

} // namespace PhotoStudio::UI
