#include "filters/artistic/artistic_filters.hpp"

#include "filters/filter_utils.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>

#include <algorithm>
#include <cmath>

namespace PhotoStudio::Filters {

// ----- Oil Paint -----

std::vector<FilterParam> OilPaintFilter::parameters() const
{
    return {{"spatial", "Brush Size", 5.0f, 40.0f, 15.0f, 1.0f},
            {"color", "Color Range", 5.0f, 80.0f, 30.0f, 1.0f}};
}

void OilPaintFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f64 sp = get(params, "spatial", 15.0f);
    const f64 sr = get(params, "color", 30.0f);

    cv::Mat rgb, alpha, out;
    Utils::splitAlpha(rgba, rgb, alpha);
    cv::pyrMeanShiftFiltering(rgb, out, sp, sr, 1);
    Utils::mergeAlpha(out, alpha, rgba);
}

// ----- Watercolor -----

std::vector<FilterParam> WatercolorFilter::parameters() const
{
    return {{"sigma_s", "Smoothing", 10.0f, 200.0f, 60.0f, 1.0f},
            {"sigma_r", "Edge Strength", 0.10f, 1.0f, 0.45f, 0.01f}};
}

void WatercolorFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 sigmaS = get(params, "sigma_s", 60.0f);
    const f32 sigmaR = get(params, "sigma_r", 0.45f);

    cv::Mat rgb, alpha, out;
    Utils::splitAlpha(rgba, rgb, alpha);
    cv::stylization(rgb, out, sigmaS, sigmaR);
    Utils::mergeAlpha(out, alpha, rgba);
}

// ----- Pencil Sketch -----

std::vector<FilterParam> PencilSketchFilter::parameters() const
{
    return {{"sigma_s", "Smoothing", 10.0f, 200.0f, 60.0f, 1.0f},
            {"sigma_r", "Detail", 0.01f, 0.50f, 0.07f, 0.01f},
            {"shade", "Shade Factor", 0.0f, 0.10f, 0.04f, 0.005f},
            {"colored", "Colored (0/1)", 0.0f, 1.0f, 0.0f, 1.0f}};
}

void PencilSketchFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 sigmaS = get(params, "sigma_s", 60.0f);
    const f32 sigmaR = get(params, "sigma_r", 0.07f);
    const f32 shade = get(params, "shade", 0.04f);
    const bool colored = get(params, "colored", 0.0f) >= 0.5f;

    cv::Mat rgb, alpha, gray, color;
    Utils::splitAlpha(rgba, rgb, alpha);
    cv::pencilSketch(rgb, gray, color, sigmaS, sigmaR, shade);
    if (colored) {
        Utils::mergeAlpha(color, alpha, rgba);
    } else {
        cv::Mat gray3;
        cv::cvtColor(gray, gray3, cv::COLOR_GRAY2RGB);
        Utils::mergeAlpha(gray3, alpha, rgba);
    }
}

// ----- Cartoon -----

std::vector<FilterParam> CartoonFilter::parameters() const
{
    return {{"edge", "Edge Strength", 1.0f, 15.0f, 7.0f, 1.0f}};
}

void CartoonFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const int blockSize = static_cast<int>(get(params, "edge", 7.0f)) * 2 + 1;

    cv::Mat rgb, alpha;
    Utils::splitAlpha(rgba, rgb, alpha);

    cv::Mat smooth;
    cv::bilateralFilter(rgb, smooth, 9, 75, 75);

    cv::Mat gray, edges;
    cv::cvtColor(rgb, gray, cv::COLOR_RGB2GRAY);
    cv::medianBlur(gray, gray, 5);
    cv::adaptiveThreshold(gray, edges, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY,
                          blockSize, 2);

    cv::Mat edges3, out;
    cv::cvtColor(edges, edges3, cv::COLOR_GRAY2RGB);
    cv::bitwise_and(smooth, edges3, out);
    Utils::mergeAlpha(out, alpha, rgba);
}

// ----- Pixelate -----

std::vector<FilterParam> PixelateFilter::parameters() const
{
    return {{"size", "Block Size", 2.0f, 64.0f, 8.0f, 1.0f}};
}

void PixelateFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const int block = std::max(2, static_cast<int>(get(params, "size", 8.0f)));
    const cv::Size original = rgba.size();
    const cv::Size small(std::max(1, original.width / block), std::max(1, original.height / block));

    cv::Mat tmp;
    cv::resize(rgba, tmp, small, 0, 0, cv::INTER_AREA);
    cv::resize(tmp, rgba, original, 0, 0, cv::INTER_NEAREST);
}

// ----- Vignette -----

std::vector<FilterParam> VignetteFilter::parameters() const
{
    return {{"strength", "Strength", 0.0f, 100.0f, 50.0f, 1.0f},
            {"radius", "Radius", 10.0f, 150.0f, 75.0f, 1.0f}};
}

void VignetteFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 strength = get(params, "strength", 50.0f) / 100.0f;
    const f32 radius = get(params, "radius", 75.0f) / 100.0f;

    const f32 cx = rgba.cols / 2.0f;
    const f32 cy = rgba.rows / 2.0f;
    const f32 maxDist = std::sqrt(cx * cx + cy * cy) * radius;

    rgba.forEach<cv::Vec4b>([&](cv::Vec4b& px, const int* pos) {
        const f32 dx = pos[1] - cx;
        const f32 dy = pos[0] - cy;
        const f32 d = std::sqrt(dx * dx + dy * dy) / maxDist;
        const f32 factor = 1.0f - strength * std::clamp(d - 0.5f, 0.0f, 1.0f);
        px[0] = cv::saturate_cast<u8>(px[0] * factor);
        px[1] = cv::saturate_cast<u8>(px[1] * factor);
        px[2] = cv::saturate_cast<u8>(px[2] * factor);
    });
}

// ----- Add Noise -----

std::vector<FilterParam> AddNoiseFilter::parameters() const
{
    return {{"amount", "Amount", 1.0f, 100.0f, 15.0f, 1.0f},
            {"monochrome", "Monochrome (0/1)", 0.0f, 1.0f, 1.0f, 1.0f}};
}

void AddNoiseFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f64 amount = get(params, "amount", 15.0f);
    const bool mono = get(params, "monochrome", 1.0f) >= 0.5f;

    cv::Mat rgb, alpha;
    Utils::splitAlpha(rgba, rgb, alpha);

    if (mono) {
        cv::Mat noise(rgb.size(), CV_16SC1);
        cv::randn(noise, 0, amount);
        cv::Mat noise3;
        cv::Mat planes[] = {noise, noise, noise};
        cv::merge(planes, 3, noise3);
        cv::Mat rgb16;
        rgb.convertTo(rgb16, CV_16SC3);
        rgb16 += noise3;
        rgb16.convertTo(rgb, CV_8UC3);
    } else {
        cv::Mat noise(rgb.size(), CV_16SC3);
        cv::randn(noise, cv::Scalar(0, 0, 0), cv::Scalar(amount, amount, amount));
        cv::Mat rgb16;
        rgb.convertTo(rgb16, CV_16SC3);
        rgb16 += noise;
        rgb16.convertTo(rgb, CV_8UC3);
    }
    Utils::mergeAlpha(rgb, alpha, rgba);
}

} // namespace PhotoStudio::Filters
