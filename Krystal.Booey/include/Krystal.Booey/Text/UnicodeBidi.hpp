#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  enum class UnicodeBidi : uint8
  {
    Normal,
    Embed,
    Override,
    Isolate,
    Plaintext,
    IsolateOverride,
  };

  KRYS_NODISCARD constexpr bool IsIsolated(UnicodeBidi unicodeBidi) noexcept
  {
    return unicodeBidi == UnicodeBidi::Isolate || unicodeBidi == UnicodeBidi::IsolateOverride
           || unicodeBidi == UnicodeBidi::Plaintext;
  }

  KRYS_NODISCARD constexpr bool IsOverride(UnicodeBidi unicodeBidi) noexcept
  {
    return unicodeBidi == UnicodeBidi::Override || unicodeBidi == UnicodeBidi::IsolateOverride;
  }
}
