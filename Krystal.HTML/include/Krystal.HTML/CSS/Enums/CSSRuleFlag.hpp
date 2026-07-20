#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSRuleFlag : uint8
  {
    None = 0,
    IsGroupingRule = 1 << 0,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::CSSRuleFlag, 2uz);