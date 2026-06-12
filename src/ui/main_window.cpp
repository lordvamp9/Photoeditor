#include "ui/main_window.hpp"

#include "filters/filter_registry.hpp"
#include "io/image_io.hpp"
#include "io/project_serializer.hpp"
#include "tools/brush/brush_tools.hpp"
#include "tools/color/color_picker_tool.hpp"
#include "tools/fill/fill_tools.hpp"
#include "tools/selection/selection_tools.hpp"
#include "tools/transform/move_tool.hpp"
#include "ui/canvas_widget.hpp"
#include "ui/dialogs/export_dialog.hpp"
#include "ui/dialogs/filter_dialog.hpp"
#include "ui/dialogs/new_document_dialog.hpp"
#include "ui/dialogs/resize_dialog.hpp"
#include "ui/dialogs/settings_dialog.hpp"
#include "ui/dialogs/text_dialog.hpp"
#include "ui/image_convert.hpp"
#include "ui/panels/colors_panel.hpp"
#include "ui/panels/history_panel.hpp"
#include "ui/panels/layers_panel.hpp"
#include "ui/panels/properties_panel.hpp"

#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QPainter>
#include <QStatusBar>
#include <QToolBar>

namespace PhotoStudio::UI {

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("PhotoStudio Premium - vamp9");
    resize(1440, 900);

    Filters::registerBuiltinFilters();

    m_canvas = new CanvasWidget(this);
    setCentralWidget(m_canvas);

    createTools();
    createDocks();
    createMenus();
    createToolBar();
    createStatusBar();

    // Shared tool context wiring.
    m_toolContext.requestRepaint = [this] { m_canvas->refresh(); };
    m_toolContext.requestRepaintRegion = [this](int x, int y, int w, int h) {
        m_canvas->refreshRegion(x, y, w, h);
    };
    m_toolContext.commitHistory = [this](const std::string& desc) {
        commitHistory(QString::fromStdString(desc));
    };
    m_toolContext.pickForeground = [this](const cv::Vec4b&) { m_colorsPanel->syncFromContext(); };
    m_canvas->setToolContext(&m_toolContext);
    m_canvas->setEraserTool(m_tools.at("eraser").get());

    onToolSelected("brush");

    // Start with a default document so the app is immediately usable.
    setDocument(std::make_unique<Core::Document>(1280, 800, 72, cv::Scalar(255, 255, 255, 255)));
}

MainWindow::~MainWindow() = default;

// ----- Setup -----

void MainWindow::createTools()
{
    using namespace Tools;
    m_tools["move"] = std::make_unique<MoveTool>();
    m_tools["select_rect"] = std::make_unique<SelectionTool>(SelectionTool::Shape::Rectangle);
    m_tools["select_ellipse"] = std::make_unique<SelectionTool>(SelectionTool::Shape::Ellipse);
    m_tools["lasso"] = std::make_unique<SelectionTool>(SelectionTool::Shape::Lasso);
    m_tools["magic_wand"] = std::make_unique<SelectionTool>(SelectionTool::Shape::MagicWand);
    m_tools["brush"] = std::make_unique<BrushTool>();
    m_tools["eraser"] = std::make_unique<EraserTool>();
    m_tools["clone"] = std::make_unique<CloneTool>();
    m_tools["fill"] = std::make_unique<BucketFillTool>();
    m_tools["gradient"] = std::make_unique<GradientTool>();
    m_tools["picker"] = std::make_unique<ColorPickerTool>();

    auto textTool = std::make_unique<TextTool>();
    textTool->onTextRequested = [this](f32 x, f32 y) { addTextAt(x, y); };
    m_tools["text"] = std::move(textTool);
}

