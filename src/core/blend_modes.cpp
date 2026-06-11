#include "core/blend_modes.hpp"

#include "math/color.hpp"

#include <opencv2/core.hpp>

#include <algorithm>
#include <array>
#include <cmath>

namespace PhotoStudio::Core {

namespace {

f32 blendChannel(BlendMode mode, f32 b, f32 s)
{
    switch (mode) {
    case BlendMode::Darken:
        return std::min(b, s);
    case BlendMode::Multiply:
        return b * s;
    case BlendMode::ColorBurn:
        return s <= 0.0f ? 0.0f : 1.0f - std::min(1.0f, (1.0f - b) / s);
    case BlendMode::LinearBurn:
        return std::max(0.0f, b + s - 1.0f);
    case BlendMode::Lighten:
        return std::max(b, s);
    case BlendMode::Screen:
        return b + s - b * s;
    case BlendMode::ColorDodge:
        return s >= 1.0f ? 1.0f : std::min(1.0f, b / (1.0f - s));
    case BlendMode::LinearDodge:
        return std::min(1.0f, b + s);
    case BlendMode::Overlay:
        return b <= 0.5f ? 2.0f * b * s : 1.0f - 2.0f * (1.0f - b) * (1.0f - s);
    case BlendMode::SoftLight: {
        if (s <= 0.5f)
            return b - (1.0f - 2.0f * s) * b * (1.0f - b);
        const f32 d = b <= 0.25f ? ((16.0f * b - 12.0f) * b + 4.0f) * b : std::sqrt(b);
        return b + (2.0f * s - 1.0f) * (d - b);
    }
    case BlendMode::HardLight:
        return s <= 0.5f ? 2.0f * b * s : 1.0f - 2.0f * (1.0f - b) * (1.0f - s);
    case BlendMode::VividLight:
        return s <= 0.5f ? blendChannel(BlendMode::ColorBurn, b, 2.0f * s)
                         : blendChannel(BlendMode::ColorDodge, b, 2.0f * (s - 0.5f));
    case BlendMode::LinearLight:
        return std::clamp(b + 2.0f * s - 1.0f, 0.0f, 1.0f);
    case BlendMode::PinLight:
        return s <= 0.5f ? std::min(b, 2.0f * s) : std::max(b, 2.0f * (s - 0.5f));
    case BlendMode::Difference:
        return std::abs(b - s);
    case BlendMode::Exclusion:
        return b + s - 2.0f * b * s;
    case BlendMode::Subtract:
        return std::max(0.0f, b - s);
    case BlendMode::Divide:
        return s <= 0.0f ? 1.0f : std::min(1.0f, b / s);
    case BlendMode::Normal:
    default:
        return s;
    }
}

bool isSeparable(BlendMode mode)
{
    switch (mode) {
    case BlendMode::Hue:
    case BlendMode::Saturation:
    case BlendMode::Color:
    case BlendMode::Luminosity:
        return false;
    default:
        return true;
    }
}

} // namespace

const std::string& blendModeName(BlendMode mode)
{
    return blendModeNames().at(static_cast<size_t>(mode));
}

const std::vector<std::string>& blendModeNames()
{
    static const std::vector<std::string> names = {
        "Normal",       "Darken",     "Multiply",   "Color Burn",  "Linear Burn", "Lighten",
        "Screen",       "Color Dodge", "Linear Dodge (Add)", "Overlay", "Soft Light", "Hard Light",
        "Vivid Light",  "Linear Light", "Pin Light", "Difference",  "Exclusion",   "Subtract",
        "Divide",       "Hue",        "Saturation", "Color",       "Luminosity"};
    return names;
}

cv::Vec3f blendPixel(BlendMode mode, const cv::Vec3f& base, const cv::Vec3f& blend)
{
    if (isSeparable(mode)) {
        return {blendChannel(mode, base[0], blend[0]), blendChannel(mode, base[1], blend[1]),
                blendChannel(mode, base[2], blend[2])};
    }
    switch (mode) {
    case BlendMode::Hue:
        return Math::setLuminance(Math::setSaturation(blend, Math::saturationOf(base)),
                                  Math::luminance(base));
    case BlendMode::Saturation:
        return Math::setLuminance(Math::setSaturation(base, Math::saturationOf(blend)),
                                  Math::luminance(base));
    case BlendMode::Color:
        return Math::setLuminance(blend, Math::luminance(base));
    case BlendMode::Luminosity:
        return Math::setLuminance(base, Math::luminance(blend));
    default:
        return blend;
    }
}

void compositeOver(cv::Mat& dst, const cv::Mat& src, const cv::Mat& mask, f32 opacity, BlendMode mode,
                   cv::Point offset)
{
    CV_Assert(dst.type() == CV_8UC4 && src.type() == CV_8UC4);
    if (opacity <= 0.0f)
        return;

    const cv::Rect docRect(0, 0, dst.cols, dst.rows);
    const cv::Rect layerRect(offset.x, offset.y, src.cols, src.rows);
    const cv::Rect roi = docRect & layerRect;
    if (roi.empty())
        return;

    const bool hasMask = !mask.empty();

    cv::parallel_for_(cv::Range(roi.y, roi.y + roi.height), [&](const cv::Range& range) {
        for (int y = range.start; y < range.end; ++y) {
            auto* drow = dst.ptr<cv::Vec4b>(y);
            const auto* srow = src.ptr<cv::Vec4b>(y - offset.y);
            const u8* mrow = hasMask ? mask.ptr<u8>(y - offset.y) : nullptr;

            for (int x = roi.x; x < roi.x + roi.width; ++x) {
                const cv::Vec4b& sp = srow[x - offset.x];
                f32 as = (sp[3] / 255.0f) * opacity;
                if (mrow)
                    as *= mrow[x - offset.x] / 255.0f;
                if (as <= 0.0f)
                    continue;

                cv::Vec4b& dp = drow[x];
                const f32 ab = dp[3] / 255.0f;
                const cv::Vec3f Cb(dp[0] / 255.0f, dp[1] / 255.0f, dp[2] / 255.0f);
                const cv::Vec3f Cs(sp[0] / 255.0f, sp[1] / 255.0f, sp[2] / 255.0f);
                const cv::Vec3f B = blendPixel(mode, Cb, Cs);

                // Porter-Duff "over" with blend-mode mixing (straight alpha).
                const f32 ao = as + ab * (1.0f - as);
                cv::Vec3f Co;
                for (int i = 0; i < 3; ++i)
                    Co[i] = (Cs[i] * (1.0f - ab) + B[i] * ab) * as + Cb[i] * ab * (1.0f - as);

                const f32 inv = 1.0f / ao;
                dp[0] = Math::toU8(Co[0] * inv);
                dp[1] = Math::toU8(Co[1] * inv);
                dp[2] = Math::toU8(Co[2] * inv);
                dp[3] = Math::toU8(ao);
            }
        }
    });
}

} // namespace PhotoStudio::Core
