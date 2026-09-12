#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#concept-document-mode
  enum class QuirksMode : uint8
  {
    NoQuirks,
    Quirks,
    LimitedQuirks
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::dom::QuirksMode, 3uz);