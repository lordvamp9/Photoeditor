#include "ui/dialogs/text_dialog.hpp"

#include <QCheckBox>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QFontComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>

#include <algorithm>

namespace PhotoStudio::UI {

TextDialog::TextDialog(const QColor& initialColor, QWidget* parent)
    : QDialog(parent), m_color(initialColor.isValid() ? initialColor : QColor(Qt::black))
{
    setWindowTitle(tr("Text"));
    setMinimumWidth(420);

    auto* layout = new QVBoxLayout(this);

    m_textEdit = new QTextEdit;
    m_textEdit->setPlaceholderText(tr("Type your text here..."));
    m_textEdit->setAcceptRichText(false);
    m_textEdit->setFixedHeight(90);
    layout->addWidget(m_textEdit);

    auto* form = new QFormLayout;
    form->setSpacing(8);

    m_fontCombo = new QFontComboBox;
    form->addRow(tr("Font:"), m_fontCombo);

    m_sizeSpin = new QSpinBox;
    m_sizeSpin->setRange(6, 720);
    m_sizeSpin->setValue(48);
    m_sizeSpin->setSuffix(" pt");
    form->addRow(tr("Size:"), m_sizeSpin);

    auto* styleRow = new QHBoxLayout;
    m_boldCheck = new QCheckBox(tr("Bold"));
    m_italicCheck = new QCheckBox(tr("Italic"));
    m_underlineCheck = new QCheckBox(tr("Underline"));
    styleRow->addWidget(m_boldCheck);
    styleRow->addWidget(m_italicCheck);
    styleRow->addWidget(m_underlineCheck);
    styleRow->addStretch(1);
    form->addRow(tr("Style:"), styleRow);

    m_colorButton = new QPushButton;
    m_colorButton->setFixedSize(64, 24);
    form->addRow(tr("Color:"), m_colorButton);

    layout->addLayout(form);

    m_preview = new QLabel(tr("AaBbCc 123"));
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setMinimumHeight(72);
    m_preview->setFrameShape(QFrame::StyledPanel);
    layout->addWidget(m_preview);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);

    connect(m_colorButton, &QPushButton::clicked, this, [this] {
        const QColor chosen = QColorDialog::getColor(m_color, this, tr("Text Color"));
        if (chosen.isValid()) {
            m_color = chosen;
            updatePreview();
        }
    });
    connect(m_fontCombo, &QFontComboBox::currentFontChanged, this,
            [this](const QFont&) { updatePreview(); });
    connect(m_sizeSpin, &QSpinBox::valueChanged, this, [this](int) { updatePreview(); });
    connect(m_boldCheck, &QCheckBox::toggled, this, [this](bool) { updatePreview(); });
    connect(m_italicCheck, &QCheckBox::toggled, this, [this](bool) { updatePreview(); });
    connect(m_underlineCheck, &QCheckBox::toggled, this, [this](bool) { updatePreview(); });
    connect(m_textEdit, &QTextEdit::textChanged, this, [this] { updatePreview(); });

    updatePreview();
}

QString TextDialog::text() const
{
    return m_textEdit->toPlainText();
}

QFont TextDialog::font() const
{
    QFont f = m_fontCombo->currentFont();
    f.setPointSize(m_sizeSpin->value());
    f.setBold(m_boldCheck->isChecked());
    f.setItalic(m_italicCheck->isChecked());
    f.setUnderline(m_underlineCheck->isChecked());
    return f;
}

void TextDialog::updatePreview()
{
    QFont f = font();
    // The preview keeps a readable size; only the layer render uses full size.
    f.setPointSize(std::min(28, m_sizeSpin->value()));
    m_preview->setFont(f);
    const QString sample = text().trimmed();
    m_preview->setText(sample.isEmpty() ? tr("AaBbCc 123") : sample);
    m_preview->setStyleSheet(QString("color: %1;").arg(m_color.name()));
    m_colorButton->setStyleSheet(
        QString("background-color: %1; border: 1px solid #444;").arg(m_color.name()));
}

} // namespace PhotoStudio::UI
