#include "filters/filter_registry.hpp"

#include <opencv2/imgproc.hpp>

#include <algorithm>

namespace PhotoStudio::Filters {

FilterRegistry& FilterRegistry::instance()
{
    static FilterRegistry registry;
    return registry;
}

void FilterRegistry::add(std::unique_ptr<FilterBase> filter)
{
    if (!find(filter->name()))
        m_filters.push_back(std::move(filter));
}

FilterBase* FilterRegistry::find(const std::string& name) const
{
    for (const auto& f : m_filters)
        if (f->name() == name)
            return f.get();
    return nullptr;
}

std::vector<std::string> FilterRegistry::names() const
{
    std::vector<std::string> out;
    out.reserve(m_filters.size());
    for (const auto& f : m_filters)
        out.push_back(f->name());
    return out;
}

std::vector<std::pair<std::string, std::vector<std::string>>> FilterRegistry::byCategory() const
{
    std::vector<std::pair<std::string, std::vector<std::string>>> out;
    for (const auto& f : m_filters) {
        auto it = std::find_if(out.begin(), out.end(),
                               [&](const auto& entry) { return entry.first == f->category(); });
        if (it == out.end()) {
            out.push_back({f->category(), {f->name()}});
        } else {
            it->second.push_back(f->name());
        }
    }
    return out;
}

void applyFilterToActiveLayer(Core::Document& doc, const FilterBase& filter, const ParamMap& params)
{
    auto layer = doc.activeLayer();
    if (!layer || layer->pixels().empty() || layer->isLocked())
        return;

    cv::Mat& pixels = layer->pixels();

    if (!doc.hasSelection()) {
        filter.apply(pixels, params);
        doc.markDirty();
        return;
    }

    // Build the selection mask in layer coordinates.
    cv::Mat layerMask(pixels.rows, pixels.cols, CV_8UC1, cv::Scalar(0));
    const cv::Rect docRect(0, 0, static_cast<int>(doc.width()), static_cast<int>(doc.height()));
    const cv::Rect layerRect(layer->offset().x, layer->offset().y, pixels.cols, pixels.rows);
    const cv::Rect roi = docRect & layerRect;
    if (!roi.empty()) {
        const cv::Rect local(roi.x - layer->offset().x, roi.y - layer->offset().y, roi.width,
                             roi.height);
        doc.selection()(roi).copyTo(layerMask(local));
    }

    cv::Mat filtered = pixels.clone();
    filter.apply(filtered, params);

    // Feathered blend between original and filtered using the mask.
    cv::Mat maskF;
    layerMask.convertTo(maskF, CV_32F, 1.0 / 255.0);
    cv::Mat origF, filtF;
    pixels.convertTo(origF, CV_32FC4);
    filtered.convertTo(filtF, CV_32FC4);
    cv::Mat mask4;
    cv::Mat channels[] = {maskF, maskF, maskF, maskF};
    cv::merge(channels, 4, mask4);
    cv::Mat blended = origF.mul(cv::Scalar(1, 1, 1, 1) - mask4) + filtF.mul(mask4);
    blended.convertTo(pixels, CV_8UC4);

    doc.markDirty();
}

} // namespace PhotoStudio::Filters
