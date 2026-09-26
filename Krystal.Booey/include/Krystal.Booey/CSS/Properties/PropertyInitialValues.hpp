#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css
{
  class Value;

  enum class PropertyId : uint16;
  enum class ValueId : uint16;

  KRYS_NODISCARD inline bool IsInitialValueForLonghand(PropertyId property, const Value &value) noexcept;

  KRYS_NODISCARD inline utf8_stringview InitialValueTextForLonghand(PropertyId property) noexcept;

  KRYS_NODISCARD inline ValueId InitialValueIDForLonghand(PropertyId property) noexcept;
}