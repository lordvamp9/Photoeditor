#pragma once

#include "core/document.hpp"

#include <QComboBox>
#include <QListWidget>
#include <QSlider>
#include <QWidget>

namespace PhotoStudio::UI {

// Layer stack: thumbnails, visibility, opacity, blend mode and stack actions.
class LayersPanel : public QWidget {
    Q_OBJECT

public:
    explicit LayersPanel(QWidget* parent = nullptr);

    void setDocument(Core::Document* doc);
    void refresh();

signals:
    void documentEdited(const QString& description);
    void compositeChanged();

private:
    void onItemChanged(QListWidgetItem* item);
    void onSelectionChanged();
    void onAddLayer();
    void onDeleteLayer();
    void onDuplicateLayer();
    void onMoveUp();
    void onMoveDown();
    void onMergeDown();
    void syncControlsToActiveLayer();

    Core::Document* m_doc = nullptr;
    QListWidget* m_list;
    QSlider* m_opacity;
    QComboBox* m_blendMode;
    bool m_updating = false;
};

} // namespace PhotoStudio::UI
