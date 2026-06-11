#include "io/image_io.hpp"

#include "utils/logger.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <algorithm>
#include <filesystem>

namespace PhotoStudio::IO {

namespace {

cv::Mat loadAsRgba(const std::string& path)
{
    cv::Mat raw = cv::imread(path, cv::IMREAD_UNCHANGED);
    if (raw.empty())
        return {};

    cv::Mat rgba;
    switch (raw.channels()) {
    case 1:
        cv::cvtColor(raw, rgba, cv::COLOR_GRAY2RGBA);
        break;
    case 3:
        cv::cvtColor(raw, rgba, cv::COLOR_BGR2RGBA);
        break;
    case 4:
        cv::cvtColor(raw, rgba, cv::COLOR_BGRA2RGBA);
        break;
    default:
        return {};
    }
    if (rgba.depth() != CV_8U)
        rgba.convertTo(rgba, CV_8UC4, 255.0 / 65535.0);
    return rgba;
}

std::string lowerExtension(const std::string& path)
{
    std::string ext = std::filesystem::path(path).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return ext;
}

} // namespace

Core::Document_Ptr importDocument(const std::string& path)
{
    cv::Mat rgba = loadAsRgba(path);
    if (rgba.empty()) {
        Utils::Logger::error("Failed to load image: " + path);
        return nullptr;
    }

    auto doc = std::make_unique<Core::Document>(static_cast<u32>(rgba.cols),
                                                static_cast<u32>(rgba.rows), 72,
                                                cv::Scalar(0, 0, 0, 0));
    auto layer = doc->activeLayer();
    layer->pixels() = rgba;
    layer->setName(std::filesystem::path(path).stem().string());
    doc->setName(std::filesystem::path(path).stem().string());
    Utils::Logger::info("Imported " + path);
    return doc;
}

Core::Layer_Ptr importAsLayer(Core::Document& doc, const std::string& path)
{
    cv::Mat rgba = loadAsRgba(path);
    if (rgba.empty()) {
        Utils::Logger::error("Failed to load image: " + path);
        return nullptr;
    }

    auto layer = std::make_shared<Core::Layer>(static_cast<u32>(rgba.cols),
                                               static_cast<u32>(rgba.rows));
    layer->pixels() = rgba;
    layer->setName(std::filesystem::path(path).stem().string());
    doc.addLayer(layer);
    return layer;
}

bool exportDocument(const Core::Document& doc, const std::string& path, int quality)
{
    cv::Mat composite = doc.renderComposite();
    const std::string ext = lowerExtension(path);

    cv::Mat out;
    std::vector<int> params;

    if (ext == ".jpg" || ext == ".jpeg") {
        // JPEG has no alpha: flatten over white.
        cv::Mat rgb(composite.size(), CV_8UC3);
        for (int y = 0; y < composite.rows; ++y) {
            const auto* src = composite.ptr<cv::Vec4b>(y);
            auto* dst = rgb.ptr<cv::Vec3b>(y);
            for (int x = 0; x < composite.cols; ++x) {
                const f32 a = src[x][3] / 255.0f;
                for (int c = 0; c < 3; ++c)
                    dst[x][c] = cv::saturate_cast<u8>(src[x][c] * a + 255.0f * (1.0f - a));
            }
        }
        cv::cvtColor(rgb, out, cv::COLOR_RGB2BGR);
        params = {cv::IMWRITE_JPEG_QUALITY, std::clamp(quality, 1, 100)};
    } else if (ext == ".webp") {
        cv::cvtColor(composite, out, cv::COLOR_RGBA2BGRA);
        params = {cv::IMWRITE_WEBP_QUALITY, std::clamp(quality, 1, 100)};
    } else {
        // PNG / TIFF / BMP keep alpha.
        cv::cvtColor(composite, out, cv::COLOR_RGBA2BGRA);
        if (ext == ".png")
            params = {cv::IMWRITE_PNG_COMPRESSION, 6};
    }

    const bool ok = cv::imwrite(path, out, params);
    if (ok)
        Utils::Logger::info("Exported " + path);
    else
        Utils::Logger::error("Export failed: " + path);
    return ok;
}

} // namespace PhotoStudio::IO
