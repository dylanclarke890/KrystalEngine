#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.PAL/Display.hpp"

// HMONITOR, without <windows.h>.
struct HMONITOR__;

namespace krys::pal::win32
{
  /// @brief What colours @p monitor shows, from DXGI's description of its output and the system's display
  /// configuration; sRGB at 8 bits a component if they don't say.
  KRYS_NODISCARD DisplayColour ReadDisplayColour(HMONITOR__ *monitor) noexcept;
}
