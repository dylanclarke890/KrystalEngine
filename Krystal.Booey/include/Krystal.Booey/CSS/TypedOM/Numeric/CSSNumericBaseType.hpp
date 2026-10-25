#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::css
{
  /// @brief The base types a numeric value's type has an exponent for.
  /// @see https://drafts.css-houdini.org/css-typed-om/#cssnumericvalue-base-type
  enum class CSSNumericBaseType : uint8
  {
    Length,
    Angle,
    Time,
    Frequency,
    Resolution,
    Flex,
    Percent,
  };

  KRYS_NODISCARD constexpr Array<CSSNumericBaseType, 7> EachBaseType() noexcept
  {
    return {CSSNumericBaseType::Length,    CSSNumericBaseType::Angle,      CSSNumericBaseType::Time,
            CSSNumericBaseType::Frequency, CSSNumericBaseType::Resolution, CSSNumericBaseType::Flex,
            CSSNumericBaseType::Percent};
  }

  KRYS_NODISCARD constexpr utf8_stringview DebugString(CSSNumericBaseType type) noexcept
  {
    switch (type)
    {
      case CSSNumericBaseType::Length:     return u8"length";
      case CSSNumericBaseType::Angle:      return u8"angle";
      case CSSNumericBaseType::Time:       return u8"time";
      case CSSNumericBaseType::Frequency:  return u8"frequency";
      case CSSNumericBaseType::Resolution: return u8"resolution";
      case CSSNumericBaseType::Flex:       return u8"flex";
      case CSSNumericBaseType::Percent:    return u8"percent";
    }
    return u8"invalid";
  }
}
