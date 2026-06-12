#pragma once

#include <QColor>
#include <QDialog>
#include <QFont>

class QCheckBox;
class QFontComboBox;
class QLabel;
class QPushButton;
class QSpinBox;
class QTextEdit;

namespace PhotoStudio::UI {

// Typography dialog for the Text tool: content, font family, size, weight,
// style and fill color, with a live preview.
class TextDialog : public QDialog {
    Q_OBJECT

public:
    explicit TextDialog(const QColor& initialColor, QWidget* parent = nullptr);

    QString text() const;
    QFont font() const;
    QColor color() const { return m_color; }

private:
    void updatePreview();

    QTextEdit* m_textEdit = nullptr;
    QFontComboBox* m_fontCombo = nullptr;
    QSpinBox* m_sizeSpin = nullptr;
    QCheckBox* m_boldCheck = nullptr;
    QCheckBox* m_italicCheck = nullptr;
    QCheckBox* m_underlineCheck = nullptr;
    QPushButton* m_colorButton = nullptr;
    QLabel* m_preview = nullptr;
    QColor m_color;
};

} // namespace PhotoStudio::UI
