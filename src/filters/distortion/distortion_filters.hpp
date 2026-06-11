#pragma once

#include "filters/filter_base.hpp"

namespace PhotoStudio::Filters {

class PinchFilter final : public FilterBase {
public:
    std::string name() const override { return "Pinch / Punch"; }
    std::string category() const override { return "Distort"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class SwirlFilter final : public FilterBase {
public:
    std::string name() const override { return "Swirl"; }
    std::string category() const override { return "Distort"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class RippleFilter final : public FilterBase {
public:
    std::string name() const override { return "Ripple"; }
    std::string category() const override { return "Distort"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class SphereFilter final : public FilterBase {
public:
    std::string name() const override { return "Spherize"; }
    std::string category() const override { return "Distort"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

} // namespace PhotoStudio::Filters
