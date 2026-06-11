#pragma once

#include "filters/filter_base.hpp"

namespace PhotoStudio::Filters {

class OilPaintFilter final : public FilterBase {
public:
    std::string name() const override { return "Oil Paint"; }
    std::string category() const override { return "Artistic"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class WatercolorFilter final : public FilterBase {
public:
    std::string name() const override { return "Watercolor"; }
    std::string category() const override { return "Artistic"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class PencilSketchFilter final : public FilterBase {
public:
    std::string name() const override { return "Pencil Sketch"; }
    std::string category() const override { return "Artistic"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class CartoonFilter final : public FilterBase {
public:
    std::string name() const override { return "Cartoon"; }
    std::string category() const override { return "Artistic"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class PixelateFilter final : public FilterBase {
public:
    std::string name() const override { return "Pixelate"; }
    std::string category() const override { return "Artistic"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class VignetteFilter final : public FilterBase {
public:
    std::string name() const override { return "Vignette"; }
    std::string category() const override { return "Artistic"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

class AddNoiseFilter final : public FilterBase {
public:
    std::string name() const override { return "Add Noise"; }
    std::string category() const override { return "Artistic"; }
    std::vector<FilterParam> parameters() const override;
    void apply(cv::Mat& rgba, const ParamMap& params) const override;
};

} // namespace PhotoStudio::Filters
