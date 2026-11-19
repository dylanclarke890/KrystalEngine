#pragma once

#include "Krystal.Core/Strings/String.hpp"

namespace krys::pal::win32
{
  /// @brief Converts the last Win32 error to a human-readable string.
  string GetLastErrorAsString() noexcept;
}
