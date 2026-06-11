#pragma once

#include "tools/tool_base.hpp"

#include <algorithm>

namespace PhotoStudio::Tools {

// Soft round brush with hardness/opacity/flow, painting onto the active layer.
class BrushTool : public ToolBase {
public:
    std::string name() const override { return "Brush"; }
    std::string iconName() const override { return "brush"; }
    std::vector<ToolOption> options() override;

    void onMouseDown(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseMove(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseUp(f32 x, f32 y, u32 mods, ToolContext& ctx) override;

    void setSize(f32 size) { m_size = std::clamp(size, 1.0f, 500.0f); }
    void setHardness(f32 h) { m_hardness = std::clamp(h, 0.0f, 1.0f); }
    void setOpacity(f32 o) { m_opacity = std::clamp(o, 0.0f, 1.0f); }
    f32 size() const { return m_size; }

protected:
    virtual void stamp(cv::Mat& pixels, const cv::Mat& selection, cv::Point layerOffset, f32 cx,
                       f32 cy, const ToolContext& ctx);
    void strokeTo(f32 x, f32 y, ToolContext& ctx);
    virtual std::string historyLabel() const { return "Brush Stroke"; }

    f32 m_size = 40.0f;
    f32 m_hardness = 0.7f;
    f32 m_opacity = 1.0f;
    f32 m_flow = 1.0f;

    bool m_drawing = false;
    cv::Point2f m_lastPos{0, 0};
};

// Same engine as the brush, but reduces alpha instead of depositing color.
class EraserTool final : public BrushTool {
public:
    std::string name() const override { return "Eraser"; }
    std::string iconName() const override { return "eraser"; }

protected:
    void stamp(cv::Mat& pixels, const cv::Mat& selection, cv::Point layerOffset, f32 cx, f32 cy,
               const ToolContext& ctx) override;
    std::string historyLabel() const override { return "Eraser"; }
};

// Clone stamp: Alt+click sets the source, then strokes copy pixels.
class CloneTool final : public BrushTool {
public:
    std::string name() const override { return "Clone Stamp"; }
    std::string iconName() const override { return "clone"; }

    void onMouseDown(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseUp(f32 x, f32 y, u32 mods, ToolContext& ctx) override;

protected:
    void stamp(cv::Mat& pixels, const cv::Mat& selection, cv::Point layerOffset, f32 cx, f32 cy,
               const ToolContext& ctx) override;
    std::string historyLabel() const override { return "Clone Stamp"; }

private:
    bool m_sourceSet = false;
    cv::Point2f m_source{0, 0};
    cv::Point2f m_delta{0, 0};
    cv::Mat m_sourceSnapshot;
};

} // namespace PhotoStudio::Tools
