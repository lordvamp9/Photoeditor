#pragma once

#include "filters/filter_base.hpp"

namespace PhotoStudio::Filters {

class GaussianBlurFilter final : public FilterBase {
public:
    std::string name() const override { return "Gaussian Blur"; }
    std::string category() const override { return "Blur"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class BoxBlurFilter final : public FilterBase {
public:
    std::string name() const override { return "Box Blur"; }
    std::string category() const override { return "Blur"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class MotionBlurFilter final : public FilterBase {
public:
    std::string name() const override { return "Motion Blur"; }
    std::string category() const override { return "Blur"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class SurfaceBlurFilter final : public FilterBase {
public:
    std::string name() const override { return "Surface Blur"; }
    std::string category() const override { return "Blur"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

} // namespace PhotoStudio::Filters
