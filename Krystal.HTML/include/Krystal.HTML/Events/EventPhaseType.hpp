#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class EventPhaseType : uint8
  {
    NONE = 0,
    CAPTURING_PHASE = 1,
    AT_TARGET = 2,
    BUBBLING_PHASE = 3
  };
}

namespace Krys
{
  template <>
  inline constexpr underlying_t<HTML::EventPhaseType> OrdinalCount<HTML::EventPhaseType> = 4u;
}