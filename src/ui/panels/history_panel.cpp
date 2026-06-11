#include "ui/panels/history_panel.hpp"

#include <QVBoxLayout>

namespace PhotoStudio::UI {

HistoryPanel::HistoryPanel(QWidget* parent) : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    m_list = new QListWidget();
    layout->addWidget(m_list);

    connect(m_list, &QListWidget::currentRowChanged, this, [this](int row) {
        if (m_updating || !m_doc || row < 0)
            return;
        m_doc->gotoState(static_cast<size_t>(row));
        emit stateRestored();
    });
}

void HistoryPanel::setDocument(Core::Document* doc)
{
    m_doc = doc;
    refresh();
}

void HistoryPanel::refresh()
{
    m_updating = true;
    m_list->clear();
    if (m_doc) {
        for (const auto& desc : m_doc->historyDescriptions())
            m_list->addItem(QString::fromStdString(desc));
        m_list->setCurrentRow(static_cast<int>(m_doc->historyIndex()));
    }
    m_updating = false;
}

} // namespace PhotoStudio::UI
