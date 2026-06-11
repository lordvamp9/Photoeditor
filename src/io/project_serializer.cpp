#include "io/project_serializer.hpp"

#include "utils/logger.hpp"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>

namespace PhotoStudio::IO {

namespace {

constexpr char kMagic[4] = {'P', 'S', 'P', 'J'};
constexpr u32 kVersion = 1;

void writeU32(std::ostream& out, u32 v)
{
    out.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

void writeU64(std::ostream& out, u64 v)
{
    out.write(reinterpret_cast<const char*>(&v), sizeof(v));
}

u32 readU32(std::istream& in)
{
    u32 v = 0;
    in.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

u64 readU64(std::istream& in)
{
    u64 v = 0;
    in.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

std::vector<u8> encodePng(const cv::Mat& mat)
{
    if (mat.empty())
        return {};
    std::vector<u8> buffer;
    cv::Mat bgra;
    if (mat.channels() == 4)
        cv::cvtColor(mat, bgra, cv::COLOR_RGBA2BGRA);
    else
        bgra = mat;
    cv::imencode(".png", bgra, buffer);
    return buffer;
}

cv::Mat decodePng(const std::vector<u8>& blob, bool fourChannels)
{
    if (blob.empty())
        return {};
    cv::Mat decoded = cv::imdecode(blob, cv::IMREAD_UNCHANGED);
    if (decoded.empty())
        return {};
    if (fourChannels && decoded.channels() == 4) {
        cv::Mat rgba;
        cv::cvtColor(decoded, rgba, cv::COLOR_BGRA2RGBA);
        return rgba;
    }
    return decoded;
}

// Minimal JSON escaping for layer names.
std::string escapeJson(const std::string& s)
{
    std::string out;
    for (char c : s) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        default:
            out += c;
        }
    }
    return out;
}

// Tiny purpose-built parser: extracts a scalar value for "key" inside `src`,
// starting at `from`. Project files are machine-written, so this stays simple.
std::string jsonValue(const std::string& src, const std::string& key, size_t from = 0)
{
    const std::string needle = "\"" + key + "\":";
    const size_t pos = src.find(needle, from);
    if (pos == std::string::npos)
        return {};
    size_t start = pos + needle.size();
    while (start < src.size() && (src[start] == ' '))
        ++start;
    if (src[start] == '"') {
        const size_t end = src.find('"', start + 1);
        return src.substr(start + 1, end - start - 1);
    }
    size_t end = start;
    while (end < src.size() && src[end] != ',' && src[end] != '}' && src[end] != ']')
        ++end;
    return src.substr(start, end - start);
}

} // namespace

bool saveProject(const Core::Document& doc, const std::string& path)
{
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        Utils::Logger::error("Cannot write project: " + path);
        return false;
    }

    std::ostringstream json;
    json << "{\"name\":\"" << escapeJson(doc.name()) << "\",\"width\":" << doc.width()
         << ",\"height\":" << doc.height() << ",\"dpi\":" << doc.dpi()
         << ",\"activeLayer\":" << doc.layerIndex(doc.activeLayerId()) << ",\"layers\":[";
    for (size_t i = 0; i < doc.layers().size(); ++i) {
        const auto& layer = doc.layers()[i];
        if (i)
            json << ",";
        json << "{\"name\":\"" << escapeJson(layer->name()) << "\",\"offsetX\":" << layer->offset().x
             << ",\"offsetY\":" << layer->offset().y << ",\"opacity\":" << layer->opacity()
             << ",\"blend\":" << static_cast<int>(layer->blendMode())
             << ",\"visible\":" << (layer->isVisible() ? 1 : 0) << "}";
    }
    json << "]}";
    const std::string jsonStr = json.str();

    out.write(kMagic, 4);
    writeU32(out, kVersion);
    writeU32(out, static_cast<u32>(jsonStr.size()));
    out.write(jsonStr.data(), static_cast<std::streamsize>(jsonStr.size()));

    for (const auto& layer : doc.layers()) {
        const auto pixelBlob = encodePng(layer->pixels());
        writeU64(out, pixelBlob.size());
        out.write(reinterpret_cast<const char*>(pixelBlob.data()),
                  static_cast<std::streamsize>(pixelBlob.size()));

        const auto maskBlob = layer->hasMask() ? encodePng(layer->mask()) : std::vector<u8>{};
        writeU64(out, maskBlob.size());
        if (!maskBlob.empty())
            out.write(reinterpret_cast<const char*>(maskBlob.data()),
                      static_cast<std::streamsize>(maskBlob.size()));
    }

