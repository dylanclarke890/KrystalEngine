#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML::Attributes
{
  /// @see A general-purpose enum for all attribute states defined by the HTML spec. May split this out later.
  enum class AttributeState : uint8
  {
    // General-purpose states
    Undefined,
    Auto,
    Inherit,
    Default,
    True,
    False,
    None,
    Yes,
    No,
    On,
    Off,

    // Specific states for certain attributes
    LTR,
    RTL,
    Hidden,
    NotHidden,
    UntilFound,
    Sentences,
    Words,
    Characters,
    High,
    Low,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::Attributes::AttributeState, 13uz);