#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class ValueSeparator : uint8
  {
    SpaceSeparator,
    CommaSeparator,
    SlashSeparator
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::css::ValueSeparator, 3uz);