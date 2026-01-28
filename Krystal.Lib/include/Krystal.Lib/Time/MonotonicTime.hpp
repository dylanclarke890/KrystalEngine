#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include <chrono>

namespace Krys
{
  struct MonotonicTime
  {
    static constexpr bool is_steady = true;

    using duration = std::chrono::steady_clock::duration;
    using rep = duration::rep;
    using period = duration::period;
    using time_point = std::chrono::time_point<std::chrono::steady_clock, duration>;

    KRYS_NODISCARD static duration Now() noexcept
    {
      return std::chrono::steady_clock::now().time_since_epoch();
    }
  };
}