#include "ui/dialogs/new_document_dialog.hpp"

#include <QDialogButtonBox>
#include <QFormLayout>

namespace PhotoStudio::UI {

NewDocumentDialog::NewDocumentDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("New Document"));
    setMinimumWidth(320);

    auto* layout = new QFormLayout(this);

    m_width = new QSpinBox();
    m_width->setRange(1, 16384);
    m_width->setValue(1920);
    m_width->setSuffix(" px");

    m_height = new QSpinBox();
    m_height->setRange(1, 16384);
    m_height->setValue(1080);
    m_height->setSuffix(" px");

    m_dpi = new QSpinBox();
    m_dpi->setRange(36, 1200);
    m_dpi->setValue(72);

    m_background = new QComboBox();
    m_background->addItems({tr("White"), tr("Transparent")});

    layout->addRow(tr("Width"), m_width);
    layout->addRow(tr("Height"), m_height);
    layout->addRow(tr("Resolution (DPI)"), m_dpi);
    layout->addRow(tr("Background"), m_background);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addRow(buttons);
}

u32 NewDocumentDialog::documentWidth() const
{
    return static_cast<u32>(m_width->value());
}

u32 NewDocumentDialog::documentHeight() const
{
    return static_cast<u32>(m_height->value());
}

u32 NewDocumentDialog::documentDpi() const
{
    return static_cast<u32>(m_dpi->value());
}

bool NewDocumentDialog::transparentBackground() const
{
    return m_background->currentIndex() == 1;
}

} // namespace PhotoStudio::UI
