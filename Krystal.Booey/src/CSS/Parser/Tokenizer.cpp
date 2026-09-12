#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <charconv>

namespace krys::boo::css
{
  Tokenizer::Tokenizer(InputStream &inputStream) noexcept : _inputStream(inputStream)
  {
  }

  bool Tokenizer::PumpTokenizer() noexcept
  {
    // NOTE(webkit-optimisation): To avoid resizing we err on the side of reserving too much space. Most
    // strings we tokenize have about 3.5 to 5 characters per token.
    _tokens.reserve(_inputStream.Size() / 3uz);

    while (true)
    {
      auto token = ConsumeToken();
      if (token.Type() == TokenType::EndOfFile) KRYS_UNLIKELY
      {
        return !_tokens.empty();
      }

      _tokens.emplace_back(krys::move(token));
    }
  }

  Token Tokenizer::ConsumeToken(bool unicodeRangesAllowed) noexcept
  {
    ConsumeComments();

    auto current = _inputStream.NextInputCharacter();
    _inputStream.Consume();
    auto next = _inputStream.NextInputCharacter();

    if (IsWhitespace(current))
    {
      ConsumeWhitespace();
      return Token {TokenType::Whitespace};
    }

    if (current == U'"')
    {
      return ConsumeStringToken(U'"');
    }

    if (current == U'#')
    {
      if (IsIdentCodePoint(next) || StartsWithValidEscape(next, _inputStream.Peek(1uz)))
      {
        auto token = Token {TokenType::Hash};

        if (StartsWithIdentifier(next, _inputStream.Peek(1uz), _inputStream.Peek(2uz)))
        {
          token.HashType(HashTokenType::Id);
        }

        token.IdentCodePoints(ConsumeIdentSequence());
        return token;
      }

      return Token {TokenType::Delim, current};
    }

    if (current == U'\'')
    {
      return ConsumeStringToken(U'\'');
    }

    if (current == U'(')
    {
      return BlockStart(TokenType::OpenParen);
    }

    if (current == U')')
    {
      return BlockEnd(TokenType::CloseParen);
    }

    if (current == U'+')
    {
      if (StartsWithNumber(current, next, _inputStream.Peek(1uz)))
      {
        _inputStream.Reconsume(current);
        return ConsumeNumericToken();
      }

      return Token {TokenType::Delim, current};
    }

    if (current == U',')
    {
      return Token {TokenType::Comma};
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
        return Token {TokenType::CDC};
      }

      if (StartsWithIdentifier(current, next, _inputStream.Peek(1uz)))
      {
        _inputStream.Reconsume(current);
        return ConsumeIdentLikeToken();
      }

      return Token {TokenType::Delim, current};
    }

    if (current == U'.')
    {
      if (StartsWithNumber(current, next, _inputStream.Peek(1uz)))
      {
        _inputStream.Reconsume(current);
        return ConsumeNumericToken();
      }

      return Token {TokenType::Delim, current};
    }

    if (current == U':')
    {
      return Token {TokenType::Colon};
    }

    if (current == U';')
    {
      return Token {TokenType::Semicolon};
    }

    if (current == U'<')
    {
      if (next == U'!' && _inputStream.Peek(1uz) == U'-' && _inputStream.Peek(2uz) == U'-')
      {
        _inputStream.Consume(3uz);
        return Token {TokenType::CDO};
      }

      return Token {TokenType::Delim, current};
    }

    if (current == U'@')
    {
      if (StartsWithIdentifier(next, _inputStream.Peek(1uz), _inputStream.Peek(2uz)))
      {
        auto token = Token {TokenType::AtKeyword};
        token.IdentCodePoints(ConsumeIdentSequence());
        return token;
      }

      return Token {TokenType::Delim, current};
    }

    if (current == U'[')
    {
      return BlockStart(TokenType::OpenSquare);
    }

    if (current == U'\\')
    {
      if (StartsWithValidEscape(current, next))
      {
        _inputStream.Reconsume(current);
        return ConsumeIdentLikeToken();
      }

      ParseError(ParseError::UnexpectedReverseSolidus);
      return Token {TokenType::Delim, current};
    }

    if (current == U']')
    {
      return BlockEnd(TokenType::CloseSquare);
    }

    if (current == U'{')
    {
      return BlockStart(TokenType::OpenCurly);
    }

    if (current == U'}')
    {
      return BlockEnd(TokenType::CloseCurly);
    }

