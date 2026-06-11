#pragma once

#include "core/types.hpp"

#include <QCheckBox>
#include <QDialog>
#include <QSpinBox>

namespace PhotoStudio::UI {

// Shared dialog for "Image Size" (scale pixels) and "Canvas Size" (crop/extend).
class ResizeDialog : public QDialog {
    Q_OBJECT

public:
    ResizeDialog(u32 currentWidth, u32 currentHeight, const QString& title,
                 QWidget* parent = nullptr);

    u32 newWidth() const;
    u32 newHeight() const;

private:
    QSpinBox* m_width;
    QSpinBox* m_height;
    QCheckBox* m_keepRatio;
    f64 m_ratio;
    bool m_updating = false;
};

} // namespace PhotoStudio::UI
