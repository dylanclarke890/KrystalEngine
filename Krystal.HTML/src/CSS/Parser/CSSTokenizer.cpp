#include "Krystal.HTML/CSS/Parser/CSSTokenizer.hpp"
#include "Krystal.Text/ASCII.hpp"

namespace Krys::HTML
{
  CSSTokenizer::CSSTokenizer(CSSInputStream &inputStream) noexcept : _inputStream(inputStream)
  {
  }

  Maybe<CSSToken> CSSTokenizer::ConsumeToken() noexcept
  {
    ConsumeComments();

    auto current = _inputStream.NextInputCharacter();
    _inputStream.Consume();
    auto next = _inputStream.NextInputCharacter();

    if (IsWhitespace(current))
    {
      ConsumeWhitespace();
      return CSSToken {CSSTokenType::Whitespace};
    }

    if (current == U'"')
    {
      return ConsumeStringToken(U'"');
    }

    if (current == U'#')
    {
      if (IsIdentCodePoint(next) || StartsWithValidEscape(current, next))
      {
        auto token = CSSToken {CSSTokenType::Hash};

        if (WouldStartIdentifier(current, next, _inputStream.Peek(1uz)))
        {
          token.HashType(HashTokenType::Id);
        }

        token.IdentCodePoints(ConsumeIdentSequence());
        return token;
      }

      return CSSToken {CSSTokenType::Delim, current};
    }

    if (current == U'\'')
    {
      return ConsumeStringToken(U'\'');
    }

    if (current == U'(')
    {
      return CSSToken {CSSTokenType::OpenParen};
    }

    if (current == U')')
    {
      return CSSToken {CSSTokenType::CloseParen};
    }

    if (current == U'+')
    {
      if (StartsWithNumber(current, next, _inputStream.Peek(1uz)))
      {
        _inputStream.Reconsume(current);
        return ConsumeNumericToken();
      }

      return CSSToken {CSSTokenType::Delim, current};
    }

    if (current == U',')
    {
      return CSSToken {CSSTokenType::Comma};
    }

    if (current == U'-')
    {
      if (StartsWithNumber(current, next, _inputStream.Peek(1uz)))
      {
        _inputStream.Reconsume(current);
        return ConsumeNumericToken();
      }

      if (next == U'-' && _inputStream.Peek(1uz) == U'>')
      {
        _inputStream.Consume(2uz);
        return CSSToken {CSSTokenType::CDC};
      }

      if (WouldStartIdentifier(current, next, _inputStream.Peek(1uz)))
      {
        _inputStream.Reconsume(current);
        return ConsumeIdentLikeToken();
      }

      return CSSToken {CSSTokenType::Delim, current};
    }

    if (current == U'.')
    {
      if (StartsWithNumber(current, next, _inputStream.Peek(1uz)))
      {
        _inputStream.Reconsume(current);
        return ConsumeNumericToken();
      }

      return CSSToken {CSSTokenType::Delim, current};
    }

    if (current == U':')
    {
      return CSSToken {CSSTokenType::Colon};
    }

    if (current == U';')
    {
      return CSSToken {CSSTokenType::Semicolon};
    }

    if (current == U'<')
    {
      if (next == U'!' && _inputStream.Peek(1uz) == U'-' && _inputStream.Peek(2uz) == U'-')
      {
        _inputStream.Consume(3uz);
        return CSSToken {CSSTokenType::CDO};
      }

      return CSSToken {CSSTokenType::Delim, current};
    }

    if (current == U'@')
    {
      if (WouldStartIdentifier(next, _inputStream.Peek(1uz), _inputStream.Peek(2uz)))
      {
        auto token = CSSToken {CSSTokenType::AtKeyword};
        token.IdentCodePoints(ConsumeIdentSequence());
        return token;
      }

      return CSSToken {CSSTokenType::Delim, current};
    }

    if (current == U'[')
    {
      return CSSToken {CSSTokenType::OpenSquare};
    }

    if (current == U'\\')
    {
      if (StartsWithValidEscape(current, next))
      {
        _inputStream.Reconsume(current);
        return ConsumeIdentLikeToken();
      }

      ParseError(CSSParseError::UnexpectedReverseSolidus);
      return CSSToken {CSSTokenType::Delim, current};
    }

    if (current == U']')
    {
      return CSSToken {CSSTokenType::CloseSquare};
    }

    if (current == U'{')
    {
      return CSSToken {CSSTokenType::OpenCurly};
    }

    if (current == U'}')
    {
      return CSSToken {CSSTokenType::CloseCurly};
    }

    if (IsDigit(current))
    {
      _inputStream.Reconsume(current);
      return ConsumeNumericToken();
    }

    if (IsIdentStartCodePoint(current))
    {
      _inputStream.Reconsume(current);
      return ConsumeIdentLikeToken();
    }

    if (current == EOFMarker)
    {
      return Null;
    }

    return CSSToken {CSSTokenType::Delim, current};
  }

