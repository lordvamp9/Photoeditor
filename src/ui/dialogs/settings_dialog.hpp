#pragma once

#include <QDialog>

class QComboBox;

namespace PhotoStudio::UI {

// Application preferences. Currently: UI language (persisted via QSettings,
// applied on the next launch).
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);

    // Returns "en" or "es".
    static QString savedLanguage();

private:
    void accept() override;

    QComboBox* m_languageCombo = nullptr;
    QString m_initialLanguage;
};

} // namespace PhotoStudio::UI
