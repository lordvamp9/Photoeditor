#pragma once

#include "core/types.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace PhotoStudio::Filters::Utils {

// Splits an RGBA image into a 3-channel RGB image and its alpha plane.
inline void splitAlpha(const cv::Mat& rgba, cv::Mat& rgb, cv::Mat& alpha)
{
    cv::Mat planes[4];
    cv::split(rgba, planes);
    cv::merge(planes, 3, rgb);
    alpha = planes[3];
}

inline void mergeAlpha(const cv::Mat& rgb, const cv::Mat& alpha, cv::Mat& rgba)
{
    cv::Mat planes[3];
    cv::split(rgb, planes);
    cv::Mat all[4] = {planes[0], planes[1], planes[2], alpha};
    cv::merge(all, 4, rgba);
}

// Applies a 256-entry LUT to the RGB channels only, leaving alpha untouched.
inline void applyLutRgb(cv::Mat& rgba, const cv::Mat& lut256)
{
    CV_Assert(lut256.total() == 256 && lut256.type() == CV_8UC1);
    cv::Mat lut4(1, 256, CV_8UC4);
    for (int i = 0; i < 256; ++i) {
        const u8 v = lut256.at<u8>(i);
        lut4.at<cv::Vec4b>(i) = {v, v, v, static_cast<u8>(i)};
    }
    cv::LUT(rgba, lut4, rgba);
}

// Per-channel LUTs for R, G and B; alpha passes through.
inline void applyLutRgb(cv::Mat& rgba, const u8 lutR[256], const u8 lutG[256], const u8 lutB[256])
{
    cv::Mat lut4(1, 256, CV_8UC4);
    for (int i = 0; i < 256; ++i)
        lut4.at<cv::Vec4b>(i) = {lutR[i], lutG[i], lutB[i], static_cast<u8>(i)};
    cv::LUT(rgba, lut4, rgba);
}

inline int oddKernel(f32 radius)
{
    int k = static_cast<int>(radius) * 2 + 1;
    return k < 1 ? 1 : k;
}

} // namespace PhotoStudio::Filters::Utils
