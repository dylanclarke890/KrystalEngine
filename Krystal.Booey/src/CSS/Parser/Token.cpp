#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <charconv>
#include <cmath>
#include <format>
#include <initializer_list>

namespace krys::boo::css
{
  namespace
  {
    struct NextTokenNeedsCommentBuilder
    {
      constexpr NextTokenNeedsCommentBuilder(std::initializer_list<TokenType> tokens) noexcept
      {
        for (auto token : tokens)
        {
          Buffer[static_cast<size_t>(token)] = true;
        }
      }

      Array<bool, EnumTraits<TokenType>::DistinctValues> Buffer {};
    };

    void AppendASCII(std::string_view ascii, CSSOMString &builder) noexcept
    {
      builder.append(ascii.begin(), ascii.end());
    }

    /// @brief Appends the shortest round-trip representation, in ECMAScript style: fixed notation below 1e21,
    /// scientific above.
    void AppendNumber(double value, CSSOMString &builder) noexcept
    {
      Array<char, 64uz> buffer {};
      auto format = std::abs(value) < 1e21 ? std::chars_format::fixed : std::chars_format::scientific;
      auto result = std::to_chars(buffer.data(), buffer.data() + buffer.size(), value, format);
      krys_debug_assert(result.ec == std::errc {});
      AppendASCII(std::string_view(buffer.data(), result.ptr), builder);
    }
  }

  bool Token::operator==(const Token &other) const noexcept
  {
    if (_type != other._type)
    {
      return false;
    }

    switch (_type)
    {
      case TokenType::Delim: return _delimiter == other._delimiter;
      case TokenType::Hash:
        if (_hashType != other._hashType)
        {
          return false;
        }
        KRYS_FALLTHROUGH;
      case TokenType::Ident:
      case TokenType::Function:
      case TokenType::AtKeyword:
      case TokenType::String:
      case TokenType::Url:       return IdentCodePoints() == other.IdentCodePoints();
      case TokenType::Dimension:
        if (Unit() != other.Unit())
        {
          return false;
        }
        KRYS_FALLTHROUGH;
      case TokenType::Number:
      case TokenType::Percentage:
        // The spec compares the original text, which isn't kept; fall back to the parsed values.
        return _numericSignChar == other._numericSignChar && _numericValue == other._numericValue
            && _numericValueType == other._numericValueType;
      case TokenType::UnicodeRange:
        return _unicodeRangeStart == other._unicodeRangeStart && _unicodeRangeEnd == other._unicodeRangeEnd;
      default: return true;
    }
  }

