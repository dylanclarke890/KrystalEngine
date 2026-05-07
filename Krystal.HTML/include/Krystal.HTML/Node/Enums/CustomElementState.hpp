#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CustomElementState : uint8
  {
    Undefined,
    Failed,
    Uncustomized,
    Custom
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CustomElementState, 4);