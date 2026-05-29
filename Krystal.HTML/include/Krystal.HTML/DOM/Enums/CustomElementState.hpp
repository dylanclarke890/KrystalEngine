#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#concept-element-custom-element-state
  enum class CustomElementState : uint8
  {
    Undefined,
    Failed,
    Uncustomized,
    Precustomized,
    Custom
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CustomElementState, 5uz);