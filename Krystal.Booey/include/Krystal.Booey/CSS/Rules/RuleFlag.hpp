#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  enum class RuleFlag : uint8
  {
    None = 0,
    IsGroupingRule = 1 << 0,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::RuleFlag, 2uz);