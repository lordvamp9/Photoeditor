#include "filters/distortion/distortion_filters.hpp"

#include <opencv2/imgproc.hpp>

#include <cmath>

namespace PhotoStudio::Filters {

namespace {

// Builds remap tables from a radial displacement function and applies them.
template <typename Fn>
void radialRemap(cv::Mat& rgba, Fn&& displace)
{
    const int w = rgba.cols, h = rgba.rows;
    const f32 cx = w / 2.0f, cy = h / 2.0f;
    const f32 maxR = std::sqrt(cx * cx + cy * cy);

    cv::Mat mapX(h, w, CV_32F), mapY(h, w, CV_32F);
    cv::parallel_for_(cv::Range(0, h), [&](const cv::Range& range) {
        for (int y = range.start; y < range.end; ++y) {
            f32* mx = mapX.ptr<f32>(y);
            f32* my = mapY.ptr<f32>(y);
            for (int x = 0; x < w; ++x) {
                const f32 dx = x - cx;
                const f32 dy = y - cy;
                const f32 r = std::sqrt(dx * dx + dy * dy);
                f32 sx = static_cast<f32>(x), sy = static_cast<f32>(y);
                displace(dx, dy, r, maxR, cx, cy, sx, sy);
                mx[x] = sx;
                my[x] = sy;
            }
        }
    });
    cv::Mat out;
    cv::remap(rgba, out, mapX, mapY, cv::INTER_LINEAR, cv::BORDER_REPLICATE);
    rgba = out;
}

} // namespace

// ----- Pinch / Punch -----

std::vector<FilterParam> PinchFilter::parameters() const
{
    return {{"amount", "Amount", -100.0f, 100.0f, 50.0f, 1.0f}};
}

void PinchFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 amount = get(params, "amount", 50.0f) / 100.0f;
    radialRemap(rgba, [amount](f32 dx, f32 dy, f32 r, f32 maxR, f32 cx, f32 cy, f32& sx, f32& sy) {
        if (r <= 0.0f || r >= maxR)
            return;
        const f32 t = r / maxR;
        const f32 factor = std::pow(t, 1.0f + amount);
        const f32 scale = factor * maxR / r;
        sx = cx + dx * scale;
        sy = cy + dy * scale;
    });
}

// ----- Swirl -----

std::vector<FilterParam> SwirlFilter::parameters() const
{
    return {{"angle", "Angle (deg)", -360.0f, 360.0f, 120.0f, 1.0f},
            {"radius", "Radius (%)", 10.0f, 150.0f, 75.0f, 1.0f}};
}

void SwirlFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 angle = get(params, "angle", 120.0f) * 3.14159265f / 180.0f;
    const f32 radiusPct = get(params, "radius", 75.0f) / 100.0f;

    radialRemap(rgba,
                [angle, radiusPct](f32 dx, f32 dy, f32 r, f32 maxR, f32 cx, f32 cy, f32& sx, f32& sy) {
                    const f32 swirlR = maxR * radiusPct;
                    if (r >= swirlR)
                        return;
                    const f32 t = 1.0f - r / swirlR;
                    const f32 theta = angle * t * t;
                    const f32 cosT = std::cos(theta), sinT = std::sin(theta);
                    sx = cx + dx * cosT - dy * sinT;
                    sy = cy + dx * sinT + dy * cosT;
                });
}

// ----- Ripple -----

std::vector<FilterParam> RippleFilter::parameters() const
{
    return {{"amplitude", "Amplitude (px)", 1.0f, 60.0f, 10.0f, 1.0f},
            {"wavelength", "Wavelength (px)", 4.0f, 200.0f, 40.0f, 1.0f}};
}

void RippleFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 amp = get(params, "amplitude", 10.0f);
    const f32 wavelength = std::max(1.0f, get(params, "wavelength", 40.0f));

    const int w = rgba.cols, h = rgba.rows;
    cv::Mat mapX(h, w, CV_32F), mapY(h, w, CV_32F);
    for (int y = 0; y < h; ++y) {
        f32* mx = mapX.ptr<f32>(y);
        f32* my = mapY.ptr<f32>(y);
        for (int x = 0; x < w; ++x) {
            mx[x] = x + amp * std::sin(2.0f * 3.14159265f * y / wavelength);
            my[x] = y + amp * std::cos(2.0f * 3.14159265f * x / wavelength);
        }
    }
    cv::Mat out;
    cv::remap(rgba, out, mapX, mapY, cv::INTER_LINEAR, cv::BORDER_REPLICATE);
    rgba = out;
}

// ----- Spherize -----

std::vector<FilterParam> SphereFilter::parameters() const
{
    return {{"amount", "Amount", -100.0f, 100.0f, 60.0f, 1.0f}};
}

void SphereFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 amount = get(params, "amount", 60.0f) / 100.0f;
    radialRemap(rgba, [amount](f32 dx, f32 dy, f32 r, f32 maxR, f32 cx, f32 cy, f32& sx, f32& sy) {
        const f32 sphereR = maxR * 0.9f;
        if (r <= 0.0f || r >= sphereR)
            return;
        const f32 t = r / sphereR;
        const f32 z = std::sqrt(1.0f - t * t);
        const f32 scale = 1.0f - amount * z * 0.5f;
        sx = cx + dx * scale;
        sy = cy + dy * scale;
    });
}

} // namespace PhotoStudio::Filters
