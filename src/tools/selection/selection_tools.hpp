#pragma once

#include "tools/tool_base.hpp"

namespace PhotoStudio::Tools {

// Rectangle / ellipse / lasso marquee and magic wand, in a single engine.
class SelectionTool final : public ToolBase {
public:
    enum class Shape : u8 { Rectangle, Ellipse, Lasso, MagicWand };

    explicit SelectionTool(Shape shape) : m_shape(shape) {}

    std::string name() const override;
    std::string iconName() const override;
    std::vector<ToolOption> options() override;

    void onMouseDown(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseMove(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseUp(f32 x, f32 y, u32 mods, ToolContext& ctx) override;

    bool hasPreviewShape() const override { return m_selecting; }
    std::vector<cv::Point2f> previewPolygon() const override;

private:
    static Core::SelectionOp opFromMods(u32 mods);
    void commit(ToolContext& ctx, u32 mods);
    void magicWand(f32 x, f32 y, u32 mods, ToolContext& ctx);

    Shape m_shape;
    bool m_selecting = false;
    cv::Point2f m_start{0, 0};
    cv::Point2f m_current{0, 0};
    std::vector<cv::Point2f> m_lassoPoints;
    f32 m_tolerance = 32.0f;
    f32 m_feather = 0.0f;
};

} // namespace PhotoStudio::Tools
