#include "tools/brush/brush_tools.hpp"

#include "math/interpolation.hpp"

#include <algorithm>
#include <cmath>

namespace PhotoStudio::Tools {

// ----- BrushTool -----

std::vector<ToolOption> BrushTool::options()
{
    return {
        {"Size", 1.0f, 300.0f, 1.0f, [this] { return m_size; }, [this](f32 v) { setSize(v); }},
        {"Hardness", 0.0f, 1.0f, 0.01f, [this] { return m_hardness; },
         [this](f32 v) { setHardness(v); }},
        {"Opacity", 0.0f, 1.0f, 0.01f, [this] { return m_opacity; },
         [this](f32 v) { setOpacity(v); }},
    };
}

void BrushTool::onMouseDown(f32 x, f32 y, u32, ToolContext& ctx)
{
    auto layer = ctx.document ? ctx.document->activeLayer() : nullptr;
    if (!layer || layer->isLocked())
        return;
    m_drawing = true;
    m_lastPos = {x, y};
    strokeTo(x, y, ctx);
}

void BrushTool::onMouseMove(f32 x, f32 y, u32, ToolContext& ctx)
{
    if (!m_drawing)
        return;
    strokeTo(x, y, ctx);
}

void BrushTool::onMouseUp(f32, f32, u32, ToolContext& ctx)
{
    if (!m_drawing)
        return;
    m_drawing = false;
    if (ctx.commitHistory)
        ctx.commitHistory(historyLabel());
}

void BrushTool::strokeTo(f32 x, f32 y, ToolContext& ctx)
{
    auto layer = ctx.document->activeLayer();
    if (!layer)
        return;

    cv::Mat& pixels = layer->pixels();
    const cv::Mat& selection =
        ctx.document->hasSelection() ? ctx.document->selection() : cv::Mat();
    const cv::Point off = layer->offset();

    const f32 dx = x - m_lastPos.x;
    const f32 dy = y - m_lastPos.y;
    const f32 dist = std::sqrt(dx * dx + dy * dy);
    const f32 spacing = std::max(1.0f, m_size * 0.15f);
    const int steps = std::max(1, static_cast<int>(dist / spacing));

    for (int i = 1; i <= steps; ++i) {
        const f32 t = static_cast<f32>(i) / steps;
        stamp(pixels, selection, off, m_lastPos.x + dx * t, m_lastPos.y + dy * t, ctx);
    }
    m_lastPos = {x, y};
    ctx.document->markDirty();
    if (ctx.requestRepaint)
        ctx.requestRepaint();
}

void BrushTool::stamp(cv::Mat& pixels, const cv::Mat& selection, cv::Point layerOffset, f32 cx,
                      f32 cy, const ToolContext& ctx)
{
    const f32 lx = cx - layerOffset.x;
    const f32 ly = cy - layerOffset.y;
    const f32 r = m_size / 2.0f;

    const int x0 = std::max(0, static_cast<int>(lx - r));
    const int y0 = std::max(0, static_cast<int>(ly - r));
    const int x1 = std::min(pixels.cols - 1, static_cast<int>(lx + r));
    const int y1 = std::min(pixels.rows - 1, static_cast<int>(ly + r));

    const cv::Vec4b& color = ctx.foreground;

    for (int py = y0; py <= y1; ++py) {
        auto* row = pixels.ptr<cv::Vec4b>(py);
        for (int px = x0; px <= x1; ++px) {
            const f32 ddx = px - lx;
            const f32 ddy = py - ly;
            const f32 d = std::sqrt(ddx * ddx + ddy * ddy) / r;
            if (d > 1.0f)
                continue;

            f32 a = d <= m_hardness ? 1.0f
                                    : 1.0f - Math::smoothstep(m_hardness, 1.0f, d);
            a *= m_opacity * m_flow;

            if (!selection.empty()) {
                const int sx = px + layerOffset.x;
                const int sy = py + layerOffset.y;
                if (sx < 0 || sy < 0 || sx >= selection.cols || sy >= selection.rows)
                    continue;
                a *= selection.at<u8>(sy, sx) / 255.0f;
            }
            if (a <= 0.0f)
                continue;

            cv::Vec4b& dst = row[px];
            const f32 da = dst[3] / 255.0f;
            const f32 oa = a + da * (1.0f - a);
            if (oa <= 0.0f)
                continue;
            for (int c = 0; c < 3; ++c)
                dst[c] = cv::saturate_cast<u8>((color[c] * a + dst[c] * da * (1.0f - a)) / oa);
            dst[3] = cv::saturate_cast<u8>(oa * 255.0f);
        }
    }
}

// ----- EraserTool -----

void EraserTool::stamp(cv::Mat& pixels, const cv::Mat& selection, cv::Point layerOffset, f32 cx,
                       f32 cy, const ToolContext&)
{
    const f32 lx = cx - layerOffset.x;
    const f32 ly = cy - layerOffset.y;
    const f32 r = m_size / 2.0f;

    const int x0 = std::max(0, static_cast<int>(lx - r));
    const int y0 = std::max(0, static_cast<int>(ly - r));
    const int x1 = std::min(pixels.cols - 1, static_cast<int>(lx + r));
    const int y1 = std::min(pixels.rows - 1, static_cast<int>(ly + r));

    for (int py = y0; py <= y1; ++py) {
        auto* row = pixels.ptr<cv::Vec4b>(py);
        for (int px = x0; px <= x1; ++px) {
            const f32 ddx = px - lx;
            const f32 ddy = py - ly;
            const f32 d = std::sqrt(ddx * ddx + ddy * ddy) / r;
            if (d > 1.0f)
                continue;

            f32 a = d <= m_hardness ? 1.0f
                                    : 1.0f - Math::smoothstep(m_hardness, 1.0f, d);
            a *= m_opacity;

            if (!selection.empty()) {
                const int sx = px + layerOffset.x;
                const int sy = py + layerOffset.y;
                if (sx < 0 || sy < 0 || sx >= selection.cols || sy >= selection.rows)
                    continue;
                a *= selection.at<u8>(sy, sx) / 255.0f;
            }
            if (a <= 0.0f)
                continue;

            cv::Vec4b& dst = row[px];
            dst[3] = cv::saturate_cast<u8>(dst[3] * (1.0f - a));
        }
    }
}

// ----- CloneTool -----

void CloneTool::onMouseDown(f32 x, f32 y, u32 mods, ToolContext& ctx)
{
    if (mods & ModAlt) {
        m_source = {x, y};
        m_sourceSet = true;
        return;
    }
    if (!m_sourceSet)
        return;

    auto layer = ctx.document ? ctx.document->activeLayer() : nullptr;
    if (!layer || layer->isLocked())
        return;

    m_delta = {m_source.x - x, m_source.y - y};
    m_sourceSnapshot = layer->pixels().clone();
    m_drawing = true;
    m_lastPos = {x, y};
    strokeTo(x, y, ctx);
}

void CloneTool::onMouseUp(f32 x, f32 y, u32 mods, ToolContext& ctx)
{
    BrushTool::onMouseUp(x, y, mods, ctx);
    m_sourceSnapshot.release();
}

void CloneTool::stamp(cv::Mat& pixels, const cv::Mat& selection, cv::Point layerOffset, f32 cx,
                      f32 cy, const ToolContext&)
{
    if (m_sourceSnapshot.empty())
        return;

    const f32 lx = cx - layerOffset.x;
    const f32 ly = cy - layerOffset.y;
    const f32 r = m_size / 2.0f;

    const int x0 = std::max(0, static_cast<int>(lx - r));
    const int y0 = std::max(0, static_cast<int>(ly - r));
    const int x1 = std::min(pixels.cols - 1, static_cast<int>(lx + r));
    const int y1 = std::min(pixels.rows - 1, static_cast<int>(ly + r));

    for (int py = y0; py <= y1; ++py) {
        auto* row = pixels.ptr<cv::Vec4b>(py);
        for (int px = x0; px <= x1; ++px) {
            const f32 ddx = px - lx;
            const f32 ddy = py - ly;
            const f32 d = std::sqrt(ddx * ddx + ddy * ddy) / r;
            if (d > 1.0f)
                continue;

            const int srcX = px + static_cast<int>(m_delta.x);
            const int srcY = py + static_cast<int>(m_delta.y);
            if (srcX < 0 || srcY < 0 || srcX >= m_sourceSnapshot.cols ||
                srcY >= m_sourceSnapshot.rows)
                continue;

            f32 a = d <= m_hardness ? 1.0f
                                    : 1.0f - Math::smoothstep(m_hardness, 1.0f, d);
            a *= m_opacity;

            if (!selection.empty()) {
                const int sx = px + layerOffset.x;
                const int sy = py + layerOffset.y;
                if (sx < 0 || sy < 0 || sx >= selection.cols || sy >= selection.rows)
                    continue;
                a *= selection.at<u8>(sy, sx) / 255.0f;
            }
            if (a <= 0.0f)
                continue;

            const cv::Vec4b& src = m_sourceSnapshot.at<cv::Vec4b>(srcY, srcX);
            cv::Vec4b& dst = row[px];
            for (int c = 0; c < 4; ++c)
                dst[c] = cv::saturate_cast<u8>(src[c] * a + dst[c] * (1.0f - a));
        }
    }
}

} // namespace PhotoStudio::Tools
