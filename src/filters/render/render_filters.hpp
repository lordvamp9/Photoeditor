#pragma once

#include "filters/filter_base.hpp"

namespace PhotoStudio::Filters {

class CloudsFilter final : public FilterBase {
public:
    std::string name() const override { return "Clouds"; }
    std::string category() const override { return "Render"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class GradientMapFilter final : public FilterBase {
public:
    std::string name() const override { return "Gradient Map"; }
    std::string category() const override { return "Render"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

} // namespace PhotoStudio::Filters
