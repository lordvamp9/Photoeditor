#include "ui/canvas_widget.hpp"

#include "ui/image_convert.hpp"

#include <opencv2/imgproc.hpp>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPointingDevice>
#include <QTabletEvent>
#include <QWheelEvent>

#include <algorithm>
#include <cstring>

namespace PhotoStudio::UI {

CanvasWidget::CanvasWidget(QWidget* parent) : QWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAutoFillBackground(false);

    // Checkerboard pattern for transparency.
    m_checkerboard = QPixmap(16, 16);
    m_checkerboard.fill(QColor(42, 48, 58));
    QPainter cp(&m_checkerboard);
    cp.fillRect(0, 0, 8, 8, QColor(58, 66, 78));
    cp.fillRect(8, 8, 8, 8, QColor(58, 66, 78));
    cp.end();

    m_antsTimer.setInterval(120);
    connect(&m_antsTimer, &QTimer::timeout, this, [this] {
        m_antsPhase = (m_antsPhase + 1) % 8;
        if (!m_selectionOutline.isEmpty())
            update();
    });
    m_antsTimer.start();
}

void CanvasWidget::setDocument(Core::Document* doc)
{
    m_doc = doc;
    refresh();
    zoomFit();
}

void CanvasWidget::refresh()
{
    if (m_doc) {
        m_composite = matToQImage(m_doc->renderComposite());
        rebuildSelectionOutline();
    } else {
        m_composite = QImage();
        m_selectionOutline.clear();
    }
    update();
}

void CanvasWidget::refreshRegion(int x, int y, int w, int h)
{
    if (!m_doc || m_composite.isNull()) {
        refresh();
        return;
    }

    const cv::Mat composite = m_doc->renderComposite(); // incremental recomposite
    const QRect bounds(0, 0, m_composite.width(), m_composite.height());
    const QRect roi = QRect(x, y, w, h).intersected(bounds);
    if (roi.isEmpty())
        return;

    // Copy only the changed scanline spans into the cached QImage.
    const size_t span = static_cast<size_t>(roi.width()) * 4;
    for (int row = roi.top(); row <= roi.bottom(); ++row) {
        const auto* src = composite.ptr<u8>(row) + static_cast<size_t>(roi.left()) * 4;
        u8* dst = m_composite.scanLine(row) + static_cast<size_t>(roi.left()) * 4;
        std::memcpy(dst, src, span);
    }

    // Repaint just the affected widget area (1px guard for zoom rounding).
    const QPointF tl = docToWidget(roi.topLeft());
    const QPointF br = docToWidget(roi.bottomRight() + QPoint(1, 1));
    update(QRectF(tl, br).toAlignedRect().adjusted(-2, -2, 2, 2));
}

void CanvasWidget::setZoom(f32 zoom)
{
    m_zoom = std::clamp(zoom, 0.05f, 32.0f);
    emit zoomChanged(m_zoom);
    update();
}

void CanvasWidget::zoomFit()
{
    if (!m_doc || width() <= 0 || height() <= 0)
        return;
    const f32 margin = 48.0f;
    const f32 sx = (width() - margin) / static_cast<f32>(m_doc->width());
    const f32 sy = (height() - margin) / static_cast<f32>(m_doc->height());
    setZoom(std::min({sx, sy, 1.0f}));
    centerDocument();
}

void CanvasWidget::zoomActual()
{
    setZoom(1.0f);
    centerDocument();
}

void CanvasWidget::centerDocument()
{
    if (!m_doc)
        return;
    m_pan = {(width() - m_doc->width() * m_zoom) / 2.0,
             (height() - m_doc->height() * m_zoom) / 2.0};
    update();
}

QPointF CanvasWidget::docToWidget(QPointF docPos) const
{
    return {docPos.x() * m_zoom + m_pan.x(), docPos.y() * m_zoom + m_pan.y()};
}

QPointF CanvasWidget::widgetToDoc(QPointF widgetPos) const
{
    return {(widgetPos.x() - m_pan.x()) / m_zoom, (widgetPos.y() - m_pan.y()) / m_zoom};
}

