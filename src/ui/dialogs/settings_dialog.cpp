#include "ui/dialogs/settings_dialog.hpp"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QVBoxLayout>

namespace PhotoStudio::UI {

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("Preferences"));
    setMinimumWidth(360);

    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout;

    m_languageCombo = new QComboBox;
    m_languageCombo->addItem(tr("English"), "en");
    m_languageCombo->addItem(tr("Spanish"), "es");
    m_initialLanguage = savedLanguage();
    m_languageCombo->setCurrentIndex(m_initialLanguage == "es" ? 1 : 0);
    form->addRow(tr("Language:"), m_languageCombo);
    layout->addLayout(form);

    auto* note = new QLabel(tr("Language changes take effect after restarting PhotoStudio."));
    note->setWordWrap(true);
    note->setProperty("level", "3");
    layout->addWidget(note);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

QString SettingsDialog::savedLanguage()
{
    return QSettings().value("ui/language", "en").toString();
}

void SettingsDialog::accept()
{
    const QString chosen = m_languageCombo->currentData().toString();
    QSettings().setValue("ui/language", chosen);
    if (chosen != m_initialLanguage) {
        QMessageBox::information(this, tr("Restart required"),
                                 tr("Please restart PhotoStudio to apply the new language."));
    }
    QDialog::accept();
}

} // namespace PhotoStudio::UI
