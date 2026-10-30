#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css
{
  class Value;

  enum class PropertyId : uint16;
  enum class ValueId : uint16;

  /// @brief Whether `value` is the initial value of `longhand`, including implicit initial values and the
  /// multi-part initial values of the border-image, mask-border, background-size and offset-rotate longhands.
  KRYS_NODISCARD bool IsInitialValueForLonghand(PropertyId longhand, const Value &value) noexcept;

  /// @brief The serialisation of the initial value of `longhand`.
  KRYS_NODISCARD CSSOMStringView InitialValueTextForLonghand(PropertyId longhand) noexcept;

  /// @brief The initial value of `longhand` if it is a keyword, otherwise `ValueId::Invalid`.
  KRYS_NODISCARD ValueId InitialValueIdForLonghand(PropertyId longhand) noexcept;
}
