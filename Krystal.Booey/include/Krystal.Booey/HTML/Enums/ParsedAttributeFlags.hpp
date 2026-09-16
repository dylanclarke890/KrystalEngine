#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html
{
  /// @see A general-purpose enum for all attribute states defined by the HTML spec. May split this out later.
  enum class ParsedAttributeFlags : uint8
  {
    None = 0,
    HasPrefix = 1 << 1,
    IsXMLNS = 1 << 2,
    IsXML = 1 << 3,
    IsXLink = 1 << 4,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::html::ParsedAttributeFlags, 5uz);