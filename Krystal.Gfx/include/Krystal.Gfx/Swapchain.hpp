#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Utils/NativeHandle.hpp"
#include "Krystal.Gfx/Formats.hpp"

namespace krys::gfx
{
  /// @brief The images a window presents.
  struct SwapchainDesc
  {
    /// @brief The native window (an `HWND` on Windows, for example).
    NativeHandle Window {};
    uint32 Width {0u};
    uint32 Height {0u};
    TextureFormat Format {TextureFormat::RGBA8Srgb};
    bool VSync {true};
    utf8_stringview DebugName {};
  };
}
