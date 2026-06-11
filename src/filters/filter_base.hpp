#pragma once

#include "core/types.hpp"

#include <opencv2/core.hpp>

#include <map>
#include <string>
#include <vector>

namespace PhotoStudio::Filters {

using ParamMap = std::map<std::string, f32>;

struct FilterParam {
    std::string key;
    std::string label;
    f32 minValue;
    f32 maxValue;
    f32 defaultValue;
    f32 step = 1.0f;
};

class FilterBase {
public:
    virtual ~FilterBase() = default;

    virtual std::string name() const = 0;
    virtual std::string category() const = 0;
    virtual std::vector<FilterParam> parameters() const { return {}; }

    // Applies the filter in place to a straight-alpha RGBA8 image.
    virtual void apply(cv::Mat& rgba, const ParamMap& params) const = 0;

protected:
    static f32 get(const ParamMap& params, const std::string& key, f32 fallback)
    {
        const auto it = params.find(key);
        return it != params.end() ? it->second : fallback;
    }
};

} // namespace PhotoStudio::Filters
