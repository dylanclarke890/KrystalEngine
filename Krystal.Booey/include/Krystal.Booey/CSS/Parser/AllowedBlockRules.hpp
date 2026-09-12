#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::css
{
  /// @brief Similar to `AllowedRules` but specific to the context of a block.
  enum class AllowedBlockRules : uint8
  {
    None = 0,
    AtRules = 1 << 0,
    QualifiedRules = 1 << 1,
    Declarations = 1 << 2,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::AllowedBlockRules, 4uz);
