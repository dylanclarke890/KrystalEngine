#pragma once

#include "Krystal.Core/Core.hpp"

namespace Krys::Platform::Win32
{
  /// @brief Converts the last Win32 error to a human-readable string.
  string GetLastErrorAsString() noexcept;
}