    Utils::Logger::info("Saved project " + path);
    return true;
}

Core::Document_Ptr loadProject(const std::string& path)
{
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        Utils::Logger::error("Cannot open project: " + path);
        return nullptr;
    }

    char magic[4];
    in.read(magic, 4);
    if (std::memcmp(magic, kMagic, 4) != 0) {
        Utils::Logger::error("Not a PhotoStudio project: " + path);
        return nullptr;
    }
    const u32 version = readU32(in);
    if (version > kVersion) {
        Utils::Logger::error("Unsupported project version");
        return nullptr;
    }

    const u32 jsonLen = readU32(in);
    std::string json(jsonLen, '\0');
    in.read(json.data(), jsonLen);

    const u32 width = static_cast<u32>(std::stoul(jsonValue(json, "width")));
    const u32 height = static_cast<u32>(std::stoul(jsonValue(json, "height")));
    const u32 dpi = static_cast<u32>(std::stoul(jsonValue(json, "dpi")));

    auto doc = std::make_unique<Core::Document>(width, height, dpi, cv::Scalar(0, 0, 0, 0));
    doc->setName(jsonValue(json, "name"));
    doc->setFilePath(path);

    // Walk layer entries in JSON order while reading the matching blobs.
    std::vector<Core::Layer_Ptr> layers;
    size_t cursor = json.find("\"layers\":");
    while ((cursor = json.find("{\"name\"", cursor)) != std::string::npos) {
        const std::string name = jsonValue(json, "name", cursor);
        const int offX = std::stoi(jsonValue(json, "offsetX", cursor));
        const int offY = std::stoi(jsonValue(json, "offsetY", cursor));
        const f32 opacity = std::stof(jsonValue(json, "opacity", cursor));
        const int blend = std::stoi(jsonValue(json, "blend", cursor));
        const bool visible = jsonValue(json, "visible", cursor) != "0";
        cursor += 7;

        const u64 pixelLen = readU64(in);
        std::vector<u8> pixelBlob(pixelLen);
        in.read(reinterpret_cast<char*>(pixelBlob.data()), static_cast<std::streamsize>(pixelLen));
        const u64 maskLen = readU64(in);
        std::vector<u8> maskBlob(maskLen);
        if (maskLen)
            in.read(reinterpret_cast<char*>(maskBlob.data()),
                    static_cast<std::streamsize>(maskLen));

        cv::Mat pixels = decodePng(pixelBlob, true);
        if (pixels.empty())
            continue;

        auto layer = std::make_shared<Core::Layer>(static_cast<u32>(pixels.cols),
                                                   static_cast<u32>(pixels.rows));
        layer->pixels() = pixels;
        layer->setName(name);
        layer->setOffset({offX, offY});
        layer->setOpacity(opacity);
        layer->setBlendMode(static_cast<Core::BlendMode>(blend));
        layer->setVisible(visible);
        if (maskLen) {
            cv::Mat mask = decodePng(maskBlob, false);
            if (!mask.empty())
                layer->addMask(mask);
        }
        layers.push_back(layer);
    }

    if (layers.empty()) {
        Utils::Logger::error("Project has no layers: " + path);
        return nullptr;
    }

    // Replace the default background layer with the loaded stack.
    doc->removeLayer(doc->activeLayerId());
    for (auto& layer : layers)
        doc->addLayer(layer);
    // removeLayer() refuses to delete the last layer, so drop the placeholder now.
    if (doc->layers().size() > layers.size())
        doc->removeLayer(doc->layers().front()->id());

    const std::string activeStr = jsonValue(json, "activeLayer");
    if (!activeStr.empty()) {
        const size_t idx = static_cast<size_t>(std::stoul(activeStr));
        if (idx < doc->layers().size())
            doc->setActiveLayer(doc->layers()[idx]->id());
    }

    doc->addState("Open Project");
    Utils::Logger::info("Loaded project " + path);
    return doc;
}

} // namespace PhotoStudio::IO
