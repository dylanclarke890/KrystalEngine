#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#enumdef-shadowrootmode
  enum class ShadowRootMode : uint8
  {
    Open,
    Closed
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::ShadowRootMode, 2u);