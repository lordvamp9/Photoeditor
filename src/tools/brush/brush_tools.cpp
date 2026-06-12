#include "tools/brush/brush_tools.hpp"

#include "math/interpolation.hpp"

#include <algorithm>
#include <cmath>

namespace PhotoStudio::Tools {

namespace {

constexpr f32 kPi = 3.14159265358979f;

// Deterministic per-pixel hash in [0,1) used for chalk grain.
inline f32 pixelNoise(int x, int y)
{
    u32 h = static_cast<u32>(x) * 374761393u + static_cast<u32>(y) * 668265263u;
    h = (h ^ (h >> 13)) * 1274126177u;
    return static_cast<f32>(h & 0xFFFFFF) / static_cast<f32>(0x1000000);
}

// Cheap xorshift for scatter jitter.
inline u32 nextRand(u32& state)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

inline f32 randSigned(u32& state)
{
    return (static_cast<f32>(nextRand(state) & 0xFFFF) / 32768.0f) - 1.0f;
}

} // namespace

const std::vector<std::string>& brushTipNames()
{
    static const std::vector<std::string> names = {
        "Soft Round", "Hard Round", "Airbrush", "Calligraphy", "Chalk", "Scatter", "Pencil"};
    return names;
}

// ----- BrushTool -----

std::vector<ToolOption> BrushTool::options()
{
    std::vector<ToolOption> opts;
    opts.push_back(ToolOption::choice(
        "Brush", brushTipNames(), [this] { return static_cast<f32>(m_tip); },
        [this](f32 v) { setTip(static_cast<BrushTip>(static_cast<int>(v))); }));
    opts.push_back({"Size", 1.0f, 300.0f, 1.0f, [this] { return m_size; },
                    [this](f32 v) { setSize(v); }});
    opts.push_back({"Hardness", 0.0f, 1.0f, 0.01f, [this] { return m_hardness; },
                    [this](f32 v) { setHardness(v); }});
    opts.push_back({"Opacity", 0.0f, 1.0f, 0.01f, [this] { return m_opacity; },
                    [this](f32 v) { setOpacity(v); }});
    opts.push_back({"Flow", 0.01f, 1.0f, 0.01f, [this] { return m_flow; },
                    [this](f32 v) { setFlow(v); }});
    opts.push_back({"Spacing", 0.01f, 1.0f, 0.01f, [this] { return m_spacing; },
                    [this](f32 v) { setSpacing(v); }});
    opts.push_back({"Angle", 0.0f, 180.0f, 1.0f, [this] { return m_angle; },
                    [this](f32 v) { setAngle(v); }});
    opts.push_back({"Smoothing", 0.0f, 0.95f, 0.01f, [this] { return m_smoothing; },
                    [this](f32 v) { setSmoothing(v); }});
    opts.push_back(ToolOption::toggle(
        "Pressure controls size", [this] { return m_pressureSize ? 1.0f : 0.0f; },
        [this](f32 v) { m_pressureSize = v >= 0.5f; }));
    opts.push_back(ToolOption::toggle(
        "Pressure controls opacity", [this] { return m_pressureOpacity ? 1.0f : 0.0f; },
        [this](f32 v) { m_pressureOpacity = v >= 0.5f; }));
    return opts;
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
    // Stroke stabilization: ease toward the cursor instead of jumping.
    if (m_smoothing > 0.0f) {
        const f32 t = 1.0f - m_smoothing;
        x = m_lastPos.x + (x - m_lastPos.x) * t;
        y = m_lastPos.y + (y - m_lastPos.y) * t;
    }
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

    // Pressure dynamics (pressure is 1.0 for mouse input).
    const f32 p = std::clamp(ctx.pressure, 0.0f, 1.0f);
    m_stampSize = std::max(1.0f, m_size * (m_pressureSize ? p : 1.0f));
    m_stampOpacity = m_opacity * (m_pressureOpacity ? p : 1.0f);

    cv::Mat& pixels = layer->pixels();
    const cv::Mat& selection =
        ctx.document->hasSelection() ? ctx.document->selection() : cv::Mat();
    const cv::Point off = layer->offset();

    const f32 dx = x - m_lastPos.x;
    const f32 dy = y - m_lastPos.y;
    const f32 dist = std::sqrt(dx * dx + dy * dy);
    const f32 spacing = std::max(1.0f, m_stampSize * m_spacing);
    const int steps = std::max(1, static_cast<int>(dist / spacing));

    for (int i = 1; i <= steps; ++i) {
        const f32 t = static_cast<f32>(i) / steps;
        stamp(pixels, selection, off, m_lastPos.x + dx * t, m_lastPos.y + dy * t, ctx);
    }

    // Dirty region: stroke segment expanded by the dab radius (scatter dabs
    // can land up to one radius outside the segment).
    const f32 reach = m_stampSize * (m_tip == BrushTip::Scatter ? 2.0f : 1.0f) / 2.0f + 2.0f;
    const int rx = static_cast<int>(std::floor(std::min(m_lastPos.x, x) - reach));
    const int ry = static_cast<int>(std::floor(std::min(m_lastPos.y, y) - reach));
    const int rw = static_cast<int>(std::ceil(std::abs(dx) + 2.0f * reach)) + 1;
    const int rh = static_cast<int>(std::ceil(std::abs(dy) + 2.0f * reach)) + 1;

    m_lastPos = {x, y};
    ctx.document->markDirtyRect(rx, ry, rw, rh);
    if (ctx.requestRepaintRegion)
        ctx.requestRepaintRegion(rx, ry, rw, rh);
    else if (ctx.requestRepaint)
        ctx.requestRepaint();
}

f32 BrushTool::tipAlpha(f32 d, f32 ddx, f32 ddy, f32 r, int px, int py) const
{
    switch (m_tip) {
    case BrushTip::HardRound:
        // Hard edge with ~1px antialiasing.
        return std::clamp((1.0f - d) * r, 0.0f, 1.0f);
    case BrushTip::Airbrush:
        // Wide gaussian-like falloff; relies on flow buildup.
        return d >= 1.0f ? 0.0f : std::exp(-d * d * 4.0f) * 0.35f;
    case BrushTip::Calligraphy: {
        // Flat elliptical nib rotated by the brush angle.
        const f32 rad = m_angle * kPi / 180.0f;
        const f32 ca = std::cos(rad), sa = std::sin(rad);
        const f32 u = (ddx * ca + ddy * sa) / r;
        const f32 v = (-ddx * sa + ddy * ca) / (r * 0.25f);
        const f32 de = std::sqrt(u * u + v * v);
        return std::clamp((1.0f - de) * r * 0.5f, 0.0f, 1.0f);
    }
    case BrushTip::Chalk: {
        if (d >= 1.0f)
            return 0.0f;
        const f32 grain = pixelNoise(px, py);
        const f32 body = 1.0f - Math::smoothstep(0.4f, 1.0f, d);
        return grain < 0.55f ? body * (0.4f + 0.6f * grain) : body * 0.08f;
    }
    case BrushTip::Pencil:
        // Crisp, fully opaque core like a 4B pencil.
        return d <= 0.85f ? 1.0f : std::clamp((1.0f - d) * r, 0.0f, 1.0f);
    case BrushTip::SoftRound:
    case BrushTip::Scatter:
    default:
        return d <= m_hardness ? 1.0f : 1.0f - Math::smoothstep(m_hardness, 1.0f, d);
    }
}

void BrushTool::stamp(cv::Mat& pixels, const cv::Mat& selection, cv::Point layerOffset, f32 cx,
                      f32 cy, const ToolContext& ctx)
{
    // Scatter sprays several small dabs jittered around the cursor.
    int dabs = 1;
    f32 radius = m_stampSize / 2.0f;
    f32 jitter = 0.0f;
    if (m_tip == BrushTip::Scatter) {
        dabs = 5;
        jitter = radius;
        radius = std::max(1.0f, radius * 0.3f);
    }

    const cv::Vec4b& color = ctx.foreground;

    for (int dab = 0; dab < dabs; ++dab) {
        f32 dabX = cx, dabY = cy;
        if (jitter > 0.0f) {
            dabX += randSigned(m_scatterSeed) * jitter;
            dabY += randSigned(m_scatterSeed) * jitter;
        }

        const f32 lx = dabX - layerOffset.x;
        const f32 ly = dabY - layerOffset.y;
        const f32 r = radius;

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

                f32 a = tipAlpha(d, ddx, ddy, r, px, py);
                a *= m_stampOpacity * m_flow;

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
                    dst[c] =
                        cv::saturate_cast<u8>((color[c] * a + dst[c] * da * (1.0f - a)) / oa);
                dst[3] = cv::saturate_cast<u8>(oa * 255.0f);
            }
        }
    }
}

// ----- EraserTool -----

void EraserTool::stamp(cv::Mat& pixels, const cv::Mat& selection, cv::Point layerOffset, f32 cx,
                       f32 cy, const ToolContext&)
{
    const f32 lx = cx - layerOffset.x;
    const f32 ly = cy - layerOffset.y;
    const f32 r = m_stampSize / 2.0f;

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

            f32 a = tipAlpha(d, ddx, ddy, r, px, py);
            a *= m_stampOpacity;

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
    const f32 r = m_stampSize / 2.0f;

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

            f32 a = tipAlpha(d, ddx, ddy, r, px, py);
            a *= m_stampOpacity;

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
