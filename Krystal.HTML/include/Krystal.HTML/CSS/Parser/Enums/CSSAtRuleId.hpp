#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSAtRuleId : uint8
  {
    Invalid = 0,
    Charset,
    Import,
    Namespace
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::CSSAtRuleId, 4uz);
