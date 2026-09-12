#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#enumdef-shadowrootmode
  enum class ShadowRootMode : uint8
  {
    Open,
    Closed
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::ShadowRootMode, 2u);