#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  /// @see https://www.w3.org/TR/cssom-1/#common-serializing-idioms
  class CSSSerializerIdioms
  {
  public:
    /// @see https://www.w3.org/TR/cssom-1/#escape-a-character
    KRYS_NODISCARD static CSSOMString EscapeCharacter(char32 character) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://www.w3.org/TR/cssom-1/#escape-a-character-as-code-point
    KRYS_NODISCARD static CSSOMString EscapeCharacterAsCodePoint(char32 character) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://www.w3.org/TR/cssom-1/#serialize-an-identifier
    KRYS_NODISCARD static CSSOMString SerializeIdentifier(const CSSOMString &identifier) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://www.w3.org/TR/cssom-1/#serialize-a-string
    KRYS_NODISCARD static CSSOMString SerializeString(const CSSOMString &str) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://www.w3.org/TR/cssom-1/#serialize-a-url
    KRYS_NODISCARD static CSSOMString SerializeUrl(const CSSOMString &str) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://www.w3.org/TR/cssom-1/#serialize-a-local
    KRYS_NODISCARD static CSSOMString SerializeLocal(const CSSOMString &str) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://www.w3.org/TR/cssom-1/#serialize-a-comma-separated-list
    KRYS_NODISCARD static CSSOMString SerializeCommaSeparatedList(const List<CSSOMString> &str) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://www.w3.org/TR/cssom-1/#serialize-a-whitespace-separated-list
    KRYS_NODISCARD static CSSOMString SerializeWhitespaceeparatedList(const List<CSSOMString> &str) noexcept
    {
      // TODO:
      return {};
    }
  };
}