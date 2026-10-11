#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css
{
  class Value;
  class CSSOMString;

  enum class PropertyId : uint16;
  enum class ValueId : uint16;

  KRYS_NODISCARD inline bool IsInitialValueForLonghand(PropertyId property, const Value &value) noexcept;

  KRYS_NODISCARD inline CSSOMString InitialValueTextForLonghand(PropertyId property) noexcept;

  KRYS_NODISCARD inline ValueId InitialValueIdForLonghand(PropertyId property) noexcept;
}