#pragma once

#include "core/document.hpp"
#include "tools/tool_base.hpp"

#include <QPolygonF>
#include <QTimer>
#include <QVector>
#include <QWidget>

namespace PhotoStudio::UI {

// Document viewport: paints the composited image with zoom/pan, draws the
// selection marching-ants overlay and routes mouse input to the active tool.
class CanvasWidget : public QWidget {
    Q_OBJECT

public:
    explicit CanvasWidget(QWidget* parent = nullptr);

    void setDocument(Core::Document* doc);
    void setTool(Tools::ToolBase* tool) { m_tool = tool; }
    void setToolContext(Tools::ToolContext* ctx) { m_ctx = ctx; }
    // Tool used when the stylus eraser tip touches the tablet.
    void setEraserTool(Tools::ToolBase* tool) { m_eraserTool = tool; }

    // Re-converts and repaints only a document-space region (fast path used
    // by paint tools during a stroke).
    void refreshRegion(int x, int y, int w, int h);

    f32 zoom() const { return m_zoom; }
    void setZoom(f32 zoom);
    void zoomFit();
    void zoomActual();

public slots:
    void refresh();

signals:
    void cursorMoved(int docX, int docY);
    void zoomChanged(f32 zoom);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void tabletEvent(QTabletEvent* event) override;

private:
    QPointF docToWidget(QPointF docPos) const;
    QPointF widgetToDoc(QPointF widgetPos) const;
    static u32 toModifiers(Qt::KeyboardModifiers mods);
    void rebuildSelectionOutline();
    void centerDocument();

    Tools::ToolBase* activeTool() const;

    Core::Document* m_doc = nullptr;
    Tools::ToolBase* m_tool = nullptr;
    Tools::ToolBase* m_eraserTool = nullptr;
    Tools::ToolContext* m_ctx = nullptr;
    bool m_tabletDown = false;
    bool m_tabletEraser = false;

    QImage m_composite;
    QPixmap m_checkerboard;

    f32 m_zoom = 1.0f;
    QPointF m_pan{0, 0};
    bool m_panning = false;
    bool m_spaceDown = false;
    QPoint m_lastMouse;

    QVector<QPolygonF> m_selectionOutline;
    QTimer m_antsTimer;
    int m_antsPhase = 0;
};

} // namespace PhotoStudio::UI