void MainWindow::createMenus()
{
    // ----- File -----
    QMenu* file = menuBar()->addMenu(tr("&File"));
    file->addAction(tr("&New..."), QKeySequence::New, this, &MainWindow::onFileNew);
    file->addAction(tr("&Open..."), QKeySequence::Open, this, &MainWindow::onFileOpen);
    file->addAction(tr("Import as &Layer..."), this, &MainWindow::onFileImportAsLayer);
    file->addSeparator();
    file->addAction(tr("&Save Project"), QKeySequence::Save, this, &MainWindow::onFileSaveProject);
    file->addAction(tr("Save Project &As..."), QKeySequence::SaveAs, this,
                    &MainWindow::onFileSaveProjectAs);
    file->addAction(tr("&Export As..."), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E), this,
                    &MainWindow::onFileExport);
    file->addSeparator();
    file->addAction(tr("E&xit"), QKeySequence::Quit, qApp, &QApplication::quit);

    // ----- Edit -----
    QMenu* edit = menuBar()->addMenu(tr("&Edit"));
    m_undoAction = edit->addAction(tr("&Undo"), QKeySequence::Undo, this, &MainWindow::onEditUndo);
    m_redoAction = edit->addAction(tr("&Redo"), QKeySequence::Redo, this, &MainWindow::onEditRedo);
    edit->addSeparator();
    edit->addAction(tr("Select &All"), QKeySequence::SelectAll, this, &MainWindow::onSelectAll);
    edit->addAction(tr("&Deselect"), QKeySequence(Qt::CTRL | Qt::Key_D), this,
                    &MainWindow::onDeselect);
    edit->addAction(tr("&Invert Selection"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_I), this,
                    &MainWindow::onInvertSelection);
    edit->addSeparator();
    edit->addAction(tr("&Preferences..."), QKeySequence::Preferences, this, [this] {
        SettingsDialog dialog(this);
        dialog.exec();
    });

    // ----- Image -----
    QMenu* image = menuBar()->addMenu(tr("&Image"));
    image->addAction(tr("Image &Size..."), this, &MainWindow::onImageSize);
    image->addAction(tr("&Canvas Size..."), this, &MainWindow::onCanvasSize);
    image->addSeparator();
    image->addAction(tr("Rotate 90 CW"), this, [this] { onRotate(true); });
    image->addAction(tr("Rotate 90 CCW"), this, [this] { onRotate(false); });
    image->addAction(tr("Flip Horizontal"), this, [this] { onFlip(true); });
    image->addAction(tr("Flip Vertical"), this, [this] { onFlip(false); });
    image->addSeparator();
    image->addAction(tr("&Flatten Image"), this, &MainWindow::onFlatten);

    // ----- Layer -----
    QMenu* layer = menuBar()->addMenu(tr("&Layer"));
    layer->addAction(tr("Add &Text Layer..."), this, &MainWindow::onAddTextLayer);

    // ----- Filter (generated from the registry) -----
    QMenu* filterMenu = menuBar()->addMenu(tr("Filte&r"));
    for (const auto& [category, names] : Filters::FilterRegistry::instance().byCategory()) {
        QMenu* sub = filterMenu->addMenu(QCoreApplication::translate("Core", category.c_str()));
        for (const auto& name : names) {
            sub->addAction(QCoreApplication::translate("Core", name.c_str()) + "...", this,
                           [this, name] { onFilterTriggered(name); });
        }
    }

    // ----- View -----
    QMenu* view = menuBar()->addMenu(tr("&View"));
    view->addAction(tr("Zoom &In"), QKeySequence::ZoomIn, this,
                    [this] { m_canvas->setZoom(m_canvas->zoom() * 1.25f); });
    view->addAction(tr("Zoom &Out"), QKeySequence::ZoomOut, this,
                    [this] { m_canvas->setZoom(m_canvas->zoom() / 1.25f); });
    view->addAction(tr("&Fit on Screen"), QKeySequence(Qt::CTRL | Qt::Key_0), this,
                    [this] { m_canvas->zoomFit(); });
    view->addAction(tr("&Actual Pixels"), QKeySequence(Qt::CTRL | Qt::Key_1), this,
                    [this] { m_canvas->zoomActual(); });

    // ----- Help -----
    QMenu* help = menuBar()->addMenu(tr("&Help"));
    help->addAction(tr("&About PhotoStudio"), this, [this] {
        QMessageBox::about(this, tr("About PhotoStudio"),
                           tr("<b>PhotoStudio Premium 1.0</b><br>"
                              "Professional photo editor.<br><br>"
                              "Published under the <b>vamp9</b> brand.<br>"
                              "Non-commercial license - see LICENSE."));
    });
}

