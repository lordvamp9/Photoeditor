#include "core/blend_modes.hpp"

#include <gtest/gtest.h>

using namespace PhotoStudio;
using namespace PhotoStudio::Core;

TEST(BlendModes, NormalReturnsBlendColor)
{
    const cv::Vec3f base(0.2f, 0.4f, 0.6f);
    const cv::Vec3f blend(0.9f, 0.1f, 0.5f);
    const cv::Vec3f out = blendPixel(BlendMode::Normal, base, blend);
    EXPECT_FLOAT_EQ(out[0], blend[0]);
    EXPECT_FLOAT_EQ(out[1], blend[1]);
    EXPECT_FLOAT_EQ(out[2], blend[2]);
}

TEST(BlendModes, MultiplyDarkens)
{
    const cv::Vec3f base(0.5f, 0.5f, 0.5f);
    const cv::Vec3f blend(0.5f, 0.5f, 0.5f);
    const cv::Vec3f out = blendPixel(BlendMode::Multiply, base, blend);
    EXPECT_NEAR(out[0], 0.25f, 1e-5f);
}

TEST(BlendModes, ScreenLightens)
{
    const cv::Vec3f base(0.5f, 0.5f, 0.5f);
    const cv::Vec3f blend(0.5f, 0.5f, 0.5f);
    const cv::Vec3f out = blendPixel(BlendMode::Screen, base, blend);
    EXPECT_NEAR(out[0], 0.75f, 1e-5f);
}

TEST(BlendModes, LuminosityPreservesBaseChroma)
{
    const cv::Vec3f base(1.0f, 0.0f, 0.0f);
    const cv::Vec3f blend(0.5f, 0.5f, 0.5f);
    const cv::Vec3f out = blendPixel(BlendMode::Luminosity, base, blend);
    // Red must stay the dominant channel.
    EXPECT_GT(out[0], out[1]);
    EXPECT_GT(out[0], out[2]);
}

TEST(BlendModes, CompositeFullyOpaqueSourceWins)
{
    cv::Mat dst(4, 4, CV_8UC4, cv::Scalar(0, 0, 0, 255));
    cv::Mat src(4, 4, CV_8UC4, cv::Scalar(255, 128, 64, 255));
    compositeOver(dst, src, cv::Mat(), 1.0f, BlendMode::Normal, {0, 0});

    const cv::Vec4b px = dst.at<cv::Vec4b>(2, 2);
    EXPECT_EQ(px[0], 255);
    EXPECT_EQ(px[1], 128);
    EXPECT_EQ(px[2], 64);
    EXPECT_EQ(px[3], 255);
}

TEST(BlendModes, CompositeRespectsOpacity)
{
    cv::Mat dst(2, 2, CV_8UC4, cv::Scalar(0, 0, 0, 255));
    cv::Mat src(2, 2, CV_8UC4, cv::Scalar(255, 255, 255, 255));
    compositeOver(dst, src, cv::Mat(), 0.5f, BlendMode::Normal, {0, 0});

    const cv::Vec4b px = dst.at<cv::Vec4b>(0, 0);
    EXPECT_NEAR(px[0], 128, 2);
}

TEST(BlendModes, AllNamesRegistered)
{
    EXPECT_EQ(blendModeNames().size(), static_cast<size_t>(BlendMode::Count));
}
