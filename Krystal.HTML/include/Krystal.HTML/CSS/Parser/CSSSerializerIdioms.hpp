#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Text/StringConversion.hpp"
#include <format>

namespace Krys::HTML
{
  /// @see https://drafts.csswg.org/cssom/#common-serializing-idioms
  class CSSSerializerIdioms
  {
  public:
    /// @see https://drafts.csswg.org/cssom/#escape-a-character
    static void EscapeCharacter(char32 character, utf32_string &output) noexcept
    {
      output.push_back(U'\\');
      output.push_back(character);
    }

    /// @see https://drafts.csswg.org/cssom/#escape-a-character-as-code-point
    static void EscapeCharacterAsCodePoint(char32 character, utf32_string &output) noexcept
    {
      output.push_back(U'\\');

      auto hex = std::format("{:x}", static_cast<int32>(character));
      output.insert(output.end(), hex.begin(), hex.end());
    }

    /// @see https://drafts.csswg.org/cssom/#serialize-an-identifier
    static void SerializeIdentifier(CSSOMStringView identifier, utf32_string &output) noexcept
    {
      // TODO: need to iterate over the codepoints in the identifier instead of doing this
      auto utf32Identifier = Krys::Text::ConvertToUTF32(identifier);

      bool isFirstCharacter = true;
      bool isSecondCharacter = false;
      for (char32 codepoint : utf32Identifier)
      {
        // If the character is NULL (U+0000), then the REPLACEMENT CHARACTER (U+FFFD).
        if (codepoint == U'\0')
        {
          output += U'\uFFFD';
        }
        // If the character is in the range [\1-\1f] (U+0001 to U+001F) or is U+007F, then the character
        // escaped as code point.
        else if ((codepoint >= U'\1' && codepoint <= U'\x1F') || codepoint == U'\x7F')
        {
          EscapeCharacterAsCodePoint(codepoint, output);
        }
        // If the character is the first character and is in the range [0-9] (U+0030 to U+0039), then the
        // character escaped as code point.
        else if (isFirstCharacter && (codepoint >= U'0' && codepoint <= U'9'))
        {
          EscapeCharacterAsCodePoint(codepoint, output);
        }
        // If the character is the second character and is in the range [0-9] (U+0030 to U+0039) and the first
        // character is a "-" (U+002D), then the character escaped as code point.
        else if (isSecondCharacter && (codepoint >= U'0' && codepoint <= U'9') && utf32Identifier[0] == U'-')
        {
          EscapeCharacterAsCodePoint(codepoint, output);
        }
        // If the character is the first character and is a "-" (U+002D), and there is no second character,
        // then the escaped character.
        else if (isFirstCharacter && codepoint == U'-' && utf32Identifier.size() == 1)
        {
          EscapeCharacter(codepoint, output);
        }
        // If the character is not handled by one of the above rules and is greater than or equal to U+0080,
        // is "-" (U+002D) or "_" (U+005F), or is in one of the ranges [0-9] (U+0030 to U+0039), [A-Z] (U+0041
        // to U+005A), or [a-z] (U+0061 to U+007A), then the character itself.
        else if (codepoint >= U'\x80' || codepoint == U'-' || codepoint == U'_'
                 || (codepoint >= U'0' && codepoint <= U'9') || (codepoint >= U'A' && codepoint <= U'Z')
                 || (codepoint >= U'a' && codepoint <= U'z'))
        {
          output.push_back(codepoint);
        }
        // Otherwise, the escaped character.
        else
        {
          EscapeCharacter(codepoint, output);
        }

        if (isFirstCharacter)
        {
          isFirstCharacter = false;
          isSecondCharacter = true;
        }
        else if (isSecondCharacter)
        {
          isSecondCharacter = false;
        }
      }
    }

    /// @see https://drafts.csswg.org/cssom/#css-serialize-a-function
    KRYS_NODISCARD static CSSOMString SerializeFunction(const CSSOMString &func) noexcept
    {
      // TODO:
      return {};
    }

    /// @see https://drafts.csswg.org/cssom/#serialize-a-string
    KRYS_NODISCARD static void SerializeString(CSSOMStringView str, utf32_string &output) noexcept
    {
      // TODO: need to iterate over the codepoints in the identifier instead of doing this
      auto utf32Identifier = Krys::Text::ConvertToUTF32(str);

      output.push_back(U'"');
      for (char32 codepoint : utf32Identifier)
      {
        // If the character is NULL (U+0000), then the REPLACEMENT CHARACTER (U+FFFD).
        if (codepoint == U'\0')
        {
          output += U'\uFFFD';
        }
        // If the character is in the range [\1-\1f] (U+0001 to U+001F) or is U+007F, then the character
        // escaped as code point.
        else if ((codepoint >= U'\1' && codepoint <= U'\x1F') || codepoint == U'\x7F')
        {
          EscapeCharacterAsCodePoint(codepoint, output);
        }
        // If the character is '"' (U+0022) or "\" (U+005C), the escaped character.
        else if (codepoint == U'"' || codepoint == U'\\')
        {
          EscapeCharacter(codepoint, output);
        }
        // Otherwise, the character itself.
        else
        {
          output.push_back(codepoint);
        }
      }
      output.push_back(U'"');
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