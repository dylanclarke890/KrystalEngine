#pragma once

#include <chrono>

namespace krys
{
  using nanoseconds = std::chrono::duration<double, std::nano>;
  using microseconds = std::chrono::duration<double, std::micro>;
  using milliseconds = std::chrono::duration<double, std::milli>;
  using seconds = std::chrono::duration<double>;

  using namespace std::chrono_literals;
}