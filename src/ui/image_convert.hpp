#pragma once

#include "core/types.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <QImage>

namespace PhotoStudio::UI {

// Deep-copies an RGBA8 cv::Mat into a QImage.
inline QImage matToQImage(const cv::Mat& rgba)
{
    if (rgba.empty() || rgba.type() != CV_8UC4)
        return {};
    return QImage(rgba.data, rgba.cols, rgba.rows, static_cast<qsizetype>(rgba.step),
                  QImage::Format_RGBA8888)
        .copy();
}

// Converts any QImage into a straight-alpha RGBA8 cv::Mat (deep copy).
inline cv::Mat qimageToMat(const QImage& image)
{
    QImage converted = image.convertToFormat(QImage::Format_RGBA8888);
    cv::Mat view(converted.height(), converted.width(), CV_8UC4,
                 const_cast<uchar*>(converted.constBits()),
                 static_cast<size_t>(converted.bytesPerLine()));
    return view.clone();
}

} // namespace PhotoStudio::UI
