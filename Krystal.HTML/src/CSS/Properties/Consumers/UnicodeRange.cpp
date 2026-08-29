#include "Krystal.HTML/CSS/Properties/Consumers/UnicodeRange.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSUnicodeRangeValue.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  namespace
  {
    KRYS_NODISCARD bool ConsumeOptionalDelimiter(CSSTokenRange &tokens, char32 delim) noexcept
    {
      CSSOMString value = Krys::Text::ConvertToUTF8(Span<const char32>(&delim, 1));
      if (!(tokens.Peek().Type() == CSSTokenType::Delim && tokens.Peek().IdentCodePoints() == value))
      {
        return false;
      }

      tokens.Discard();
      return true;
    }

    KRYS_NODISCARD CSSOMStringView ConsumeIdentifier(CSSTokenRange &tokens) noexcept
    {
      if (tokens.Peek().Type() != CSSTokenType::Ident)
      {
        return {};
      }

      return tokens.Consume().IdentCodePoints();
    }

    KRYS_NODISCARD bool ConsumeAndAppendOptionalNumber(CSSOMString &str, CSSTokenRange &tokens,
                                                       CSSTokenType type = CSSTokenType::Number) noexcept
    {
      if (tokens.Peek().Type() != type)
      {
        return false;
      }

      auto originalText = tokens.Consume().IdentCodePoints();
      if (originalText.empty())
      {
        return false;
      }

      str.append_range(originalText);
      return true;
    }

    KRYS_NODISCARD bool ConsumeAndAppendOptionalDelimiter(CSSOMString &str, CSSTokenRange &tokens,
                                                          char32 value) noexcept
    {
      if (!ConsumeOptionalDelimiter(tokens, value))
      {
        return false;
      }

      str.append_range(Krys::Text::ConvertToUTF8(Span<const char32>(&value, 1)));
      return true;
    }

    static void ConsumeAndAppendOptionalQuestionMarks(CSSOMString &str, CSSTokenRange &tokens) noexcept
    {
      while (ConsumeAndAppendOptionalDelimiter(str, tokens, '?'))
      {
      }
    }

    KRYS_NODISCARD CSSOMString ConsumeUnicodeRangeString(CSSTokenRange &tokens) noexcept
    {
      if (!Krys::Text::ASCIICaseInsensitiveMatch(ConsumeIdentifier(tokens), u8"u"))
      {
        return {};
      }

      CSSOMString builder;
      if (ConsumeAndAppendOptionalNumber(builder, tokens, CSSTokenType::Dimension))
      {
        ConsumeAndAppendOptionalQuestionMarks(builder, tokens);
      }
      else if (ConsumeAndAppendOptionalNumber(builder, tokens))
      {
        if (!(ConsumeAndAppendOptionalNumber(builder, tokens, CSSTokenType::Dimension)
              || ConsumeAndAppendOptionalNumber(builder, tokens)))
        {
          ConsumeAndAppendOptionalQuestionMarks(builder, tokens);
        }
      }
      else if (ConsumeOptionalDelimiter(tokens, '+'))
      {
        builder.push_back(u8'+');
        if (auto identifier = ConsumeIdentifier(tokens); !identifier.empty())
        {
          builder.append(identifier);
        }
        else if (!ConsumeAndAppendOptionalDelimiter(builder, tokens, '?'))
        {
          return {};
        }

        ConsumeAndAppendOptionalQuestionMarks(builder, tokens);
      }
      else
      {
        return {};
      }

      return builder;
    }

    struct UnicodeRange
    {
      char32 start;
      char32 end;
    };
  }

  // MARK: <unicode-tokens-token> consuming (unresolved)
  static Maybe<UnicodeRange> ConsumeUnicodeRangeTokenUnresolved(CSSTokenRange &tokens) noexcept
  {
    auto unicodeRangeString = ConsumeUnicodeRangeString(tokens);
    if (unicodeRangeString.empty())
    {
      return Null;
    }

    auto asUTF32 = Krys::Text::ConvertToUTF32(CSSOMStringView(unicodeRangeString));

    utf32_stringview buffer = asUTF32;
    if (buffer.empty() || buffer[0] != U'+')
    {
      return Null;
    }
    buffer = buffer.substr(1uz);

    char32 start = 0;
    size_t hexDigitCount = 0uz;

    while (!buffer.empty() && Krys::Text::IsASCIIHexDigit(buffer[0]))
    {
      if (++hexDigitCount > 6)
      {
        return Null;
      }

      start <<= 4;
      start |= Krys::Text::ToASCIIHexValue(buffer[0]);
      buffer = buffer.substr(1uz);
    }

    auto end = start;
    while (!buffer.empty() && buffer[0] == U'?')
    {
      if (++hexDigitCount > 6)
      {
        return Null;
      }

      start <<= 4;
      end <<= 4;
      end |= 0xF;
    }

    if (!hexDigitCount)
    {
      return Null;
    }

    if (start == end && !buffer.empty())
    {
      if (buffer[0] != U'-')
      {
        return Null;
      }
      buffer = buffer.substr(1uz);

      end = 0;
      hexDigitCount = 0;
      while (!buffer.empty() && Krys::Text::IsASCIIHexDigit(buffer[0]))
      {
        if (++hexDigitCount > 6)
        {
          return Null;
        }

        end <<= 4;
        end |= Krys::Text::ToASCIIHexValue(buffer[0]);
        buffer = buffer.substr(1uz);
      }

      if (!hexDigitCount)
      {
        return Null;
      }
    }

    if (!buffer.empty())
    {
      return Null;
    }

    return UnicodeRange {start, end};
  }

  RefPtr<CSSValue> ConsumeUnicodeRangeToken(CSSTokenRange &tokens) noexcept
  {
    auto rangeCopy = tokens;

    auto unicodeRange = ConsumeUnicodeRangeTokenUnresolved(rangeCopy);
    rangeCopy.Discard();
    rangeCopy.DiscardWhitespace();

    if (!unicodeRange || unicodeRange->end > Krys::Text::Unicode::LastUnicodeCodePoint
        || unicodeRange->start > unicodeRange->end)
    {
      return nullptr;
    }

    tokens = rangeCopy;
    return CSSUnicodeRangeValue::Create(unicodeRange->start, unicodeRange->end);
  }
}