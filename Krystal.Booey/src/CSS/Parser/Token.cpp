#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Core/Text/HexNumber.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <algorithm>
#include <charconv>
#include <cmath>
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

  void Token::Serialise(StringBuilder &builder, const Token *nextToken,
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
        builder.Append("/**/");
        return;
      }

      if (nextType == TokenType::Delim
          && ((static_cast<char32>(delimitersNeedingComment) == nextToken->Delimiter()) || ... || false))
      {
        builder.Append("/**/");
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
        builder.Append(u8'(');
        break;
      case TokenType::AtKeyword:
        builder.Append(u8'@');
        SerialiseIdentifier(CSSOMString(IdentCodePoints()), builder);
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension,
                               TokenType::CDC},
                              '-');
        break;
      case TokenType::Hash:
        builder.Append(u8'#');
        SerialiseIdentifier(CSSOMString(IdentCodePoints()), builder,
                            _hashType == HashTokenType::Unrestricted);
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension,
                               TokenType::CDC},
                              '-');
        break;
      case TokenType::Url:
        builder.Append("url(");
        SerialiseIdentifier(CSSOMString(IdentCodePoints()), builder);
        builder.Append(u8')');
        break;
      case TokenType::Delim:
        switch (_delimiter)
        {
          case U'\\': builder.Append("\\\n"); break;
          case U'#':
          case U'-':
            builder.Append(IdentCodePoints());
            appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                                   TokenType::Number, TokenType::Percentage, TokenType::Dimension},
                                  '-');
            break;
          case U'@':
            builder.Append(u8'@');
            appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl},
                                  '-');
            break;
          case U'.':
          case U'+':
            builder.Append(IdentCodePoints());
            appendCommentIfNeeded({TokenType::Number, TokenType::Percentage, TokenType::Dimension});
            break;
          case U'/':
            builder.Append(u8'/');
            appendCommentIfNeeded({}, '*');
            break;
          default: builder.Append(IdentCodePoints()); break;
        }
        break;
      case TokenType::Number:
        if (mode == TokenSerialisationMode::CustomProperty)
        {
          builder.Append(OriginalText());
        }
        else
        {
          if (_numericSignChar == NumericSignChar::Plus)
          {
            builder.Append(u8'+');
          }
          builder.Append(_numericValue);
        }
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension},
                              '%');
        break;
      case TokenType::Percentage:
        if (mode == TokenSerialisationMode::CustomProperty)
        {
          builder.Append(OriginalText());
        }
        else
        {
          builder.Append(_numericValue);
        }
        builder.Append(u8'%');
        break;
      case TokenType::Dimension:
        if (mode == TokenSerialisationMode::CustomProperty)
        {
          builder.Append(OriginalText());
        }
        else
        {
          builder.Append(_numericValue);
          SerialiseIdentifier(CSSOMString(Unit()), builder);
        }
        appendCommentIfNeeded({TokenType::Ident, TokenType::Function, TokenType::Url, TokenType::BadUrl,
                               TokenType::Number, TokenType::Percentage, TokenType::Dimension,
                               TokenType::CDC},
                              '-');
        break;
      case TokenType::UnicodeRange:
      {
        builder.Append(u8"U+", Hex(static_cast<uint32>(_unicodeRangeStart)));
        if (_unicodeRangeStart != _unicodeRangeEnd)
        {
          builder.Append(u8'-', Hex(static_cast<uint32>(_unicodeRangeEnd)));
        }
        break;
      }
      case TokenType::String:    SerialiseString(CSSOMString(IdentCodePoints()), builder); break;

      case TokenType::CDO:       builder.Append("<!--"); break;
      case TokenType::CDC:       builder.Append("-->"); break;
      case TokenType::BadString: builder.Append("'\n"); break;
      case TokenType::BadUrl:    builder.Append("url(()"); break;
      case TokenType::Whitespace:
        if (OriginalText().empty())
        {
          // Created without its text; serialise as a single space.
          builder.Append(u8' ');
        }
        else if (mode == TokenSerialisationMode::CustomProperty)
        {
          for (auto c : OriginalText())
          {
            builder.Append(WhitespaceForCustomProperty(c));
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
              builder.Append(u8' ');
              inRun = false;
            }
            else if (!inRun)
            {
              builder.Append(u8' ');
              inRun = true;
            }
          }
        }
        break;
      case TokenType::Colon:         builder.Append(u8':'); break;
      case TokenType::Semicolon:     builder.Append(u8';'); break;
      case TokenType::Comma:         builder.Append(u8','); break;
      case TokenType::OpenParen:     builder.Append(u8'('); break;
      case TokenType::CloseParen:    builder.Append(u8')'); break;
      case TokenType::OpenSquare:    builder.Append(u8'['); break;
      case TokenType::CloseSquare:   builder.Append(u8']'); break;
      case TokenType::OpenCurly:     builder.Append(u8'{'); break;
      case TokenType::CloseCurly:    builder.Append(u8'}'); break;

      case TokenType::Uninitialized:
      case TokenType::EndOfFile:     krys_unreachable();
    }
  }
}
