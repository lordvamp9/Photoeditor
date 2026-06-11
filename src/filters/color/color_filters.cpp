#include "filters/color/color_filters.hpp"

#include "filters/filter_utils.hpp"
#include "math/interpolation.hpp"

#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <cmath>

namespace PhotoStudio::Filters {

// ----- Curves -----

std::vector<FilterParam> CurvesFilter::parameters() const
{
    return {{"shadows", "Shadows", -100.0f, 100.0f, 0.0f, 1.0f},
            {"midtones", "Midtones", -100.0f, 100.0f, 0.0f, 1.0f},
            {"highlights", "Highlights", -100.0f, 100.0f, 0.0f, 1.0f}};
}

void CurvesFilter::buildLut(f32 shadows, f32 midtones, f32 highlights, u8 lut[256])
{
    // Control points: endpoints fixed, three movable interior points.
    const f32 px[5] = {0.0f, 0.25f, 0.50f, 0.75f, 1.0f};
    const f32 py[5] = {0.0f, std::clamp(0.25f + shadows * 0.25f, 0.0f, 1.0f),
                       std::clamp(0.50f + midtones * 0.25f, 0.0f, 1.0f),
                       std::clamp(0.75f + highlights * 0.25f, 0.0f, 1.0f), 1.0f};

    for (int i = 0; i < 256; ++i) {
        const f32 x = i / 255.0f;
        int seg = 0;
        while (seg < 3 && x > px[seg + 1])
            ++seg;
        const f32 t = (x - px[seg]) / (px[seg + 1] - px[seg]);
        const f32 p0 = seg > 0 ? py[seg - 1] : py[0];
        const f32 p3 = seg < 3 ? py[seg + 2] : py[4];
        const f32 v = Math::catmullRom(p0, py[seg], py[seg + 1], p3, t);
        lut[i] = static_cast<u8>(std::lround(std::clamp(v, 0.0f, 1.0f) * 255.0f));
    }
}

void CurvesFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    u8 lut[256];
    buildLut(get(params, "shadows", 0.0f) / 100.0f, get(params, "midtones", 0.0f) / 100.0f,
             get(params, "highlights", 0.0f) / 100.0f, lut);
    Utils::applyLutRgb(rgba, lut, lut, lut);
}

// ----- Levels -----

std::vector<FilterParam> LevelsFilter::parameters() const
{
    return {{"in_black", "Input Black", 0.0f, 254.0f, 0.0f, 1.0f},
            {"in_white", "Input White", 1.0f, 255.0f, 255.0f, 1.0f},
            {"gamma", "Gamma", 0.10f, 3.0f, 1.0f, 0.01f},
            {"out_black", "Output Black", 0.0f, 254.0f, 0.0f, 1.0f},
            {"out_white", "Output White", 1.0f, 255.0f, 255.0f, 1.0f}};
}

void LevelsFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 inB = get(params, "in_black", 0.0f);
    const f32 inW = std::max(inB + 1.0f, get(params, "in_white", 255.0f));
    const f32 gamma = std::max(0.01f, get(params, "gamma", 1.0f));
    const f32 outB = get(params, "out_black", 0.0f);
    const f32 outW = get(params, "out_white", 255.0f);

    u8 lut[256];
    for (int i = 0; i < 256; ++i) {
        f32 v = std::clamp((i - inB) / (inW - inB), 0.0f, 1.0f);
        v = std::pow(v, 1.0f / gamma);
        lut[i] = static_cast<u8>(std::lround(std::clamp(outB + v * (outW - outB), 0.0f, 255.0f)));
    }
    Utils::applyLutRgb(rgba, lut, lut, lut);
}

// ----- Hue / Saturation / Lightness -----

std::vector<FilterParam> HslFilter::parameters() const
{
    return {{"hue", "Hue", -180.0f, 180.0f, 0.0f, 1.0f},
            {"saturation", "Saturation", -100.0f, 100.0f, 0.0f, 1.0f},
            {"lightness", "Lightness", -100.0f, 100.0f, 0.0f, 1.0f}};
}

void HslFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 hueShift = get(params, "hue", 0.0f) / 2.0f; // OpenCV hue range: 0..179
    const f32 satScale = 1.0f + get(params, "saturation", 0.0f) / 100.0f;
    const f32 lightScale = 1.0f + get(params, "lightness", 0.0f) / 100.0f;

    cv::Mat rgb, alpha, hls;
    Utils::splitAlpha(rgba, rgb, alpha);
    cv::cvtColor(rgb, hls, cv::COLOR_RGB2HLS);

    hls.forEach<cv::Vec3b>([&](cv::Vec3b& px, const int*) {
        int h = static_cast<int>(px[0]) + static_cast<int>(hueShift);
        h = ((h % 180) + 180) % 180;
        px[0] = static_cast<u8>(h);
        px[1] = cv::saturate_cast<u8>(px[1] * lightScale);
        px[2] = cv::saturate_cast<u8>(px[2] * satScale);
    });

    cv::cvtColor(hls, rgb, cv::COLOR_HLS2RGB);
    Utils::mergeAlpha(rgb, alpha, rgba);
}

// ----- Color Balance -----

std::vector<FilterParam> ColorBalanceFilter::parameters() const
{
    return {{"red", "Cyan - Red", -100.0f, 100.0f, 0.0f, 1.0f},
            {"green", "Magenta - Green", -100.0f, 100.0f, 0.0f, 1.0f},
            {"blue", "Yellow - Blue", -100.0f, 100.0f, 0.0f, 1.0f}};
}

void ColorBalanceFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 dr = get(params, "red", 0.0f) * 0.6f;
    const f32 dg = get(params, "green", 0.0f) * 0.6f;
    const f32 db = get(params, "blue", 0.0f) * 0.6f;

    u8 lutR[256], lutG[256], lutB[256];
    for (int i = 0; i < 256; ++i) {
        // Strongest effect in the midtones, fading toward black and white.
        const f32 w = std::sin(static_cast<f32>(i) / 255.0f * 3.14159265f);
        lutR[i] = cv::saturate_cast<u8>(i + dr * w);
        lutG[i] = cv::saturate_cast<u8>(i + dg * w);
        lutB[i] = cv::saturate_cast<u8>(i + db * w);
    }
    Utils::applyLutRgb(rgba, lutR, lutG, lutB);
}

// ----- Exposure -----

std::vector<FilterParam> ExposureFilter::parameters() const
{
    return {{"exposure", "Exposure (stops)", -4.0f, 4.0f, 0.0f, 0.05f},
            {"offset", "Offset", -0.5f, 0.5f, 0.0f, 0.005f},
            {"gamma", "Gamma", 0.10f, 3.0f, 1.0f, 0.01f}};
}

void ExposureFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 exposure = get(params, "exposure", 0.0f);
    const f32 offset = get(params, "offset", 0.0f);
    const f32 gamma = std::max(0.01f, get(params, "gamma", 1.0f));
    const f32 mult = std::pow(2.0f, exposure);

    u8 lut[256];
    for (int i = 0; i < 256; ++i) {
        f32 v = std::clamp(i / 255.0f * mult + offset, 0.0f, 1.0f);
        v = std::pow(v, 1.0f / gamma);
        lut[i] = static_cast<u8>(std::lround(v * 255.0f));
    }
    Utils::applyLutRgb(rgba, lut, lut, lut);
}

// ----- Brightness / Contrast -----

std::vector<FilterParam> BrightnessContrastFilter::parameters() const
{
    return {{"brightness", "Brightness", -100.0f, 100.0f, 0.0f, 1.0f},
            {"contrast", "Contrast", -100.0f, 100.0f, 0.0f, 1.0f}};
}

void BrightnessContrastFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 brightness = get(params, "brightness", 0.0f) * 1.275f;
    const f32 c = get(params, "contrast", 0.0f) / 100.0f;
    const f32 slope = std::tan((c + 1.0f) * 3.14159265f / 4.0f);

    u8 lut[256];
    for (int i = 0; i < 256; ++i)
        lut[i] = cv::saturate_cast<u8>((i - 127.5f) * slope + 127.5f + brightness);
    Utils::applyLutRgb(rgba, lut, lut, lut);
}

// ----- Vibrance -----

std::vector<FilterParam> VibranceFilter::parameters() const
{
    return {{"vibrance", "Vibrance", -100.0f, 100.0f, 40.0f, 1.0f}};
}

void VibranceFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 amount = get(params, "vibrance", 40.0f) / 100.0f;

    cv::Mat rgb, alpha, hsv;
    Utils::splitAlpha(rgba, rgb, alpha);
    cv::cvtColor(rgb, hsv, cv::COLOR_RGB2HSV);

    hsv.forEach<cv::Vec3b>([&](cv::Vec3b& px, const int*) {
        const f32 s = px[1] / 255.0f;
        // Boost muted colors more than already-saturated ones.
        const f32 boost = amount * (1.0f - s);
        px[1] = cv::saturate_cast<u8>(px[1] * (1.0f + boost));
    });

    cv::cvtColor(hsv, rgb, cv::COLOR_HSV2RGB);
    Utils::mergeAlpha(rgb, alpha, rgba);
}

// ----- Invert -----

void InvertFilter::apply(cv::Mat& rgba, const ParamMap&) const
{
    u8 lut[256];
    for (int i = 0; i < 256; ++i)
        lut[i] = static_cast<u8>(255 - i);
    Utils::applyLutRgb(rgba, lut, lut, lut);
}

// ----- Desaturate -----

void DesaturateFilter::apply(cv::Mat& rgba, const ParamMap&) const
{
    rgba.forEach<cv::Vec4b>([](cv::Vec4b& px, const int*) {
        const u8 g = cv::saturate_cast<u8>(0.30f * px[0] + 0.59f * px[1] + 0.11f * px[2]);
        px[0] = px[1] = px[2] = g;
    });
}

// ----- Sepia -----

void SepiaFilter::apply(cv::Mat& rgba, const ParamMap&) const
{
    rgba.forEach<cv::Vec4b>([](cv::Vec4b& px, const int*) {
        const f32 r = px[0], g = px[1], b = px[2];
        px[0] = cv::saturate_cast<u8>(0.393f * r + 0.769f * g + 0.189f * b);
        px[1] = cv::saturate_cast<u8>(0.349f * r + 0.686f * g + 0.168f * b);
        px[2] = cv::saturate_cast<u8>(0.272f * r + 0.534f * g + 0.131f * b);
    });
}

// ----- Posterize -----

std::vector<FilterParam> PosterizeFilter::parameters() const
{
    return {{"levels", "Levels", 2.0f, 32.0f, 6.0f, 1.0f}};
}

void PosterizeFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const int levels = std::max(2, static_cast<int>(get(params, "levels", 6.0f)));
    const f32 stepSize = 255.0f / (levels - 1);

    u8 lut[256];
    for (int i = 0; i < 256; ++i)
        lut[i] = cv::saturate_cast<u8>(std::round(i / stepSize) * stepSize);
    Utils::applyLutRgb(rgba, lut, lut, lut);
}

// ----- Threshold -----

std::vector<FilterParam> ThresholdFilter::parameters() const
{
    return {{"level", "Level", 1.0f, 254.0f, 128.0f, 1.0f}};
}

void ThresholdFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const int level = static_cast<int>(get(params, "level", 128.0f));
    rgba.forEach<cv::Vec4b>([level](cv::Vec4b& px, const int*) {
        const f32 lum = 0.30f * px[0] + 0.59f * px[1] + 0.11f * px[2];
        const u8 v = lum >= level ? 255 : 0;
        px[0] = px[1] = px[2] = v;
    });
}

} // namespace PhotoStudio::Filters
