#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  /// @brief Appends `number` formatted for CSS: like printf("%.6f"), with trailing zeros (and a trailing
  /// decimal point) removed and "-0" written as "0". Non-finite values are written as "infinity", "-infinity"
  /// or "NaN".
  void FormatCSSNumber(utf8_string &builder, double number) noexcept;

  /// @brief Returns `number` formatted for CSS.
  /// @see FormatCSSNumber(utf8_string &, double)
  KRYS_NODISCARD utf8_string FormatCSSNumber(double number) noexcept;
}
