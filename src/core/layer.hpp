#pragma once

#include "core/blend_modes.hpp"
#include "core/types.hpp"

#include <opencv2/core.hpp>

#include <string>
#include <vector>

namespace PhotoStudio::Core {

enum class LayerType : u8 {
    Raster = 0,
    Adjustment,
    Fill,
    Group,
    Text,
};

class Layer {
public:
    Layer(u32 width, u32 height, LayerType type = LayerType::Raster,
          const cv::Scalar& fill = {0, 0, 0, 0});

    Layer_Ptr clone() const;

    u32 id() const { return m_id; }
    LayerType type() const { return m_type; }

    const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    f32 opacity() const { return m_opacity; }
    void setOpacity(f32 opacity);

    BlendMode blendMode() const { return m_blendMode; }
    void setBlendMode(BlendMode mode) { m_blendMode = mode; }

    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    bool isLocked() const { return m_locked; }
    void setLocked(bool locked) { m_locked = locked; }

    cv::Point offset() const { return m_offset; }
    void setOffset(cv::Point offset) { m_offset = offset; }

    u32 width() const { return static_cast<u32>(m_pixels.cols); }
    u32 height() const { return static_cast<u32>(m_pixels.rows); }

    // Pixel data: straight-alpha RGBA, CV_8UC4.
    cv::Mat& pixels() { return m_pixels; }
    const cv::Mat& pixels() const { return m_pixels; }

    // Optional 8-bit grayscale layer mask (same size as pixels).
    bool hasMask() const { return !m_mask.empty(); }
    cv::Mat& mask() { return m_mask; }
    const cv::Mat& mask() const { return m_mask; }
    void addMask(const cv::Mat& mask) { m_mask = mask.clone(); }
    void removeMask() { m_mask.release(); }

private:
    static u32 nextId();

    u32 m_id;
    LayerType m_type;
    std::string m_name = "Layer";

    cv::Mat m_pixels;
    cv::Mat m_mask;
    cv::Point m_offset{0, 0};

    f32 m_opacity = 1.0f;
    BlendMode m_blendMode = BlendMode::Normal;
    bool m_visible = true;
    bool m_locked = false;
};

} // namespace PhotoStudio::Core
