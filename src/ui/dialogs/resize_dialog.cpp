#include "ui/dialogs/resize_dialog.hpp"

#include <QDialogButtonBox>
#include <QFormLayout>

#include <cmath>

namespace PhotoStudio::UI {

ResizeDialog::ResizeDialog(u32 currentWidth, u32 currentHeight, const QString& title,
                           QWidget* parent)
    : QDialog(parent), m_ratio(static_cast<f64>(currentWidth) / currentHeight)
{
    setWindowTitle(title);
    setMinimumWidth(300);

    auto* layout = new QFormLayout(this);

    m_width = new QSpinBox();
    m_width->setRange(1, 16384);
    m_width->setValue(static_cast<int>(currentWidth));
    m_width->setSuffix(" px");

    m_height = new QSpinBox();
    m_height->setRange(1, 16384);
    m_height->setValue(static_cast<int>(currentHeight));
    m_height->setSuffix(" px");

    m_keepRatio = new QCheckBox(tr("Constrain proportions"));
    m_keepRatio->setChecked(true);

    layout->addRow(tr("Width"), m_width);
    layout->addRow(tr("Height"), m_height);
    layout->addRow(QString(), m_keepRatio);

    connect(m_width, &QSpinBox::valueChanged, this, [this](int value) {
        if (m_updating || !m_keepRatio->isChecked())
            return;
        m_updating = true;
        m_height->setValue(std::max(1, static_cast<int>(std::lround(value / m_ratio))));
        m_updating = false;
    });
    connect(m_height, &QSpinBox::valueChanged, this, [this](int value) {
        if (m_updating || !m_keepRatio->isChecked())
            return;
        m_updating = true;
        m_width->setValue(std::max(1, static_cast<int>(std::lround(value * m_ratio))));
        m_updating = false;
    });

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addRow(buttons);
}

u32 ResizeDialog::newWidth() const
{
    return static_cast<u32>(m_width->value());
}

u32 ResizeDialog::newHeight() const
{
    return static_cast<u32>(m_height->value());
}

} // namespace PhotoStudio::UI
