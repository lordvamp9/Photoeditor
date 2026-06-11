#pragma once

#include "filters/filter_base.hpp"

namespace PhotoStudio::Filters {

class CurvesFilter final : public FilterBase {
public:
    std::string name() const override { return "Curves"; }
    std::string category() const override { return "Adjustments"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;

    // Builds a 256-entry tone LUT through (0,0)..(1,1) with three offsets
    // applied at x = 0.25 / 0.5 / 0.75 (values in [-1, 1]).
    static void buildLut(f32 shadows, f32 midtones, f32 highlights, u8 lut[256]);
};

class LevelsFilter final : public FilterBase {
public:
    std::string name() const override { return "Levels"; }
    std::string category() const override { return "Adjustments"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class HslFilter final : public FilterBase {
public:
    std::string name() const override { return "Hue / Saturation"; }
    std::string category() const override { return "Adjustments"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class ColorBalanceFilter final : public FilterBase {
public:
    std::string name() const override { return "Color Balance"; }
    std::string category() const override { return "Adjustments"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class ExposureFilter final : public FilterBase {
public:
    std::string name() const override { return "Exposure"; }
    std::string category() const override { return "Adjustments"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class BrightnessContrastFilter final : public FilterBase {
public:
    std::string name() const override { return "Brightness / Contrast"; }
    std::string category() const override { return "Adjustments"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class VibranceFilter final : public FilterBase {
public:
    std::string name() const override { return "Vibrance"; }
    std::string category() const override { return "Adjustments"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class InvertFilter final : public FilterBase {
public:
    std::string name() const override { return "Invert"; }
    std::string category() const override { return "Adjustments"; }
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class DesaturateFilter final : public FilterBase {
public:
    std::string name() const override { return "Desaturate"; }
    std::string category() const override { return "Adjustments"; }
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class SepiaFilter final : public FilterBase {
public:
    std::string name() const override { return "Sepia"; }
    std::string category() const override { return "Adjustments"; }
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class PosterizeFilter final : public FilterBase {
public:
    std::string name() const override { return "Posterize"; }
    std::string category() const override { return "Adjustments"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class ThresholdFilter final : public FilterBase {
public:
    std::string name() const override { return "Threshold"; }
    std::string category() const override { return "Adjustments"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

} // namespace PhotoStudio::Filters
