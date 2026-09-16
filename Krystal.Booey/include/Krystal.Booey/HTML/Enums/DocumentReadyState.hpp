#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#documentreadystate
  enum class DocumentReadyState : uint8
  {
    Loading,
    Interactive,
    Complete,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::DocumentReadyState, 3uz);