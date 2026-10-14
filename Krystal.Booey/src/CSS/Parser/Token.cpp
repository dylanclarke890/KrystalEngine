#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <algorithm>
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

    // Spaces and tabs are counted as spaces, and newlines kept, as the original whitespace is serialised for
    // custom properties.
    constexpr char8 WhitespaceForCustomProperty(char8 c) noexcept
    {
      return c == u8'\n' ? u8'\n' : u8' ';
    }

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
      case TokenType::Percentage: return OriginalText() == other.OriginalText();
      case TokenType::Whitespace:
        return std::ranges::equal(
          OriginalText(), other.OriginalText(),
          [](char8 a, char8 b) { return WhitespaceForCustomProperty(a) == WhitespaceForCustomProperty(b); });
      case TokenType::UnicodeRange:
        return _unicodeRangeStart == other._unicodeRangeStart && _unicodeRangeEnd == other._unicodeRangeEnd;
      default: return true;
    }
  }

  void Token::Serialise(CSSOMString &builder, const Token *nextToken,
                        TokenSerialisationMode mode) const noexcept
  {
    // This is currently only used for @supports CSSOM. To keep our implementation simple we handle some of
    // the edge cases incorrectly (see comments below).
    auto appendCommentIfNeeded =
      [&](const NextTokenNeedsCommentBuilder &tokensNeedingComment, auto... delimitersNeedingComment) noexcept
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
        SerialiseIdentifier(CSSOMString(IdentCodePoints()), builder);
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension, TokenType::CDC,
                               TokenType::OpenParen},
                              '-');
        break;
      case TokenType::Function:
        SerialiseIdentifier(CSSOMString(IdentCodePoints()), builder);
        builder.push_back(u8'(');
        break;
      case TokenType::AtKeyword:
        builder.push_back(u8'@');
        SerialiseIdentifier(CSSOMString(IdentCodePoints()), builder);
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension,
                               TokenType::CDC},
                              '-');
        break;
      case TokenType::Hash:
        builder.push_back(u8'#');
        SerialiseIdentifier(CSSOMString(IdentCodePoints()), builder,
                            _hashType == HashTokenType::Unrestricted);
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension,
                               TokenType::CDC},
                              '-');
        break;
      case TokenType::Url:
        AppendASCII("url(", builder);
        SerialiseIdentifier(CSSOMString(IdentCodePoints()), builder);
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
            appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl},
                                  '-');
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
        if (mode == TokenSerialisationMode::CustomProperty)
        {
          builder.append(OriginalText());
        }
        else
        {
          if (_numericSignChar == NumericSignChar::Plus)
          {
            builder.push_back(u8'+');
          }
          AppendNumber(_numericValue, builder);
        }
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension},
                              '%');
        break;
      case TokenType::Percentage:
        if (mode == TokenSerialisationMode::CustomProperty)
        {
          builder.append(OriginalText());
        }
        else
        {
          AppendNumber(_numericValue, builder);
        }
        builder.push_back(u8'%');
        break;
      case TokenType::Dimension:
        if (mode == TokenSerialisationMode::CustomProperty)
        {
          builder.append(OriginalText());
        }
        else
        {
          AppendNumber(_numericValue, builder);
          SerialiseIdentifier(CSSOMString(Unit()), builder);
        }
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension,
                               TokenType::CDC},
                              '-');
        break;
      case TokenType::UnicodeRange:
      {
        auto range = _unicodeRangeStart == _unicodeRangeEnd
                       ? std::format("U+{:X}", static_cast<uint32>(_unicodeRangeStart))
                       : std::format("U+{:X}-{:X}", static_cast<uint32>(_unicodeRangeStart),
                                     static_cast<uint32>(_unicodeRangeEnd));
        AppendASCII(range, builder);
        break;
      }
      case TokenType::String:    SerialiseString(CSSOMString(IdentCodePoints()), builder); break;

      case TokenType::CDO:       AppendASCII("<!--", builder); break;
      case TokenType::CDC:       AppendASCII("-->", builder); break;
      case TokenType::BadString: AppendASCII("'\n", builder); break;
      case TokenType::BadUrl:    AppendASCII("url(()", builder); break;
      case TokenType::Whitespace:
        if (OriginalText().empty())
        {
          // Created without its text; serialise as a single space.
          builder.push_back(u8' ');
        }
        else if (mode == TokenSerialisationMode::CustomProperty)
        {
          for (auto c : OriginalText())
          {
            builder.push_back(WhitespaceForCustomProperty(c));
          }
        }
        else
        {
          // One whitespace token covers what upstream tokenizes as a token per run of spaces and tabs and a
          // token per newline; each of those serialises as a space.
          bool inRun = false;
          for (auto c : OriginalText())
          {
            if (c == u8'\n')
            {
              builder.push_back(u8' ');
              inRun = false;
            }
            else if (!inRun)
            {
              builder.push_back(u8' ');
              inRun = true;
            }
          }
        }
        break;
      case TokenType::Colon:         builder.push_back(u8':'); break;
      case TokenType::Semicolon:     builder.push_back(u8';'); break;
      case TokenType::Comma:         builder.push_back(u8','); break;
      case TokenType::OpenParen:     builder.push_back(u8'('); break;
      case TokenType::CloseParen:    builder.push_back(u8')'); break;
      case TokenType::OpenSquare:    builder.push_back(u8'['); break;
      case TokenType::CloseSquare:   builder.push_back(u8']'); break;
      case TokenType::OpenCurly:     builder.push_back(u8'{'); break;
      case TokenType::CloseCurly:    builder.push_back(u8'}'); break;

      case TokenType::Uninitialized:
      case TokenType::EndOfFile:     krys_unreachable();
    }
  }
}
