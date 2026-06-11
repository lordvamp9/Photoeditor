#include "core/document.hpp"

#include <gtest/gtest.h>

using namespace PhotoStudio;
using namespace PhotoStudio::Core;

TEST(Document, StartsWithBackgroundLayer)
{
    Document doc(100, 80);
    EXPECT_EQ(doc.width(), 100u);
    EXPECT_EQ(doc.height(), 80u);
    ASSERT_EQ(doc.layers().size(), 1u);
    EXPECT_EQ(doc.layers().front()->name(), "Background");
}

TEST(Document, AddAndRemoveLayers)
{
    Document doc(64, 64);
    auto layer = std::make_shared<Layer>(64, 64);
    doc.addLayer(layer);
    EXPECT_EQ(doc.layers().size(), 2u);
    EXPECT_EQ(doc.activeLayerId(), layer->id());

    doc.removeLayer(layer->id());
    EXPECT_EQ(doc.layers().size(), 1u);

    // The last layer can never be removed.
    doc.removeLayer(doc.layers().front()->id());
    EXPECT_EQ(doc.layers().size(), 1u);
}

TEST(Document, UndoRedoRestoresPixels)
{
    Document doc(8, 8, 72, cv::Scalar(255, 255, 255, 255));
    auto layer = doc.activeLayer();
    layer->pixels().setTo(cv::Scalar(0, 0, 0, 255));
    doc.markDirty();
    doc.addState("Paint Black");

    EXPECT_TRUE(doc.canUndo());
    doc.undo();
    const cv::Vec4b before = doc.activeLayer()->pixels().at<cv::Vec4b>(0, 0);
    EXPECT_EQ(before[0], 255);

    EXPECT_TRUE(doc.canRedo());
    doc.redo();
    const cv::Vec4b after = doc.activeLayer()->pixels().at<cv::Vec4b>(0, 0);
    EXPECT_EQ(after[0], 0);
}

TEST(Document, SelectionOps)
{
    Document doc(16, 16);
    EXPECT_FALSE(doc.hasSelection());

    doc.selectAll();
    EXPECT_TRUE(doc.hasSelection());
    EXPECT_EQ(cv::countNonZero(doc.selection()), 16 * 16);

    doc.invertSelection();
    EXPECT_FALSE(doc.hasSelection()); // fully empty selections collapse to none

    cv::Mat half(16, 16, CV_8UC1, cv::Scalar(0));
    half(cv::Rect(0, 0, 8, 16)).setTo(255);
    doc.setSelection(half);
    doc.invertSelection();
    EXPECT_EQ(cv::countNonZero(doc.selection()), 8 * 16);
}

TEST(Document, MergeDownCombinesLayers)
{
    Document doc(10, 10, 72, cv::Scalar(255, 0, 0, 255));
    auto top = std::make_shared<Layer>(10, 10);
    top->pixels().setTo(cv::Scalar(0, 255, 0, 255));
    doc.addLayer(top);

    ASSERT_TRUE(doc.mergeDown(top->id()));
    EXPECT_EQ(doc.layers().size(), 1u);

    const cv::Vec4b px = doc.renderComposite().at<cv::Vec4b>(5, 5);
    EXPECT_EQ(px[1], 255); // top layer color wins
}

TEST(Document, CompositeRespectsVisibility)
{
    Document doc(10, 10, 72, cv::Scalar(255, 255, 255, 255));
    auto top = std::make_shared<Layer>(10, 10);
    top->pixels().setTo(cv::Scalar(0, 0, 0, 255));
    doc.addLayer(top);

    cv::Vec4b px = doc.renderComposite().at<cv::Vec4b>(5, 5);
    EXPECT_EQ(px[0], 0);

    top->setVisible(false);
    doc.markDirty();
    px = doc.renderComposite().at<cv::Vec4b>(5, 5);
    EXPECT_EQ(px[0], 255);
}
