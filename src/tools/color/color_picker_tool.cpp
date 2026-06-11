#include "tools/color/color_picker_tool.hpp"

namespace PhotoStudio::Tools {

void ColorPickerTool::onMouseDown(f32 x, f32 y, u32, ToolContext& ctx)
{
    m_picking = true;
    sample(x, y, ctx);
}

void ColorPickerTool::onMouseMove(f32 x, f32 y, u32, ToolContext& ctx)
{
    if (m_picking)
        sample(x, y, ctx);
}

void ColorPickerTool::sample(f32 x, f32 y, ToolContext& ctx)
{
    if (!ctx.document)
        return;
    const int ix = static_cast<int>(x);
    const int iy = static_cast<int>(y);
    if (ix < 0 || iy < 0 || ix >= static_cast<int>(ctx.document->width()) ||
        iy >= static_cast<int>(ctx.document->height()))
        return;

    const cv::Mat composite = ctx.document->renderComposite();
    const cv::Vec4b px = composite.at<cv::Vec4b>(iy, ix);
    ctx.foreground = {px[0], px[1], px[2], 255};
    if (ctx.pickForeground)
        ctx.pickForeground(ctx.foreground);
}

} // namespace PhotoStudio::Tools