void MainWindow::createToolBar()
{
    auto* toolbar = addToolBar(tr("Tools"));
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));
    m_toolGroup = new QActionGroup(this);
    m_toolGroup->setExclusive(true);

    const std::vector<std::string> order = {"move",   "select_rect", "select_ellipse", "lasso",
                                            "magic_wand", "brush",   "eraser",         "clone",
                                            "fill",   "gradient",    "text",           "picker"};
    for (const auto& key : order) {
        auto& tool = m_tools.at(key);
        const QString iconPath =
            QString(":/icons/tools/%1.svg").arg(QString::fromStdString(tool->iconName()));
        const QString toolName = QCoreApplication::translate("Core", tool->name().c_str());
        auto* action = toolbar->addAction(QIcon(iconPath), toolName);
        action->setCheckable(true);
        action->setToolTip(toolName);
        m_toolGroup->addAction(action);
        connect(action, &QAction::triggered, this, [this, key] { onToolSelected(key); });
        if (key == "brush")
            action->setChecked(true);
    }
}

void MainWindow::createDocks()
{
    m_layersPanel = new LayersPanel(this);
    auto* layersDock = new QDockWidget(tr("Layers"), this);
    layersDock->setWidget(m_layersPanel);
    addDockWidget(Qt::RightDockWidgetArea, layersDock);

    m_historyPanel = new HistoryPanel(this);
    auto* historyDock = new QDockWidget(tr("History"), this);
    historyDock->setWidget(m_historyPanel);
    addDockWidget(Qt::RightDockWidgetArea, historyDock);
    tabifyDockWidget(layersDock, historyDock);
    layersDock->raise();

    m_propertiesPanel = new PropertiesPanel(this);
    auto* propsDock = new QDockWidget(tr("Tool Options"), this);
    propsDock->setWidget(m_propertiesPanel);
    addDockWidget(Qt::LeftDockWidgetArea, propsDock);

    m_colorsPanel = new ColorsPanel(&m_toolContext, this);
    auto* colorsDock = new QDockWidget(tr("Color"), this);
    colorsDock->setWidget(m_colorsPanel);
    addDockWidget(Qt::LeftDockWidgetArea, colorsDock);

    connect(m_layersPanel, &LayersPanel::compositeChanged, m_canvas, &CanvasWidget::refresh);
    connect(m_layersPanel, &LayersPanel::documentEdited, this,
            [this](const QString& desc) { commitHistory(desc); });
    connect(m_historyPanel, &HistoryPanel::stateRestored, this, [this] {
        m_canvas->refresh();
        m_layersPanel->refresh();
    });
}

void MainWindow::createStatusBar()
{
    m_statusPosition = new QLabel("0, 0");
    m_statusZoom = new QLabel("100%");
    m_statusSize = new QLabel("-");
    statusBar()->addWidget(m_statusPosition);
    statusBar()->addPermanentWidget(m_statusSize);
    statusBar()->addPermanentWidget(m_statusZoom);

    connect(m_canvas, &CanvasWidget::cursorMoved, this, [this](int x, int y) {
        m_statusPosition->setText(QString("%1, %2").arg(x).arg(y));
    });
    connect(m_canvas, &CanvasWidget::zoomChanged, this, [this](f32 zoom) {
        m_statusZoom->setText(QString::number(static_cast<int>(zoom * 100)) + "%");
    });
}

// ----- Document plumbing -----

void MainWindow::setDocument(Core::Document_Ptr doc)
{
    m_document = std::move(doc);
    m_toolContext.document = m_document.get();
    m_canvas->setDocument(m_document.get());
    m_layersPanel->setDocument(m_document.get());
    m_historyPanel->setDocument(m_document.get());
    if (m_document) {
        m_statusSize->setText(
            QString("%1 x %2 px").arg(m_document->width()).arg(m_document->height()));
        setWindowTitle(QString("PhotoStudio Premium - %1 - vamp9")
                           .arg(QString::fromStdString(m_document->name())));
    }
    refreshAll();
}

