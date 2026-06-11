#pragma once

#include "core/types.hpp"

#include <QComboBox>
#include <QDialog>
#include <QSpinBox>

namespace PhotoStudio::UI {

class NewDocumentDialog : public QDialog {
    Q_OBJECT

public:
    explicit NewDocumentDialog(QWidget* parent = nullptr);

    u32 documentWidth() const;
    u32 documentHeight() const;
    u32 documentDpi() const;
    // RGBA scalar for the background layer.
    bool transparentBackground() const;

private:
    QSpinBox* m_width;
    QSpinBox* m_height;
    QSpinBox* m_dpi;
    QComboBox* m_background;
};

} // namespace PhotoStudio::UI
