#pragma once

#include <chrono>

namespace Krys
{
  using Nanoseconds = std::chrono::duration<double, std::nano>;
  using Microseconds = std::chrono::duration<double, std::micro>;
  using Milliseconds = std::chrono::duration<double, std::milli>;
  using Seconds = std::chrono::duration<double>;

  using namespace std::chrono_literals;
}