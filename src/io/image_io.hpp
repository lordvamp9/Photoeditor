#pragma once

#include "core/document.hpp"
#include "core/types.hpp"

#include <string>

namespace PhotoStudio::IO {

// Opens an image file (PNG/JPEG/WebP/TIFF/BMP) as a new single-layer document.
Core::Document_Ptr importDocument(const std::string& path);

// Imports an image file as a new layer on top of an existing document.
Core::Layer_Ptr importAsLayer(Core::Document& doc, const std::string& path);

// Flattens and writes the document. Format is inferred from the extension.
// `quality` applies to lossy formats (JPEG/WebP), range 1-100.
bool exportDocument(const Core::Document& doc, const std::string& path, int quality = 92);

} // namespace PhotoStudio::IO
