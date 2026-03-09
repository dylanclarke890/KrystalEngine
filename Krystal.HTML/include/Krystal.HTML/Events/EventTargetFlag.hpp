#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class EventTargetFlag : uint8
  {
    None = 0,
    IsNode = 1 << 0,
    IsConnected = 1 << 1,
    IsInShadowTree = 1 << 2,
    IsWindow = 1 << 3,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::EventTargetFlag, 5u)