void MainWindow::refreshAll()
{
    m_canvas->refresh();
    m_layersPanel->refresh();
    m_historyPanel->refresh();
    if (m_document) {
        m_undoAction->setEnabled(m_document->canUndo());
        m_redoAction->setEnabled(m_document->canRedo());
        m_statusSize->setText(
            QString("%1 x %2 px").arg(m_document->width()).arg(m_document->height()));
    }
}

void MainWindow::commitHistory(const QString& description)
{
    if (!m_document)
        return;
    m_document->addState(description.toStdString());
    m_historyPanel->refresh();
    m_layersPanel->refresh();
    m_undoAction->setEnabled(m_document->canUndo());
    m_redoAction->setEnabled(m_document->canRedo());
}

// ----- File actions -----

void MainWindow::onFileNew()
{
    NewDocumentDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    const cv::Scalar bg = dialog.transparentBackground() ? cv::Scalar(0, 0, 0, 0)
                                                         : cv::Scalar(255, 255, 255, 255);
    setDocument(std::make_unique<Core::Document>(dialog.documentWidth(), dialog.documentHeight(),
                                                 dialog.documentDpi(), bg));
}

void MainWindow::onFileOpen()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Open"), QString(),
        tr("All Supported (*.psproj *.png *.jpg *.jpeg *.webp *.tif *.tiff *.bmp);;"
           "PhotoStudio Project (*.psproj);;Images (*.png *.jpg *.jpeg *.webp *.tif *.tiff *.bmp)"));
    if (path.isEmpty())
        return;

    Core::Document_Ptr doc;
    if (path.endsWith(".psproj", Qt::CaseInsensitive))
        doc = IO::loadProject(path.toStdString());
    else
        doc = IO::importDocument(path.toStdString());

    if (!doc) {
        QMessageBox::warning(this, tr("Open"), tr("Could not open file:\n%1").arg(path));
        return;
    }
    setDocument(std::move(doc));
}

void MainWindow::onFileImportAsLayer()
{
    if (!m_document)
        return;
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Import as Layer"), QString(),
        tr("Images (*.png *.jpg *.jpeg *.webp *.tif *.tiff *.bmp)"));
    if (path.isEmpty())
        return;
    if (IO::importAsLayer(*m_document, path.toStdString())) {
        commitHistory(tr("Import Layer"));
        refreshAll();
    }
}

void MainWindow::onFileSaveProject()
{
    if (!m_document)
        return;
    if (m_document->filePath().empty()) {
        onFileSaveProjectAs();
        return;
    }
    IO::saveProject(*m_document, m_document->filePath());
    statusBar()->showMessage(tr("Project saved"), 2000);
}

void MainWindow::onFileSaveProjectAs()
{
    if (!m_document)
        return;
    QString path = QFileDialog::getSaveFileName(this, tr("Save Project As"), QString(),
                                                tr("PhotoStudio Project (*.psproj)"));
    if (path.isEmpty())
        return;
    if (!path.endsWith(".psproj", Qt::CaseInsensitive))
        path += ".psproj";
    if (IO::saveProject(*m_document, path.toStdString())) {
        m_document->setFilePath(path.toStdString());
        statusBar()->showMessage(tr("Project saved"), 2000);
    }
}

void MainWindow::onFileExport()
{
    if (!m_document)
        return;
    ExportDialog dialog(this);
    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString ext = dialog.formatExtension();
    QString path = QFileDialog::getSaveFileName(this, tr("Export As"), QString(),
                                                QString("*%1").arg(ext));
    if (path.isEmpty())
        return;
    if (!path.endsWith(ext, Qt::CaseInsensitive))
        path += ext;

    if (IO::exportDocument(*m_document, path.toStdString(), dialog.quality()))
        statusBar()->showMessage(tr("Exported %1").arg(path), 3000);
    else
        QMessageBox::warning(this, tr("Export"), tr("Export failed."));
}

// ----- Edit actions -----

void MainWindow::onEditUndo()
{
    if (!m_document || !m_document->canUndo())
        return;
    m_document->undo();
    refreshAll();
}

void MainWindow::onEditRedo()
{
    if (!m_document || !m_document->canRedo())
        return;
    m_document->redo();
    refreshAll();
}

