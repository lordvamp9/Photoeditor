#include "ui/panels/properties_panel.hpp"

#include <QLabel>
#include <QSlider>

#include <cmath>

namespace PhotoStudio::UI {

PropertiesPanel::PropertiesPanel(QWidget* parent) : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(8, 8, 8, 8);
    m_layout->setSpacing(8);
    rebuild();
}

void PropertiesPanel::setTool(Tools::ToolBase* tool)
{
    m_tool = tool;
    rebuild();
}

void PropertiesPanel::rebuild()
{
    delete m_content;
    m_content = new QWidget(this);
    auto* layout = new QVBoxLayout(m_content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    if (!m_tool) {
        auto* hint = new QLabel(tr("Select a tool to edit its options"));
        hint->setProperty("level", "3");
        hint->setWordWrap(true);
        layout->addWidget(hint);
    } else {
        auto* title = new QLabel(QString::fromStdString(m_tool->name()));
        title->setProperty("level", "1");
        layout->addWidget(title);

        for (const auto& option : m_tool->options()) {
            auto* label = new QLabel(QString::fromStdString(option.label));
            label->setProperty("level", "2");
            layout->addWidget(label);

            auto* slider = new QSlider(Qt::Horizontal);
            const f32 scale = option.step > 0 ? 1.0f / option.step : 1.0f;
            slider->setRange(static_cast<int>(std::lround(option.minValue * scale)),
                             static_cast<int>(std::lround(option.maxValue * scale)));
            slider->setValue(static_cast<int>(std::lround(option.get() * scale)));

            const auto setter = option.set;
            connect(slider, &QSlider::valueChanged, this,
                    [setter, scale](int value) { setter(value / scale); });
            layout->addWidget(slider);
        }
        layout->addStretch(1);
    }

    m_layout->addWidget(m_content);
}

} // namespace PhotoStudio::UI
