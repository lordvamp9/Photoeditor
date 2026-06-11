#include "filters/detail/detail_filters.hpp"

#include "filters/filter_utils.hpp"

#include <opencv2/imgproc.hpp>
#include <opencv2/photo.hpp>

namespace PhotoStudio::Filters {

// ----- Sharpen -----

std::vector<FilterParam> SharpenFilter::parameters() const
{
    return {{"amount", "Amount", 0.0f, 5.0f, 1.0f, 0.05f}};
}

void SharpenFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 amount = get(params, "amount", 1.0f);
    cv::Mat kernel = (cv::Mat_<f32>(3, 3) << 0, -amount, 0, -amount, 1 + 4 * amount, -amount, 0,
                      -amount, 0);
    cv::filter2D(rgba, rgba, -1, kernel, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
}

// ----- Unsharp Mask -----

std::vector<FilterParam> UnsharpMaskFilter::parameters() const
{
    return {{"radius", "Radius (px)", 1.0f, 50.0f, 4.0f, 1.0f},
            {"amount", "Amount", 0.0f, 5.0f, 1.0f, 0.05f}};
}

void UnsharpMaskFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 radius = get(params, "radius", 4.0f);
    const f32 amount = get(params, "amount", 1.0f);
    const int k = Utils::oddKernel(radius);

    cv::Mat blurred;
    cv::GaussianBlur(rgba, blurred, cv::Size(k, k), radius / 3.0, radius / 3.0,
                     cv::BORDER_REPLICATE);
    cv::addWeighted(rgba, 1.0 + amount, blurred, -amount, 0, rgba);
}

// ----- High Pass -----

std::vector<FilterParam> HighPassFilter::parameters() const
{
    return {{"radius", "Radius (px)", 1.0f, 100.0f, 10.0f, 1.0f}};
}

void HighPassFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 radius = get(params, "radius", 10.0f);
    const int k = Utils::oddKernel(radius);

    cv::Mat rgb, alpha;
    Utils::splitAlpha(rgba, rgb, alpha);

    cv::Mat blurred, rgbF, blurF;
    cv::GaussianBlur(rgb, blurred, cv::Size(k, k), radius / 3.0, radius / 3.0,
                     cv::BORDER_REPLICATE);
    rgb.convertTo(rgbF, CV_32FC3);
    blurred.convertTo(blurF, CV_32FC3);
    cv::Mat highPass = rgbF - blurF + cv::Scalar(128, 128, 128);
    highPass.convertTo(rgb, CV_8UC3);

    Utils::mergeAlpha(rgb, alpha, rgba);
}

// ----- Clarity (local contrast) -----

std::vector<FilterParam> ClarityFilter::parameters() const
{
    return {{"amount", "Amount", 0.0f, 100.0f, 30.0f, 1.0f}};
}

void ClarityFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 amount = get(params, "amount", 30.0f) / 100.0f;
    const int k = Utils::oddKernel(30.0f);

    cv::Mat blurred;
    cv::GaussianBlur(rgba, blurred, cv::Size(k, k), 10.0, 10.0, cv::BORDER_REPLICATE);
    cv::addWeighted(rgba, 1.0 + amount, blurred, -amount, 0, rgba);
}

// ----- Reduce Noise -----

std::vector<FilterParam> ReduceNoiseFilter::parameters() const
{
    return {{"strength", "Strength", 1.0f, 30.0f, 7.0f, 1.0f}};
}

void ReduceNoiseFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 h = get(params, "strength", 7.0f);

    cv::Mat rgb, alpha, out;
    Utils::splitAlpha(rgba, rgb, alpha);
    cv::fastNlMeansDenoisingColored(rgb, out, h, h, 7, 21);
    Utils::mergeAlpha(out, alpha, rgba);
}

} // namespace PhotoStudio::Filters