  void CSSTokenizer::ConsumeComments() noexcept
  {
    while (true)
    {
      if (_inputStream.NextInputCharacter() != U'/')
      {
        return;
      }

      if (_inputStream.Peek(1uz) != U'*')
      {
        return;
      }
      _inputStream.Consume(2uz);

      while (true)
      {
        if (_inputStream.NextInputCharacter() == EOFMarker)
        {
          ParseError(CSSParseError::UnexpectedEOFInComment);
          return;
        }

        if (_inputStream.NextInputCharacter() == U'*' && _inputStream.Peek(1uz) == U'/')
        {
          _inputStream.Consume(2uz);
          break;
        }

        _inputStream.Consume();
      }
    }
  }

  CSSToken CSSTokenizer::ConsumeNumericToken() noexcept
  {
    auto number = ConsumeNumber();

    if (WouldStartIdentifier(_inputStream.NextInputCharacter(), _inputStream.Peek(1uz),
                             _inputStream.Peek(2uz)))
    {
      auto token = CSSToken {CSSTokenType::Dimension};
      token.Unit(ConsumeIdentSequence());
      token.NumericValue(number);

      return token;
    }

    if (_inputStream.NextInputCharacter() == U'%')
    {
      _inputStream.Consume();

      auto token = CSSToken {CSSTokenType::Percentage};
      token.NumericValue(number);

      return token;
    }

    auto token = CSSToken {CSSTokenType::Number};
    token.NumericValue(number);

    return token;
  }

  CSSToken CSSTokenizer::ConsumeIdentLikeToken() noexcept
  {
    auto string = ConsumeIdentSequence();

    if (Krys::Text::ASCIICaseInsensitiveMatch(string, U"url") && _inputStream.NextInputCharacter() == U'(')
    {
      _inputStream.Consume();

      while (IsWhitespace(_inputStream.NextInputCharacter()) && IsWhitespace(_inputStream.Peek(1uz)))
      {
        _inputStream.Consume();
      }

      auto next = _inputStream.NextInputCharacter();
      if (next == U'"' || next == U'\''
          || (IsWhitespace(next) && (_inputStream.Peek(1uz) == U'"' || _inputStream.Peek(1uz) == U'\'')))
      {
        return CSSToken {CSSTokenType::Function, string};
      }

      return ConsumeUrlToken();
    }

    if (_inputStream.NextInputCharacter() == U'(')
    {
      _inputStream.Consume();
      return CSSToken {CSSTokenType::Function, string};
    }

    return CSSToken {CSSTokenType::Ident, string};
  }

  CSSToken CSSTokenizer::ConsumeStringToken(char32 endingCodePoint) noexcept
  {
    utf32_string codePoints;
    while (true)
    {
      auto current = _inputStream.NextInputCharacter();
      _inputStream.Consume();
      auto next = _inputStream.NextInputCharacter();

      if (current == endingCodePoint)
      {
        return CSSToken {CSSTokenType::String, codePoints};
      }

      if (current == EOFMarker)
      {
        ParseError(CSSParseError::UnexpectedEOFInString);
        return CSSToken {CSSTokenType::String, codePoints};
      }

      if (current == U'\n')
      {
        ParseError(CSSParseError::UnexpectedNewlineInString);
        _inputStream.Reconsume(current);
        return CSSToken {CSSTokenType::BadString};
      }

      if (current == U'\\')
      {
        if (next == EOFMarker)
        {
          continue;
        }

        if (next == U'\n')
        {
          _inputStream.Consume();
          continue;
        }

        assert(StartsWithValidEscape(current, next));
        codePoints.push_back(ConsumeEscapedCodePoint());
        continue;
      }

      codePoints.push_back(current);
      continue;
    }
  }

