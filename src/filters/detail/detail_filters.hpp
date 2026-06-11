#pragma once

#include "filters/filter_base.hpp"

namespace PhotoStudio::Filters {

class SharpenFilter final : public FilterBase {
public:
    std::string name() const override { return "Sharpen"; }
    std::string category() const override { return "Detail"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class UnsharpMaskFilter final : public FilterBase {
public:
    std::string name() const override { return "Unsharp Mask"; }
    std::string category() const override { return "Detail"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class HighPassFilter final : public FilterBase {
public:
    std::string name() const override { return "High Pass"; }
    std::string category() const override { return "Detail"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class ClarityFilter final : public FilterBase {
public:
    std::string name() const override { return "Clarity"; }
    std::string category() const override { return "Detail"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class ReduceNoiseFilter final : public FilterBase {
public:
    std::string name() const override { return "Reduce Noise"; }
    std::string category() const override { return "Detail"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

} // namespace PhotoStudio::Filters
