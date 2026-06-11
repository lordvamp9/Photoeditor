#pragma once

#include "core/document.hpp"
#include "core/types.hpp"

#include <string>

namespace PhotoStudio::IO {

// PhotoStudio project format (.psproj):
//   magic "PSPJ" | u32 version | u32 jsonLength | JSON metadata |
//   per layer: u64 pixelBlobLength | PNG blob | u64 maskBlobLength | PNG blob
// JSON carries document size/dpi and per-layer name/offset/opacity/blend/visibility.
bool saveProject(const Core::Document& doc, const std::string& path);
Core::Document_Ptr loadProject(const std::string& path);

} // namespace PhotoStudio::IO
