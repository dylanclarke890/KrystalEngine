#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::CustomElementState, 5uz);