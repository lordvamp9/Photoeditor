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
    // Cheaper repaint covering only a document-space rectangle (x, y, w, h).
    std::function<void(int, int, int, int)> requestRepaintRegion;
    std::function<void(const std::string&)> commitHistory;
    std::function<void(const cv::Vec4b&)> pickForeground;
    cv::Vec4b foreground{0, 0, 0, 255};
    cv::Vec4b background{255, 255, 255, 255};
    // Stylus pressure in [0,1]; 1.0 for mouse input.
    f32 pressure = 1.0f;
};

// A tool option exposed in the Properties panel: a numeric slider (default),
// a named-choice combo (get/set exchange the choice index) or an on/off toggle.
struct ToolOption {
    enum class Kind : u8 { Slider, Choice, Toggle };

    std::string label;
    f32 minValue = 0.0f;
    f32 maxValue = 1.0f;
    f32 step = 1.0f;
    std::function<f32()> get;
    std::function<void(f32)> set;
    Kind kind = Kind::Slider;
    std::vector<std::string> choices{};

    static ToolOption choice(std::string label, std::vector<std::string> names,
                             std::function<f32()> get, std::function<void(f32)> set)
    {
        ToolOption o;
        o.label = std::move(label);
        o.minValue = 0.0f;
        o.maxValue = static_cast<f32>(names.size() - 1);
        o.step = 1.0f;
        o.get = std::move(get);
        o.set = std::move(set);
        o.kind = Kind::Choice;
        o.choices = std::move(names);
        return o;
    }

    static ToolOption toggle(std::string label, std::function<f32()> get,
                             std::function<void(f32)> set)
    {
        ToolOption o;
        o.label = std::move(label);
        o.maxValue = 1.0f;
        o.get = std::move(get);
        o.set = std::move(set);
        o.kind = Kind::Toggle;
        return o;
    }
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
