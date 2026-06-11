#pragma once

#include "core/types.hpp"

#include <opencv2/core.hpp>

#include <string>
#include <vector>

namespace PhotoStudio::Core {

enum class BlendMode : u8 {
    Normal = 0,
    Darken,
    Multiply,
    ColorBurn,
    LinearBurn,
    Lighten,
    Screen,
    ColorDodge,
    LinearDodge, // Add
    Overlay,
    SoftLight,
    HardLight,
    VividLight,
    LinearLight,
    PinLight,
    Difference,
    Exclusion,
    Subtract,
    Divide,
    Hue,
    Saturation,
    Color,
    Luminosity,
    Count
};

const std::string& blendModeName(BlendMode mode);
const std::vector<std::string>& blendModeNames();

// Blends one source pixel over a base pixel (RGB in [0,1]) without alpha.
cv::Vec3f blendPixel(BlendMode mode, const cv::Vec3f& base, const cv::Vec3f& blend);

// Composites `src` (straight-alpha RGBA8) over `dst` (document-sized RGBA8)
// at `offset`, honoring an optional 8-bit layer mask, opacity and blend mode.
void compositeOver(cv::Mat& dst, const cv::Mat& src, const cv::Mat& mask, f32 opacity, BlendMode mode,
                   cv::Point offset);

} // namespace PhotoStudio::Core
