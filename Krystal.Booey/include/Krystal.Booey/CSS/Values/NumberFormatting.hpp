#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief Appends `number` formatted for CSS: like printf("%.6f"), with trailing zeros (and a trailing
  /// decimal point) removed and "-0" written as "0". Non-finite values are written as "infinity", "-infinity"
  /// or "NaN".
  void FormatCSSNumber(StringBuilder &builder, double number) noexcept;

  /// @brief Returns `number` formatted for CSS.
  /// @see FormatCSSNumber(StringBuilder &, double)
  KRYS_NODISCARD utf8_string FormatCSSNumber(double number) noexcept;

  /// @brief Appends `number` with `significantFigures` significant digits, like ECMAScript's
  /// `Number.prototype.toPrecision`: exponential notation (`1.5e+21`) when the exponent is below -6 or at
  /// least `significantFigures`, fixed notation otherwise. Non-finite values are written as "Infinity",
  /// "-Infinity" or "NaN", and "-0" as "0".
  /// @param truncateTrailingZeros Removes trailing zeros from the digits (and a then-trailing decimal point).
  void FormatNumberFixedPrecision(StringBuilder &builder, double number, uint32 significantFigures = 6,
                                  bool truncateTrailingZeros = true) noexcept;
}
