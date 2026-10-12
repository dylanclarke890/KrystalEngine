#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  struct SerializableNumber
  {
    double Value;
    CSSOMStringView Suffix;
  };

  /// @brief Appends a non-finite number as `infinity`, `-infinity` or `NaN`, multiplied by 1 of the suffix's unit
  /// if there is one (e.g. `infinity * 1px`).
  void FormatNonfiniteCSSNumberValue(CSSOMString &builder, const SerializableNumber &number) noexcept;
  KRYS_NODISCARD CSSOMString FormatNonfiniteCSSNumberValue(const SerializableNumber &number) noexcept;

  /// @brief Appends a number rounded to at most 6 decimal places, without trailing zeros, followed by its suffix.
  void FormatCSSNumberValue(CSSOMString &builder, const SerializableNumber &number) noexcept;
  KRYS_NODISCARD CSSOMString FormatCSSNumberValue(const SerializableNumber &number) noexcept;
}
