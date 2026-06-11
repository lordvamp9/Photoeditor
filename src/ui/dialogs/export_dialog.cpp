#include "ui/dialogs/export_dialog.hpp"

#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>

namespace PhotoStudio::UI {

ExportDialog::ExportDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("Export Image"));
    setMinimumWidth(320);

    auto* layout = new QFormLayout(this);

    m_format = new QComboBox();
    m_format->addItem("PNG", ".png");
    m_format->addItem("JPEG", ".jpg");
    m_format->addItem("WebP", ".webp");
    m_format->addItem("TIFF", ".tiff");
    m_format->addItem("BMP", ".bmp");

    m_quality = new QSlider(Qt::Horizontal);
    m_quality->setRange(1, 100);
    m_quality->setValue(92);

    auto* qualityLabel = new QLabel(tr("92"));
    connect(m_quality, &QSlider::valueChanged, qualityLabel,
            [qualityLabel](int v) { qualityLabel->setText(QString::number(v)); });

    layout->addRow(tr("Format"), m_format);
    layout->addRow(tr("Quality"), m_quality);
    layout->addRow(QString(), qualityLabel);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addRow(buttons);
}

QString ExportDialog::formatExtension() const
{
    return m_format->currentData().toString();
}

int ExportDialog::quality() const
{
    return m_quality->value();
}

} // namespace PhotoStudio::UI
