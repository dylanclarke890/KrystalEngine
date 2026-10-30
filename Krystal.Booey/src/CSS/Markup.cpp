#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"
#include "Krystal.Core/Text/HexNumber.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Text/Unicode.hpp"

namespace krys::boo::css
{
  namespace
  {
    // FIXME: Replace with a shared UTF-8 code point iterator once one exists in Krystal.Core/Text.
    template <typename F>
    void ForEachCodePoint(CSSOMStringView string, F &&callback) noexcept
    {
      auto current = string.begin();
      while (current != string.end())
      {
        auto result = krys::text::UTF8::DecodeOne(current, string.end());
        current += result.CodeUnitsConsumed;
        callback(result.Output, current == string.end());
      }
    }

    void AppendCodePoint(char32 c, StringBuilder &appendTo) noexcept
    {
      appendTo.Append(c);
    }

    /// @brief "ident" from the CSS tokenizer, minus backslash-escape sequences.
    KRYS_NODISCARD bool IsCSSTokenizerIdentifier(CSSOMStringView string) noexcept
    {
      if (string.empty())
      {
        return false;
      }

      // -?
      while (!string.empty() && string.front() == u8'-')
      {
        string.remove_prefix(1);
      }

      // {nmstart}
      // Any non-ASCII code point is a name start code point, so checking the lead byte is enough.
      if (string.empty() || !IsNameStartCodePoint(string.front()))
      {
        return false;
      }
      string.remove_prefix(1);

      // {nmchar}*
      // Continuation bytes are non-ASCII, so they pass IsNameCodePoint too.
      while (!string.empty() && IsNameCodePoint(string.front()))
      {
        string.remove_prefix(1);
      }

      return string.empty();
    }

    void SerialiseCharacter(char32 c, StringBuilder &appendTo) noexcept
    {
      appendTo.Append(u8'\\');
      AppendCodePoint(c, appendTo);
    }

    void SerialiseCharacterAsCodePoint(char32 c, StringBuilder &appendTo) noexcept
    {
      appendTo.Append(u8'\\', Hex(static_cast<uint32>(c), HexConversionMode::Lowercase), u8' ');
    }
  }

  void SerialiseIdentifier(const CSSOMString &identifier, StringBuilder &appendTo,
                           bool skipStartChecks) noexcept
  {
    bool isFirst = !skipStartChecks;
    bool isSecond = false;
    bool isFirstCharHyphen = false;
    ForEachCodePoint(identifier,
                     [&](char32 c, bool isLast)
                     {
                       if (!c)
                       {
                         AppendCodePoint(krys::text::UnicodeReplacementChar, appendTo);
                       }
                       else if (c <= 0x1F || c == 0x7F
                                || (0x30 <= c && c <= 0x39 && (isFirst || (isSecond && isFirstCharHyphen))))
                       {
                         SerialiseCharacterAsCodePoint(c, appendTo);
                       }
                       else if (c == U'-' && isFirst && isLast)
                       {
                         SerialiseCharacter(c, appendTo);
                       }
                       else if (0x80 <= c || c == U'-' || c == U'_' || (0x30 <= c && c <= 0x39)
                                || (0x41 <= c && c <= 0x5A) || (0x61 <= c && c <= 0x7A))
                       {
                         AppendCodePoint(c, appendTo);
                       }
                       else
                       {
                         SerialiseCharacter(c, appendTo);
                       }

                       if (isFirst)
                       {
                         isFirst = false;
                         isSecond = true;
                         isFirstCharHyphen = (c == U'-');
                       }
                       else if (isSecond)
                       {
                         isSecond = false;
                       }
                     });
  }

  void SerialiseString(const CSSOMString &string, StringBuilder &appendTo) noexcept
  {
    appendTo.Append(u8'"');

    ForEachCodePoint(string,
                     [&](char32 c, bool)
                     {
                       if (c <= 0x1F || c == 0x7F)
                       {
                         SerialiseCharacterAsCodePoint(c, appendTo);
                       }
                       else if (c == U'"' || c == U'\\')
                       {
                         SerialiseCharacter(c, appendTo);
                       }
                       else
                       {
                         AppendCodePoint(c, appendTo);
                       }
                     });

    appendTo.Append(u8'"');
  }

  CSSOMString SerialiseString(const CSSOMString &value) noexcept
  {
    StringBuilder builder;
    SerialiseString(value, builder);
    return krys::move(builder).ToString();
  }

  CSSOMString SerialiseURL(const CSSOMString &value) noexcept
  {
    StringBuilder builder;
    builder.Append(u8"url(");
    SerialiseString(value, builder);
    builder.Append(u8')');
    return krys::move(builder).ToString();
  }

  CSSOMString SerialiseFontFamily(const CSSOMString &value) noexcept
  {
    return IsCSSTokenizerIdentifier(value) ? value : SerialiseString(value);
  }
}
