#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

// Helper functions for converting from CSSValues to text.
namespace krys::boo::css
{
  // Common serializing methods. See: http://dev.w3.org/csswg/cssom/#common-serializing-idioms
  void SerialiseIdentifier(const CSSOMString &identifier, StringBuilder &appendTo,
                           bool skipStartChecks = false) noexcept;

  void SerialiseString(const CSSOMString &, StringBuilder &appendTo) noexcept;

  KRYS_NODISCARD CSSOMString SerialiseString(const CSSOMString &value) noexcept;

  KRYS_NODISCARD CSSOMString SerialiseURL(const CSSOMString &value) noexcept;

  KRYS_NODISCARD CSSOMString SerialiseFontFamily(const CSSOMString &value) noexcept;
}