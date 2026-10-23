#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#enumdef-shadowrootmode
  enum class ShadowRootMode : uint8
  {
    Open,
    Closed,
    /// @brief A shadow root the user agent attaches for an element's insides (a form control's), which
    /// pages can't see.
    UserAgent
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::ShadowRootMode, 3u);