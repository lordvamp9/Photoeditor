#include "io/project_serializer.hpp"

#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>

using namespace PhotoStudio;
using namespace PhotoStudio::Core;

TEST(ProjectIO, SaveAndLoadRoundTrip)
{
    const std::string path =
        (std::filesystem::temp_directory_path() / "photostudio_test.psproj").string();

    {
        Document doc(40, 30, 96, cv::Scalar(255, 0, 0, 255));
        doc.setName("RoundTrip");

        auto layer = std::make_shared<Layer>(40, 30);
        layer->pixels().setTo(cv::Scalar(0, 255, 0, 128));
        layer->setName("Green");
        layer->setOpacity(0.5f);
        layer->setBlendMode(BlendMode::Multiply);
        layer->setOffset({3, -2});
        doc.addLayer(layer);

        ASSERT_TRUE(IO::saveProject(doc, path));
    }

    auto loaded = IO::loadProject(path);
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->width(), 40u);
    EXPECT_EQ(loaded->height(), 30u);
    EXPECT_EQ(loaded->dpi(), 96u);
    EXPECT_EQ(loaded->name(), "RoundTrip");
    ASSERT_EQ(loaded->layers().size(), 2u);

    const auto& green = loaded->layers()[1];
    EXPECT_EQ(green->name(), "Green");
    EXPECT_NEAR(green->opacity(), 0.5f, 0.01f);
    EXPECT_EQ(green->blendMode(), BlendMode::Multiply);
    EXPECT_EQ(green->offset(), cv::Point(3, -2));

    const cv::Vec4b px = green->pixels().at<cv::Vec4b>(0, 0);
    EXPECT_EQ(px[1], 255);
    EXPECT_EQ(px[3], 128);

    std::remove(path.c_str());
}
