#pragma once

#include "tools/tool_base.hpp"

namespace PhotoStudio::Tools {

// Drags the active layer by adjusting its document offset.
class MoveTool final : public ToolBase {
public:
    std::string name() const override { return "Move"; }
    std::string iconName() const override { return "move"; }

    void onMouseDown(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseMove(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseUp(f32 x, f32 y, u32 mods, ToolContext& ctx) override;

private:
    bool m_dragging = false;
    cv::Point2f m_grab{0, 0};
    cv::Point m_originalOffset{0, 0};
};

} // namespace PhotoStudio::Tools
