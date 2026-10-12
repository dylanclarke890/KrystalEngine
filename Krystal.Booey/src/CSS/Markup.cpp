#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <format>

namespace krys::boo::css
{
  namespace
  {
    // TODO: replace with a shared UTF-8 code point iterator once one exists in Krystal.Core/Text.
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

    void AppendCodePoint(char32 c, CSSOMString &appendTo) noexcept
    {
      auto result = krys::text::UTF8::EncodeOne(c);
      appendTo.append(result.Output.data(), result.CodeUnitsProduced);
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

    void SerializeCharacter(char32 c, CSSOMString &appendTo) noexcept
    {
      appendTo.push_back(u8'\\');
      AppendCodePoint(c, appendTo);
    }

    void SerializeCharacterAsCodePoint(char32 c, CSSOMString &appendTo) noexcept
    {
      auto hex = std::format("\\{:x} ", static_cast<uint32>(c));
      appendTo.append(hex.begin(), hex.end());
    }
  }

  void SerializeIdentifier(const CSSOMString &identifier, CSSOMString &appendTo, bool skipStartChecks) noexcept
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
                         SerializeCharacterAsCodePoint(c, appendTo);
                       }
                       else if (c == U'-' && isFirst && isLast)
                       {
                         SerializeCharacter(c, appendTo);
                       }
                       else if (0x80 <= c || c == U'-' || c == U'_' || (0x30 <= c && c <= 0x39)
                                || (0x41 <= c && c <= 0x5A) || (0x61 <= c && c <= 0x7A))
                       {
                         AppendCodePoint(c, appendTo);
                       }
                       else
                       {
                         SerializeCharacter(c, appendTo);
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

  void SerializeString(const CSSOMString &string, CSSOMString &appendTo) noexcept
  {
    appendTo.push_back(u8'"');

    ForEachCodePoint(string,
                     [&](char32 c, bool)
                     {
                       if (c <= 0x1F || c == 0x7F)
                       {
                         SerializeCharacterAsCodePoint(c, appendTo);
                       }
                       else if (c == U'"' || c == U'\\')
                       {
                         SerializeCharacter(c, appendTo);
                       }
                       else
                       {
                         AppendCodePoint(c, appendTo);
                       }
                     });

    appendTo.push_back(u8'"');
  }

  CSSOMString SerializeString(const CSSOMString &value) noexcept
  {
    CSSOMString builder;
    SerializeString(value, builder);
    return builder;
  }

  CSSOMString SerializeURL(const CSSOMString &value) noexcept
  {
    CSSOMString builder = u8"url(";
    SerializeString(value, builder);
    builder.push_back(u8')');
    return builder;
  }

  CSSOMString SerializeFontFamily(const CSSOMString &value) noexcept
  {
    return IsCSSTokenizerIdentifier(value) ? value : SerializeString(value);
  }
}
