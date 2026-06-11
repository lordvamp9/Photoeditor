#pragma once

#include "core/document.hpp"

#include <QListWidget>
#include <QWidget>

namespace PhotoStudio::UI {

// Edit-history list; clicking a state jumps back/forward to it.
class HistoryPanel : public QWidget {
    Q_OBJECT

public:
    explicit HistoryPanel(QWidget* parent = nullptr);

    void setDocument(Core::Document* doc);
    void refresh();

signals:
    void stateRestored();

private:
    Core::Document* m_doc = nullptr;
    QListWidget* m_list;
    bool m_updating = false;
};

} // namespace PhotoStudio::UI
