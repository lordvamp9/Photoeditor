#pragma once

#include "core/types.hpp"

#include <opencv2/core.hpp>

#include <algorithm>
#include <cmath>

namespace PhotoStudio::Math {

// Perceptual luminance (Rec. 601, as used by Photoshop blend modes).
inline f32 luminance(const cv::Vec3f& c)
{
    return 0.30f * c[0] + 0.59f * c[1] + 0.11f * c[2];
}

inline cv::Vec3f clipColor(cv::Vec3f c)
{
    const f32 l = luminance(c);
    const f32 n = std::min({c[0], c[1], c[2]});
    const f32 x = std::max({c[0], c[1], c[2]});
    if (n < 0.0f) {
        for (int i = 0; i < 3; ++i)
            c[i] = l + (c[i] - l) * l / (l - n);
    }
    if (x > 1.0f) {
        for (int i = 0; i < 3; ++i)
            c[i] = l + (c[i] - l) * (1.0f - l) / (x - l);
    }
    return c;
}

inline cv::Vec3f setLuminance(const cv::Vec3f& c, f32 l)
{
    const f32 d = l - luminance(c);
    return clipColor({c[0] + d, c[1] + d, c[2] + d});
}

inline f32 saturationOf(const cv::Vec3f& c)
{
    return std::max({c[0], c[1], c[2]}) - std::min({c[0], c[1], c[2]});
}

inline cv::Vec3f setSaturation(cv::Vec3f c, f32 s)
{
    int iMin = 0, iMid = 1, iMax = 2;
    int idx[3] = {0, 1, 2};
    std::sort(idx, idx + 3, [&](int a, int b) { return c[a] < c[b]; });
    iMin = idx[0];
    iMid = idx[1];
    iMax = idx[2];

    if (c[iMax] > c[iMin]) {
        c[iMid] = (c[iMid] - c[iMin]) * s / (c[iMax] - c[iMin]);
        c[iMax] = s;
    } else {
        c[iMid] = 0.0f;
        c[iMax] = 0.0f;
    }
    c[iMin] = 0.0f;
    return c;
}

inline u8 toU8(f32 v)
{
    return static_cast<u8>(std::lround(std::clamp(v, 0.0f, 1.0f) * 255.0f));
}

} // namespace PhotoStudio::Math
