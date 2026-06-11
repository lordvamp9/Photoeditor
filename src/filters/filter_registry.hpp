#pragma once

#include "core/document.hpp"
#include "filters/filter_base.hpp"

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace PhotoStudio::Filters {

class FilterRegistry {
public:
    static FilterRegistry& instance();

    void add(std::unique_ptr<FilterBase> filter);
    FilterBase* find(const std::string& name) const;

    std::vector<std::string> names() const;
    // Category -> filter names, in registration order.
    std::vector<std::pair<std::string, std::vector<std::string>>> byCategory() const;

private:
    FilterRegistry() = default;
    std::vector<std::unique_ptr<FilterBase>> m_filters;
};

// Registers every built-in filter exactly once. Safe to call repeatedly.
void registerBuiltinFilters();

// Applies a filter to the document's active layer, restricted to the current
// selection when one exists. Does NOT push a history state.
void applyFilterToActiveLayer(Core::Document& doc, const FilterBase& filter, const ParamMap& params);

} // namespace PhotoStudio::Filters
