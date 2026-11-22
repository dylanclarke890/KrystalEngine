#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Core/Strings/CodePointIterator.hpp"
#include "Krystal.Core/Strings/HexNumber.hpp"
#include "Krystal.Core/Strings/Unicode.hpp"
#include <algorithm>

namespace krys::boo::css
{
  namespace
  {
    /// @see https://drafts.csswg.org/cssom/#escape-a-character
    void AppendEscapedCharacter(StringBuilder &appendTo, char32 c) noexcept
    {
      appendTo.Append(u8'\\', c);
    }

    /// @see https://drafts.csswg.org/cssom/#escape-a-character-as-code-point
    void AppendEscapedCodePoint(StringBuilder &appendTo, char32 c) noexcept
    {
      appendTo.Append(u8'\\', Hex(static_cast<uint32>(c), HexConversionMode::Lowercase), u8' ');
    }

    /// @brief Whether @p c is in [\1-\1f] or is \7f, which serialising escapes as a code point.
    KRYS_NODISCARD constexpr bool IsEscapedControl(char32 c) noexcept
    {
      return (c >= 0x1 && c <= 0x1F) || c == 0x7F;
    }

    /// @brief Appends @p identifier as an identifier, applying the rules for how one starts if
    /// @p checkStart.
    /// @see https://drafts.csswg.org/cssom/#serialize-an-identifier
    void AppendIdentifier(const CSSOMString &identifier, StringBuilder &appendTo, bool checkStart) noexcept
    {
      size_t index = 0;
      auto firstIsHyphen = false;
      for (text::CodePointIterator it {utf8_stringview {identifier}}; !it.AtEnd(); ++index)
      {
        auto c = *it;
        auto isLast = (++it).AtEnd();
        auto isFirst = checkStart && index == 0;
        auto isSecond = checkStart && index == 1;
        if (!c)
        {
          appendTo.Append(text::UnicodeReplacementChar);
        }
        // A digit can't start an identifier, nor follow a hyphen that does.
        else if (IsEscapedControl(c) || (text::IsASCIIDigit(c) && (isFirst || (isSecond && firstIsHyphen))))
        {
          AppendEscapedCodePoint(appendTo, c);
        }
        // A hyphen alone isn't an identifier.
        else if (c == U'-' && isFirst && isLast)
        {
          AppendEscapedCharacter(appendTo, c);
        }
        else if (c >= 0x80 || c == U'-' || c == U'_' || text::IsASCIIAlphanumeric(c))
        {
          appendTo.Append(c);
        }
        else
        {
          AppendEscapedCharacter(appendTo, c);
        }
        if (isFirst)
        {
          firstIsHyphen = c == U'-';
        }
      }
    }

    /// @brief Whether @p name is an identifier as it is, with no escapes: an optional hyphen, then a hyphen
    /// or a name-start code point, then name code points. Non-ASCII code points are name code points, so the
    /// UTF-8 can be checked a byte at a time.
    /// @see https://drafts.csswg.org/css-syntax/#would-start-an-identifier
    KRYS_NODISCARD bool IsIdentifierAsWritten(CSSOMStringView name) noexcept
    {
      if (name.starts_with(u8'-'))
      {
        name.remove_prefix(1);
        if (name.empty() || !(name.front() == u8'-' || IsNameStartCodePoint(name.front())))
        {
          return false;
        }
      }
      else if (name.empty() || !IsNameStartCodePoint(name.front()))
      {
        return false;
      }
      return std::ranges::all_of(name, [](char8 c) { return IsNameCodePoint(c); });
    }
  }

  void SerialiseIdentifier(const CSSOMString &identifier, StringBuilder &appendTo) noexcept
  {
    AppendIdentifier(identifier, appendTo, true);
  }

  void SerialiseName(const CSSOMString &name, StringBuilder &appendTo) noexcept
  {
    AppendIdentifier(name, appendTo, false);
  }

  void SerialiseString(const CSSOMString &string, StringBuilder &appendTo) noexcept
  {
    appendTo.Append(u8'"');
    for (text::CodePointIterator it {utf8_stringview {string}}; !it.AtEnd(); ++it)
    {
      auto c = *it;
      if (!c)
      {
        appendTo.Append(text::UnicodeReplacementChar);
      }
      else if (IsEscapedControl(c))
      {
        AppendEscapedCodePoint(appendTo, c);
      }
      else if (c == U'"' || c == U'\\')
      {
        AppendEscapedCharacter(appendTo, c);
      }
      else
      {
        appendTo.Append(c);
      }
    }
    appendTo.Append(u8'"');
  }

  CSSOMString SerialiseString(const CSSOMString &string) noexcept
  {
    StringBuilder builder;
    SerialiseString(string, builder);
    return krys::move(builder).ToString();
  }

  CSSOMString SerialiseUrl(const CSSOMString &url) noexcept
  {
    StringBuilder builder;
    builder.Append(u8"url(");
    SerialiseString(url, builder);
    builder.Append(u8')');
    return krys::move(builder).ToString();
  }

  CSSOMString SerialiseFontFamily(const CSSOMString &name) noexcept
  {
    return IsIdentifierAsWritten(name) ? name : SerialiseString(name);
  }
}
