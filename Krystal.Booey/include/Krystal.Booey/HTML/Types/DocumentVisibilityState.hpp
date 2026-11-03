#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#documentvisibilitystate
  enum class DocumentVisibilityState : uint8
  {
    Visible,
    Hidden,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::DocumentVisibilityState, 2uz);