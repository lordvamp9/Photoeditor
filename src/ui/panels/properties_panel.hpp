#pragma once

#include "tools/tool_base.hpp"

#include <QVBoxLayout>
#include <QWidget>

namespace PhotoStudio::UI {

// Shows slider controls for the active tool's options.
class PropertiesPanel : public QWidget {
    Q_OBJECT

public:
    explicit PropertiesPanel(QWidget* parent = nullptr);

    void setTool(Tools::ToolBase* tool);

private:
    void rebuild();

    Tools::ToolBase* m_tool = nullptr;
    QVBoxLayout* m_layout;
    QWidget* m_content = nullptr;
};

} // namespace PhotoStudio::UI
