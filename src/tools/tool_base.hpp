#pragma once

#include "core/document.hpp"
#include "core/types.hpp"

#include <opencv2/core.hpp>

#include <functional>
#include <string>
#include <vector>

namespace PhotoStudio::Tools {

enum Modifier : u32 {
    ModNone = 0,
    ModShift = 1u << 0,
    ModCtrl = 1u << 1,
    ModAlt = 1u << 2,
};

// Shared context handed to tools by the UI layer. Tools stay Qt-free.
struct ToolContext {
    Core::Document* document = nullptr;
    std::function<void()> requestRepaint;
    std::function<void(const std::string&)> commitHistory;
    std::function<void(const cv::Vec4b&)> pickForeground;
    cv::Vec4b foreground{0, 0, 0, 255};
    cv::Vec4b background{255, 255, 255, 255};
};

// A numeric, slider-editable tool option (exposed in the Properties panel).
struct ToolOption {
    std::string label;
    f32 minValue;
    f32 maxValue;
    f32 step;
    std::function<f32()> get;
    std::function<void(f32)> set;
};

class ToolBase {
public:
    virtual ~ToolBase() = default;

    virtual std::string name() const = 0;
    virtual std::string iconName() const = 0;
    virtual std::vector<ToolOption> options() { return {}; }

    // Coordinates are in document space.
    virtual void onMouseDown(f32 x, f32 y, u32 mods, ToolContext& ctx) = 0;
    virtual void onMouseMove(f32 x, f32 y, u32 mods, ToolContext& ctx) = 0;
    virtual void onMouseUp(f32 x, f32 y, u32 mods, ToolContext& ctx) = 0;

    // Optional live outline (e.g. selection rectangle) drawn by the canvas.
    virtual bool hasPreviewShape() const { return false; }
    virtual std::vector<cv::Point2f> previewPolygon() const { return {}; }
};

} // namespace PhotoStudio::Tools