  CSSToken CSSTokenizer::ConsumeUrlToken() noexcept
  {
    ConsumeWhitespace();

    utf32_string url;
    while (true)
    {
      auto current = _inputStream.NextInputCharacter();
      _inputStream.Consume();
      auto next = _inputStream.NextInputCharacter();

      if (current == U')')
      {
        return CSSToken {CSSTokenType::Url, url};
      }

      if (current == EOFMarker)
      {
        ParseError(CSSParseError::UnexpectedEOFInUrl);
        return CSSToken {CSSTokenType::Url, url};
      }

      if (IsWhitespace(current))
      {
        ConsumeWhitespace();

        if (_inputStream.NextInputCharacter() == U')')
        {
          _inputStream.Consume();
          return CSSToken {CSSTokenType::Url, url};
        }

        if (_inputStream.NextInputCharacter() == EOFMarker)
        {
          ParseError(CSSParseError::UnexpectedEOFInUrl);
          return CSSToken {CSSTokenType::Url, url};
        }

        ParseError(CSSParseError::InvalidCharacterInUrl);
        ConsumeRemnantsOfBadUrl();
        return CSSToken {CSSTokenType::BadUrl};
      }

      if (current == U'"' || current == U'\'' || current == U'(' || IsNonPrintableCodePoint(current))
      {
        ParseError(CSSParseError::InvalidCharacterInUrl);
        ConsumeRemnantsOfBadUrl();
        return CSSToken {CSSTokenType::BadUrl};
      }

      if (current == U'\\')
      {
        if (StartsWithValidEscape(current, next))
        {
          url.push_back(ConsumeEscapedCodePoint());
          continue;
        }

        ParseError(CSSParseError::InvalidCharacterInUrl);
        ConsumeRemnantsOfBadUrl();
        return CSSToken {CSSTokenType::BadUrl};
      }

      url.push_back(current);
      continue;
    }
  }

  char32 CSSTokenizer::ConsumeEscapedCodePoint() noexcept
  {
    auto current = _inputStream.NextInputCharacter();
    _inputStream.Consume();

    if (IsHexDigit(current))
    {
      uint32 value = Krys::Text::ToASCIIHexValue(current);

      size_t digits = 1uz;
      while (digits < 6uz && IsHexDigit(_inputStream.NextInputCharacter()))
      {
        current = _inputStream.NextInputCharacter();
        _inputStream.Consume();

        value <<= 4u;
        value |= Krys::Text::ToASCIIHexValue(current);
        ++digits;
      }

      if (IsWhitespace(_inputStream.NextInputCharacter()))
      {
        _inputStream.Consume();
      }

      if (value == 0u || value > 0x10FFFFu || Krys::Text::Unicode::IsSurrogate(value))
      {
        ParseError(CSSParseError::InvalidEscapeSequence);
        return 0xFFFD;
      }

      return value;
    }

    if (current == EOFMarker)
    {
      ParseError(CSSParseError::UnexpectedEOFInEscapeSequence);
      return 0xFFFD;
    }

    return current;
  }

  bool CSSTokenizer::StartsWithValidEscape(char32 first, char32 second) const noexcept
  {
    if (first != U'\\')
    {
      return false;
    }

    if (IsNewline(second))
    {
      return false;
    }

    return true;
  }

  bool CSSTokenizer::WouldStartIdentifier(char32 first, char32 second, char32 third) const noexcept
  {
    if (first == U'-')
    {
      if (IsIdentStartCodePoint(second) || second == U'-' || StartsWithValidEscape(second, third))
      {
        return true;
      }

      return false;
    }

    if (IsIdentStartCodePoint(first))
    {
      return true;
    }

    if (first == U'\\')
    {
      return StartsWithValidEscape(first, second);
    }

    return false;
  }

  bool CSSTokenizer::StartsWithNumber(char32 first, char32 second, char32 third) const noexcept
  {
    if (first == U'+' || first == U'-')
    {
      if (IsDigit(second))
      {
        return true;
      }

      if (second == U'.' && IsDigit(third))
      {
        return true;
      }

      return false;
    }

    if (first == U'.')
    {
      if (IsDigit(second))
      {
        return true;
      }

      return false;
    }

    if (IsDigit(first))
    {
      return true;
    }

    return false;
  }

  utf32_string CSSTokenizer::ConsumeIdentSequence() noexcept
  {
    utf32_string result;

    while (true)
    {
      auto current = _inputStream.NextInputCharacter();
      _inputStream.Consume();
      auto next = _inputStream.NextInputCharacter();

      if (IsIdentCodePoint(current))
      {
        result.push_back(current);
        continue;
      }

      if (StartsWithValidEscape(current, next))
      {
        result.push_back(ConsumeEscapedCodePoint());
        continue;
      }

      _inputStream.Reconsume(current);
      return result;
    }
  }

