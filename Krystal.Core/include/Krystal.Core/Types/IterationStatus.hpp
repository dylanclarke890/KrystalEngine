#pragma once

namespace krys
{
  /// @brief Whether a visitor wants iteration to go on or stop.
  enum class IterationStatus : bool
  {
    Continue,
    Done
  };
}