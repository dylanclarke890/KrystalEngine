#pragma once

#include "Krystal.Core/Time/Duration.hpp"

namespace krys::web
{
  /// @brief A time in milliseconds, relative to a time origin (usually the window's).
  /// @see https://w3c.github.io/hr-time/#dom-domhighrestimestamp
  using DOMHighResTimeStamp = milliseconds;
}
