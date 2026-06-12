#pragma once

#include "core/document.hpp"
#include "tools/tool_base.hpp"

#include <QActionGroup>
#include <QLabel>
#include <QMainWindow>

#include <map>
#include <memory>
#include <string>

namespace PhotoStudio::UI {

class CanvasWidget;
class LayersPanel;
class PropertiesPanel;
class HistoryPanel;
class ColorsPanel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private slots:
    void onFileNew();
    void onFileOpen();
    void onFileImportAsLayer();
    void onFileSaveProject();
    void onFileSaveProjectAs();
    void onFileExport();

    void onEditUndo();
    void onEditRedo();
    void onSelectAll();
    void onDeselect();
    void onInvertSelection();

    void onImageSize();
    void onCanvasSize();
    void onRotate(bool clockwise);
    void onFlip(bool horizontal);
    void onFlatten();

    void onAddTextLayer();
    void onFilterTriggered(const std::string& filterName);
    void onToolSelected(const std::string& toolKey);

private:
    void addTextAt(f32 x, f32 y);

private:
    void createTools();
    void createMenus();
    void createToolBar();
    void createDocks();
    void createStatusBar();

    void setDocument(Core::Document_Ptr doc);
    void refreshAll();
    void commitHistory(const QString& description);

    Core::Document_Ptr m_document;
    Tools::ToolContext m_toolContext;
    std::map<std::string, std::unique_ptr<Tools::ToolBase>> m_tools;
    std::string m_activeToolKey;

    CanvasWidget* m_canvas = nullptr;
    LayersPanel* m_layersPanel = nullptr;
    PropertiesPanel* m_propertiesPanel = nullptr;
    HistoryPanel* m_historyPanel = nullptr;
    ColorsPanel* m_colorsPanel = nullptr;

    QActionGroup* m_toolGroup = nullptr;
    QAction* m_undoAction = nullptr;
    QAction* m_redoAction = nullptr;

    QLabel* m_statusPosition = nullptr;
    QLabel* m_statusZoom = nullptr;
    QLabel* m_statusSize = nullptr;
};

} // namespace PhotoStudio::UI
