#include "tools/selection/selection_tools.hpp"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cmath>

namespace PhotoStudio::Tools {

std::string SelectionTool::name() const
{
    switch (m_shape) {
    case Shape::Rectangle:
        return "Rectangular Marquee";
    case Shape::Ellipse:
        return "Elliptical Marquee";
    case Shape::Lasso:
        return "Lasso";
    case Shape::MagicWand:
        return "Magic Wand";
    }
    return "Selection";
}

std::string SelectionTool::iconName() const
{
    switch (m_shape) {
    case Shape::Rectangle:
        return "select_rect";
    case Shape::Ellipse:
        return "select_ellipse";
    case Shape::Lasso:
        return "lasso";
    case Shape::MagicWand:
        return "magic_wand";
    }
    return "select_rect";
}

std::vector<ToolOption> SelectionTool::options()
{
    std::vector<ToolOption> opts = {
        {"Feather", 0.0f, 100.0f, 1.0f, [this] { return m_feather; },
         [this](f32 v) { m_feather = v; }},
    };
    if (m_shape == Shape::MagicWand) {
        opts.push_back({"Tolerance", 1.0f, 128.0f, 1.0f, [this] { return m_tolerance; },
                        [this](f32 v) { m_tolerance = v; }});
    }
    return opts;
}

Core::SelectionOp SelectionTool::opFromMods(u32 mods)
{
    if ((mods & ModShift) && (mods & ModAlt))
        return Core::SelectionOp::Intersect;
    if (mods & ModShift)
        return Core::SelectionOp::Add;
    if (mods & ModAlt)
        return Core::SelectionOp::Subtract;
    return Core::SelectionOp::Replace;
}

void SelectionTool::onMouseDown(f32 x, f32 y, u32 mods, ToolContext& ctx)
{
    if (!ctx.document)
        return;
    if (m_shape == Shape::MagicWand) {
        magicWand(x, y, mods, ctx);
        return;
    }
    m_selecting = true;
    m_start = {x, y};
    m_current = {x, y};
    m_lassoPoints.clear();
    m_lassoPoints.push_back({x, y});
}

void SelectionTool::onMouseMove(f32 x, f32 y, u32, ToolContext& ctx)
{
    if (!m_selecting)
        return;
    m_current = {x, y};
    if (m_shape == Shape::Lasso)
        m_lassoPoints.push_back({x, y});
    if (ctx.requestRepaint)
        ctx.requestRepaint();
}

void SelectionTool::onMouseUp(f32 x, f32 y, u32 mods, ToolContext& ctx)
{
    if (!m_selecting)
        return;
    m_selecting = false;
    m_current = {x, y};
    if (m_shape == Shape::Lasso)
        m_lassoPoints.push_back({x, y});
    commit(ctx, mods);
}

std::vector<cv::Point2f> SelectionTool::previewPolygon() const
{
    if (m_shape == Shape::Lasso)
        return m_lassoPoints;

    const f32 x0 = std::min(m_start.x, m_current.x);
    const f32 y0 = std::min(m_start.y, m_current.y);
    const f32 x1 = std::max(m_start.x, m_current.x);
    const f32 y1 = std::max(m_start.y, m_current.y);

    if (m_shape == Shape::Rectangle)
        return {{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}};

    // Ellipse approximated with 48 segments.
    std::vector<cv::Point2f> pts;
    const f32 cx = (x0 + x1) / 2.0f, cy = (y0 + y1) / 2.0f;
    const f32 rx = (x1 - x0) / 2.0f, ry = (y1 - y0) / 2.0f;
    for (int i = 0; i < 48; ++i) {
        const f32 a = i / 48.0f * 2.0f * 3.14159265f;
        pts.push_back({cx + rx * std::cos(a), cy + ry * std::sin(a)});
    }
    return pts;
}

void SelectionTool::commit(ToolContext& ctx, u32 mods)
{
    Core::Document& doc = *ctx.document;
    cv::Mat mask(static_cast<int>(doc.height()), static_cast<int>(doc.width()), CV_8UC1,
                 cv::Scalar(0));

    const auto poly = previewPolygon();
    if (poly.size() < 3) {
        if (ctx.requestRepaint)
            ctx.requestRepaint();
        return;
    }
    std::vector<cv::Point> ipoly;
    ipoly.reserve(poly.size());
    for (const auto& p : poly)
        ipoly.emplace_back(static_cast<int>(p.x), static_cast<int>(p.y));
    cv::fillPoly(mask, std::vector<std::vector<cv::Point>>{ipoly}, cv::Scalar(255));

    if (m_feather > 0.0f) {
        const int k = static_cast<int>(m_feather) * 2 + 1;
        cv::GaussianBlur(mask, mask, cv::Size(k, k), m_feather / 2.0);
    }

    doc.setSelection(mask, opFromMods(mods));
    if (ctx.requestRepaint)
        ctx.requestRepaint();
}

void SelectionTool::magicWand(f32 x, f32 y, u32 mods, ToolContext& ctx)
{
    Core::Document& doc = *ctx.document;
    const int ix = static_cast<int>(x);
    const int iy = static_cast<int>(y);
    if (ix < 0 || iy < 0 || ix >= static_cast<int>(doc.width()) ||
        iy >= static_cast<int>(doc.height()))
        return;

    cv::Mat composite = doc.renderComposite();
    cv::Mat rgb;
    cv::cvtColor(composite, rgb, cv::COLOR_RGBA2RGB);

    cv::Mat ffMask(rgb.rows + 2, rgb.cols + 2, CV_8UC1, cv::Scalar(0));
    const int tol = static_cast<int>(m_tolerance);
    cv::floodFill(rgb, ffMask, cv::Point(ix, iy), cv::Scalar(), nullptr,
                  cv::Scalar(tol, tol, tol), cv::Scalar(tol, tol, tol),
                  4 | cv::FLOODFILL_MASK_ONLY | (255 << 8));

    cv::Mat mask = ffMask(cv::Rect(1, 1, rgb.cols, rgb.rows)).clone();
    if (m_feather > 0.0f) {
        const int k = static_cast<int>(m_feather) * 2 + 1;
        cv::GaussianBlur(mask, mask, cv::Size(k, k), m_feather / 2.0);
    }

    doc.setSelection(mask, opFromMods(mods));
    if (ctx.requestRepaint)
        ctx.requestRepaint();
}

} // namespace PhotoStudio::Tools
