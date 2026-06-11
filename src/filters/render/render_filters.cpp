#include "filters/render/render_filters.hpp"

#include "filters/filter_utils.hpp"

#include <opencv2/imgproc.hpp>

#include <cmath>

namespace PhotoStudio::Filters {

// ----- Clouds (fractal value noise) -----

std::vector<FilterParam> CloudsFilter::parameters() const
{
    return {{"octaves", "Octaves", 1.0f, 8.0f, 5.0f, 1.0f},
            {"seed", "Seed", 0.0f, 1000.0f, 42.0f, 1.0f}};
}

void CloudsFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const int octaves = static_cast<int>(get(params, "octaves", 5.0f));
    const u64 seed = static_cast<u64>(get(params, "seed", 42.0f));

    cv::Mat accum = cv::Mat::zeros(rgba.size(), CV_32F);
    cv::RNG rng(seed);
    f32 amplitude = 0.5f;
    int cell = 4;

    for (int o = 0; o < octaves; ++o) {
        cv::Mat noise(cell + 1, cell + 1, CV_32F);
        rng.fill(noise, cv::RNG::UNIFORM, 0.0f, 1.0f);
        cv::Mat upscaled;
        cv::resize(noise, upscaled, rgba.size(), 0, 0, cv::INTER_CUBIC);
        accum += upscaled * amplitude;
        amplitude *= 0.5f;
        cell *= 2;
    }

    cv::normalize(accum, accum, 0.0, 255.0, cv::NORM_MINMAX);
    cv::Mat gray8;
    accum.convertTo(gray8, CV_8U);

    rgba.forEach<cv::Vec4b>([&gray8](cv::Vec4b& px, const int* pos) {
        const u8 v = gray8.at<u8>(pos[0], pos[1]);
        px = {v, v, v, 255};
    });
}

// ----- Gradient Map -----

std::vector<FilterParam> GradientMapFilter::parameters() const
{
    // Shadow color -> highlight color, packed as RGB components in [0,255].
    return {{"r0", "Shadow R", 0.0f, 255.0f, 15.0f, 1.0f},
            {"g0", "Shadow G", 0.0f, 255.0f, 20.0f, 1.0f},
            {"b0", "Shadow B", 0.0f, 255.0f, 60.0f, 1.0f},
            {"r1", "Highlight R", 0.0f, 255.0f, 255.0f, 1.0f},
            {"g1", "Highlight G", 0.0f, 255.0f, 180.0f, 1.0f},
            {"b1", "Highlight B", 0.0f, 255.0f, 90.0f, 1.0f}};
}

void GradientMapFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 r0 = get(params, "r0", 15.0f), g0 = get(params, "g0", 20.0f),
              b0 = get(params, "b0", 60.0f);
    const f32 r1 = get(params, "r1", 255.0f), g1 = get(params, "g1", 180.0f),
              b1 = get(params, "b1", 90.0f);

    u8 lutR[256], lutG[256], lutB[256];
    for (int i = 0; i < 256; ++i) {
        const f32 t = i / 255.0f;
        lutR[i] = cv::saturate_cast<u8>(r0 + (r1 - r0) * t);
        lutG[i] = cv::saturate_cast<u8>(g0 + (g1 - g0) * t);
        lutB[i] = cv::saturate_cast<u8>(b0 + (b1 - b0) * t);
    }

    rgba.forEach<cv::Vec4b>([&](cv::Vec4b& px, const int*) {
        const u8 lum = cv::saturate_cast<u8>(0.30f * px[0] + 0.59f * px[1] + 0.11f * px[2]);
        px[0] = lutR[lum];
        px[1] = lutG[lum];
        px[2] = lutB[lum];
    });
}

} // namespace PhotoStudio::Filters
