#include "core/layer.hpp"

#include <algorithm>
#include <atomic>

namespace PhotoStudio::Core {

Layer::Layer(u32 width, u32 height, LayerType type, const cv::Scalar& fill)
    : m_id(nextId()), m_type(type)
{
    m_pixels = cv::Mat(static_cast<int>(height), static_cast<int>(width), CV_8UC4, fill);
}

Layer_Ptr Layer::clone() const
{
    auto copy = std::make_shared<Layer>(width(), height(), m_type);
    copy->m_id = m_id; // History snapshots must preserve identity.
    copy->m_name = m_name;
    copy->m_pixels = m_pixels.clone();
    copy->m_mask = m_mask.empty() ? cv::Mat() : m_mask.clone();
    copy->m_offset = m_offset;
    copy->m_opacity = m_opacity;
    copy->m_blendMode = m_blendMode;
    copy->m_visible = m_visible;
    copy->m_locked = m_locked;
    return copy;
}

void Layer::setOpacity(f32 opacity)
{
    m_opacity = std::clamp(opacity, 0.0f, 1.0f);
}

u32 Layer::nextId()
{
    static std::atomic<u32> counter{1};
    return counter.fetch_add(1);
}

} // namespace PhotoStudio::Core