void MainWindow::onSelectAll()
{
    if (!m_document)
        return;
    m_document->selectAll();
    m_canvas->refresh();
}

void MainWindow::onDeselect()
{
    if (!m_document)
        return;
    m_document->deselect();
    m_canvas->refresh();
}

void MainWindow::onInvertSelection()
{
    if (!m_document)
        return;
    m_document->invertSelection();
    m_canvas->refresh();
}

// ----- Image actions -----

void MainWindow::onImageSize()
{
    if (!m_document)
        return;
    ResizeDialog dialog(m_document->width(), m_document->height(), tr("Image Size"), this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    m_document->scaleImage(dialog.newWidth(), dialog.newHeight());
    commitHistory(tr("Image Size"));
    refreshAll();
    m_canvas->zoomFit();
}

void MainWindow::onCanvasSize()
{
    if (!m_document)
        return;
    ResizeDialog dialog(m_document->width(), m_document->height(), tr("Canvas Size"), this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    m_document->resizeCanvas(dialog.newWidth(), dialog.newHeight());
    commitHistory(tr("Canvas Size"));
    refreshAll();
    m_canvas->zoomFit();
}

void MainWindow::onRotate(bool clockwise)
{
    if (!m_document)
        return;
    m_document->rotate90(clockwise);
    commitHistory(tr("Rotate 90"));
    refreshAll();
    m_canvas->zoomFit();
}

void MainWindow::onFlip(bool horizontal)
{
    if (!m_document)
        return;
    m_document->flip(horizontal);
    commitHistory(horizontal ? tr("Flip Horizontal") : tr("Flip Vertical"));
    refreshAll();
}

void MainWindow::onFlatten()
{
    if (!m_document)
        return;
    m_document->flatten();
    commitHistory(tr("Flatten Image"));
    refreshAll();
}

// ----- Layer / filter / tool actions -----

void MainWindow::onAddTextLayer()
{
    if (!m_document)
        return;
    // Menu entry: place the text centered on the canvas.
    addTextAt(-1.0f, -1.0f);
}

void MainWindow::addTextAt(f32 x, f32 y)
{
    if (!m_document)
        return;

    const QColor initial(m_toolContext.foreground[0], m_toolContext.foreground[1],
                         m_toolContext.foreground[2]);
    TextDialog dialog(initial, this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    const QString text = dialog.text();
    if (text.trimmed().isEmpty())
        return;

    const int docW = static_cast<int>(m_document->width());
    const int docH = static_cast<int>(m_document->height());

    QImage image(docW, docH, QImage::Format_RGBA8888);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setFont(dialog.font());
    painter.setPen(dialog.color());
    if (x < 0.0f || y < 0.0f) {
        painter.drawText(image.rect(), Qt::AlignCenter, text);
    } else {
        // Anchor the text's top-left at the clicked document position.
        painter.drawText(QRectF(x, y, docW - x, docH - y),
                         Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap, text);
    }
    painter.end();

    auto layer = std::make_shared<Core::Layer>(m_document->width(), m_document->height(),
                                               Core::LayerType::Text);
    layer->pixels() = qimageToMat(image);
    layer->setName(text.left(32).toStdString());
    m_document->addLayer(layer);
    commitHistory(tr("Text Layer"));
    refreshAll();
}

void MainWindow::onFilterTriggered(const std::string& filterName)
{
    if (!m_document)
        return;
    auto* filter = Filters::FilterRegistry::instance().find(filterName);
    if (!filter)
        return;

    FilterDialog dialog(*filter, *m_document, [this] { m_canvas->refresh(); }, this);
    if (dialog.exec() == QDialog::Accepted) {
        commitHistory(QString::fromStdString(filterName));
        refreshAll();
    } else {
        m_canvas->refresh();
    }
}

void MainWindow::onToolSelected(const std::string& toolKey)
{
    const auto it = m_tools.find(toolKey);
    if (it == m_tools.end())
        return;
    m_activeToolKey = toolKey;
    m_canvas->setTool(it->second.get());
    m_propertiesPanel->setTool(it->second.get());
}

} // namespace PhotoStudio::UI