  ParsedInt64OrDouble CSSTokenizer::ConsumeNumber() noexcept
  {
    NumericTokenType type = NumericTokenType::Integer;
    utf32_string repr;

    if (_inputStream.NextInputCharacter() == U'+' || _inputStream.NextInputCharacter() == U'-')
    {
      repr.push_back(_inputStream.NextInputCharacter());
      _inputStream.Consume();
    }

    while (IsDigit(_inputStream.NextInputCharacter()))
    {
      repr.push_back(_inputStream.NextInputCharacter());
      _inputStream.Consume();
    }

    if (_inputStream.NextInputCharacter() == U'.' && IsDigit(_inputStream.Peek(1uz)))
    {
      repr.push_back(_inputStream.NextInputCharacter());
      repr.push_back(_inputStream.Peek(1uz));
      _inputStream.Consume(2uz);
      type = NumericTokenType::Number;

      while (IsDigit(_inputStream.NextInputCharacter()))
      {
        repr.push_back(_inputStream.NextInputCharacter());
        _inputStream.Consume();
      }
    }

    if (_inputStream.NextInputCharacter() == U'e' || _inputStream.NextInputCharacter() == U'E')
    {
      if (IsDigit(_inputStream.Peek(1uz)))
      {
        repr.push_back(_inputStream.NextInputCharacter());
        repr.push_back(_inputStream.Peek(1uz));
        _inputStream.Consume(2uz);
        type = NumericTokenType::Number;

        while (IsDigit(_inputStream.NextInputCharacter()))
        {
          repr.push_back(_inputStream.NextInputCharacter());
          _inputStream.Consume();
        }
      }
      else if ((_inputStream.Peek(1uz) == U'+' || _inputStream.Peek(1uz) == U'-')
               && IsDigit(_inputStream.Peek(2uz)))
      {
        repr.push_back(_inputStream.NextInputCharacter());
        repr.push_back(_inputStream.Peek(1uz));
        repr.push_back(_inputStream.Peek(2uz));
        _inputStream.Consume(3uz);
        type = NumericTokenType::Number;

        while (IsDigit(_inputStream.NextInputCharacter()))
        {
          repr.push_back(_inputStream.NextInputCharacter());
          _inputStream.Consume();
        }
      }
    }

    return ConvertStringToNumber(repr, type);
  }

  ParsedInt64OrDouble CSSTokenizer::ConvertStringToNumber(utf32_stringview codePoints,
                                                          NumericTokenType type) noexcept
  {
    auto position = codePoints.begin();

    auto peek = [&]() -> char32_t
    {
      return position != codePoints.end() ? *position : U'\0';
    };

    auto consume = [&]() -> char32_t
    {
      assert(position != codePoints.end());
      return *position++;
    };

    int sign = 1;
    if (peek() == U'-')
    {
      sign = -1;
      consume();
    }
    else if (peek() == U'+')
    {
      consume();
    }

    uint64 integer = 0;
    while (IsDigit(peek()))
    {
      integer = integer * 10U + (static_cast<uint64>(consume()) - U'0');
    }

    if (type == NumericTokenType::Integer)
    {
      int64 value = static_cast<int64>(integer) * sign;
      return ParsedInt64OrDouble {.Value = value, .Type = NumericTokenType::Integer};
    }

    uint64 fraction = 0;
    int fractionDigits = 0;

    if (peek() == U'.')
    {
      consume();

      while (IsDigit(peek()))
      {
        fraction = fraction * 10U + (static_cast<uint64>(consume()) - U'0');
        ++fractionDigits;
      }
    }

    int exponentSign = 1;
    uint64 exponent = 0;

    if (peek() == U'e' || peek() == U'E')
    {
      consume();

      if (peek() == U'-')
      {
        exponentSign = -1;
        consume();
      }
      else if (peek() == U'+')
      {
        consume();
      }

      while (IsDigit(peek()))
      {
        exponent = exponent * 10U + (static_cast<uint64>(consume()) - U'0');
      }
    }

    double value = sign * (integer + fraction * std::pow(10.0, -fractionDigits))
                   * std::pow(10.0, exponentSign * static_cast<int>(exponent));

    return ParsedInt64OrDouble {.Value = value, .Type = NumericTokenType::Number};
  }
  void CSSTokenizer::ConsumeRemnantsOfBadUrl() noexcept
  {
    while (true)
    {
      auto current = _inputStream.NextInputCharacter();
      _inputStream.Consume();

      if (current == U')' || current == EOFMarker)
      {
        return;
      }

      if (StartsWithValidEscape(current, _inputStream.NextInputCharacter()))
      {
        // avoids interpreting the escaped code point as a close paren for the bad url.
        (void)ConsumeEscapedCodePoint();
        continue;
      }

      continue; // do nothing, just consume the character
    }
  }

  void CSSTokenizer::ConsumeWhitespace() noexcept
  {
    while (true)
    {
      if (IsWhitespace(_inputStream.NextInputCharacter()))
      {
        _inputStream.Consume();
        continue;
      }

      return;
    }
  }

  void CSSTokenizer::ParseError(CSSParseError error) noexcept
  {
    _parseErrors.push_back({
      .Error = error,
      .Location = _inputStream.CurrentLocation(),
    });
  }
}