#pragma once

#include "tools/tool_base.hpp"

namespace PhotoStudio::Tools {

// Samples the composited color under the cursor into the foreground swatch.
class ColorPickerTool final : public ToolBase {
public:
    std::string name() const override { return "Color Picker"; }
    std::string iconName() const override { return "picker"; }

    void onMouseDown(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseMove(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseUp(f32, f32, u32, ToolContext&) override { m_picking = false; }

private:
    void sample(f32 x, f32 y, ToolContext& ctx);
    bool m_picking = false;
};

} // namespace PhotoStudio::Tools
