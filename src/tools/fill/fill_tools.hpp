#pragma once

#include "tools/tool_base.hpp"

namespace PhotoStudio::Tools {

// Paint bucket: flood-fills contiguous color with the foreground color.
class BucketFillTool final : public ToolBase {
public:
    std::string name() const override { return "Paint Bucket"; }
    std::string iconName() const override { return "fill"; }
    std::vector<ToolOption> options() override;

    void onMouseDown(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseMove(f32, f32, u32, ToolContext&) override {}
    void onMouseUp(f32, f32, u32, ToolContext&) override {}

private:
    f32 m_tolerance = 32.0f;
};

// Linear gradient from foreground to background along the dragged line.
class GradientTool final : public ToolBase {
public:
    std::string name() const override { return "Gradient"; }
    std::string iconName() const override { return "gradient"; }

    void onMouseDown(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseMove(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseUp(f32 x, f32 y, u32 mods, ToolContext& ctx) override;

    bool hasPreviewShape() const override { return m_dragging; }
    std::vector<cv::Point2f> previewPolygon() const override { return {m_start, m_current}; }

private:
    bool m_dragging = false;
    cv::Point2f m_start{0, 0};
    cv::Point2f m_current{0, 0};
};

} // namespace PhotoStudio::Tools
