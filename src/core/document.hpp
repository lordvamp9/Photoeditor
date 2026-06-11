#pragma once

#include "core/layer.hpp"
#include "core/types.hpp"

#include <opencv2/core.hpp>

#include <string>
#include <vector>

namespace PhotoStudio::Core {

enum class SelectionOp : u8 {
    Replace = 0,
    Add,
    Subtract,
    Intersect,
};

class Document {
public:
    Document(u32 width, u32 height, u32 dpi = 72, const cv::Scalar& background = {255, 255, 255, 255});

    u32 width() const { return m_width; }
    u32 height() const { return m_height; }
    u32 dpi() const { return m_dpi; }
    void setDpi(u32 dpi) { m_dpi = dpi; }

    const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    const std::string& filePath() const { return m_filePath; }
    void setFilePath(const std::string& path) { m_filePath = path; }

    // ----- Layers (bottom-first order) -----
    const std::vector<Layer_Ptr>& layers() const { return m_layers; }
    void addLayer(Layer_Ptr layer, size_t index = SIZE_MAX);
    void removeLayer(u32 layerId);
    void moveLayer(u32 layerId, size_t newIndex);
    Layer_Ptr duplicateLayer(u32 layerId);
    bool mergeDown(u32 layerId);
    void flatten();

    Layer_Ptr layerById(u32 layerId) const;
    Layer_Ptr activeLayer() const;
    u32 activeLayerId() const { return m_activeLayerId; }
    void setActiveLayer(u32 layerId) { m_activeLayerId = layerId; }
    size_t layerIndex(u32 layerId) const;

    // ----- Selection (document-sized 8-bit mask; empty = no selection) -----
    bool hasSelection() const { return !m_selection.empty(); }
    const cv::Mat& selection() const { return m_selection; }
    void setSelection(const cv::Mat& mask, SelectionOp op = SelectionOp::Replace);
    void selectAll();
    void deselect();
    void invertSelection();

    // ----- Compositing -----
    cv::Mat renderComposite() const;
    cv::Mat renderPreview(u32 maxSize = 512) const;
    void markDirty() { m_needsRedraw = true; }

    // ----- Whole-image operations -----
    void resizeCanvas(u32 newWidth, u32 newHeight);
    void scaleImage(u32 newWidth, u32 newHeight);
    void rotate90(bool clockwise);
    void flip(bool horizontal);

    // ----- History -----
    void addState(const std::string& description);
    void undo();
    void redo();
    bool canUndo() const { return m_historyIndex > 0; }
    bool canRedo() const { return m_historyIndex + 1 < m_history.size(); }
    void gotoState(size_t index);
    size_t historyIndex() const { return m_historyIndex; }
    std::vector<std::string> historyDescriptions() const;

private:
    struct Snapshot {
        std::string description;
        std::vector<Layer_Ptr> layers;
        u32 activeLayerId = 0;
        cv::Mat selection;
        u32 width = 0, height = 0;
    };

    Snapshot captureSnapshot(const std::string& description) const;
    void restoreSnapshot(const Snapshot& snapshot);

    u32 m_width;
    u32 m_height;
    u32 m_dpi;
    std::string m_name = "Untitled";
    std::string m_filePath;

    std::vector<Layer_Ptr> m_layers;
    u32 m_activeLayerId = 0;
    cv::Mat m_selection;

    std::vector<Snapshot> m_history;
    size_t m_historyIndex = 0;
    static constexpr size_t kMaxHistory = 48;

    mutable cv::Mat m_compositeCache;
    mutable bool m_needsRedraw = true;
};

} // namespace PhotoStudio::Core
