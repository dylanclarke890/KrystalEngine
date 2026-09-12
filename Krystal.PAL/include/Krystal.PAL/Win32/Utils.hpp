#pragma once

#include "Krystal.Core/Types/String.hpp"

namespace krys::pal::win32
{
  /// @brief Converts the last Win32 error to a human-readable string.
  string GetLastErrorAsString() noexcept;
}