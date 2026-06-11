#pragma once

#include <QComboBox>
#include <QDialog>
#include <QSlider>

namespace PhotoStudio::UI {

class ExportDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExportDialog(QWidget* parent = nullptr);

    QString formatExtension() const; // ".png", ".jpg", ".webp", ".tiff", ".bmp"
    int quality() const;

private:
    QComboBox* m_format;
    QSlider* m_quality;
};

} // namespace PhotoStudio::UI
