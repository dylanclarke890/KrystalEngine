#pragma once

#include "Krystal.Booey/Css/Types/CSSOMString.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"

/// @brief The CSSOM's common serialising idioms, for writing names and strings out as CSS.
/// @see https://drafts.csswg.org/cssom/#common-serializing-idioms
namespace krys::boo::css
{
  /// @brief Appends @p identifier as a CSS identifier, escaping what can't appear in one as it is.
  /// @see https://drafts.csswg.org/cssom/#serialize-an-identifier
  void SerialiseIdentifier(const CSSOMString &identifier, StringBuilder &appendTo) noexcept;

  /// @brief Appends @p name as an identifier would be, but without the rules for how one starts: for the name
  /// of an unrestricted hash token, such as `#123`.
  void SerialiseName(const CSSOMString &name, StringBuilder &appendTo) noexcept;

  /// @brief Appends @p string as a double-quoted CSS string.
  /// @see https://drafts.csswg.org/cssom/#serialize-a-string
  void SerialiseString(const CSSOMString &string, StringBuilder &appendTo) noexcept;

  /// @see https://drafts.csswg.org/cssom/#serialize-a-string
  KRYS_NODISCARD CSSOMString SerialiseString(const CSSOMString &string) noexcept;

  /// @brief @p url as a `url()` function holding it as a string.
  /// @see https://drafts.csswg.org/cssom/#serialize-a-url
  KRYS_NODISCARD CSSOMString SerialiseUrl(const CSSOMString &url) noexcept;

  /// @brief A font family name: as it is if it's an identifier needing no escapes, otherwise as a string.
  KRYS_NODISCARD CSSOMString SerialiseFontFamily(const CSSOMString &name) noexcept;
}
