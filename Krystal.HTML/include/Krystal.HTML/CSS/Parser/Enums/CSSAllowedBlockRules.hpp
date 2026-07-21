#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @brief Similar to `CSSAllowedRules` but specific to the context of a block.
  enum class CSSAllowedBlockRules : uint8
  {
    None = 0,
    AtRules = 1 << 0,
    QualifiedRules = 1 << 1,
    Declarations = 1 << 2,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::CSSAllowedBlockRules, 4uz);
