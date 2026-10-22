#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Platform/Decimal.hpp"
#include "Krystal.Core/Attributes.hpp"
#include <limits>

namespace krys::boo::html
{
  /// @brief Whether @p character is a line feed or a carriage return.
  KRYS_NODISCARD constexpr bool IsHTMLLineBreak(char8 character) noexcept
  {
    return character == u8'\n' || character == u8'\r';
  }

  /// @brief The best representation of @p number as a floating-point number, for number-like input types.
  /// @see https://html.spec.whatwg.org/#best-representation-of-the-number-as-a-floating-point-number
  KRYS_NODISCARD dom::DOMString SerializeForNumberType(const Decimal &number) noexcept;

  /// @brief The best representation of @p number as a floating-point number: what script's `String(number)`
  /// gives.
  KRYS_NODISCARD dom::DOMString SerializeForNumberType(double number) noexcept;

  /// @brief Parses @p string as a valid floating-point number (no leading `+` or whitespace), or returns
  /// @p fallbackValue.
  /// @see https://html.spec.whatwg.org/#rules-for-parsing-floating-point-number-values
  KRYS_NODISCARD Decimal ParseToDecimalForNumberType(dom::DOMStringView string,
                                                     const Decimal &fallbackValue) noexcept;

  KRYS_NODISCARD Decimal ParseToDecimalForNumberType(dom::DOMStringView string) noexcept;

  /// @brief Parses @p string as a valid floating-point number, or returns @p fallbackValue.
  /// @see https://html.spec.whatwg.org/#rules-for-parsing-floating-point-number-values
  KRYS_NODISCARD double ParseToDoubleForNumberType(dom::DOMStringView string, double fallbackValue) noexcept;

  KRYS_NODISCARD double ParseToDoubleForNumberType(dom::DOMStringView string) noexcept;
}
