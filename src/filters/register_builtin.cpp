#include "filters/artistic/artistic_filters.hpp"
#include "filters/blur/blur_filters.hpp"
#include "filters/color/color_filters.hpp"
#include "filters/detail/detail_filters.hpp"
#include "filters/distortion/distortion_filters.hpp"
#include "filters/filter_registry.hpp"
#include "filters/render/render_filters.hpp"

namespace PhotoStudio::Filters {

void registerBuiltinFilters()
{
    static bool done = false;
    if (done)
        return;
    done = true;

    auto& registry = FilterRegistry::instance();

    // Adjustments
    registry.add(std::make_unique<BrightnessContrastFilter>());
    registry.add(std::make_unique<LevelsFilter>());
    registry.add(std::make_unique<CurvesFilter>());
    registry.add(std::make_unique<ExposureFilter>());
    registry.add(std::make_unique<HslFilter>());
    registry.add(std::make_unique<ColorBalanceFilter>());
    registry.add(std::make_unique<VibranceFilter>());
    registry.add(std::make_unique<InvertFilter>());
    registry.add(std::make_unique<DesaturateFilter>());
    registry.add(std::make_unique<SepiaFilter>());
    registry.add(std::make_unique<PosterizeFilter>());
    registry.add(std::make_unique<ThresholdFilter>());

    // Blur
    registry.add(std::make_unique<GaussianBlurFilter>());
    registry.add(std::make_unique<BoxBlurFilter>());
    registry.add(std::make_unique<MotionBlurFilter>());
    registry.add(std::make_unique<SurfaceBlurFilter>());

    // Detail
    registry.add(std::make_unique<SharpenFilter>());
    registry.add(std::make_unique<UnsharpMaskFilter>());
    registry.add(std::make_unique<HighPassFilter>());
    registry.add(std::make_unique<ClarityFilter>());
    registry.add(std::make_unique<ReduceNoiseFilter>());

    // Artistic
    registry.add(std::make_unique<OilPaintFilter>());
    registry.add(std::make_unique<WatercolorFilter>());
    registry.add(std::make_unique<PencilSketchFilter>());
    registry.add(std::make_unique<CartoonFilter>());
    registry.add(std::make_unique<PixelateFilter>());
    registry.add(std::make_unique<VignetteFilter>());
    registry.add(std::make_unique<AddNoiseFilter>());

    // Distort
    registry.add(std::make_unique<PinchFilter>());
    registry.add(std::make_unique<SwirlFilter>());
    registry.add(std::make_unique<RippleFilter>());
    registry.add(std::make_unique<SphereFilter>());

    // Render
    registry.add(std::make_unique<CloudsFilter>());
    registry.add(std::make_unique<GradientMapFilter>());
}

} // namespace PhotoStudio::Filters
