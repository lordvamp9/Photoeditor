#include "tools/fill/fill_tools.hpp"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cmath>

namespace PhotoStudio::Tools {

// ----- BucketFillTool -----

std::vector<ToolOption> BucketFillTool::options()
{
    return {{"Tolerance", 0.0f, 128.0f, 1.0f, [this] { return m_tolerance; },
             [this](f32 v) { m_tolerance = v; }}};
}

void BucketFillTool::onMouseDown(f32 x, f32 y, u32, ToolContext& ctx)
{
    auto layer = ctx.document ? ctx.document->activeLayer() : nullptr;
    if (!layer || layer->isLocked())
        return;

    cv::Mat& pixels = layer->pixels();
    const int lx = static_cast<int>(x) - layer->offset().x;
    const int ly = static_cast<int>(y) - layer->offset().y;
    if (lx < 0 || ly < 0 || lx >= pixels.cols || ly >= pixels.rows)
        return;

    cv::Mat rgb;
    cv::cvtColor(pixels, rgb, cv::COLOR_RGBA2RGB);
    cv::Mat ffMask(rgb.rows + 2, rgb.cols + 2, CV_8UC1, cv::Scalar(0));
    const int tol = static_cast<int>(m_tolerance);
    cv::floodFill(rgb, ffMask, cv::Point(lx, ly), cv::Scalar(), nullptr,
                  cv::Scalar(tol, tol, tol), cv::Scalar(tol, tol, tol),
                  4 | cv::FLOODFILL_MASK_ONLY | (255 << 8));
    cv::Mat fillMask = ffMask(cv::Rect(1, 1, rgb.cols, rgb.rows));

    const cv::Vec4b color = ctx.foreground;
    const bool hasSel = ctx.document->hasSelection();
    const cv::Mat& sel = ctx.document->selection();
    const cv::Point off = layer->offset();

    for (int py = 0; py < pixels.rows; ++py) {
        auto* row = pixels.ptr<cv::Vec4b>(py);
        const u8* fm = fillMask.ptr<u8>(py);
        for (int px = 0; px < pixels.cols; ++px) {
            if (!fm[px])
                continue;
            f32 a = 1.0f;
            if (hasSel) {
                const int sx = px + off.x;
                const int sy = py + off.y;
                if (sx < 0 || sy < 0 || sx >= sel.cols || sy >= sel.rows)
                    continue;
                a = sel.at<u8>(sy, sx) / 255.0f;
                if (a <= 0.0f)
                    continue;
            }
            cv::Vec4b& dst = row[px];
            for (int c = 0; c < 4; ++c)
                dst[c] = cv::saturate_cast<u8>(color[c] * a + dst[c] * (1.0f - a));
        }
    }

    ctx.document->markDirty();
    if (ctx.requestRepaint)
        ctx.requestRepaint();
    if (ctx.commitHistory)
        ctx.commitHistory("Paint Bucket");
}

// ----- GradientTool -----

void GradientTool::onMouseDown(f32 x, f32 y, u32, ToolContext& ctx)
{
    auto layer = ctx.document ? ctx.document->activeLayer() : nullptr;
    if (!layer || layer->isLocked())
        return;
    m_dragging = true;
    m_start = {x, y};
    m_current = {x, y};
}

void GradientTool::onMouseMove(f32 x, f32 y, u32, ToolContext& ctx)
{
    if (!m_dragging)
        return;
    m_current = {x, y};
    if (ctx.requestRepaint)
        ctx.requestRepaint();
}

void GradientTool::onMouseUp(f32 x, f32 y, u32, ToolContext& ctx)
{
    if (!m_dragging)
        return;
    m_dragging = false;
    m_current = {x, y};

    auto layer = ctx.document->activeLayer();
    if (!layer)
        return;

    cv::Mat& pixels = layer->pixels();
    const cv::Point off = layer->offset();
    const f32 dx = m_current.x - m_start.x;
    const f32 dy = m_current.y - m_start.y;
    const f32 len2 = dx * dx + dy * dy;
    if (len2 < 1.0f)
        return;

    const cv::Vec4b fg = ctx.foreground;
    const cv::Vec4b bg = ctx.background;
    const bool hasSel = ctx.document->hasSelection();
    const cv::Mat& sel = ctx.document->selection();

    cv::parallel_for_(cv::Range(0, pixels.rows), [&](const cv::Range& range) {
        for (int py = range.start; py < range.end; ++py) {
            auto* row = pixels.ptr<cv::Vec4b>(py);
            for (int px = 0; px < pixels.cols; ++px) {
                const f32 gx = px + off.x;
                const f32 gy = py + off.y;
                f32 a = 1.0f;
                if (hasSel) {
                    const int sx = static_cast<int>(gx);
                    const int sy = static_cast<int>(gy);
                    if (sx < 0 || sy < 0 || sx >= sel.cols || sy >= sel.rows)
                        continue;
                    a = sel.at<u8>(sy, sx) / 255.0f;
                    if (a <= 0.0f)
                        continue;
                }
                const f32 t = std::clamp(
                    ((gx - m_start.x) * dx + (gy - m_start.y) * dy) / len2, 0.0f, 1.0f);
                cv::Vec4b grad;
                for (int c = 0; c < 4; ++c)
                    grad[c] = cv::saturate_cast<u8>(fg[c] + (bg[c] - fg[c]) * t);
                cv::Vec4b& dst = row[px];
                for (int c = 0; c < 4; ++c)
                    dst[c] = cv::saturate_cast<u8>(grad[c] * a + dst[c] * (1.0f - a));
            }
        }
    });

    ctx.document->markDirty();
    if (ctx.requestRepaint)
        ctx.requestRepaint();
    if (ctx.commitHistory)
        ctx.commitHistory("Gradient");
}

} // namespace PhotoStudio::Tools
