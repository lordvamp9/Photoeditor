#include "ui/theme/theme_engine.hpp"

#include <QFile>
#include <QPalette>
#include <QStyleFactory>

namespace PhotoStudio::UI {

void ThemeEngine::apply(QApplication& app)
{
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette palette;
    const QColor bgPrimary(15, 20, 25);
    const QColor bgSecondary(26, 32, 40);
    const QColor bgTertiary(36, 45, 56);
    const QColor textPrimary(245, 245, 247);
    const QColor textSecondary(161, 166, 177);
    const QColor accent(0, 184, 255);

    palette.setColor(QPalette::Window, bgPrimary);
    palette.setColor(QPalette::WindowText, textPrimary);
    palette.setColor(QPalette::Base, bgSecondary);
    palette.setColor(QPalette::AlternateBase, bgTertiary);
    palette.setColor(QPalette::Text, textPrimary);
    palette.setColor(QPalette::Button, bgSecondary);
    palette.setColor(QPalette::ButtonText, textPrimary);
    palette.setColor(QPalette::Highlight, accent);
    palette.setColor(QPalette::HighlightedText, bgPrimary);
    palette.setColor(QPalette::ToolTipBase, bgTertiary);
    palette.setColor(QPalette::ToolTipText, textPrimary);
    palette.setColor(QPalette::PlaceholderText, textSecondary);
    palette.setColor(QPalette::Disabled, QPalette::Text, textSecondary);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, textSecondary);
    app.setPalette(palette);

    QFile qss(":/theme/aero_theme.qss");
    if (qss.open(QIODevice::ReadOnly | QIODevice::Text))
        app.setStyleSheet(QString::fromUtf8(qss.readAll()));
}

} // namespace PhotoStudio::UI
