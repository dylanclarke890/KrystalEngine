#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class CSSSelectorFlag : uint8
  {
    None = 0,
    CaseInsensitiveAttributeValueMatching = 1 << 0,
    IsLastSelectorInSequence = 1 << 1
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::CSSSelectorFlag, 2uz);