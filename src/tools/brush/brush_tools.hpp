#pragma once

#include "tools/tool_base.hpp"

#include <algorithm>

namespace PhotoStudio::Tools {

// Brush tip shapes available in the brush engine.
enum class BrushTip : u8 {
    SoftRound = 0,
    HardRound,
    Airbrush,
    Calligraphy,
    Chalk,
    Scatter,
    Pencil,
    Count
};

const std::vector<std::string>& brushTipNames();

// Professional brush engine: multiple tip shapes, spacing, angle, stroke
// smoothing and stylus pressure dynamics (size/opacity), painting onto the
// active layer.
class BrushTool : public ToolBase {
public:
    std::string name() const override { return "Brush"; }
    std::string iconName() const override { return "brush"; }
    std::vector<ToolOption> options() override;

    void onMouseDown(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseMove(f32 x, f32 y, u32 mods, ToolContext& ctx) override;
    void onMouseUp(f32 x, f32 y, u32 mods, ToolContext& ctx) override;

    void setTip(BrushTip tip) { m_tip = tip; }
    void setSize(f32 size) { m_size = std::clamp(size, 1.0f, 500.0f); }
    void setHardness(f32 h) { m_hardness = std::clamp(h, 0.0f, 1.0f); }
    void setOpacity(f32 o) { m_opacity = std::clamp(o, 0.0f, 1.0f); }
    void setFlow(f32 fl) { m_flow = std::clamp(fl, 0.01f, 1.0f); }
    void setSpacing(f32 s) { m_spacing = std::clamp(s, 0.01f, 2.0f); }
    void setAngle(f32 deg) { m_angle = deg; }
    void setSmoothing(f32 s) { m_smoothing = std::clamp(s, 0.0f, 0.95f); }
    BrushTip tip() const { return m_tip; }
    f32 size() const { return m_size; }

protected:
    virtual void stamp(cv::Mat& pixels, const cv::Mat& selection, cv::Point layerOffset, f32 cx,
                       f32 cy, const ToolContext& ctx);
    void strokeTo(f32 x, f32 y, ToolContext& ctx);
    virtual std::string historyLabel() const { return "Brush Stroke"; }

    // Per-pixel tip coverage in [0,1]; d is normalized distance, (ddx, ddy)
    // the offset from the dab center in pixels, r the dab radius.
    f32 tipAlpha(f32 d, f32 ddx, f32 ddy, f32 r, int px, int py) const;

    BrushTip m_tip = BrushTip::SoftRound;
    f32 m_size = 40.0f;
    f32 m_hardness = 0.7f;
    f32 m_opacity = 1.0f;
    f32 m_flow = 1.0f;
    f32 m_spacing = 0.15f;
    f32 m_angle = 45.0f;
    f32 m_smoothing = 0.0f;
    bool m_pressureSize = true;
    bool m_pressureOpacity = false;

    // Effective per-dab values after pressure dynamics are applied.
    f32 m_stampSize = 40.0f;
    f32 m_stampOpacity = 1.0f;

    bool m_drawing = false;
    cv::Point2f m_lastPos{0, 0};
    u32 m_scatterSeed = 1;
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

// Click-to-place text: the UI installs a callback that opens the typography
// dialog and rasterizes the text into a new layer. Keeps core Qt-free.
class TextTool final : public ToolBase {
public:
    std::string name() const override { return "Text"; }
    std::string iconName() const override { return "text"; }

    void onMouseDown(f32 x, f32 y, u32, ToolContext&) override
    {
        if (onTextRequested)
            onTextRequested(x, y);
    }
    void onMouseMove(f32, f32, u32, ToolContext&) override {}
    void onMouseUp(f32, f32, u32, ToolContext&) override {}

    std::function<void(f32, f32)> onTextRequested;
};

} // namespace PhotoStudio::Tools
