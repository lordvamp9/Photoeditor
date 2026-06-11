#include "tools/transform/move_tool.hpp"

namespace PhotoStudio::Tools {

void MoveTool::onMouseDown(f32 x, f32 y, u32, ToolContext& ctx)
{
    auto layer = ctx.document ? ctx.document->activeLayer() : nullptr;
    if (!layer || layer->isLocked())
        return;
    m_dragging = true;
    m_grab = {x, y};
    m_originalOffset = layer->offset();
}

void MoveTool::onMouseMove(f32 x, f32 y, u32, ToolContext& ctx)
{
    if (!m_dragging)
        return;
    auto layer = ctx.document->activeLayer();
    if (!layer)
        return;
    layer->setOffset(m_originalOffset + cv::Point(static_cast<int>(x - m_grab.x),
                                                  static_cast<int>(y - m_grab.y)));
    ctx.document->markDirty();
    if (ctx.requestRepaint)
        ctx.requestRepaint();
}

void MoveTool::onMouseUp(f32, f32, u32, ToolContext& ctx)
{
    if (!m_dragging)
        return;
    m_dragging = false;
    auto layer = ctx.document->activeLayer();
    if (layer && layer->offset() != m_originalOffset && ctx.commitHistory)
        ctx.commitHistory("Move Layer");
}

} // namespace PhotoStudio::Tools
