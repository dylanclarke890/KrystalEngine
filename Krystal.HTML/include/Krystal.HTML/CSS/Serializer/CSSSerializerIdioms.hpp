#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  /// @see https://drafts.csswg.org/cssom/#common-serializing-idioms
  class CSSSerializerIdioms
  {
  public:
    /// @see https://drafts.csswg.org/cssom/#escape-a-character
    KRYS_NODISCARD static CSSOMString EscapeCharacter(char32 character) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://drafts.csswg.org/cssom/#escape-a-character-as-code-point
    KRYS_NODISCARD static CSSOMString EscapeCharacterAsCodePoint(char32 character) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://drafts.csswg.org/cssom/#serialize-an-identifier
    KRYS_NODISCARD static CSSOMString SerializeIdentifier(const CSSOMString &identifier) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://drafts.csswg.org/cssom/#css-serialize-a-function
    KRYS_NODISCARD static CSSOMString SerializeFunction(const CSSOMString &func) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://drafts.csswg.org/cssom/#serialize-a-string
    KRYS_NODISCARD static CSSOMString SerializeString(const CSSOMString &str) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://drafts.csswg.org/cssom/#serialize-a-url
    KRYS_NODISCARD static CSSOMString SerializeUrl(const CSSOMString &str) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://drafts.csswg.org/cssom/#serialize-a-local
    KRYS_NODISCARD static CSSOMString SerializeLocal(const CSSOMString &str) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://drafts.csswg.org/cssom/#serialize-a-comma-separated-list
    KRYS_NODISCARD static CSSOMString SerializeCommaSeparatedList(const List<CSSOMString> &str) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://drafts.csswg.org/cssom/#serialize-a-whitespace-separated-list
    KRYS_NODISCARD static CSSOMString SerializeWhitespaceeparatedList(const List<CSSOMString> &str) noexcept
    {
      // TODO:
      return {};
    }
  };
}