    if (IsDigit(current))
    {
      _inputStream.Reconsume(current);
      return ConsumeNumericToken();
    }

    if (current == U'U' || current == U'u')
    {
      if (unicodeRangesAllowed && StartsWithUnicodeRange(current, next, _inputStream.Peek(1uz)))
      {
        _inputStream.Reconsume(current);
        return ConsumeUnicodeRangeToken();
      }

      _inputStream.Reconsume(current);
      return ConsumeIdentLikeToken();
    }

    if (IsIdentStartCodePoint(current))
    {
      _inputStream.Reconsume(current);
      return ConsumeIdentLikeToken();
    }

    if (current == EOFMarker)
    {
      return Token {TokenType::EndOfFile};
    }

    return Token {TokenType::Delim, current};
  }

  void Tokenizer::ConsumeComments() noexcept
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
          ParseError(ParseError::UnexpectedEOFInComment);
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

  Token Tokenizer::ConsumeNumericToken() noexcept
  {
    auto number = ConsumeNumber();

    if (StartsWithIdentifier(_inputStream.NextInputCharacter(), _inputStream.Peek(1uz),
                             _inputStream.Peek(2uz)))
    {
      auto token = Token {TokenType::Dimension};
      token.Unit(ConsumeIdentSequence());
      token.NumericValue(number);

      return token;
    }

    if (_inputStream.NextInputCharacter() == U'%')
    {
      _inputStream.Consume();

      auto token = Token {TokenType::Percentage};
      token.NumericValue(number);

      return token;
    }

    auto token = Token {TokenType::Number};
    token.NumericValue(number);

    return token;
  }

  Token Tokenizer::ConsumeIdentLikeToken() noexcept
  {
    auto string = ConsumeIdentSequence();

    if (krys::text::IsASCIICaselessEqual(string, utf32_stringview(U"url"))
        && _inputStream.NextInputCharacter() == U'(')
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
        return Token {TokenType::Function, string};
      }

      return ConsumeUrlToken();
    }

    if (_inputStream.NextInputCharacter() == U'(')
    {
      _inputStream.Consume();
      return Token {TokenType::Function, string};
    }

    return Token {TokenType::Ident, string};
  }

  Token Tokenizer::ConsumeStringToken(char32 endingCodePoint) noexcept
  {
    utf32_string codePoints;
    while (true)
    {
      auto current = _inputStream.NextInputCharacter();
      _inputStream.Consume();
      auto next = _inputStream.NextInputCharacter();

      if (current == endingCodePoint)
      {
        return Token {TokenType::String, codePoints};
      }

      if (current == EOFMarker)
      {
        ParseError(ParseError::UnexpectedEOFInString);
        return Token {TokenType::String, codePoints};
      }

      if (current == U'\n')
      {
        ParseError(ParseError::UnexpectedNewlineInString);
        _inputStream.Reconsume(current);
        return Token {TokenType::BadString};
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

  Token Tokenizer::ConsumeUrlToken() noexcept
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
        return Token {TokenType::Url, url};
      }

      if (current == EOFMarker)
      {
        ParseError(ParseError::UnexpectedEOFInUrl);
        return Token {TokenType::Url, url};
      }

      if (IsWhitespace(current))
      {
        ConsumeWhitespace();

        if (_inputStream.NextInputCharacter() == U')')
        {
          _inputStream.Consume();
          return Token {TokenType::Url, url};
        }

        if (_inputStream.NextInputCharacter() == EOFMarker)
        {
          ParseError(ParseError::UnexpectedEOFInUrl);
          return Token {TokenType::Url, url};
        }

        ParseError(ParseError::InvalidCharacterInUrl);
        ConsumeRemnantsOfBadUrl();
        return Token {TokenType::BadUrl};
      }

      if (current == U'"' || current == U'\'' || current == U'(' || IsNonPrintableCodePoint(current))
      {
        ParseError(ParseError::InvalidCharacterInUrl);
        ConsumeRemnantsOfBadUrl();
        return Token {TokenType::BadUrl};
      }

      if (current == U'\\')
      {
        if (StartsWithValidEscape(current, next))
        {
          url.push_back(ConsumeEscapedCodePoint());
          continue;
        }

        ParseError(ParseError::InvalidCharacterInUrl);
        ConsumeRemnantsOfBadUrl();
        return Token {TokenType::BadUrl};
      }

      url.push_back(current);
      continue;
    }
  }

  char32 Tokenizer::ConsumeEscapedCodePoint() noexcept
  {
    auto current = _inputStream.NextInputCharacter();
    _inputStream.Consume();

    if (IsHexDigit(current))
    {
      uint32 value = krys::text::ToASCIIHexValue(current);

      size_t digits = 1uz;
      while (digits < 6uz && IsHexDigit(_inputStream.NextInputCharacter()))
      {
        current = _inputStream.NextInputCharacter();
        _inputStream.Consume();

        value <<= 4u;
        value |= krys::text::ToASCIIHexValue(current);
        ++digits;
      }

      if (IsWhitespace(_inputStream.NextInputCharacter()))
      {
        _inputStream.Consume();
      }

      if (value == 0u || value > 0x10FFFFu || krys::text::IsSurrogate(value))
      {
        ParseError(ParseError::InvalidEscapeSequence);
        return 0xFFFD;
      }

      return value;
    }

    if (current == EOFMarker)
    {
      ParseError(ParseError::UnexpectedEOFInEscapeSequence);
      return 0xFFFD;
    }

    return current;
  }

  bool Tokenizer::StartsWithValidEscape(char32 first, char32 second) const noexcept
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

  bool Tokenizer::StartsWithIdentifier(char32 first, char32 second, char32 third) const noexcept
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

  bool Tokenizer::StartsWithNumber(char32 first, char32 second, char32 third) const noexcept
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

  bool Tokenizer::StartsWithUnicodeRange(char32 first, char32 second, char32 third) const noexcept
  {
    if ((first == U'U' || first == U'u') && second == U'+' && (IsHexDigit(third) || third == U'?'))
    {
      return true;
    }

    return false;
  }

  bool Tokenizer::StartsWithExponent(char32 first, char32 second, char32 third) const noexcept
  {
    if (first != U'E' && first != U'e')
    {
      return false;
    }

    if (second == U'-' || second == U'+')
    {
      return IsDigit(third);
    }

    return IsDigit(second);
  }

  utf32_string Tokenizer::ConsumeIdentSequence() noexcept
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

  NumericValue Tokenizer::ConsumeNumber() noexcept
  {
    NumericValueType type = NumericValueType::Integer;
    NumericSignChar sign = NumericSignChar::Missing;

    // The characters below are all ASCII characters, so we can safely use a string buffer to store them.
    string numberPart = "";
    string exponentPart = "";

    if (_inputStream.NextInputCharacter() == U'+' || _inputStream.NextInputCharacter() == U'-')
    {
      if (_inputStream.NextInputCharacter() == U'+')
      {
        sign = NumericSignChar::Plus;
      }
      else
      {
        sign = NumericSignChar::Minus;
        numberPart.push_back(static_cast<char8>(_inputStream.NextInputCharacter()));
      }

      _inputStream.Consume();
    }

    while (IsDigit(_inputStream.NextInputCharacter()))
    {
      numberPart.push_back(static_cast<char8>(_inputStream.NextInputCharacter()));
      _inputStream.Consume();
    }

    if (_inputStream.NextInputCharacter() == U'.' && IsDigit(_inputStream.Peek(1uz)))
    {
      numberPart.push_back(static_cast<char8>(_inputStream.NextInputCharacter()));
      numberPart.push_back(static_cast<char8>(_inputStream.Peek(1uz)));
      _inputStream.Consume(2uz);

      while (IsDigit(_inputStream.NextInputCharacter()))
      {
        numberPart.push_back(static_cast<char8>(_inputStream.NextInputCharacter()));
        _inputStream.Consume();
      }

      type = NumericValueType::Number;
    }

    if (StartsWithExponent(_inputStream.NextInputCharacter(), _inputStream.Peek(1uz), _inputStream.Peek(2uz)))
    {
      _inputStream.Consume();

      if (_inputStream.NextInputCharacter() == U'+')
      {
        _inputStream.Consume(); // skip the plus
      }
      else if (_inputStream.NextInputCharacter() == U'-')
      {
        exponentPart.push_back('-');
        _inputStream.Consume();
      }

      while (IsDigit(_inputStream.NextInputCharacter()))
      {
        exponentPart.push_back(static_cast<char8>(_inputStream.NextInputCharacter()));
        _inputStream.Consume();
      }

      type = NumericValueType::Number;
    }

    double value;
    auto [nptr, nec] = std::from_chars(numberPart.data(), numberPart.data() + numberPart.size(), value);
    assert(nec == std::errc());

    if (!exponentPart.empty())
    {
      int64 exponent;
      auto [eptr, eec] =
        std::from_chars(exponentPart.data(), exponentPart.data() + exponentPart.size(), exponent);
      assert(eec == std::errc());

      value *= std::pow(10.0, static_cast<double>(exponent));
    }

    return NumericValue {.Value = value, .Type = type, .SignCharacter = sign};
  }

  Token Tokenizer::ConsumeUnicodeRangeToken() noexcept
  {
    assert(StartsWithUnicodeRange(_inputStream.NextInputCharacter(), _inputStream.Peek(1uz),
                                  _inputStream.Peek(2uz)));

    auto ConvertHexStrToInt = [](const string &str) noexcept -> int32
    {
      int32 value = 0;
      auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value, 16);
      assert(ec == std::errc());
      return value;
    };

    string consumed;
    while (consumed.size() < 6uz && IsHexDigit(_inputStream.NextInputCharacter()))
    {
      consumed.push_back(static_cast<char8>(_inputStream.NextInputCharacter()));
      _inputStream.Consume();
    }

    bool hasQuestionMark = false;
    while (consumed.size() < 6uz && _inputStream.NextInputCharacter() == U'?')
    {
      hasQuestionMark = true;
      consumed.push_back(static_cast<char8>(_inputStream.NextInputCharacter()));
      _inputStream.Consume();
    }

    Token token {TokenType::UnicodeRange};
    if (hasQuestionMark)
    {
      string start = consumed;

      auto questionMarkIndex = start.find_first_of('?');
      while (questionMarkIndex != string::npos)
      {
        start[questionMarkIndex] = '0';
        questionMarkIndex = start.find_first_of('?', questionMarkIndex + 1uz);
      }

      // reuse the consumed string to store the end of the tokens, since we don't need it anymore
      string &end = consumed;

      questionMarkIndex = end.find_first_of('?');
      while (questionMarkIndex != string::npos)
      {
        end[questionMarkIndex] = 'F';
        questionMarkIndex = end.find_first_of('?', questionMarkIndex + 1uz);
      }

      token.UnicodeRange(ConvertHexStrToInt(start), ConvertHexStrToInt(end));
      return token;
    }

    int32 startOfRange = ConvertHexStrToInt(consumed);
    if (_inputStream.NextInputCharacter() == U'-' && IsHexDigit(_inputStream.Peek(1uz)))
    {
      _inputStream.Consume();
      consumed.clear();

      while (consumed.size() < 6uz && IsHexDigit(_inputStream.NextInputCharacter()))
      {
        consumed.push_back(static_cast<char8>(_inputStream.NextInputCharacter()));
        _inputStream.Consume();
      }

      int32 endOfRange = ConvertHexStrToInt(consumed);
      token.UnicodeRange(startOfRange, endOfRange);
      return token;
    }

    token.UnicodeRange(startOfRange, startOfRange);
    return token;
  }

  void Tokenizer::ConsumeRemnantsOfBadUrl() noexcept
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

  void Tokenizer::ConsumeWhitespace() noexcept
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

  void Tokenizer::ParseError(css::ParseError error) noexcept
  {
    _errors.push_back({.Error = error, .Location = _inputStream.CurrentLocation()});
  }

  Token Tokenizer::BlockStart(TokenType type) noexcept
  {
    assert(type == TokenType::OpenSquare || type == TokenType::OpenParen || type == TokenType::OpenCurly);

    _blockStack.push_back(type);
    return Token(type, BlockTokenType::Start);
  }

  Token Tokenizer::BlockEnd(TokenType type) noexcept
  {
    assert(type == TokenType::CloseSquare || type == TokenType::CloseParen || type == TokenType::CloseCurly);

    auto startType = [](TokenType endType)
    {
      switch (endType)
      {
        case TokenType::CloseSquare: return TokenType::OpenSquare;
        case TokenType::CloseParen:  return TokenType::OpenParen;
        case TokenType::CloseCurly:  return TokenType::OpenCurly;
      }
      return TokenType::Uninitialized; // unreachable
    }(type);

    if (!_blockStack.empty() && _blockStack.back() == startType)
    {
      _blockStack.pop_back();
      return Token(type, BlockTokenType::End);
    }

    return Token(type);
  }
}