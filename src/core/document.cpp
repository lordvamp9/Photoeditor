#include "core/document.hpp"

#include <opencv2/imgproc.hpp>

#include <algorithm>

namespace PhotoStudio::Core {

Document::Document(u32 width, u32 height, u32 dpi, const cv::Scalar& background)
    : m_width(width), m_height(height), m_dpi(dpi)
{
    auto bg = std::make_shared<Layer>(width, height, LayerType::Raster, background);
    bg->setName("Background");
    m_layers.push_back(bg);
    m_activeLayerId = bg->id();
    m_history.push_back(captureSnapshot("New Document"));
}

// ----- Layers -----

void Document::addLayer(Layer_Ptr layer, size_t index)
{
    if (index >= m_layers.size())
        m_layers.push_back(layer);
    else
        m_layers.insert(m_layers.begin() + static_cast<ptrdiff_t>(index), layer);
    m_activeLayerId = layer->id();
    markDirty();
}

void Document::removeLayer(u32 layerId)
{
    if (m_layers.size() <= 1)
        return;
    const size_t idx = layerIndex(layerId);
    if (idx == SIZE_MAX)
        return;
    m_layers.erase(m_layers.begin() + static_cast<ptrdiff_t>(idx));
    if (m_activeLayerId == layerId)
        m_activeLayerId = m_layers[std::min(idx, m_layers.size() - 1)]->id();
    markDirty();
}

void Document::moveLayer(u32 layerId, size_t newIndex)
{
    const size_t idx = layerIndex(layerId);
    if (idx == SIZE_MAX || newIndex >= m_layers.size())
        return;
    Layer_Ptr layer = m_layers[idx];
    m_layers.erase(m_layers.begin() + static_cast<ptrdiff_t>(idx));
    m_layers.insert(m_layers.begin() + static_cast<ptrdiff_t>(newIndex), layer);
    markDirty();
}

Layer_Ptr Document::duplicateLayer(u32 layerId)
{
    const size_t idx = layerIndex(layerId);
    if (idx == SIZE_MAX)
        return nullptr;
    auto copy = std::make_shared<Layer>(m_layers[idx]->width(), m_layers[idx]->height());
    auto cloned = m_layers[idx]->clone();
    copy->pixels() = cloned->pixels();
    copy->setName(m_layers[idx]->name() + " copy");
    copy->setOffset(m_layers[idx]->offset());
    copy->setOpacity(m_layers[idx]->opacity());
    copy->setBlendMode(m_layers[idx]->blendMode());
    if (m_layers[idx]->hasMask())
        copy->addMask(m_layers[idx]->mask());
    addLayer(copy, idx + 1);
    return copy;
}

bool Document::mergeDown(u32 layerId)
{
    const size_t idx = layerIndex(layerId);
    if (idx == SIZE_MAX || idx == 0)
        return false;

    Layer_Ptr top = m_layers[idx];
    Layer_Ptr below = m_layers[idx - 1];

    auto merged = std::make_shared<Layer>(m_width, m_height);
    merged->setName(below->name());
    cv::Mat& canvas = merged->pixels();
    canvas.setTo(cv::Scalar(0, 0, 0, 0));
    if (below->isVisible())
        compositeOver(canvas, below->pixels(), below->mask(), below->opacity(), below->blendMode(),
                      below->offset());
    if (top->isVisible())
        compositeOver(canvas, top->pixels(), top->mask(), top->opacity(), top->blendMode(),
                      top->offset());

    m_layers[idx - 1] = merged;
    m_layers.erase(m_layers.begin() + static_cast<ptrdiff_t>(idx));
    m_activeLayerId = merged->id();
    markDirty();
    return true;
}

void Document::flatten()
{
    cv::Mat composite = renderComposite();
    auto flat = std::make_shared<Layer>(m_width, m_height);
    flat->setName("Background");
    flat->pixels() = composite.clone();
    m_layers.clear();
    m_layers.push_back(flat);
    m_activeLayerId = flat->id();
    markDirty();
}

Layer_Ptr Document::layerById(u32 layerId) const
{
    for (const auto& layer : m_layers)
        if (layer->id() == layerId)
            return layer;
    return nullptr;
}

Layer_Ptr Document::activeLayer() const
{
    return layerById(m_activeLayerId);
}

size_t Document::layerIndex(u32 layerId) const
{
    for (size_t i = 0; i < m_layers.size(); ++i)
        if (m_layers[i]->id() == layerId)
            return i;
    return SIZE_MAX;
}

// ----- Selection -----

void Document::setSelection(const cv::Mat& mask, SelectionOp op)
{
    CV_Assert(mask.empty() || mask.type() == CV_8UC1);
    if (mask.empty()) {
        deselect();
        return;
    }
    if (m_selection.empty() || op == SelectionOp::Replace) {
        m_selection = mask.clone();
        return;
    }
    switch (op) {
    case SelectionOp::Add:
        cv::max(m_selection, mask, m_selection);
        break;
    case SelectionOp::Subtract:
        cv::subtract(m_selection, mask, m_selection);
        break;
    case SelectionOp::Intersect:
        cv::min(m_selection, mask, m_selection);
        break;
    default:
        m_selection = mask.clone();
        break;
    }
    if (cv::countNonZero(m_selection) == 0)
        deselect();
}

void Document::selectAll()
{
    m_selection = cv::Mat(static_cast<int>(m_height), static_cast<int>(m_width), CV_8UC1,
                          cv::Scalar(255));
}

void Document::deselect()
{
    m_selection.release();
}

void Document::invertSelection()
{
    if (m_selection.empty())
        return;
    cv::bitwise_not(m_selection, m_selection);
    if (cv::countNonZero(m_selection) == 0)
        deselect();
}

// ----- Compositing -----

void Document::markDirtyRect(int x, int y, int w, int h)
{
    if (m_needsRedraw)
        return; // The whole cache is already invalid.
    const cv::Rect docRect(0, 0, static_cast<int>(m_width), static_cast<int>(m_height));
    const cv::Rect rect = cv::Rect(x, y, std::max(0, w), std::max(0, h)) & docRect;
    if (rect.empty())
        return;
    m_dirtyRect = m_dirtyRect.empty() ? rect : (m_dirtyRect | rect);
}

cv::Mat Document::renderComposite() const
{
    if (!m_needsRedraw && !m_compositeCache.empty()) {
        if (m_dirtyRect.empty())
            return m_compositeCache;

        // Incremental path: recomposite only the dirty region into the cache.
        // This keeps brush strokes O(stroke area) instead of O(document area).
        const cv::Rect roi = m_dirtyRect;
        cv::Mat target = m_compositeCache(roi);
        target.setTo(cv::Scalar(0, 0, 0, 0));
        for (const auto& layer : m_layers) {
            if (!layer->isVisible() || layer->opacity() <= 0.0f)
                continue;
            const cv::Point off = layer->offset() - roi.tl();
            cv::Mat targetView = target; // compositeOver clips against this view
            compositeOver(targetView, layer->pixels(), layer->mask(), layer->opacity(),
                          layer->blendMode(), off);
        }
        m_dirtyRect = cv::Rect();
        return m_compositeCache;
    }

    cv::Mat out(static_cast<int>(m_height), static_cast<int>(m_width), CV_8UC4,
                cv::Scalar(0, 0, 0, 0));
    for (const auto& layer : m_layers) {
        if (!layer->isVisible() || layer->opacity() <= 0.0f)
            continue;
        compositeOver(out, layer->pixels(), layer->mask(), layer->opacity(), layer->blendMode(),
                      layer->offset());
    }
    m_compositeCache = out;
    m_needsRedraw = false;
    m_dirtyRect = cv::Rect();
    return out;
}

cv::Mat Document::renderPreview(u32 maxSize) const
{
    cv::Mat composite = renderComposite();
    const f32 scale = std::min(1.0f, static_cast<f32>(maxSize) /
                                          static_cast<f32>(std::max(m_width, m_height)));
    if (scale >= 1.0f)
        return composite.clone();
    cv::Mat preview;
    cv::resize(composite, preview, cv::Size(), scale, scale, cv::INTER_AREA);
    return preview;
}

// ----- Whole-image operations -----

void Document::resizeCanvas(u32 newWidth, u32 newHeight)
{
    const i32 dx = (static_cast<i32>(newWidth) - static_cast<i32>(m_width)) / 2;
    const i32 dy = (static_cast<i32>(newHeight) - static_cast<i32>(m_height)) / 2;
    m_width = newWidth;
    m_height = newHeight;
    for (auto& layer : m_layers)
        layer->setOffset(layer->offset() + cv::Point(dx, dy));
    deselect();
    markDirty();
}

void Document::scaleImage(u32 newWidth, u32 newHeight)
{
    const f64 sx = static_cast<f64>(newWidth) / m_width;
    const f64 sy = static_cast<f64>(newHeight) / m_height;
    for (auto& layer : m_layers) {
        cv::Mat scaled;
        cv::resize(layer->pixels(), scaled,
                   cv::Size(std::max(1, static_cast<int>(std::lround(layer->width() * sx))),
                            std::max(1, static_cast<int>(std::lround(layer->height() * sy)))),
                   0, 0, cv::INTER_LANCZOS4);
        layer->pixels() = scaled;
        if (layer->hasMask()) {
            cv::Mat mask;
            cv::resize(layer->mask(), mask, scaled.size(), 0, 0, cv::INTER_LINEAR);
            layer->mask() = mask;
        }
        layer->setOffset({static_cast<int>(std::lround(layer->offset().x * sx)),
                          static_cast<int>(std::lround(layer->offset().y * sy))});
    }
    m_width = newWidth;
    m_height = newHeight;
    deselect();
    markDirty();
}

void Document::rotate90(bool clockwise)
{
    const auto code = clockwise ? cv::ROTATE_90_CLOCKWISE : cv::ROTATE_90_COUNTERCLOCKWISE;
    for (auto& layer : m_layers) {
        cv::Mat rotated;
        cv::rotate(layer->pixels(), rotated, code);
        layer->pixels() = rotated;
        if (layer->hasMask()) {
            cv::Mat mask;
            cv::rotate(layer->mask(), mask, code);
            layer->mask() = mask;
        }
        layer->setOffset({0, 0});
    }
    std::swap(m_width, m_height);
    deselect();
    markDirty();
}

void Document::flip(bool horizontal)
{
    const int code = horizontal ? 1 : 0;
    for (auto& layer : m_layers) {
        cv::flip(layer->pixels(), layer->pixels(), code);
        if (layer->hasMask())
            cv::flip(layer->mask(), layer->mask(), code);
    }
    deselect();
    markDirty();
}

// ----- History -----

Document::Snapshot Document::captureSnapshot(const std::string& description) const
{
    Snapshot snap;
    snap.description = description;
    snap.layers.reserve(m_layers.size());
    for (const auto& layer : m_layers)
        snap.layers.push_back(layer->clone());
    snap.activeLayerId = m_activeLayerId;
    snap.selection = m_selection.empty() ? cv::Mat() : m_selection.clone();
    snap.width = m_width;
    snap.height = m_height;
    return snap;
}

void Document::restoreSnapshot(const Snapshot& snapshot)
{
    m_layers.clear();
    m_layers.reserve(snapshot.layers.size());
    for (const auto& layer : snapshot.layers)
        m_layers.push_back(layer->clone());
    m_activeLayerId = snapshot.activeLayerId;
    m_selection = snapshot.selection.empty() ? cv::Mat() : snapshot.selection.clone();
    m_width = snapshot.width;
    m_height = snapshot.height;
    markDirty();
}

void Document::addState(const std::string& description)
{
    m_history.resize(m_historyIndex + 1);
    m_history.push_back(captureSnapshot(description));
    if (m_history.size() > kMaxHistory)
        m_history.erase(m_history.begin());
    m_historyIndex = m_history.size() - 1;
}

void Document::undo()
{
    if (!canUndo())
        return;
    --m_historyIndex;
    restoreSnapshot(m_history[m_historyIndex]);
}

void Document::redo()
{
    if (!canRedo())
        return;
    ++m_historyIndex;
    restoreSnapshot(m_history[m_historyIndex]);
}

void Document::gotoState(size_t index)
{
    if (index >= m_history.size() || index == m_historyIndex)
        return;
    m_historyIndex = index;
    restoreSnapshot(m_history[m_historyIndex]);
}

std::vector<std::string> Document::historyDescriptions() const
{
    std::vector<std::string> out;
    out.reserve(m_history.size());
    for (const auto& snap : m_history)
        out.push_back(snap.description);
    return out;
}

} // namespace PhotoStudio::Core
