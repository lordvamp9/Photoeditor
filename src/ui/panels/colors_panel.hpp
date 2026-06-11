#pragma once

#include "tools/tool_base.hpp"

#include <QPushButton>
#include <QWidget>

namespace PhotoStudio::UI {

// Foreground/background swatches bound to the shared ToolContext.
class ColorsPanel : public QWidget {
    Q_OBJECT

public:
    explicit ColorsPanel(Tools::ToolContext* ctx, QWidget* parent = nullptr);

    // Called when a tool (e.g. the color picker) changes the foreground.
    void syncFromContext();

private:
    void pickColor(bool foreground);
    void swapColors();
    void updateSwatches();

    Tools::ToolContext* m_ctx;
    QPushButton* m_fgSwatch;
    QPushButton* m_bgSwatch;
};

} // namespace PhotoStudio::UI
