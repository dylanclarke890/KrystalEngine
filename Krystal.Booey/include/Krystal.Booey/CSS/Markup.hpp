#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"

// Helper functions for converting from CSSValues to text.
namespace krys::boo::css
{
  // Common serializing methods. See: http://dev.w3.org/csswg/cssom/#common-serializing-idioms
  void SerializeIdentifier(const CSSOMString &identifier, CSSOMString &appendTo,
                           bool skipStartChecks = false) noexcept;

  void SerializeString(const CSSOMString &, CSSOMString &appendTo) noexcept;

  KRYS_NODISCARD CSSOMString SerializeString(const CSSOMString &value) noexcept;

  KRYS_NODISCARD CSSOMString SerializeURL(const CSSOMString &value) noexcept;

  KRYS_NODISCARD CSSOMString SerializeFontFamily(const CSSOMString &value) noexcept;
}