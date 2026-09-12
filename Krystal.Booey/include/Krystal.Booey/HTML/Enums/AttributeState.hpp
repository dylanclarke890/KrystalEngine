#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html::Attributes
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
    NoCORS,
    Anonymous,
    UseCredentials,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::Attributes::AttributeState, 24uz);