#include "Krystal.Booey/CSS/Properties/Consumers/UnicodeRange.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSUnicodeRangeValue.hpp"
#include "Krystal.Core/Text/Encodings/Decode.hpp"
#include "Krystal.Core/Text/Encodings/Encode.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  namespace
  {
    KRYS_NODISCARD bool ConsumeOptionalDelimiter(TokenRange &tokens, char32 delim) noexcept
    {
      auto encodeResult = krys::text::Encode<krys::text::UTF8>(Span<const char32>(&delim, 1));
      krys_debug_assert(encodeResult.Error == krys::text::EncodeError::None);
      if (!(tokens.Peek().Type() == TokenType::Delim
            && tokens.Peek().IdentCodePoints() == encodeResult.Output))
      {
        return false;
      }

      tokens.Discard();
      return true;
    }

    KRYS_NODISCARD CSSOMStringView ConsumeIdentifier(TokenRange &tokens) noexcept
    {
      if (tokens.Peek().Type() != TokenType::Ident)
      {
        return {};
      }

      return tokens.Consume().IdentCodePoints();
    }

    KRYS_NODISCARD bool ConsumeAndAppendOptionalNumber(CSSOMString &str, TokenRange &tokens,
                                                       TokenType type = TokenType::Number) noexcept
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

    KRYS_NODISCARD bool ConsumeAndAppendOptionalDelimiter(CSSOMString &str, TokenRange &tokens,
                                                          char32 value) noexcept
    {
      if (!ConsumeOptionalDelimiter(tokens, value))
      {
        return false;
      }

      auto encodeResult = krys::text::Encode<krys::text::UTF8>(Span<const char32>(&value, 1));
      krys_debug_assert(encodeResult.Error == krys::text::EncodeError::None);
      str.append_range(encodeResult.Output);
      return true;
    }

    static void ConsumeAndAppendOptionalQuestionMarks(CSSOMString &str, TokenRange &tokens) noexcept
    {
      while (ConsumeAndAppendOptionalDelimiter(str, tokens, '?'))
      {
      }
    }

    KRYS_NODISCARD CSSOMString ConsumeUnicodeRangeString(TokenRange &tokens) noexcept
    {
      if (!krys::text::IsASCIICaselessEqual(ConsumeIdentifier(tokens), utf8_stringview(u8"u")))
      {
        return {};
      }

      CSSOMString builder;
      if (ConsumeAndAppendOptionalNumber(builder, tokens, TokenType::Dimension))
      {
        ConsumeAndAppendOptionalQuestionMarks(builder, tokens);
      }
      else if (ConsumeAndAppendOptionalNumber(builder, tokens))
      {
        if (!(ConsumeAndAppendOptionalNumber(builder, tokens, TokenType::Dimension)
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
  static Maybe<UnicodeRange> ConsumeUnicodeRangeTokenUnresolved(TokenRange &tokens) noexcept
  {
    auto unicodeRangeString = ConsumeUnicodeRangeString(tokens);
    if (unicodeRangeString.empty())
    {
      return null;
    }

    auto decodeResult = krys::text::Decode<krys::text::UTF8>(unicodeRangeString);
    krys_debug_assert(decodeResult.Error == krys::text::DecodeError::None);
    auto &asUTF32 = decodeResult.Output;

    utf32_stringview buffer = asUTF32;
    if (buffer.empty() || buffer[0] != U'+')
    {
      return null;
    }
    buffer = buffer.substr(1uz);

    char32 start = 0;
    size_t hexDigitCount = 0uz;

    while (!buffer.empty() && krys::text::IsASCIIHexDigit(buffer[0]))
    {
      if (++hexDigitCount > 6)
      {
        return null;
      }

      start <<= 4;
      start |= krys::text::ToASCIIHexValue(buffer[0]);
      buffer = buffer.substr(1uz);
    }

    auto end = start;
    while (!buffer.empty() && buffer[0] == U'?')
    {
      if (++hexDigitCount > 6)
      {
        return null;
      }

      start <<= 4;
      end <<= 4;
      end |= 0xF;
    }

    if (!hexDigitCount)
    {
      return null;
    }

    if (start == end && !buffer.empty())
    {
      if (buffer[0] != U'-')
      {
        return null;
      }
      buffer = buffer.substr(1uz);

      end = 0;
      hexDigitCount = 0;
      while (!buffer.empty() && krys::text::IsASCIIHexDigit(buffer[0]))
      {
        if (++hexDigitCount > 6)
        {
          return null;
        }

        end <<= 4;
        end |= krys::text::ToASCIIHexValue(buffer[0]);
        buffer = buffer.substr(1uz);
      }

      if (!hexDigitCount)
      {
        return null;
      }
    }

    if (!buffer.empty())
    {
      return null;
    }

    return UnicodeRange {start, end};
  }

  RefPtr<CSSValue> ConsumeUnicodeRangeToken(TokenRange &tokens) noexcept
  {
    auto rangeCopy = tokens;

    auto unicodeRange = ConsumeUnicodeRangeTokenUnresolved(rangeCopy);
    rangeCopy.Discard();
    rangeCopy.DiscardWhitespace();

    if (!unicodeRange || unicodeRange->end > krys::text::MaxUnicodeChar
        || unicodeRange->start > unicodeRange->end)
    {
      return nullptr;
    }

    tokens = rangeCopy;
    return CSSUnicodeRangeValue::Create(unicodeRange->start, unicodeRange->end);
  }
}