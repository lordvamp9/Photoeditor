#include "filters/blur/blur_filters.hpp"

#include "filters/filter_utils.hpp"

#include <opencv2/imgproc.hpp>

#include <cmath>

namespace PhotoStudio::Filters {

// ----- Gaussian Blur -----

std::vector<FilterParam> GaussianBlurFilter::parameters() const
{
    return {{"radius", "Radius (px)", 1.0f, 100.0f, 8.0f, 1.0f}};
}

void GaussianBlurFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const f32 radius = get(params, "radius", 8.0f);
    const int k = Utils::oddKernel(radius);
    cv::GaussianBlur(rgba, rgba, cv::Size(k, k), radius / 3.0, radius / 3.0,
                     cv::BORDER_REPLICATE);
}

// ----- Box Blur -----

std::vector<FilterParam> BoxBlurFilter::parameters() const
{
    return {{"radius", "Radius (px)", 1.0f, 100.0f, 5.0f, 1.0f}};
}

void BoxBlurFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const int k = Utils::oddKernel(get(params, "radius", 5.0f));
    cv::blur(rgba, rgba, cv::Size(k, k), cv::Point(-1, -1), cv::BORDER_REPLICATE);
}

// ----- Motion Blur -----

std::vector<FilterParam> MotionBlurFilter::parameters() const
{
    return {{"length", "Length (px)", 3.0f, 200.0f, 20.0f, 1.0f},
            {"angle", "Angle (deg)", 0.0f, 360.0f, 0.0f, 1.0f}};
}

void MotionBlurFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const int length = std::max(3, static_cast<int>(get(params, "length", 20.0f)));
    const f32 angle = get(params, "angle", 0.0f) * 3.14159265f / 180.0f;

    cv::Mat kernel = cv::Mat::zeros(length, length, CV_32F);
    const f32 c = length / 2.0f;
    const cv::Point p1(static_cast<int>(c - std::cos(angle) * c),
                       static_cast<int>(c - std::sin(angle) * c));
    const cv::Point p2(static_cast<int>(c + std::cos(angle) * c),
                       static_cast<int>(c + std::sin(angle) * c));
    cv::line(kernel, p1, p2, cv::Scalar(1.0), 1);
    kernel /= std::max(1.0, cv::sum(kernel)[0]);
    cv::filter2D(rgba, rgba, -1, kernel, cv::Point(-1, -1), 0, cv::BORDER_REPLICATE);
}

// ----- Surface Blur (edge preserving) -----

std::vector<FilterParam> SurfaceBlurFilter::parameters() const
{
    return {{"radius", "Radius (px)", 1.0f, 30.0f, 9.0f, 1.0f},
            {"threshold", "Threshold", 1.0f, 150.0f, 40.0f, 1.0f}};
}

void SurfaceBlurFilter::apply(cv::Mat& rgba, const ParamMap& params) const
{
    const int d = static_cast<int>(get(params, "radius", 9.0f)) * 2 + 1;
    const f64 sigmaColor = get(params, "threshold", 40.0f);

    cv::Mat rgb, alpha, out;
    Utils::splitAlpha(rgba, rgb, alpha);
    cv::bilateralFilter(rgb, out, d, sigmaColor, d);
    Utils::mergeAlpha(out, alpha, rgba);
}

} // namespace PhotoStudio::Filters
