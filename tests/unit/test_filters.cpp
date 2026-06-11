#include "filters/color/color_filters.hpp"
#include "filters/blur/blur_filters.hpp"
#include "filters/filter_registry.hpp"

#include <gtest/gtest.h>

using namespace PhotoStudio;
using namespace PhotoStudio::Filters;

namespace {

cv::Mat gradientImage()
{
    cv::Mat img(32, 32, CV_8UC4);
    for (int y = 0; y < 32; ++y)
        for (int x = 0; x < 32; ++x)
            img.at<cv::Vec4b>(y, x) = {static_cast<u8>(x * 8), static_cast<u8>(y * 8), 128, 255};
    return img;
}

} // namespace

TEST(Filters, RegistryContainsBuiltins)
{
    registerBuiltinFilters();
    auto& registry = FilterRegistry::instance();
    EXPECT_GE(registry.names().size(), 30u);
    EXPECT_NE(registry.find("Gaussian Blur"), nullptr);
    EXPECT_NE(registry.find("Curves"), nullptr);
    EXPECT_NE(registry.find("Invert"), nullptr);
}

TEST(Filters, InvertTwiceIsIdentity)
{
    cv::Mat img = gradientImage();
    const cv::Mat original = img.clone();

    InvertFilter invert;
    invert.apply(img, {});
    invert.apply(img, {});

    EXPECT_EQ(cv::norm(img, original, cv::NORM_INF), 0.0);
}

TEST(Filters, GaussianBlurSmoothsImage)
{
    cv::Mat img = gradientImage();
    img.at<cv::Vec4b>(16, 16) = {255, 255, 255, 255}; // spike

    GaussianBlurFilter blur;
    blur.apply(img, {{"radius", 5.0f}});

    const cv::Vec4b px = img.at<cv::Vec4b>(16, 16);
    EXPECT_LT(px[2], 255); // spike got spread out
    EXPECT_EQ(img.type(), CV_8UC4);
}

TEST(Filters, CurvesLutIsMonotonicAtIdentity)
{
    u8 lut[256];
    CurvesFilter::buildLut(0.0f, 0.0f, 0.0f, lut);
    EXPECT_EQ(lut[0], 0);
    EXPECT_EQ(lut[255], 255);
    for (int i = 1; i < 256; ++i)
        EXPECT_GE(lut[i] + 2, lut[i - 1]); // allow tiny spline wiggle
}

TEST(Filters, DesaturateProducesGray)
{
    cv::Mat img = gradientImage();
    DesaturateFilter desaturate;
    desaturate.apply(img, {});

    const cv::Vec4b px = img.at<cv::Vec4b>(10, 20);
    EXPECT_EQ(px[0], px[1]);
    EXPECT_EQ(px[1], px[2]);
}

TEST(Filters, FilterRespectsSelection)
{
    Core::Document doc(16, 16, 72, cv::Scalar(200, 200, 200, 255));

    cv::Mat sel(16, 16, CV_8UC1, cv::Scalar(0));
    sel(cv::Rect(0, 0, 8, 16)).setTo(255);
    doc.setSelection(sel);

    InvertFilter invert;
    applyFilterToActiveLayer(doc, invert, {});

    const cv::Mat& pixels = doc.activeLayer()->pixels();
    EXPECT_EQ(pixels.at<cv::Vec4b>(8, 2)[0], 55);   // inside selection: inverted
    EXPECT_EQ(pixels.at<cv::Vec4b>(8, 12)[0], 200); // outside: untouched
}
