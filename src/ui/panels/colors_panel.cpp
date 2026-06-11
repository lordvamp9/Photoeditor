#include "ui/panels/colors_panel.hpp"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace PhotoStudio::UI {

namespace {

QString swatchStyle(const cv::Vec4b& color)
{
    return QString("background-color: rgb(%1, %2, %3); border: 1px solid rgba(255,255,255,0.25); "
                   "border-radius: 4px;")
        .arg(color[0])
        .arg(color[1])
        .arg(color[2]);
}

} // namespace

ColorsPanel::ColorsPanel(Tools::ToolContext* ctx, QWidget* parent) : QWidget(parent), m_ctx(ctx)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    auto* row = new QHBoxLayout();
    m_fgSwatch = new QPushButton();
    m_fgSwatch->setFixedSize(44, 44);
    m_fgSwatch->setToolTip(tr("Foreground color"));
    m_bgSwatch = new QPushButton();
    m_bgSwatch->setFixedSize(44, 44);
    m_bgSwatch->setToolTip(tr("Background color"));

    auto* swap = new QPushButton("X");
    swap->setFixedSize(28, 28);
    swap->setToolTip(tr("Swap colors"));

    row->addWidget(m_fgSwatch);
    row->addWidget(m_bgSwatch);
    row->addWidget(swap);
    row->addStretch(1);
    layout->addLayout(row);
    layout->addStretch(1);

    connect(m_fgSwatch, &QPushButton::clicked, this, [this] { pickColor(true); });
    connect(m_bgSwatch, &QPushButton::clicked, this, [this] { pickColor(false); });
    connect(swap, &QPushButton::clicked, this, &ColorsPanel::swapColors);

    updateSwatches();
}

void ColorsPanel::syncFromContext()
{
    updateSwatches();
}

void ColorsPanel::pickColor(bool foreground)
{
    const cv::Vec4b& current = foreground ? m_ctx->foreground : m_ctx->background;
    const QColor initial(current[0], current[1], current[2]);
    const QColor chosen = QColorDialog::getColor(initial, this, tr("Select Color"));
    if (!chosen.isValid())
        return;

    cv::Vec4b& target = foreground ? m_ctx->foreground : m_ctx->background;
    target = {static_cast<u8>(chosen.red()), static_cast<u8>(chosen.green()),
              static_cast<u8>(chosen.blue()), 255};
    updateSwatches();
}

void ColorsPanel::swapColors()
{
    std::swap(m_ctx->foreground, m_ctx->background);
    updateSwatches();
}

void ColorsPanel::updateSwatches()
{
    m_fgSwatch->setStyleSheet(swatchStyle(m_ctx->foreground));
    m_bgSwatch->setStyleSheet(swatchStyle(m_ctx->background));
}

} // namespace PhotoStudio::UI