  void Token::Serialise(CSSOMString &builder, const Token *nextToken,
                        KRYS_MAYBE_UNUSED TokenSerialisationMode mode) const noexcept
  {
    // This is currently only used for @supports CSSOM. To keep our implementation simple we handle some of the
    // edge cases incorrectly (see comments below).
    auto appendCommentIfNeeded = [&](const NextTokenNeedsCommentBuilder &tokensNeedingComment,
                                     auto... delimitersNeedingComment) noexcept
    {
      if (!nextToken)
      {
        return;
      }

      TokenType nextType = nextToken->Type();
      if (tokensNeedingComment.Buffer[static_cast<size_t>(nextType)])
      {
        AppendASCII("/**/", builder);
        return;
      }

      if (nextType == TokenType::Delim
          && ((static_cast<char32>(delimitersNeedingComment) == nextToken->Delimiter()) || ... || false))
      {
        AppendASCII("/**/", builder);
        return;
      }
    };

    switch (_type)
    {
      case TokenType::Ident:
        SerializeIdentifier(CSSOMString(IdentCodePoints()), builder);
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension, TokenType::CDC,
                               TokenType::OpenParen},
                              '-');
        break;
      case TokenType::Function:
        SerializeIdentifier(CSSOMString(IdentCodePoints()), builder);
        builder.push_back(u8'(');
        break;
      case TokenType::AtKeyword:
        builder.push_back(u8'@');
        SerializeIdentifier(CSSOMString(IdentCodePoints()), builder);
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension, TokenType::CDC},
                              '-');
        break;
      case TokenType::Hash:
        builder.push_back(u8'#');
        SerializeIdentifier(CSSOMString(IdentCodePoints()), builder, _hashType == HashTokenType::Unrestricted);
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension, TokenType::CDC},
                              '-');
        break;
      case TokenType::Url:
        AppendASCII("url(", builder);
        SerializeIdentifier(CSSOMString(IdentCodePoints()), builder);
        builder.push_back(u8')');
        break;
      case TokenType::Delim:
        switch (_delimiter)
        {
          case U'\\': AppendASCII("\\\n", builder); break;
          case U'#':
          case U'-':
            builder.append(IdentCodePoints());
            appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                                   TokenType::Number, TokenType::Percentage, TokenType::Dimension},
                                  '-');
            break;
          case U'@':
            builder.push_back(u8'@');
            appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl}, '-');
            break;
          case U'.':
          case U'+':
            builder.append(IdentCodePoints());
            appendCommentIfNeeded({TokenType::Number, TokenType::Percentage, TokenType::Dimension});
            break;
          case U'/':
            builder.push_back(u8'/');
            appendCommentIfNeeded({}, '*');
            break;
          default: builder.append(IdentCodePoints()); break;
        }
        break;
      case TokenType::Number:
        // FIXME: TokenSerialisationMode::CustomProperty should append the original text, which the tokenizer
        // doesn't keep. Serialised from the parsed value instead.
        if (_numericSignChar == NumericSignChar::Plus)
        {
          builder.push_back(u8'+');
        }
        AppendNumber(_numericValue, builder);
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension},
                              '%');
        break;
      case TokenType::Percentage:
        // FIXME: Same as Number for TokenSerialisationMode::CustomProperty.
        AppendNumber(_numericValue, builder);
        builder.push_back(u8'%');
        break;
      case TokenType::Dimension:
        // FIXME: Same as Number for TokenSerialisationMode::CustomProperty.
        AppendNumber(_numericValue, builder);
        SerializeIdentifier(CSSOMString(Unit()), builder);
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension, TokenType::CDC},
                              '-');
        break;
      case TokenType::UnicodeRange:
      {
        // Not in WebKit, which has no unicode-range token.
        auto range = _unicodeRangeStart == _unicodeRangeEnd
                     ? std::format("U+{:X}", static_cast<uint32>(_unicodeRangeStart))
                     : std::format("U+{:X}-{:X}", static_cast<uint32>(_unicodeRangeStart),
                                   static_cast<uint32>(_unicodeRangeEnd));
        AppendASCII(range, builder);
        break;
      }
      case TokenType::String: SerializeString(CSSOMString(IdentCodePoints()), builder); break;

      case TokenType::CDO:       AppendASCII("<!--", builder); break;
      case TokenType::CDC:       AppendASCII("-->", builder); break;
      case TokenType::BadString: AppendASCII("'\n", builder); break;
      case TokenType::BadUrl:    AppendASCII("url(()", builder); break;
      case TokenType::Whitespace:
        // FIXME: TokenSerialisationMode::CustomProperty should preserve the original whitespace (count and
        // newlines), which the tokenizer collapses into one token.
        builder.push_back(u8' ');
        break;
      case TokenType::Colon:      builder.push_back(u8':'); break;
      case TokenType::Semicolon:  builder.push_back(u8';'); break;
      case TokenType::Comma:      builder.push_back(u8','); break;
      case TokenType::OpenParen:  builder.push_back(u8'('); break;
      case TokenType::CloseParen: builder.push_back(u8')'); break;
      case TokenType::OpenSquare: builder.push_back(u8'['); break;
      case TokenType::CloseSquare: builder.push_back(u8']'); break;
      case TokenType::OpenCurly:  builder.push_back(u8'{'); break;
      case TokenType::CloseCurly: builder.push_back(u8'}'); break;

      case TokenType::Uninitialized:
      case TokenType::EndOfFile:     krys_unreachable();
    }
  }
}
