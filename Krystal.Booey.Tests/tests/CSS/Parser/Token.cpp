#include "Krystal.Booey/CSS/Parser/Token.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD string ToNarrow(utf8_stringview str) noexcept
    {
      return string(reinterpret_cast<const char *>(str.data()), str.size());
    }

    KRYS_NODISCARD List<Token> Tokenize(utf8_stringview css) noexcept
    {
      InputStream inputStream {utf8_string(css)};
      Tokenizer tokenizer(inputStream);
      tokenizer.PumpTokenizer();

      auto tokens = tokenizer.Tokens();
      List<Token> result;
      while (!tokens.IsAtEnd())
      {
        result.push_back(tokens.Consume());
      }
      return result;
    }

    KRYS_NODISCARD Token TokenizeOne(utf8_stringview css) noexcept
    {
      auto tokens = Tokenize(css);
      REQUIRE(tokens.size() == 1uz);
      return tokens.front();
    }

    KRYS_NODISCARD string Serialise(utf8_stringview css) noexcept
    {
      InputStream inputStream {utf8_string(css)};
      Tokenizer tokenizer(inputStream);
      tokenizer.PumpTokenizer();
      return ToNarrow(tokenizer.Tokens().Serialise(TokenSerialisationMode::Normal));
    }

    KRYS_NODISCARD string SerialiseForCustomProperty(utf8_stringview css) noexcept
    {
      InputStream inputStream {utf8_string(css)};
      Tokenizer tokenizer(inputStream);
      tokenizer.PumpTokenizer();
      return ToNarrow(tokenizer.Tokens().Serialise(TokenSerialisationMode::CustomProperty));
    }

    KRYS_NODISCARD string Serialise(const Token &token) noexcept
    {
      CSSOMString builder;
      token.Serialise(builder);
      return ToNarrow(builder);
    }

    KRYS_NODISCARD Token UnicodeRangeToken(char32 start, char32 end) noexcept
    {
      Token token {TokenType::UnicodeRange};
      token.UnicodeRange(start, end);
      return token;
    }
  }

  TEST_CASE("Token::Type", "[CSS][Token]")
  {
    CHECK(TokenizeOne(u8"foo").Type() == TokenType::Ident);
    CHECK(TokenizeOne(u8"foo(").Type() == TokenType::Function);
    CHECK(TokenizeOne(u8"@foo").Type() == TokenType::AtKeyword);
    CHECK(TokenizeOne(u8"#foo").Type() == TokenType::Hash);
    CHECK(TokenizeOne(u8"\"foo\"").Type() == TokenType::String);
    CHECK(TokenizeOne(u8"url(foo)").Type() == TokenType::Url);
    CHECK(TokenizeOne(u8"url(a b)").Type() == TokenType::BadUrl);
    CHECK(TokenizeOne(u8"/").Type() == TokenType::Delim);
    CHECK(TokenizeOne(u8"1").Type() == TokenType::Number);
    CHECK(TokenizeOne(u8"1%").Type() == TokenType::Percentage);
    CHECK(TokenizeOne(u8"1px").Type() == TokenType::Dimension);
    CHECK(TokenizeOne(u8" \t\n").Type() == TokenType::Whitespace);
    CHECK(TokenizeOne(u8"<!--").Type() == TokenType::CDO);
    CHECK(TokenizeOne(u8"-->").Type() == TokenType::CDC);
    CHECK(TokenizeOne(u8":").Type() == TokenType::Colon);
    CHECK(TokenizeOne(u8";").Type() == TokenType::Semicolon);
    CHECK(TokenizeOne(u8",").Type() == TokenType::Comma);
    CHECK(UnicodeRangeToken(0x25, 0xFF).Type() == TokenType::UnicodeRange);

    auto badString = Tokenize(u8"\"foo\n");
    REQUIRE(!badString.empty());
    CHECK(badString.front().Type() == TokenType::BadString);
  }

  TEST_CASE("Token::BlockType", "[CSS][Token]")
  {
    auto tokens = Tokenize(u8"([{}])");
    REQUIRE(tokens.size() == 6uz);

    CHECK(tokens[0].Type() == TokenType::OpenParen);
    CHECK(tokens[1].Type() == TokenType::OpenSquare);
    CHECK(tokens[2].Type() == TokenType::OpenCurly);
    for (size_t i = 0; i < 3uz; ++i)
    {
      INFO(i);
      CHECK(tokens[i].BlockType() == BlockTokenType::Start);
      CHECK(tokens[i].IsBlockStart());
      CHECK_FALSE(tokens[i].IsBlockEnd());
    }

    CHECK(tokens[3].Type() == TokenType::CloseCurly);
    CHECK(tokens[4].Type() == TokenType::CloseSquare);
    CHECK(tokens[5].Type() == TokenType::CloseParen);
    for (size_t i = 3; i < 6uz; ++i)
    {
      INFO(i);
      CHECK(tokens[i].BlockType() == BlockTokenType::End);
      CHECK(tokens[i].IsBlockEnd());
      CHECK_FALSE(tokens[i].IsBlockStart());
    }

    auto ident = TokenizeOne(u8"foo");
    CHECK(ident.BlockType() == BlockTokenType::None);
    CHECK_FALSE(ident.IsBlockStart());
    CHECK_FALSE(ident.IsBlockEnd());

    Token constructed {TokenType::OpenParen, BlockTokenType::Start};
    CHECK(constructed.IsBlockStart());
  }

  TEST_CASE("Token::BlockType(function)", "[CSS][Token]")
  {
    // A function token opens a block that the matching ) closes.
    auto tokens = Tokenize(u8"foo(a)");
    REQUIRE(tokens.size() == 3uz);
    CHECK(tokens[0].Type() == TokenType::Function);
    CHECK(tokens[0].IsBlockStart());
    CHECK(tokens[2].Type() == TokenType::CloseParen);
    CHECK(tokens[2].IsBlockEnd());
  }

  TEST_CASE("Token::IdentCodePoints", "[CSS][Token]")
  {
    CHECK(ToNarrow(TokenizeOne(u8"foo").IdentCodePoints()) == "foo");
    CHECK(ToNarrow(TokenizeOne(u8"foo(").IdentCodePoints()) == "foo");
    CHECK(ToNarrow(TokenizeOne(u8"@foo").IdentCodePoints()) == "foo");
    CHECK(ToNarrow(TokenizeOne(u8"#foo").IdentCodePoints()) == "foo");
    CHECK(ToNarrow(TokenizeOne(u8"\"foo\"").IdentCodePoints()) == "foo");
    CHECK(ToNarrow(TokenizeOne(u8"url(foo)").IdentCodePoints()) == "foo");
    CHECK(ToNarrow(TokenizeOne(u8"/").IdentCodePoints()) == "/");

    // Escapes are resolved.
    CHECK(ToNarrow(TokenizeOne(u8"\\66 oo").IdentCodePoints()) == "foo");
    // Non-ASCII is stored as UTF-8.
    CHECK(ToNarrow(TokenizeOne(u8"café").IdentCodePoints()) == "caf\xC3\xA9");

    // Empty values.
    CHECK(TokenizeOne(u8"\"\"").IdentCodePoints().empty());
    CHECK(TokenizeOne(u8"url()").IdentCodePoints().empty());

    // The setter replaces the value.
    Token token {TokenType::Ident, utf32_stringview(U"foo")};
    token.IdentCodePoints(U"bar");
    CHECK(ToNarrow(token.IdentCodePoints()) == "bar");
    token.IdentCodePoints(U"");
    CHECK(token.IdentCodePoints().empty());
  }

  TEST_CASE("Token::Delimiter", "[CSS][Token]")
  {
    CHECK(TokenizeOne(u8"/").Delimiter() == U'/');
    CHECK(TokenizeOne(u8"*").Delimiter() == U'*');
    CHECK(TokenizeOne(u8"!").Delimiter() == U'!');

    Token nonASCII {TokenType::Delim, U'é'};
    CHECK(nonASCII.Delimiter() == U'é');
    CHECK(ToNarrow(nonASCII.IdentCodePoints()) == "\xC3\xA9");
  }

  TEST_CASE("Token::HashType", "[CSS][Token]")
  {
    CHECK(TokenizeOne(u8"#foo").HashType() == HashTokenType::Id);
    CHECK(TokenizeOne(u8"#-foo").HashType() == HashTokenType::Id);
    CHECK(TokenizeOne(u8"#1a").HashType() == HashTokenType::Unrestricted);
    CHECK(TokenizeOne(u8"#--").HashType() == HashTokenType::Id);

    Token token {TokenType::Hash};
    CHECK(token.HashType() == HashTokenType::Unrestricted);
    token.HashType(HashTokenType::Id);
    CHECK(token.HashType() == HashTokenType::Id);
  }

  TEST_CASE("Token::ValueId", "[CSS][Token]")
  {
    CHECK(TokenizeOne(u8"red").ValueId() == ValueId::Red);
    CHECK(TokenizeOne(u8"auto").ValueId() == ValueId::Auto);
    CHECK(TokenizeOne(u8"not-a-keyword").ValueId() == ValueId::Invalid);

    // Only ident tokens have a value keyword.
    CHECK(TokenizeOne(u8"red(").ValueId() == ValueId::Invalid);
    CHECK(TokenizeOne(u8"\"red\"").ValueId() == ValueId::Invalid);
    CHECK(TokenizeOne(u8"#red").ValueId() == ValueId::Invalid);
    CHECK(TokenizeOne(u8"1").ValueId() == ValueId::Invalid);
  }

  TEST_CASE("Token::ValueId(case-insensitive)", "[CSS][Token]")
  {
    // Keywords are ASCII case-insensitive.
    CHECK(TokenizeOne(u8"RED").ValueId() == ValueId::Red);
    CHECK(TokenizeOne(u8"Auto").ValueId() == ValueId::Auto);
  }

  TEST_CASE("Token::ValueId(cached)", "[CSS][Token]")
  {
    Token token {TokenType::Ident, utf32_stringview(U"red")};
    CHECK(token.ValueId() == ValueId::Red);
    CHECK(token.ValueId() == ValueId::Red);

    // Changing the text invalidates the cached keyword.
    token.IdentCodePoints(U"auto");
    CHECK(token.ValueId() == ValueId::Auto);

    // Copies keep the cached keyword.
    auto copy = token;
    CHECK(copy.ValueId() == ValueId::Auto);
  }

  TEST_CASE("Token::FunctionId", "[CSS][Token]")
  {
    CHECK(TokenizeOne(u8"calc(").FunctionId() == ValueId::Calc);
    CHECK(TokenizeOne(u8"rgb(").FunctionId() == ValueId::Rgb);
    CHECK(TokenizeOne(u8"not-a-function(").FunctionId() == ValueId::Invalid);

    // Only function tokens have a function keyword.
    CHECK(TokenizeOne(u8"calc").FunctionId() == ValueId::Invalid);
  }

  TEST_CASE("Token::FunctionId(case-insensitive)", "[CSS][Token]")
  {
    CHECK(TokenizeOne(u8"CALC(").FunctionId() == ValueId::Calc);
  }

  TEST_CASE("Token::PropertyId", "[CSS][Token]")
  {
    CHECK(TokenizeOne(u8"color").PropertyId() == PropertyId::Color);
    CHECK(TokenizeOne(u8"margin-top").PropertyId() == PropertyId::MarginTop);
    CHECK(TokenizeOne(u8"not-a-property").PropertyId() == PropertyId::Invalid);

    // Only ident tokens name a property.
    CHECK(TokenizeOne(u8"\"color\"").PropertyId() == PropertyId::Invalid);
    CHECK(TokenizeOne(u8"color(").PropertyId() == PropertyId::Invalid);
  }

  TEST_CASE("Token::PropertyId(case-insensitive)", "[CSS][Token]")
  {
    // Property names are ASCII case-insensitive.
    CHECK(TokenizeOne(u8"COLOR").PropertyId() == PropertyId::Color);
  }

  TEST_CASE("Token::NumericValue", "[CSS][Token]")
  {
    auto integer = TokenizeOne(u8"42");
    CHECK(integer.NumericValue() == 42.0);
    CHECK(integer.NumericValueType() == NumericValueType::Integer);
    CHECK(integer.NumericSign() == NumericSignChar::Missing);

    auto number = TokenizeOne(u8"4.5");
    CHECK(number.NumericValue() == 4.5);
    CHECK(number.NumericValueType() == NumericValueType::Number);

    CHECK(TokenizeOne(u8"1e2").NumericValueType() == NumericValueType::Number);
    CHECK(TokenizeOne(u8"1e2").NumericValue() == 100.0);

    auto plus = TokenizeOne(u8"+3");
    CHECK(plus.NumericValue() == 3.0);
    CHECK(plus.NumericSign() == NumericSignChar::Plus);

    auto minus = TokenizeOne(u8"-3");
    CHECK(minus.NumericValue() == -3.0);
    CHECK(minus.NumericSign() == NumericSignChar::Minus);

    auto percentage = TokenizeOne(u8"50%");
    CHECK(percentage.NumericValue() == 50.0);
    CHECK(percentage.NumericValueType() == NumericValueType::Integer);

    auto dimension = TokenizeOne(u8"2.5em");
    CHECK(dimension.NumericValue() == 2.5);
    CHECK(dimension.NumericValueType() == NumericValueType::Number);

    Token token {TokenType::Number};
    token.NumericValue(css::NumericValue {
      .Value = 7.0, .Type = NumericValueType::Number, .SignCharacter = NumericSignChar::Plus});
    CHECK(token.NumericValue() == 7.0);
    CHECK(token.NumericValueType() == NumericValueType::Number);
    CHECK(token.NumericSign() == NumericSignChar::Plus);
  }

  TEST_CASE("Token::Unit", "[CSS][Token]")
  {
    CHECK(ToNarrow(TokenizeOne(u8"10px").Unit()) == "px");
    CHECK(ToNarrow(TokenizeOne(u8"1.5em").Unit()) == "em");
    // Units keep their original case.
    CHECK(ToNarrow(TokenizeOne(u8"10PX").Unit()) == "PX");
    // Escapes in units are resolved.
    CHECK(ToNarrow(TokenizeOne(u8"10\\70 x").Unit()) == "px");
    // An "e" not followed by digits is a unit.
    CHECK(ToNarrow(TokenizeOne(u8"1em").Unit()) == "em");

    Token token {TokenType::Dimension};
    token.Unit(U"vh");
    CHECK(ToNarrow(token.Unit()) == "vh");
  }

  TEST_CASE("Token::OriginalText", "[CSS][Token]")
  {
    CHECK(ToNarrow(TokenizeOne(u8"42").OriginalText()) == "42");
    CHECK(ToNarrow(TokenizeOne(u8"+3").OriginalText()) == "+3");
    CHECK(ToNarrow(TokenizeOne(u8"-1.50E+2").OriginalText()) == "-1.50E+2");
    CHECK(ToNarrow(TokenizeOne(u8".5").OriginalText()) == ".5");
    // Percentages don't include the %.
    CHECK(ToNarrow(TokenizeOne(u8"050%").OriginalText()) == "050");
    // Dimensions include the unit as written.
    auto dimension = TokenizeOne(u8"2.50PX");
    CHECK(ToNarrow(dimension.OriginalText()) == "2.50PX");
    CHECK(ToNarrow(dimension.Unit()) == "PX");
    auto escaped = TokenizeOne(u8"10\\70 x");
    CHECK(ToNarrow(escaped.OriginalText()) == "10\\70 x");
    CHECK(ToNarrow(escaped.Unit()) == "px");
    // Whitespace keeps every character (after input preprocessing).
    CHECK(ToNarrow(TokenizeOne(u8" \t\r\n ").OriginalText()) == " \t\n ");

    // The original text and the unit can be set in either order.
    Token token {TokenType::Dimension};
    token.Unit(U"vh");
    token.OriginalText(U"1vh");
    token.Unit(U"vw");
    CHECK(ToNarrow(token.OriginalText()) == "1vh");
    CHECK(ToNarrow(token.Unit()) == "vw");
  }

  TEST_CASE("Token::UnicodeRange", "[CSS][Token]")
  {
    auto range = UnicodeRangeToken(0x25, 0xFF);
    CHECK(range.UnicodeRangeStart() == 0x25);
    CHECK(range.UnicodeRangeEnd() == 0xFF);

    auto single = UnicodeRangeToken(0x1F600, 0x1F600);
    CHECK(single.UnicodeRangeStart() == 0x1F600);
    CHECK(single.UnicodeRangeEnd() == 0x1F600);
  }

  TEST_CASE("Token::Serialise(ident-like)", "[CSS][Token]")
  {
    CHECK(Serialise(u8"foo") == "foo");
    CHECK(Serialise(u8"-foo") == "-foo");
    CHECK(Serialise(u8"--custom") == "--custom");
    CHECK(Serialise(u8"foo(") == "foo(");
    CHECK(Serialise(u8"@media") == "@media");
    CHECK(Serialise(u8"url(foo)") == "url(foo)");
    CHECK(Serialise(u8"url()") == "url()");
  }

  TEST_CASE("Token::Serialise(url escaping)", "[CSS][Token]")
  {
    // URLs are serialised as identifiers, so non-name characters get escaped. The escaped form still
    // tokenizes back to the same URL.
    CHECK(Serialise(u8"url(foo.png)") == "url(foo\\.png)");
    CHECK(Serialise(u8"url(a/b.png)") == "url(a\\/b\\.png)");

    auto original = TokenizeOne(u8"url(foo.png)");
    auto serialised = Serialise(original);
    CHECK(TokenizeOne(utf8_stringview(reinterpret_cast<const char8 *>(serialised.data()), serialised.size()))
          == original);
  }

  TEST_CASE("Token::Serialise(ident escaping)", "[CSS][Token]")
  {
    // A leading digit has to be escaped as a code point.
    CHECK(Serialise(u8"\\31 a") == "\\31 a");
    // A digit after a leading hyphen too.
    CHECK(Serialise(u8"-\\31 a") == "-\\31 a");
    // Characters outside the name code points are escaped.
    CHECK(Serialise(u8"a\\.b") == "a\\.b");
    // Non-ASCII passes through unescaped.
    CHECK(Serialise(u8"café") == "caf\xC3\xA9");
  }

  TEST_CASE("Token::Serialise(hash)", "[CSS][Token]")
  {
    CHECK(Serialise(u8"#abc") == "#abc");
    // Unrestricted hashes skip the leading-digit escape.
    CHECK(Serialise(u8"#1a") == "#1a");
    CHECK(TokenizeOne(u8"#1a").HashType() == HashTokenType::Unrestricted);
    CHECK(TokenizeOne(u8"#abc").HashType() == HashTokenType::Id);
  }

  TEST_CASE("Token::Serialise(string)", "[CSS][Token]")
  {
    CHECK(Serialise(u8"\"abc\"") == "\"abc\"");
    // Single-quoted strings are serialised with double quotes.
    CHECK(Serialise(u8"'abc'") == "\"abc\"");
    CHECK(Serialise(u8"'a\"b'") == "\"a\\\"b\"");
    CHECK(Serialise(u8"\"a\\\\b\"") == "\"a\\\\b\"");
    CHECK(Serialise(u8"\"\"") == "\"\"");
  }

  TEST_CASE("Token::Serialise(numeric)", "[CSS][Token]")
  {
    CHECK(Serialise(u8"1") == "1");
    CHECK(Serialise(u8"1.5") == "1.5");
    CHECK(Serialise(u8"-0.5") == "-0.5");
    CHECK(Serialise(u8"+5") == "+5");
    CHECK(Serialise(u8"1e3") == "1000");
    CHECK(Serialise(u8"1e21") == "1e+21");
    CHECK(Serialise(u8"10%") == "10%");
    CHECK(Serialise(u8"10px") == "10px");
    CHECK(Serialise(u8"1.5em") == "1.5em");
  }

  TEST_CASE("Token::Serialise(delim and punctuation)", "[CSS][Token]")
  {
    CHECK(Serialise(u8"a, b") == "a, b");
    CHECK(Serialise(u8"a:b;") == "a:b;");
    CHECK(Serialise(u8"[a]") == "[a]");
    CHECK(Serialise(u8"(a)") == "(a)");
    CHECK(Serialise(u8"{a}") == "{a}");
    CHECK(Serialise(u8"a ~= b") == "a ~= b");
    CHECK(Serialise(u8"<!-- -->") == "<!-- -->");
    // A backslash that doesn't start a valid escape.
    CHECK(Serialise(u8"\\\n") == "\\\n ");
  }

  TEST_CASE("Token::Serialise(whitespace)", "[CSS][Token]")
  {
    // A run of spaces and tabs is a single space, and each newline is another.
    CHECK(Serialise(u8"a   b") == "a b");
    CHECK(Serialise(u8"a\n\tb") == "a  b");
    CHECK(Serialise(u8"a \n\n b") == "a    b");

    // A whitespace token without its text is a single space.
    CHECK(Serialise(Token {TokenType::Whitespace}) == " ");
  }

  TEST_CASE("Token::Serialise(custom property)", "[CSS][Token]")
  {
    // Numbers keep their original text.
    CHECK(SerialiseForCustomProperty(u8"1.50") == "1.50");
    CHECK(SerialiseForCustomProperty(u8"+1") == "+1");
    CHECK(SerialiseForCustomProperty(u8"1e3") == "1e3");
    CHECK(SerialiseForCustomProperty(u8"050%") == "050%");
    CHECK(SerialiseForCustomProperty(u8"1.0PX") == "1.0PX");
    CHECK(SerialiseForCustomProperty(u8"10\\70 x") == "10\\70 x");
    // Whitespace keeps its length and newlines; tabs are spaces.
    CHECK(SerialiseForCustomProperty(u8"a  \t\n b") == "a   \n b");
    // Everything else is as in normal mode.
    CHECK(SerialiseForCustomProperty(u8"foo(\"a\")") == "foo(\"a\")");
  }

  TEST_CASE("Token::Serialise(comments between tokens)", "[CSS][Token]")
  {
    // Tokens that would merge into one when re-tokenized get an empty comment between them.
    CHECK(Serialise(u8"a/**/b") == "a/**/b");
    CHECK(Serialise(u8"1/**/2") == "1/**/2");
    CHECK(Serialise(u8"10px/**/5") == "10px/**/5");
    CHECK(Serialise(u8"1/**/%") == "1/**/%");
    CHECK(Serialise(u8"a/**/(") == "a/**/(");
    CHECK(Serialise(u8"#a/**/b") == "#a/**/b");
    CHECK(Serialise(u8"@a/**/b") == "@a/**/b");
    CHECK(Serialise(u8"//**/*") == "//**/*");

    // Tokens that can't merge don't.
    CHECK(Serialise(u8"a/**/,") == "a,");
    CHECK(Serialise(u8"foo(/**/a") == "foo(a");
  }

  TEST_CASE("Token::Serialise(comments after delimiters)", "[CSS][Token]")
  {
    // # and - before an ident-like or numeric token, or before a -.
    CHECK(Serialise(u8"#/**/a") == "#/**/a");
    CHECK(Serialise(u8"#/**/1") == "#/**/1");
    CHECK(Serialise(u8"#/**/-") == "#/**/-");
    CHECK(Serialise(u8"-/**/a") == "-/**/a");
    CHECK(Serialise(u8"-/**/1") == "-/**/1");
    CHECK(Serialise(u8"-/**/-") == "-/**/-");
    CHECK(Serialise(u8"-/**/,") == "-,");

    // @ before an ident-like token or a -, but not a number.
    CHECK(Serialise(u8"@/**/a") == "@/**/a");
    CHECK(Serialise(u8"@/**/-") == "@/**/-");
    CHECK(Serialise(u8"@/**/1") == "@1");

    // . and + before a numeric token only.
    CHECK(Serialise(u8"./**/1") == "./**/1");
    CHECK(Serialise(u8"./**/a") == ".a");
    CHECK(Serialise(u8"+/**/1") == "+/**/1");
    CHECK(Serialise(u8"+/**/a") == "+a");

    // Numbers before a %.
    CHECK(Serialise(u8"1/**/%") == "1/**/%");
    // Percentages never need one.
    CHECK(Serialise(u8"1%/**/a") == "1%a");
    // Dimensions before a - or a CDC.
    CHECK(Serialise(u8"1px/**/-") == "1px/**/-");
    CHECK(Serialise(u8"1px/**/-->") == "1px/**/-->");
    // Idents before a CDC.
    CHECK(Serialise(u8"a/**/-->") == "a/**/-->");
  }

  TEST_CASE("Token::Serialise(escaping in other tokens)", "[CSS][Token]")
  {
    CHECK(Serialise(u8"\\31 a(") == "\\31 a(");
    CHECK(Serialise(u8"@\\31 a") == "@\\31 a");
    // Control characters in strings are escaped as code points.
    CHECK(Serialise(u8"\"a\\1 b\"") == "\"a\\1 b\"");
    CHECK(Serialise(u8"\"a\\7f b\"") == "\"a\\7f b\"");
    // Units are serialised as identifiers.
    CHECK(Serialise(u8"1\\.5") == "1\\.5");
  }

  TEST_CASE("Token::Serialise(bad tokens)", "[CSS][Token]")
  {
    CHECK(Serialise(TokenizeOne(u8"url(a b)")) == "url(()");
    CHECK(Serialise(Tokenize(u8"\"foo\n").front()) == "'\n");
  }

  TEST_CASE("Token::Serialise(appends)", "[CSS][Token]")
  {
    // Serialise appends to the builder rather than replacing it.
    CSSOMString builder = u8"x:";
    TokenizeOne(u8"foo").Serialise(builder);
    CHECK(ToNarrow(builder) == "x:foo");

    // No next token means no comment.
    builder.clear();
    TokenizeOne(u8"a").Serialise(builder, nullptr);
    CHECK(ToNarrow(builder) == "a");
  }

  TEST_CASE("Token::Serialise(unicode-range)", "[CSS][Token]")
  {
    CHECK(Serialise(UnicodeRangeToken(0x25, 0xFF)) == "U+25-FF");
    CHECK(Serialise(UnicodeRangeToken(0x1F600, 0x1F600)) == "U+1F600");
  }

  TEST_CASE("Token::Serialise(round-trip)", "[CSS][Token]")
  {
    for (auto css :
         {u8"color: red", u8"margin: 0 auto 10px -5%", u8"font-family: \"Helvetica Neue\", sans-serif",
          u8"background: url(a) no-repeat", u8"@media (min-width: 600px)", u8"a > b + c ~ d",
          u8"[data-x=\"y\"]", u8"calc(100% - 2em)"})
    {
      INFO(ToNarrow(css));
      CHECK(Serialise(css) == ToNarrow(css));
    }
  }

  TEST_CASE("Token::operator==(same input)", "[CSS][Token]")
  {
    for (auto css : {u8"foo", u8"foo(", u8"@foo", u8"#foo", u8"#1a", u8"\"foo\"", u8"url(foo)", u8"/", u8"1",
                     u8"1.5", u8"+1",   u8"10%",  u8"10px", u8",",   u8":",       u8";",        u8"(", u8")",
                     u8"[",   u8"]",    u8"{",    u8"}",    u8" ",   u8"<!--",    u8"-->"})
    {
      INFO(ToNarrow(css));
      CHECK(TokenizeOne(css) == TokenizeOne(css));
    }

    CHECK(UnicodeRangeToken(0x25, 0xFF) == UnicodeRangeToken(0x25, 0xFF));
  }

  TEST_CASE("Token::operator==(different)", "[CSS][Token]")
  {
    // Type.
    CHECK_FALSE(TokenizeOne(u8"foo") == TokenizeOne(u8"foo("));
    CHECK_FALSE(TokenizeOne(u8"foo") == TokenizeOne(u8"\"foo\""));
    CHECK_FALSE(TokenizeOne(u8"1") == TokenizeOne(u8"1%"));

    // Text.
    CHECK_FALSE(TokenizeOne(u8"foo") == TokenizeOne(u8"bar"));
    CHECK_FALSE(TokenizeOne(u8"\"foo\"") == TokenizeOne(u8"\"bar\""));
    CHECK_FALSE(TokenizeOne(u8"url(a)") == TokenizeOne(u8"url(b)"));

    // Hash type.
    CHECK_FALSE(TokenizeOne(u8"#a1") == TokenizeOne(u8"#1a"));

    // Delimiter.
    CHECK_FALSE(TokenizeOne(u8"/") == TokenizeOne(u8"*"));

    // Numeric value, sign and type.
    CHECK_FALSE(TokenizeOne(u8"1") == TokenizeOne(u8"2"));
    CHECK_FALSE(TokenizeOne(u8"1") == TokenizeOne(u8"+1"));
    CHECK_FALSE(TokenizeOne(u8"1") == TokenizeOne(u8"1.0"));
    CHECK_FALSE(TokenizeOne(u8"10%") == TokenizeOne(u8"20%"));
    // Numbers compare their original text, not their value.
    CHECK_FALSE(TokenizeOne(u8"1e2") == TokenizeOne(u8"100"));
    CHECK_FALSE(TokenizeOne(u8"1.0px") == TokenizeOne(u8"1px"));

    // Whitespace length and newlines; a tab counts as a space.
    CHECK(TokenizeOne(u8" ") == TokenizeOne(u8"\t"));
    CHECK_FALSE(TokenizeOne(u8" ") == TokenizeOne(u8"  "));
    CHECK_FALSE(TokenizeOne(u8" ") == TokenizeOne(u8"\n"));

    // Dimension unit.
    CHECK_FALSE(TokenizeOne(u8"10px") == TokenizeOne(u8"10em"));
    CHECK_FALSE(TokenizeOne(u8"10px") == TokenizeOne(u8"20px"));

    // Unicode range.
    CHECK_FALSE(UnicodeRangeToken(0x25, 0xFF) == UnicodeRangeToken(0x25, 0xFE));
  }
}
