#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys
{
  /// @brief Whether this is the main thread: the one the program started on, which owns the window and runs
  /// everything not handed to another thread.
  KRYS_NODISCARD bool IsMainThread() noexcept;
}
