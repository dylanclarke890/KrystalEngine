#pragma once

#include "Krystal.HTML/DOMHighResTimeStamp.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include <chrono>

namespace Krys::HTML
{
  class Performance
  {
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;

  public:
    TimePoint _timeOrigin;

  public:
    Performance() noexcept : _timeOrigin(Clock::now())
    {
    }

    KRYS_NODISCARD DOMHighResTimeStamp Now() const noexcept
    {
      const auto now = Clock::now();
      const auto elapsed = now - _timeOrigin;
      const auto milliseconds =
        std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(elapsed).count();

      return DOMHighResTimeStamp(milliseconds);
    }
  };
}