u32 CanvasWidget::toModifiers(Qt::KeyboardModifiers mods)
{
    u32 out = Tools::ModNone;
    if (mods & Qt::ShiftModifier)
        out |= Tools::ModShift;
    if (mods & Qt::ControlModifier)
        out |= Tools::ModCtrl;
    if (mods & Qt::AltModifier)
        out |= Tools::ModAlt;
    return out;
}

void CanvasWidget::rebuildSelectionOutline()
{
    m_selectionOutline.clear();
    if (!m_doc || !m_doc->hasSelection())
        return;

    cv::Mat binary;
    cv::threshold(m_doc->selection(), binary, 127, 255, cv::THRESH_BINARY);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    for (const auto& contour : contours) {
        QPolygonF poly;
        poly.reserve(static_cast<int>(contour.size()));
        for (const auto& p : contour)
            poly << QPointF(p.x, p.y);
        m_selectionOutline << poly;
    }
}

void CanvasWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(15, 20, 25));

    if (!m_doc || m_composite.isNull()) {
        painter.setPen(QColor(113, 117, 128));
        painter.drawText(rect(), Qt::AlignCenter,
                         tr("No document open\nFile - New  or  File - Open"));
        return;
    }

    const QRectF docRect(m_pan, QSizeF(m_doc->width() * m_zoom, m_doc->height() * m_zoom));

    // Subtle glow border around the canvas.
    painter.setPen(QPen(QColor(0, 184, 255, 60), 1));
    painter.drawRect(docRect.adjusted(-1, -1, 1, 1));

    painter.save();
    painter.setClipRect(docRect);
    painter.drawTiledPixmap(docRect, m_checkerboard);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, m_zoom < 2.0f);
    painter.drawImage(docRect, m_composite);
    painter.restore();

    // Marching ants.
    if (!m_selectionOutline.isEmpty()) {
        painter.setRenderHint(QPainter::Antialiasing, false);
        for (const auto& poly : m_selectionOutline) {
            QPolygonF mapped;
            mapped.reserve(poly.size());
            for (const auto& p : poly)
                mapped << docToWidget(p);

            QPen black(Qt::black, 1);
            painter.setPen(black);
            painter.setBrush(Qt::NoBrush);
            painter.drawPolygon(mapped);

            QPen white(Qt::white, 1, Qt::CustomDashLine);
            white.setDashPattern({4.0, 4.0});
            white.setDashOffset(m_antsPhase);
            painter.setPen(white);
            painter.drawPolygon(mapped);
        }
    }

    // Tool preview (marquee rectangle, lasso path, gradient line...).
    if (m_tool && m_tool->hasPreviewShape()) {
        const auto poly = m_tool->previewPolygon();
        if (poly.size() >= 2) {
            QPolygonF mapped;
            for (const auto& p : poly)
                mapped << docToWidget(QPointF(p.x, p.y));
            QPen pen(QColor(0, 184, 255), 1, Qt::DashLine);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);
            if (poly.size() == 2)
                painter.drawLine(mapped[0], mapped[1]);
            else
                painter.drawPolygon(mapped);
        }
    }
}

Tools::ToolBase* CanvasWidget::activeTool() const
{
    // While the stylus eraser tip is in contact, route input to the eraser.
    if (m_tabletEraser && m_eraserTool)
        return m_eraserTool;
    return m_tool;
}

void CanvasWidget::mousePressEvent(QMouseEvent* event)
{
    setFocus();
    if (event->button() == Qt::MiddleButton || (m_spaceDown && event->button() == Qt::LeftButton)) {
        m_panning = true;
        m_lastMouse = event->pos();
        setCursor(Qt::ClosedHandCursor);
        return;
    }
    if (event->button() == Qt::LeftButton && m_tool && m_ctx && m_doc) {
        if (m_ctx)
            m_ctx->pressure = 1.0f;
        const QPointF doc = widgetToDoc(event->position());
        m_tool->onMouseDown(static_cast<f32>(doc.x()), static_cast<f32>(doc.y()),
                            toModifiers(event->modifiers()), *m_ctx);
        update();
    }
}

void CanvasWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_panning) {
        m_pan += event->pos() - m_lastMouse;
        m_lastMouse = event->pos();
        update();
        return;
    }

    const QPointF doc = widgetToDoc(event->position());
    emit cursorMoved(static_cast<int>(doc.x()), static_cast<int>(doc.y()));

    if ((event->buttons() & Qt::LeftButton) && m_tool && m_ctx && m_doc) {
        if (m_ctx)
            m_ctx->pressure = 1.0f;
        m_tool->onMouseMove(static_cast<f32>(doc.x()), static_cast<f32>(doc.y()),
                            toModifiers(event->modifiers()), *m_ctx);
        update();
    }
}

void CanvasWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_panning && (event->button() == Qt::MiddleButton || event->button() == Qt::LeftButton)) {
        m_panning = false;
        setCursor(m_spaceDown ? Qt::OpenHandCursor : Qt::ArrowCursor);
        return;
    }
    if (event->button() == Qt::LeftButton && m_tool && m_ctx && m_doc) {
        const QPointF doc = widgetToDoc(event->position());
        m_tool->onMouseUp(static_cast<f32>(doc.x()), static_cast<f32>(doc.y()),
                          toModifiers(event->modifiers()), *m_ctx);
        refresh();
    }
}

void CanvasWidget::tabletEvent(QTabletEvent* event)
{
    // Accepting tablet events stops Qt from synthesizing duplicate mouse
    // events, so pen input flows through here exclusively.
    event->accept();
    if (!m_tool || !m_ctx || !m_doc)
        return;

    const QPointF doc = widgetToDoc(event->position());
    const u32 mods = toModifiers(event->modifiers());

    switch (event->type()) {
    case QEvent::TabletPress:
        setFocus();
        // The barrel button (mapped to middle/right by most drivers) pans.
        if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) {
            m_panning = true;
            m_lastMouse = event->position().toPoint();
            setCursor(Qt::ClosedHandCursor);
            return;
        }
        if (event->button() != Qt::LeftButton)
            return;
        m_tabletEraser = event->pointingDevice() &&
                         event->pointingDevice()->pointerType() == QPointingDevice::PointerType::Eraser;
        m_ctx->pressure = static_cast<f32>(event->pressure());
        m_tabletDown = true;
        activeTool()->onMouseDown(static_cast<f32>(doc.x()), static_cast<f32>(doc.y()), mods,
                                  *m_ctx);
        update();
        break;

    case QEvent::TabletMove:
        if (m_panning) {
            m_pan += event->position().toPoint() - m_lastMouse;
            m_lastMouse = event->position().toPoint();
            update();
            return;
        }
        emit cursorMoved(static_cast<int>(doc.x()), static_cast<int>(doc.y()));
        if (!m_tabletDown)
            return;
        m_ctx->pressure = static_cast<f32>(event->pressure());
        activeTool()->onMouseMove(static_cast<f32>(doc.x()), static_cast<f32>(doc.y()), mods,
                                  *m_ctx);
        break;

    case QEvent::TabletRelease:
        if (m_panning) {
            m_panning = false;
            setCursor(m_spaceDown ? Qt::OpenHandCursor : Qt::ArrowCursor);
            return;
        }
        if (!m_tabletDown)
            return;
        activeTool()->onMouseUp(static_cast<f32>(doc.x()), static_cast<f32>(doc.y()), mods,
                                *m_ctx);
        m_tabletDown = false;
        m_tabletEraser = false;
        m_ctx->pressure = 1.0f;
        refresh();
        break;

    default:
        break;
    }
}

void CanvasWidget::wheelEvent(QWheelEvent* event)
{
    if (!m_doc)
        return;
    const f32 factor = event->angleDelta().y() > 0 ? 1.15f : 1.0f / 1.15f;
    const QPointF anchor = widgetToDoc(event->position());
    setZoom(m_zoom * factor);
    // Keep the point under the cursor stationary.
    const QPointF after = docToWidget(anchor);
    m_pan += event->position() - after;
    update();
}

void CanvasWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_spaceDown = true;
        setCursor(Qt::OpenHandCursor);
        return;
    }
    QWidget::keyPressEvent(event);
}

void CanvasWidget::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        m_spaceDown = false;
        setCursor(Qt::ArrowCursor);
        return;
    }
    QWidget::keyReleaseEvent(event);
}

} // namespace PhotoStudio::UI
