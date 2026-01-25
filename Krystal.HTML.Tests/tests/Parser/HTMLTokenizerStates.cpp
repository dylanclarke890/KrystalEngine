#include "Krystal.HTML.Tests/ParserTestUtils.hpp"
#include "Krystal.HTML/Parser/HTMLTokenizer.hpp"
#include <catch_all.hpp>
#include <format>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  KRYS_NODISCARD static utf32_string &&InsertNull(utf32_string &&str, utf32_string &&suffix) noexcept
  {
    str.push_back(U'\0');
    str.append(std::move(suffix));
    return std::move(str);
  }

#define SETUP_TEST(initialState)                                                                             \
  HTMLInputStream inputStream;                                                                               \
  HTMLTokenizer tokenizer(inputStream);                                                                      \
  tokenizer.SetState(initialState);                                                                          \
  const auto &errors = tokenizer.GetParseErrors();                                                           \
  size_t expectedErrorCount = 0;                                                                             \
  utf32_string expected = U"";

#define COMMON_TEST_CASES(tokenType)                                                                         \
  REQUIRE(token);                                                                                            \
  REQUIRE(token->GetType() == tokenType);                                                                    \
  REQUIRE(Compare(token->GetDataBuffer(), expected));                                                        \
  REQUIRE(errors.size() == expectedErrorCount);

#pragma region CharacterReference

  TEST("CharacterReference", "Non-character reference",
       (UnitTest {.Input = U"&_", .Output = {CreateCharacterToken(U"&_")}}));

#pragma region NamedCharacterReference

  TEST("NamedCharacterReference", "Basic named character reference",
       (UnitTest {.Input = U"&copy;", .Output = {CreateCharacterToken(U"©")}}))

  TEST("NamedCharacterReference", "missing semicolon",
       (UnitTest {.AppendEOF = true,
                  .Input = U"&Agrave",
                  .Output = {CreateCharacterToken(U"À"), CreateEOFToken()},
                  .Errors = {{HTMLParseError::MissingSemicolonAfterCharacterReference}}}))

  TEST("NamedCharacterReference", "no match, incomplete reference",
       (UnitTest {.Input = U"&nonentity", .Output = {CreateCharacterToken(U"&nonentity")}}))

  TEST("NamedCharacterReference", "no match, ends in semicolon",
       (UnitTest {.Input = U"&nonentity;",
                  .Output = {CreateCharacterToken(U"&nonentity;")},
                  .Errors = {{HTMLParseError::UnknownNamedCharacterReference}}}))

  TEST("NamedCharacterReference", "mixed case reference",
       (UnitTest {.Input = U"&vsupnE;", .Output = {CreateCharacterToken(U"⫌︀")}}))

  TEST("NamedCharacterReference", "partial match with valid prefix",
       (UnitTest {.Input = U"&notit;", .Output = {CreateCharacterToken(U"¬it;")}}))

  TEST("NamedCharacterReference", "EOF in middle of otherwise valid reference",
       (UnitTest {
         .AppendEOF = true, .Input = U"&co", .Output = {CreateCharacterToken(U"&co"), CreateEOFToken()}}))

  // TODO: test cases for when character references are consumed as part of attributes

#pragma endregion

#pragma region DecimalCharacterReference

  TEST("DecimalCharacterReference", "happy path",
       (UnitTest {.Input = U"&#8482;", .Output = {CreateCharacterToken(U"™")}}))

  TEST("DecimalCharacterReference", "lookup table",
       (UnitTest {.Input = U"&#128;", .Output = {CreateCharacterToken(U"€")}}))

  TEST("DecimalCharacterReference", "leading zeros",
       (UnitTest {.Input = U"&#00008482;", .Output = {CreateCharacterToken(U"™")}}))

  TEST("DecimalCharacterReference", "missing semicolon",
       (UnitTest {.AppendEOF = true,
                  .Input = U"&#8482",
                  .Output = {CreateCharacterToken(U"™"), CreateEOFToken()},
                  .Errors = {{HTMLParseError::MissingSemicolonAfterCharacterReference}}}))

  TEST("DecimalCharacterReference", "no numbers provided after #",
       (UnitTest {.Input = U"&#;",
                  .Output = {CreateCharacterToken(U"&#;")},
                  .Errors = {{HTMLParseError::AbsenceOfDigitsInNumericCharacterReference}}}));

  TEST("DecimalCharacterReference", "mixed with non-digit characters",
       (UnitTest {.Input = U"&#123abc;",
                  .Output = {CreateCharacterToken(U"{abc;")},
                  .Errors = {{HTMLParseError::MissingSemicolonAfterCharacterReference}}}));

  TEST("DecimalCharacterReference", "stops at semicolon",
       (UnitTest {.Input = U"&#65;BC", .Output = {CreateCharacterToken(U"ABC")}}));

  TEST("DecimalCharacterReference", "stops at non-digit character",
       (UnitTest {.Input = U"&#65BC;",
                  .Output = {CreateCharacterToken(U"ABC;")},
                  .Errors = {{HTMLParseError::MissingSemicolonAfterCharacterReference}}}));

  TEST("DecimalCharacterReference", "null character reference",
       (UnitTest {.Input = U"&#0;",
                  .Output = {CreateCharacterToken(U"\xFFFD")},
                  .Errors = {{HTMLParseError::NullCharacterReference}}}));

  TEST("DecimalCharacterReference", "character reference outside unicode range",
       (UnitTest {.Input = U"&#1114112;",
                  .Output = {CreateCharacterToken(U"\xFFFD")},
                  .Errors = {{HTMLParseError::CharacterReferenceOutsideUnicodeRange}}}));

  TEST("DecimalCharacterReference", "surrogate",
       (UnitTest {.Input = U"&#55296;",
                  .Output = {CreateCharacterToken(U"\xFFFD")},
                  .Errors = {{HTMLParseError::SurrogateCharacterReference}}}));

  TEST("DecimalCharacterReference", "non character",
       (UnitTest {.Input = U"&#65534;",
                  .Output = {CreateCharacterToken(U"\xFFFE")},
                  .Errors = {{HTMLParseError::NonCharacterCharacterReference}}}));

  TEST("DecimalCharacterReference", "control character",
       (UnitTest {.Input = U"&#13;",
                  .Output = {CreateCharacterToken(U"\x0D")},
                  .Errors = {{HTMLParseError::ControlCharacterReference}}}));

#pragma endregion

#pragma region HexadecimalCharacterReference

  TEST("HexadecimalCharacterReference", "happy path",
       (UnitTest {.Input = U"&#x152;", .Output = {CreateCharacterToken(U"Œ")}}))

  TEST("HexadecimalCharacterReference", "lookup table",
       (UnitTest {.Input = U"&#x80;", .Output = {CreateCharacterToken(U"€")}}))

  TEST("HexadecimalCharacterReference", "mixed case hex digits",
       (UnitTest {.Input = U"&#x20Ac;", .Output = {CreateCharacterToken(U"€")}}))

  TEST("HexadecimalCharacterReference", "uppercase X",
       (UnitTest {.Input = U"&#X152;", .Output = {CreateCharacterToken(U"Œ")}}))

  TEST("HexadecimalCharacterReference", "missing semicolon",
       (UnitTest {.AppendEOF = true,
                  .Input = U"&#X152",
                  .Output = {CreateCharacterToken(U"Œ"), CreateEOFToken()},
                  .Errors = {{HTMLParseError::MissingSemicolonAfterCharacterReference}}}))

  TEST("HexadecimalCharacterReference", "no numbers provided after #X",
       (UnitTest {.Input = U"&#X;",
                  .Output = {CreateCharacterToken(U"&#X;")},
                  .Errors = {{HTMLParseError::AbsenceOfDigitsInNumericCharacterReference}}}));

  TEST("HexadecimalCharacterReference", "null character reference",
       (UnitTest {.Input = U"&#x00;",
                  .Output = {CreateCharacterToken(U"\xFFFD")},
                  .Errors = {{HTMLParseError::NullCharacterReference}}}));

  TEST("HexadecimalCharacterReference", "character reference outside unicode range",
       (UnitTest {.Input = U"&#x110000;",
                  .Output = {CreateCharacterToken(U"\xFFFD")},
                  .Errors = {{HTMLParseError::CharacterReferenceOutsideUnicodeRange}}}));

  TEST("HexadecimalCharacterReference", "surrogate",
       (UnitTest {.Input = U"&#xD800;",
                  .Output = {CreateCharacterToken(U"\xFFFD")},
                  .Errors = {{HTMLParseError::SurrogateCharacterReference}}}));

  TEST("HexadecimalCharacterReference", "non character",
       (UnitTest {.Input = U"&#xFFFE;",
                  .Output = {CreateCharacterToken(U"\xFFFE")},
                  .Errors = {{HTMLParseError::NonCharacterCharacterReference}}}));

  TEST("HexadecimalCharacterReference", "control character",
       (UnitTest {.Input = U"&#x0D;",
                  .Output = {CreateCharacterToken(U"\x0D")},
                  .Errors = {{HTMLParseError::ControlCharacterReference}}}));

#pragma endregion

  TEST("CharacterReference", "Multiple character references",
       (UnitTest {.Input = U"&copy;&Agrave&#128;&#X152;&a;",
                  .Output = {CreateCharacterToken(U"©À€Œ&a;")},
                  .Errors = {
                    {HTMLParseError::MissingSemicolonAfterCharacterReference},
                    {HTMLParseError::UnknownNamedCharacterReference}}}));

#pragma endregion

#pragma region Data

  TEST_CASE("HTMLTokenizer(Data) - Replaces character references", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"Some data © some more data";
    inputStream.Append(U"Some data &copy; some more data", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(Data) - Batches characters up to less than sign", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"a string of characters; 123145";
    inputStream.Append(U"a string of characters; 123145<", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(Data) - switches to TagOpen when parsing LessThanSign", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    utf32_string input = U"<";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::TagOpen);
  }

  TEST_CASE("HTMLTokenizer(Data) - Batches characters up to EOF then emits EOF", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"a string of characters; 123145";
    inputStream.Append(U"a string of characters; 123145", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(Data) - Emits null character as-is with parse error", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"1234";
    expected.append(1uz, U'\x0');

    expectedErrorCount = 1;

    utf32_string input = U"1234";
    input.append(1uz, U'\x0');
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

#pragma endregion

#pragma region RCDATA

  TEST_CASE("HTMLTokenizer(RCDATA) - Replaces character references", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RCDATA);

    expected = U"Some data © some more data";
    inputStream.Append(U"Some data &copy; some more data", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(RCDATA) - switches to RCDATALessThanSign when parsing LessThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RCDATA);

    utf32_string input = U"<";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::RCDATALessThanSign);
  }

  TEST_CASE("HTMLTokenizer(RCDATA) - Replaces null character with U+FFFD", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RCDATA);

    expected = U"1234\xFFFD";
    expectedErrorCount = 1;

    utf32_string input = U"1234";
    input.append(1uz, U'\x0');
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE("HTMLTokenizer(RCDATA) - Batches characters up to EOF then emits EOF", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RCDATA);

    expected = U"a string of characters; 123145";
    inputStream.Append(U"a string of characters; 123145", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(RCDATA) - Batches characters", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RCDATA);

    expected = U"a string of characters; 123145";
    inputStream.Append(U"a string of characters; 123145", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma endregion

#pragma region RAWTEXT

  TEST_CASE("HTMLTokenizer(RAWTEXT) - switches to RAWTEXTLessThanSign when parsing LessThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RAWTEXT);

    utf32_string input = U"<";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::RAWTEXTLessThanSign);
  }

  TEST_CASE("HTMLTokenizer(RAWTEXT) - Replaces null character with U+FFFD", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RAWTEXT);

    expected = U"1234\xFFFD";
    expectedErrorCount = 1;

    utf32_string input = U"1234";
    input.append(1uz, U'\x0');
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE("HTMLTokenizer(RAWTEXT) - Batches characters up to EOF then emits EOF", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RAWTEXT);

    expected = U"a string of characters; 123145";
    inputStream.Append(U"a string of characters; 123145", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(RAWTEXT) - Batches characters", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RAWTEXT);

    expected = U"a string of characters; 123145";
    inputStream.Append(U"a string of characters; 123145", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma endregion

#pragma region ScriptData

  TEST_CASE("HTMLTokenizer(ScriptData) - switches to ScriptDataLessThanSign when parsing LessThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptData);

    utf32_string input = U"<";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataLessThanSign);
  }

  TEST_CASE("HTMLTokenizer(ScriptData) - Replaces null character with U+FFFD", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptData);

    expected = U"1234\xFFFD";
    expectedErrorCount = 1;

    utf32_string input = U"1234";
    input.append(1uz, U'\x0');
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE("HTMLTokenizer(ScriptData) - Batches characters up to EOF then emits EOF", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptData);

    expected = U"a string of characters; 123145";
    inputStream.Append(U"a string of characters; 123145", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptData) - Batches characters", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptData);

    expected = U"a string of characters; 123145";
    inputStream.Append(U"a string of characters; 123145", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma endregion

#pragma region PLAINTEXT

  TEST_CASE("HTMLTokenizer(PLAINTEXT) - Replaces null character with U+FFFD", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::PLAINTEXT);

    expected = U"1234\xFFFD";
    expectedErrorCount = 1;

    utf32_string input = U"1234";
    input.append(1uz, U'\x0');
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE("HTMLTokenizer(PLAINTEXT) - Batches characters up to EOF then emits EOF", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::PLAINTEXT);

    expected = U"a string of characters; 123145";
    inputStream.Append(U"a string of characters; 123145", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(PLAINTEXT) - Batches characters", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::PLAINTEXT);

    expected = U"a string of characters; 123145";
    inputStream.Append(U"a string of characters; 123145", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma endregion

#pragma region TagOpen

  TEST_CASE("HTMLTokenizer(TagOpen) - Switches to MarkupDeclarationOpen after parsing ExclamationMark",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::TagOpen);

    expected = U"";

    utf32_string input = U"!";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::MarkupDeclarationOpen);
  }

  TEST_CASE("HTMLTokenizer(TagOpen) - Switches to EndTagOpen after parsing Solidus", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::TagOpen);

    expected = U"";

    utf32_string input = U"/";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::EndTagOpen);
  }

  TEST_CASE("HTMLTokenizer(TagOpen) - Switches to TagName after parsing valid tag name start",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::TagOpen);

    expected = U"";

    utf32_string input = U"div";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::TagName);
  }

  TEST_CASE("HTMLTokenizer(TagOpen) - Switches to TagName after parsing valid tag name start, mixed case",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::TagOpen);

    expected = U"";

    utf32_string input = U"dIV";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::TagName);
  }

  TEST_CASE("HTMLTokenizer(TagOpen) - Switches to BogusComment after parsing QuestionMark",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::TagOpen);

    expected = U"";
    expectedErrorCount = 1;

    utf32_string input = U"?";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BogusComment);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedQuestionMarkInsteadOfTagName);
  }

  TEST_CASE("HTMLTokenizer(TagOpen) - Emits LessThanSign if EOF encountered", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"<";
    expectedErrorCount = 1;
    inputStream.Append(U"<", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::EOFBeforeTagName);
  }

  TEST_CASE("HTMLTokenizer(TagOpen) - Emits LessThanSign if first character invalid", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::TagOpen);

    expected = U"<*";
    expectedErrorCount = 1;
    inputStream.Append(U"*");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::InvalidFirstCharacterOfTagName);
  }

#pragma endregion

#pragma region EndTagOpen

  TEST_CASE("HTMLTokenizer(EndTagOpen) - Switches to TagName after parsing valid tag name start",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::EndTagOpen);

    expected = U"";

    utf32_string input = U"div";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::TagName);
  }

  TEST_CASE("HTMLTokenizer(EndTagOpen) - Switches to Data if parsing GreaterThanSign (emits nothing)",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::EndTagOpen);

    expectedErrorCount = 1;

    inputStream.Append(U">");
    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
    REQUIRE(errors.back() == HTMLParseError::MissingEndTagName);
  }

  TEST_CASE("HTMLTokenizer(EndTagOpen) - Emits LessThanSign and Solidus if EOF encountered",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::TagOpen);

    expected = U"</";
    expectedErrorCount = 1;

    inputStream.Append(U"/", IsEOF(true));
    NextTokenPtr token = tokenizer.NextToken();

    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::EOFBeforeTagName);
  }

  TEST_CASE("HTMLTokenizer(EndTagOpen) - Switches to BogusComment if first character invalid",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::EndTagOpen);

    expectedErrorCount = 1;

    inputStream.Append(U"*");
    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BogusComment);
    REQUIRE(errors.back() == HTMLParseError::InvalidFirstCharacterOfTagName);
  }

#pragma endregion

#pragma region TagName

  TEST_CASE("HTMLTokenizer(TagName) - Switches to BeforeAttributeName when parsing whitespace",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::TagName);

    inputStream.Append(U" ");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeName);
  }

  TEST_CASE("HTMLTokenizer(TagName) - Switches to SelfClosingStartTag when parsing solidus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::TagName);

    inputStream.Append(U"/");

    NextTokenPtr token = tokenizer.NextToken();

    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::SelfClosingStartTag);
  }

  TEST_CASE("HTMLTokenizer(TagName) - Emits tag token when parsing GreaterThanSign", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"a";

    inputStream.Append(U"<a>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::StartTag);

    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE("HTMLTokenizer(TagName) - allows mixed case", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"div";

    inputStream.Append(U"<DiV>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::StartTag);
  }

  TEST_CASE("HTMLTokenizer(TagName) - replaces Null with ReplacementCharacter", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"a\xFFFD";
    expectedErrorCount = 1;

    utf32_string input = U"<a";
    input.append(1uz, U'\0');
    input.append(1uz, U'>');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE("HTMLTokenizer(TagName) - Emits EOF instead of TagName if EOF reached", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<a", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    REQUIRE(errors.back() == HTMLParseError::EOFInTag);
  }

#pragma endregion

#pragma region RCDATALessThanSign

  TEST_CASE("HTMLTokenizer(RCDATALessThanSign) - Switches to RCDATAEndTagOpen after parsing solidus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RCDATALessThanSign);

    expected = U"";

    utf32_string input = U"/";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::RCDATAEndTagOpen);
  }

  TEST_CASE("HTMLTokenizer(RCDATALessThanSign) - emits less than sign after parsing non solidus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RCDATALessThanSign);

    expected = U"<©";

    utf32_string input = U"©";
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(RCDATALessThanSign) - emits less than sign if EOF reached", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RCDATA);

    expected = U"<";

    utf32_string input = U"<";
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma endregion

#pragma region RCDATAEndTagOpen

  TEST_CASE(
    "HTMLTokenizer(RCDATAEndTagOpen) - Switches to RCDATAEndTagName after parsing valid tag name start",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RCDATAEndTagOpen);

    expected = U"";

    utf32_string input = U"a";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::RCDATAEndTagName);
  }

  TEST_CASE(
    "HTMLTokenizer(RCDATAEndTagOpen) - emits less than sign and solidus after parsing invalid tag name start",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RCDATAEndTagOpen);

    expected = U"</©";

    utf32_string input = U"©";
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(RCDATAEndTagOpen) - emits less than sign and solidus if EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RCDATA);

    expected = U"</";

    utf32_string input = U"</";
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma endregion

#pragma region RCDATAEndTagName

  TEST_CASE("HTMLTokenizer(RCDATAEndTagName) - switches to BeforeAttributeName when parsing whitespace and "
            "end tag name matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<div></div ");

    // Start tag
    expected = U"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::RCDATA);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeName);
    }
  }

  TEST_CASE("HTMLTokenizer(RCDATAEndTagName) - switches to SelfClosingStartTag when parsing solidus and "
            "end tag name matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<div></div/");

    // Start tag
    expected = U"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::RCDATA);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.GetState() == TokenizerState::SelfClosingStartTag);
    }
  }

  TEST_CASE("HTMLTokenizer(RCDATAEndTagName) - emits end tag token when parsing GreaterThanSign end tag name "
            "matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<div></div>");

    // Start tag
    expected = U"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::RCDATA);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndTag);
    }
  }

  TEST_CASE("HTMLTokenizer(RCDATAEndTagName) - allows mixed case", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<div></DiV>");

    // Start tag
    expected = U"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::RCDATA);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndTag);
    }
  }

  TEST_CASE("HTMLTokenizer(RCDATAEndTagName) - emits less than sign, solidus, and characters when tag name "
            "doesn't match",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<div></span©", IsEOF(true));

    // Start tag
    expected = U"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::RCDATA);

    expected = U"</span©";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
    }
  }

#pragma endregion

#pragma region RAWTEXTLessThanSign

  TEST_CASE("HTMLTokenizer(RAWTEXTLessThanSign) - Switches to RAWTEXTEndTagOpen after parsing solidus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RAWTEXTLessThanSign);

    utf32_string input = U"/";

    expected = U"";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::RAWTEXTEndTagOpen);
  }

  TEST_CASE("HTMLTokenizer(RAWTEXTLessThanSign) - emits less than sign after parsing non solidus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RAWTEXTLessThanSign);

    utf32_string input = U"©";

    expected = U"<©";
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(RAWTEXTLessThanSign) - emits less than sign if EOF reached", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RAWTEXT);

    utf32_string input = U"<";

    expected = U"<";
    inputStream.Append(std::move(input), IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

#pragma endregion

#pragma region RAWTEXTEndTagOpen

  TEST_CASE(
    "HTMLTokenizer(RAWTEXTEndTagOpen) - Switches to RAWTEXTEndTagName after parsing valid tag name start",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RAWTEXTEndTagOpen);
    utf32_string input = U"a";

    expected = U"";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::RAWTEXTEndTagName);
  }

  TEST_CASE("HTMLTokenizer(RAWTEXTEndTagOpen) - emits less than sign and solidus after parsing invalid tag "
            "name start",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RAWTEXTEndTagOpen);
    utf32_string input = U"©";

    expected = U"</©";
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(RAWTEXTEndTagOpen) - emits less than sign and solidus if EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::RAWTEXT);
    utf32_string input = U"</";

    expected = U"</";
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma endregion

#pragma region RAWTEXTEndTagName

  TEST_CASE("HTMLTokenizer(RAWTEXTEndTagName) - switches to BeforeAttributeName when parsing whitespace and "
            "end tag name matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<div></div ");

    // Start tag
    expected = U"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::RAWTEXT);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeName);
    }
  }

  TEST_CASE("HTMLTokenizer(RAWTEXTEndTagName) - switches to SelfClosingStartTag when parsing solidus and "
            "end tag name matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<div></div/");

    // Start tag
    expected = U"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::RAWTEXT);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.GetState() == TokenizerState::SelfClosingStartTag);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(RAWTEXTEndTagName) - emits end tag token when parsing GreaterThanSign end tag name "
    "matches start tag",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<div></div>");

    // Start tag
    expected = U"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::RAWTEXT);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndTag);
    }
  }

  TEST_CASE("HTMLTokenizer(RAWTEXTEndTagName) - allows mixed case", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<div></DiV>");

    // Start tag
    expected = U"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::RAWTEXT);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndTag);
    }
  }

  TEST_CASE("HTMLTokenizer(RAWTEXTEndTagName) - emits less than sign, solidus, and characters when tag name "
            "doesn't match",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<div></span©", IsEOF(true));

    // Start tag
    expected = U"div";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::RAWTEXT);

    expected = U"</span©";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
    }
  }

#pragma endregion

#pragma region ScriptDataLessThanSign

  TEST_CASE("HTMLTokenizer(ScriptDataLessThanSign) - Switches to ScriptDataEndTagOpen after parsing solidus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataLessThanSign);
    utf32_string input = U"/";

    expected = U"";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEndTagOpen);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataLessThanSign) - emits less than sign after parsing non solidus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataLessThanSign);
    utf32_string input = U"©";

    expected = U"<©";
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataLessThanSign) - emits less than sign if EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptData);
    utf32_string input = U"<";

    expected = U"<";
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataLessThanSign) - switches to ScriptDataEscapeStart when parsing "
            "ExclamationMark and emits a LessThanSign and a ExclamationMark",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataLessThanSign);
    expected = U"<!";

    utf32_string input = U"!";
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma endregion

#pragma region ScriptDataEndTagOpen

  TEST_CASE("HTMLTokenizer(ScriptDataEndTagOpen) - Switches to ScriptDataEndTagName after parsing valid tag "
            "name start",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEndTagOpen);
    utf32_string input = U"a";

    expected = U"";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEndTagName);
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEndTagOpen) - emits less than sign and solidus after parsing invalid tag "
    "name start",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEndTagOpen);
    utf32_string input = U"©";

    expected = U"</©";
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEndTagOpen) - emits less than sign and solidus if EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptData);
    utf32_string input = U"</";

    expected = U"</";
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma endregion

#pragma region ScriptDataEndTagName

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEndTagName) - switches to BeforeAttributeName when parsing whitespace and "
    "end tag name matches start tag",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></script ");

    // Start tag
    expected = U"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::ScriptData);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeName);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEndTagName) - switches to SelfClosingStartTag when parsing solidus and "
            "end tag name matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></script/");

    // Start tag
    expected = U"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::ScriptData);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.GetState() == TokenizerState::SelfClosingStartTag);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEndTagName) - emits end tag token when parsing GreaterThanSign end tag name "
    "matches start tag",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></script>");

    // Start tag
    expected = U"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::ScriptData);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndTag);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEndTagName) - allows mixed case", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></ScRiPt>");

    // Start tag
    expected = U"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::ScriptData);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndTag);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEndTagName) - emits less than sign, solidus, and characters when tag name "
    "doesn't match",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></span©", IsEOF(true));

    // Start tag
    expected = U"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::ScriptData);

    expected = U"</span©";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
    }
  }

#pragma endregion

#pragma region ScriptDataEscapeStart

  TEST_CASE("HTMLTokenizer(ScriptDataEscapeStart) - switches to ScriptDataEscapeStartDash after parsing "
            "HyphenMinus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapeStart);
    utf32_string input = U"-";

    expected = U"-";
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscapeStartDash);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapeStart) - switches to ScriptData after parsing non HyphenMinus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapeStart);
    utf32_string input = U"©";

    expected = U"©";
    inputStream.Append(std::move(input), IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma endregion

#pragma region ScriptDataEscaped

  TEST_CASE("HTMLTokenizer(ScriptDataEscaped) -  switches to ScriptDataEscapedDash when parsing "
            "HyphenMinus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscaped);

    expected = U"-";
    inputStream.Append(U"-");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscapedDash);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscaped) - switches to ScriptDataEscapedLessThanSign when parsing "
            "LessThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscaped);

    inputStream.Append(U"<");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscapedLessThanSign);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscaped) - Replaces null character with U+FFFD", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscaped);

    expected = U"1234\xFFFD";
    expectedErrorCount = 1;

    utf32_string input = U"1234";
    input.append(1uz, U'\x0');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscaped) - Batches characters up to EOF then emits EOF",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscaped);

    expected = U"a string of characters; 123145";
    expectedErrorCount = 1;

    inputStream.Append(U"a string of characters; 123145", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
      REQUIRE(errors.back() == HTMLParseError::EOFInScriptHTMLCommentLikeText);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscaped) - Batches characters", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscaped);

    expected = U"a string of characters; 123145";
    inputStream.Append(U"a string of characters; 123145");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma endregion

#pragma region ScriptDataEscapedDash

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedDash) - switches to ScriptDataEscapedDashDash when parsing "
            "HyphenMinus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedDash);

    expected = U"-";
    inputStream.Append(U"-");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscapedDashDash);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedDash) - switches to ScriptDataEscapedLessThanSign when parsing "
            "LessThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedDash);

    inputStream.Append(U"<");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscapedLessThanSign);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedDash) - Replaces null character with U+FFFD and switches to "
            "ScriptDataEscaped",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedDash);

    expected = U"1234\xFFFD";
    expectedErrorCount = 1;

    utf32_string input = U"1234";
    input.append(1uz, U'\x0');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedDash) - Batches characters up to EOF then emits EOF",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedDash);

    expected = U"a string of characters; 123145";
    expectedErrorCount = 1;
    inputStream.Append(U"a string of characters; 123145", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
      REQUIRE(errors.back() == HTMLParseError::EOFInScriptHTMLCommentLikeText);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedDash) - Batches characters", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedDash);

    expected = U"a string of characters; 123145";
    inputStream.Append(U"a string of characters; 123145");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma endregion

#pragma region ScriptDataEscapedDashDash

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEscapedDashDash) - emits HyphenMinus and stays in the same state when parsing "
    "HyphenMinus",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedDashDash);

    expected = U"-";
    inputStream.Append(U"-");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscapedDashDash);
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEscapedDashDash) - switches to ScriptDataEscapedLessThanSign when parsing "
    "LessThanSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedDashDash);

    inputStream.Append(U"<");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscapedLessThanSign);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedDashDash) - switches to ScriptData when parsing GreaterThanSign "
            "and emits GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedDashDash);

    expected = U">";
    inputStream.Append(U">");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptData);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedDashDash) - Replaces null character with U+FFFD and switches to "
            "ScriptDataEscaped",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedDashDash);

    expected = U"1234\xFFFD";
    expectedErrorCount = 1;

    utf32_string input = U"1234";
    input.append(1uz, U'\x0');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedDashDash) - emits EOF with parser error when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedDash);

    expected = U"-";
    expectedErrorCount = 1;

    inputStream.Append(U"-", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
      REQUIRE(errors.back() == HTMLParseError::EOFInScriptHTMLCommentLikeText);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

#pragma endregion

#pragma region ScriptDataEscapedLessThanSign

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedLessThanSign) - switches to ScriptDataEscapedEndTagOpen after "
            "parsing Solidus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedLessThanSign);

    expected = U"";
    inputStream.Append(U"/");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscapedEndTagOpen);
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEscapedLessThanSign) - emits LessThanSign and next character and switches "
    "to ScriptDataDoubleEscapeStart if ASCII alpha ",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedLessThanSign);

    expected = U"<A";
    inputStream.Append(U"A");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscapeStart);
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEscapedLessThanSign) - emits LessThanSign and next character and switches "
    "to ScriptDataEscaped if not ASCII alpha ",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedLessThanSign);

    expected = U"<©";
    inputStream.Append(U"©");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscaped);
  }

#pragma endregion

#pragma region ScriptDataEscapedEndTagOpen

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedEndTagOpen) - switches to ScriptDataEscapedEndTagName after "
            "parsing valid tag name start",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedEndTagOpen);

    expected = U"";
    inputStream.Append(U"a");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscapedEndTagName);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedEndTagOpen) - emits less than sign and solidus after parsing "
            "invalid tag name start",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscapedEndTagOpen);

    expected = U"</©";
    inputStream.Append(U"©");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedEndTagOpen) - emits less than sign and solidus if EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataEscaped);

    inputStream.Append(U"</", IsEOF(true));

    expected = U"</";
    expectedErrorCount = 1;
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
      REQUIRE(errors.back() == HTMLParseError::EOFInScriptHTMLCommentLikeText);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

#pragma endregion

#pragma region ScriptDataEscapedEndTagName

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEscapedEndTagName) - switches to BeforeAttributeName when parsing whitespace "
    "and end tag name matches start tag",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></script ");

    // Start tag
    expected = U"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::ScriptDataEscaped);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeName);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedEndTagName) - switches to SelfClosingStartTag when parsing "
            "solidus and end tag name matches start tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></script/");

    // Start tag
    expected = U"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::ScriptDataEscaped);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      REQUIRE(!token);
      REQUIRE(tokenizer.GetState() == TokenizerState::SelfClosingStartTag);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataEscapedEndTagName) - emits end tag token when parsing GreaterThanSign end "
    "tag name matches start tag",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></script>");

    // Start tag
    expected = U"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::ScriptDataEscaped);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndTag);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedEndTagName) - allows mixed case", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></ScRiPt>");

    // Start tag
    expected = U"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::ScriptDataEscaped);

    // End tag
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndTag);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptDataEscapedEndTagName) - emits less than sign, solidus, and characters when "
            "tag name doesn't match",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<script></span©");

    // Start tag
    expected = U"script";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    }

    tokenizer.SetState(TokenizerState::ScriptDataEscaped);

    expected = U"</span©";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
    }
  }

#pragma endregion

#pragma region ScriptDataDoubleEscapeStart

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapeStart) - switches to ScriptDataEscaped after parsing "
            "whitespace if temporary buffer is not 'script'",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeStart);

    expected = U" ";
    inputStream.Append(U" ");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapeStart) - switches to ScriptDataEscaped after "
            "parsing Solidus if temporary buffer is not 'script'",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeStart);

    expected = U"/";
    inputStream.Append(U"/");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapeStart) - switches to ScriptDataEscaped after "
            "parsing GreaterThanSign if temporary buffer is not 'script'",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeStart);

    expected = U">";
    inputStream.Append(U">");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapeStart) - switches to ScriptDataDoubleEscaped after parsing "
            "whitespace if temporary buffer is 'script'",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeStart);

    expected = U"script ";
    inputStream.Append(U"script ");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapeStart) - switches to ScriptDataDoubleEscaped after "
            "parsing Solidus if temporary buffer is 'script'",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeStart);

    expected = U"script/";
    inputStream.Append(U"script/");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapeStart) - switches to ScriptDataDoubleEscaped after "
            "parsing GreaterThanSign if temporary buffer is 'script'",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeStart);

    expected = U"script>";
    inputStream.Append(U"script>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapeStart) - continues building temporary buffer when parsing "
            "ASCII alpha",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeStart);

    expected = U"sCr";
    inputStream.Append(U"sCr");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscapeStart);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapeStart) - switches to ScriptDataEscaped if not ASCII alpha ",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeStart);

    expected = U"©";
    inputStream.Append(U"©");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscaped);
  }

#pragma endregion

#pragma region ScriptDataDoubleEscaped

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscaped) -  switches to ScriptDataDoubleEscapedDash when parsing "
            "HyphenMinus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscaped);

    expected = U"-";
    inputStream.Append(U"-");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscapedDash);
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataDoubleEscaped) - switches to ScriptDataDoubleEscapedLessThanSign when parsing "
    "LessThanSign and emits LessThanSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscaped);

    expected = U"<";
    inputStream.Append(U"<");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscapedLessThanSign);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscaped) - Replaces null character with U+FFFD",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscaped);

    expected = U"1234\xFFFD";
    expectedErrorCount = 1;

    utf32_string input = U"1234";
    input.append(1uz, U'\0');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscaped) - Batches characters up to EOF then emits EOF",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscaped);

    expected = U"a string of characters; 123145";
    expectedErrorCount = 1;

    inputStream.Append(U"a string of characters; 123145", IsEOF(true));
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
      REQUIRE(errors.back() == HTMLParseError::EOFInScriptHTMLCommentLikeText);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

#pragma endregion

#pragma region ScriptDataDoubleEscapedDash

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapedDash) - switches to ScriptDataDoubleEscapedDashDash when "
            "parsing HyphenMinus and emits HyphenMinus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapedDash);

    expected = U"-";
    inputStream.Append(U"-");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscapedDashDash);
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataDoubleEscapedDash) - switches to ScriptDataDoubleEscapedLessThanSign when "
    "parsing LessThanSign and emits LessThanSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapedDash);

    expected = U"<";
    inputStream.Append(U"<");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscapedLessThanSign);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapedDash) - Replaces null character with U+FFFD and switches "
            "to ScriptDataDoubleEscaped",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapedDash);

    expected = U"1234\xFFFD";
    expectedErrorCount = 1;

    utf32_string input = U"1234";
    input.append(1uz, U'\0');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapedDash) - Batches characters up to EOF then emits EOF",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapedDash);

    expected = U"a string of characters; 123145";
    expectedErrorCount = 1;

    inputStream.Append(U"a string of characters; 123145", IsEOF(true));
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
      REQUIRE(errors.back() == HTMLParseError::EOFInScriptHTMLCommentLikeText);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataDoubleEscapedDash) - Switches to ScriptDataDoubleEscaped after parsing any "
    "other character and emits that character",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapedDash);

    expected = U"A";
    inputStream.Append(U"A");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscaped);
  }

#pragma endregion

#pragma region ScriptDataDoubleEscapedDashDash

  TEST_CASE(
    "HTMLTokenizer(ScriptDataDoubleEscapedDashDash) - emits HyphenMinus and stays in the same state when "
    "parsing HyphenMinus",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapedDashDash);

    expected = U"-";
    inputStream.Append(U"-");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscapedDashDash);
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataDoubleEscapedDashDash) - switches to ScriptDataDoubleEscapedLessThanSign "
    "when parsing LessThanSign and emits LessThanSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapedDashDash);

    expected = U"<";
    inputStream.Append(U"<");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscapedLessThanSign);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapedDashDash) - switches to ScriptData after "
            "parsing GreaterThanSign and emits GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapedDashDash);

    expected = U">";
    inputStream.Append(U">");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptData);
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataDoubleEscapedDashDash) - Replaces null character with U+FFFD and switches "
    "to ScriptDataDoubleEscaped",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapedDashDash);

    expected = U"1234\xFFFD";
    expectedErrorCount = 1;

    utf32_string input = U"1234";
    input.append(1uz, U'\0');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapedDashDash) - emits EOF with parser error when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapedDashDash);

    expected = U"-";
    expectedErrorCount = 1;

    inputStream.Append(U"-", IsEOF(true));
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
      REQUIRE(errors.back() == HTMLParseError::EOFInScriptHTMLCommentLikeText);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapedDashDash) - Switches to ScriptDataDoubleEscaped after "
            "parsing any other character and emits that character",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapedDashDash);

    expected = U"A";
    inputStream.Append(U"A");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscaped);
  }

#pragma endregion

#pragma region ScriptDataDoubleEscapedLessThanSign

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapedLessThanSign) - switches to "
            "ScriptDataDoubleEscapeEnd after parsing Solidus and emits Solidus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapedLessThanSign);

    expected = U"/";
    inputStream.Append(U"/");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscapeEnd);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapedLessThanSign) - Switches to ScriptDataDoubleEscaped after "
            "parsing any other character and emits that character",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapedLessThanSign);

    expected = U"A";
    inputStream.Append(U"A");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscaped);
  }

#pragma endregion

#pragma region ScriptDataDoubleEscapeEnd

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapeEnd) - switches to ScriptDataDoubleEscaped after parsing "
            "whitespace if temporary buffer is not 'script'",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeEnd);

    expected = U" ";
    inputStream.Append(U" ");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapeEnd) - switches to ScriptDataDoubleEscaped after parsing "
            "GreaterThanSign if temporary buffer is not 'script'",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeEnd);

    expected = U">";
    inputStream.Append(U">");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapeEnd) - switches to ScriptDataDoubleEscaped after parsing "
            "any other character if temporary buffer is not 'script'",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeEnd);

    expected = U"_";
    inputStream.Append(U"_");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscaped);
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataDoubleEscapeEnd) - switches to ScriptDataEscaped after parsing whitespace if "
    "temporary buffer is 'script'",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeEnd);

    expected = U"script ";
    inputStream.Append(U"script ");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscaped);
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataDoubleEscapeEnd) - switches to ScriptDataEscaped after parsing Solidus if "
    "temporary buffer is 'script'",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeEnd);

    expected = U"script/";
    inputStream.Append(U"script/");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapeEnd) - switches to ScriptDataEscaped after parsing "
            "GreaterThanSign if temporary buffer is 'script'",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeEnd);

    expected = U"script>";
    inputStream.Append(U"script>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataEscaped);
  }

  TEST_CASE("HTMLTokenizer(ScriptDataDoubleEscapeEnd) - continues building temporary buffer when parsing "
            "ASCII alpha",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeEnd);

    expected = U"sCr";
    inputStream.Append(U"sCr");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscapeEnd);
  }

  TEST_CASE(
    "HTMLTokenizer(ScriptDataDoubleEscapeEnd) - switches to ScriptDataDoubleEscaped if not ASCII alpha ",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::ScriptDataDoubleEscapeEnd);

    expected = U"©";
    inputStream.Append(U"©");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::ScriptDataDoubleEscaped);
  }

#pragma endregion

#pragma region BeforeAttributeName

  TEST_CASE("HTMLTokenizer(BeforeAttributeName) - ignores whitespace", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::BeforeAttributeName);

    expected = U"";
    inputStream.Append(U"   \t\n\r");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeName);
  }

  TEST_CASE("HTMLTokenizer(BeforeAttributeName) - switches to SelfClosingStartTag after parsing Solidus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::BeforeAttributeName);

    expected = U"";
    inputStream.Append(U"/");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::SelfClosingStartTag);
  }

  TEST_CASE("HTMLTokenizer(BeforeAttributeName) - switches to Data after parsing GreaterThanSign and emits "
            "the current tag",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"div";
    inputStream.Append(U"<div >");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE("HTMLTokenizer(BeforeAttributeName) - switches to Data and emits EOF when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<div ", IsEOF(true));
    expected = U"";
    expectedErrorCount = 1;

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    REQUIRE(errors.back() == HTMLParseError::EOFInTag);
  }

  TEST_CASE("HTMLTokenizer(BeforeAttributeName) - switches to AttributeName with parser error after parsing "
            "an EqualsSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<a =");
    expected = U"";
    expectedErrorCount = 1;

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedEqualsSignBeforeAttributeName);
    REQUIRE(tokenizer.GetState() == TokenizerState::AttributeName);
  }

  TEST_CASE(
    "HTMLTokenizer(BeforeAttributeName) - switches to AttributeName after parsing valid attribute name start",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<a b");
    expected = U"";
    NextTokenPtr token = tokenizer.NextToken();

    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AttributeName);
  }

#pragma endregion

#pragma region AttributeName

  TEST_CASE(
    "HTMLTokenizer(AttributeName) - ignores whitespace and switches to AfterAttributeName when parsing "
    "whitespace",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<a b   \t\n\r");
    expected = U"";

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AfterAttributeName);
  }

  TEST_CASE("HTMLTokenizer(AttributeName) - switches to SelfClosingStartTag when parsing Solidus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<a b/");
    expected = U"";

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::SelfClosingStartTag);
  }

  TEST("AttributeName", "switches to Data and emits the current tag when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<div b>",
         .Output = {CreateStartTagToken({.Name = U"div", .Attributes = {{.Name = U"b", .Value = U""}}})}}))

  TEST_CASE("HTMLTokenizer(AttributeName) - parses attribute name and switches to BeforeAttributeValue when "
            "parsing EqualsSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<a b=");
    expected = U"";

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeValue);
  }

  TEST_CASE("HTMLTokenizer(AttributeName) - appends to attribute name when parsing valid attribute name "
            "characters",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<a data-value=");
    expected = U"";

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeValue);
  }

  TEST_CASE("HTMLTokenizer(AttributeName) - Replaces null character with U+FFFD in attribute name",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;

    utf32_string input = U"<a data";
    input.append(1uz, U'\0');
    input.append(U"value=");
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeValue);
  }

  TEST_CASE("HTMLTokenizer(AttributeName) - Treats QuotationMark as anything else but with parse error",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<a data\"value=");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedCharacterInAttributeName);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeValue);
  }

  TEST_CASE("HTMLTokenizer(AttributeName) - Treats Apostrophe as anything else but with parse error",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<a data'value=");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedCharacterInAttributeName);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeValue);
  }

  TEST_CASE("HTMLTokenizer(AttributeName) - Treats LessThanSign as anything else but with parse error",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<a data<value=");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedCharacterInAttributeName);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeValue);
  }

#pragma endregion

#pragma region AfterAttributeName

  TEST_CASE("HTMLTokenizer(AfterAttributeName) - ignores whitespace and stays in the same state when parsing "
            "whitespace",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::AfterAttributeName);

    expected = U"";
    inputStream.Append(U"   \t\n\r");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AfterAttributeName);
  }

  TEST_CASE("HTMLTokenizer(AfterAttributeName) - switches to SelfClosingStartTag when parsing Solidus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::AfterAttributeName);

    expected = U"";
    inputStream.Append(U"/");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::SelfClosingStartTag);
  }

  TEST_CASE(
    "HTMLTokenizer(AfterAttributeName) - switches to BeforeAttributeValue after parsing an EqualsSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<a b=");
    expected = U"";

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeValue);
  }

  TEST("AfterAttributeName", "switches to Data and emits the current tag when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<div b>",
         .Output = {CreateStartTagToken({.Name = U"div", .Attributes = {{.Name = U"b", .Value = U""}}})}}))

  TEST_CASE("HTMLTokenizer(AfterAttributeName) - Emits EOF token if EOF is reached", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<div b ", IsEOF(true));
    expected = U"";
    expectedErrorCount = 1;

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    REQUIRE(errors.back() == HTMLParseError::EOFInTag);
  }

  TEST_CASE("HTMLTokenizer(AfterAttributeName) - treats any other character as the start of a new "
            "attribute name",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<div a b");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AttributeName);
  }

#pragma endregion

#pragma region BeforeAttributeValue

  TEST_CASE(
    "HTMLTokenizer(BeforeAttributeValue) - ignores whitespace and stays in the same state when parsing "
    "whitespace",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::BeforeAttributeValue);

    expected = U"";
    inputStream.Append(U"   \t\n\r");
    NextTokenPtr token = tokenizer.NextToken();

    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeValue);
  }

  TEST_CASE("HTMLTokenizer(BeforeAttributeValue) - switches to AttributeValueDoubleQuoted when parsing "
            "QuotationMark",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::BeforeAttributeValue);

    expected = U"";
    inputStream.Append(U"\"");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AttributeValueDoubleQuoted);
  }

  TEST_CASE("HTMLTokenizer(BeforeAttributeValue) - switches to AttributeValueSingleQuoted when parsing "
            "Apostrophe",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::BeforeAttributeValue);

    expected = U"";
    inputStream.Append(U"'");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AttributeValueSingleQuoted);
  }

  TEST_CASE("HTMLTokenizer(BeforeAttributeValue) - emits current tag token and switches to Data with parser "
            "error when parsing "
            "GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"div";
    expectedErrorCount = 1;
    inputStream.Append(U"<div a=>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::StartTag);

    REQUIRE(errors.back() == HTMLParseError::MissingAttributeValue);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE("HTMLTokenizer(BeforeAttributeValue) - switches to AttributeValueUnquoted when parsing any "
            "other character",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<div a=a");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AttributeValueUnquoted);
  }

#pragma endregion

#pragma region AttributeValueDoubleQuoted

  TEST_CASE("HTMLTokenizer(AttributeValueDoubleQuoted) - switches to AfterAttributeValueQuoted when parsing "
            "QuotationMark",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::AttributeValueDoubleQuoted);

    inputStream.Append(U"\"");
    expected = U"";

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AfterAttributeValueQuoted);
  }

  TEST_CASE(
    "HTMLTokenizer(AttributeValueDoubleQuoted) - Switches to CharacterReference when parsing Ampersand",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::AttributeValueDoubleQuoted);

    inputStream.Append(U"&");
    expected = U"";

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CharacterReference);
  }

  TEST("AttributeValueDoubleQuoted", "Replaces null character with U+FFFD",
       (UnitTest {.Input = InsertNull(U"<div a=\"value", U"\">"),
                  .Output = {CreateStartTagToken({.Name = U"div",
                                                  .Attributes = {{.Name = U"a", .Value = U"value\xFFFD"}}})},
                  .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter}}}))

  TEST("AttributeValueDoubleQuoted", "Emits EOF instead of tag if EOF reached",
       (UnitTest {.AppendEOF = true,
                  .Input = U"<div a=\"value",
                  .Output = {CreateEOFToken()},
                  .Errors = {{.Error = HTMLParseError::EOFInTag}}}))

  TEST("AttributeValueDoubleQuoted", "appends to attribute value when parsing any other character",
       (UnitTest {
         .Input = U"<div a=\"valúe\">",
         .Output = {CreateStartTagToken({.Name = U"div", .Attributes = {{.Name = U"a", .Value = U"valúe"}}})},
       }))

#pragma endregion

#pragma region AttributeValueSingleQuoted

  TEST_CASE("HTMLTokenizer(AttributeValueSingleQuoted) - switches to AfterAttributeValueQuoted when parsing "
            "Apostrophe",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::AttributeValueSingleQuoted);

    inputStream.Append(U"'");
    expected = U"";

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AfterAttributeValueQuoted);
  }

  TEST_CASE(
    "HTMLTokenizer(AttributeValueSingleQuoted) - Switches to CharacterReference when parsing Ampersand",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::AttributeValueSingleQuoted);

    inputStream.Append(U"&");
    expected = U"";

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CharacterReference);
  }

  TEST("AttributeValueSingleQuoted", "Replaces null character with U+FFFD",
       (UnitTest {.Input = InsertNull(U"<div a='value", U"'>"),
                  .Output = {CreateStartTagToken({.Name = U"div",
                                                  .Attributes = {{.Name = U"a", .Value = U"value\xFFFD"}}})},
                  .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter}}}))

  TEST_CASE("HTMLTokenizer(AttributeValueSingleQuoted) - Emits EOF instead of tag if EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    expected = U"";
    expectedErrorCount = 1;

    inputStream.Append(U"<div a='value", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    REQUIRE(errors.back() == HTMLParseError::EOFInTag);
  }

  TEST_CASE("HTMLTokenizer(AttributeValueSingleQuoted) - appends to attribute value when parsing any "
            "other character",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<div a='value");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AttributeValueSingleQuoted);
  }

#pragma endregion

#pragma region AttributeValueUnquoted

  TEST_CASE("HTMLTokenizer(AttributeValueUnquoted) - switches to BeforeAttributeName when parsing "
            "whitespace",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<div a=value ");
    expected = U"";

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeName);
  }

  TEST_CASE("HTMLTokenizer(AttributeValueUnquoted) - Switches to CharacterReference when parsing Ampersand",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::AttributeValueUnquoted);

    inputStream.Append(U"&");
    expected = U"";

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CharacterReference);
  }

  TEST("AttributeValueUnquoted", "switches to Data and emits the current tag when parsing GreaterThanSign",
       (UnitTest {
         .Input = U"<div a=value>",
         .Output = {CreateStartTagToken({.Name = U"div", .Attributes = {{.Name = U"a", .Value = U"value"}}})},
       }))

  TEST("AttributeValueUnquoted", "Replaces null character with U+FFFD",
       (UnitTest {.Input = InsertNull(U"<div a=value", U">"),
                  .Output = {CreateStartTagToken({.Name = U"div",
                                                  .Attributes = {{.Name = U"a", .Value = U"value\xFFFD"}}})},
                  .Errors = {{.Error = HTMLParseError::UnexpectedNullCharacter}}}))

  TEST("AttributeValueUnquoted", "treats QuotationMark as anything else but with parse error",
       (UnitTest {.Input = U"<div a=val\"ue>",
                  .Output = {CreateStartTagToken({.Name = U"div",
                                                  .Attributes = {{.Name = U"a", .Value = U"val\"ue"}}})},
                  .Errors = {{.Error = HTMLParseError::UnexpectedCharacterInUnquotedAttributeValue}}}))

  TEST("AttributeValueUnquoted", "treats Apostrophe as anything else but with parse error",
       (UnitTest {.Input = U"<div a=val'ue>",
                  .Output = {CreateStartTagToken({.Name = U"div",
                                                  .Attributes = {{.Name = U"a", .Value = U"val'ue"}}})},
                  .Errors = {{.Error = HTMLParseError::UnexpectedCharacterInUnquotedAttributeValue}}}))

  TEST("AttributeValueUnquoted", "treats LessThanSign as anything else but with parse error",
       (UnitTest {.Input = U"<div a=val<ue>",
                  .Output = {CreateStartTagToken({.Name = U"div",
                                                  .Attributes = {{.Name = U"a", .Value = U"val<ue"}}})},
                  .Errors = {{.Error = HTMLParseError::UnexpectedCharacterInUnquotedAttributeValue}}}))

  TEST("AttributeValueUnquoted", "treats EqualsSign as anything else but with parse error",
       (UnitTest {.Input = U"<div a=val=ue>",
                  .Output = {CreateStartTagToken({.Name = U"div",
                                                  .Attributes = {{.Name = U"a", .Value = U"val=ue"}}})},
                  .Errors = {{.Error = HTMLParseError::UnexpectedCharacterInUnquotedAttributeValue}}}))

  TEST("AttributeValueUnquoted", "treats GraveAccent as anything else but with parse error",
       (UnitTest {.Input = U"<div a=val`ue>",
                  .Output = {CreateStartTagToken({.Name = U"div",
                                                  .Attributes = {{.Name = U"a", .Value = U"val`ue"}}})},
                  .Errors = {{.Error = HTMLParseError::UnexpectedCharacterInUnquotedAttributeValue}}}))

  TEST_CASE("HTMLTokenizer(AttributeValueUnquoted) - Emits EOF instead of tag if EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<div a=value", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    REQUIRE(errors.back() == HTMLParseError::EOFInTag);
  }

#pragma endregion

#pragma region AfterAttributeValueQuoted

  TEST_CASE(
    "HTMLTokenizer(AfterAttributeValueQuoted) - ignores whitespace and switches to BeforeAttributeName "
    "when parsing whitespace",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<div a=\"value\"   \t\n\r");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeAttributeName);
  }

  TEST_CASE("HTMLTokenizer(AfterAttributeValueQuoted) - switches to SelfClosingStartTag when parsing Solidus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<div a=\"value\"/");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::SelfClosingStartTag);
  }

  TEST("AfterAttributeValueQuoted", "switches to Data and emits the current tag when parsing GreaterThanSign",
       (UnitTest {.Input = U"<div a=\"value\">",
                  .Output = {CreateStartTagToken({.Name = U"div",
                                                  .Attributes = {{.Name = U"a", .Value = U"value"}}})}}))

  TEST_CASE("HTMLTokenizer(AfterAttributeValueQuoted) - Emits EOF instead of tag if EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<div a=\"value\"", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    REQUIRE(errors.back() == HTMLParseError::EOFInTag);
  }

  TEST_CASE("HTMLTokenizer(AfterAttributeValueQuoted) - Treats anything else as missing whitespace and "
            "switches to AttributeName",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<div a=\"value\"b");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AttributeName);
    REQUIRE(errors.back() == HTMLParseError::MissingWhitespaceBetweenAttributes);
  }

#pragma endregion

#pragma region SelfClosingStartTag

  TEST_CASE("HTMLTokenizer(SelfClosingStartTag) - switches to Data and emits the current tag as self-"
            "closing when parsing GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"div";
    inputStream.Append(U"<div />");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::StartTag);
    REQUIRE(token->IsSelfClosing());
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE("HTMLTokenizer(SelfClosingStartTag) - Emits EOF instead of tag if EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<div /", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    REQUIRE(errors.back() == HTMLParseError::EOFInTag);
  }

  TEST_CASE("HTMLTokenizer(SelfClosingStartTag) - Treats anything else as missing Solidus with "
            "parser error and switches to AttributeName",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<div /a");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AttributeName);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedSolidusInTag);
  }

#pragma endregion

#pragma region BogusComment

  TEST_CASE("HTMLTokenizer(BogusComment) - emits comment and switches to Data when parsing GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"?comment";
    expectedErrorCount = 1;
    inputStream.Append(U"<?comment>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Comment);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE("HTMLTokenizer(BogusComment) - emits comment and then EOF when EOF reached", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    inputStream.Append(U"<?comment", IsEOF(true));

    expected = U"?comment";
    expectedErrorCount = 1;
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Comment);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(BogusComment) - appends to comment when parsing NullCharacter",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"?com\xFFFDm";
    expectedErrorCount = 2;
    utf32_string input = U"<?com";
    input.append(1uz, U'\0');
    input.append(U"m>");
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Comment);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE("HTMLTokenizer(BogusComment) - appends to comment when parsing any other character",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"?comm";
    expectedErrorCount = 1;
    inputStream.Append(U"<?comm>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Comment);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

#pragma endregion

#pragma region MarkupDeclarationOpen

  TEST_CASE("HTMLTokenizer(MarkupDeclarationOpen) - switches to CommentStart when parsing DoubleHyphen",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::MarkupDeclarationOpen);

    expected = U"";
    inputStream.Append(U"--");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CommentStart);
  }

  TEST_CASE("HTMLTokenizer(MarkupDeclarationOpen) - switches to DOCTYPE when parsing 'DOCTYPE'",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::MarkupDeclarationOpen);

    expected = U"";
    inputStream.Append(U"DOCTYPE");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPE);
  }

  TEST_CASE("HTMLTokenizer(MarkupDeclarationOpen) - switches to DOCTYPE when parsing 'DOCTYPE', mixed case",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::MarkupDeclarationOpen);

    expected = U"";
    inputStream.Append(U"DoCtYpe");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPE);
  }

  TEST_CASE(
    "HTMLTokenizer(MarkupDeclarationOpen) - switches to CDATASection when parsing '[CDATA[' and CDATA "
    "is allowed",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::MarkupDeclarationOpen);
    tokenizer.SetCDATASectionsAllowed(true);

    expected = U"";
    inputStream.Append(U"[CDATA[");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CDATASection);
  }

  TEST_CASE(
    "HTMLTokenizer(MarkupDeclarationOpen) - switches to BogusComment when parsing '[CDATA[' and CDATA "
    "is not allowed",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::MarkupDeclarationOpen);
    tokenizer.SetCDATASectionsAllowed(false);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"[CDATA[");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BogusComment);
    REQUIRE(errors.back() == HTMLParseError::CDATAInHTMLContent);
  }

  TEST_CASE("HTMLTokenizer(MarkupDeclarationOpen) - switches to BogusComment with parser error when "
            "parsing anything else",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::MarkupDeclarationOpen);

    expected = U"!comment";
    expectedErrorCount = 1;
    inputStream.Append(U"!comment>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Comment);
    REQUIRE(errors.back() == HTMLParseError::IncorrectlyOpenedComment);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

#pragma endregion

#pragma region CommentStart

  TEST_CASE("HTMLTokenizer(CommentStart) - switches to CommentStartDash when parsing HyphenMinus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!---");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CommentStartDash);
  }

  TEST_CASE("HTMLTokenizer(CommentStart) - switches to Data and emits comment when parsing GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!-->");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Comment);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
    REQUIRE(errors.back() == HTMLParseError::AbruptClosingOfEmptyComment);
  }

  TEST_CASE("HTMLTokenizer(CommentStart) - reconsumes in Comment for anything else", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--c");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::Comment);
  }

#pragma endregion

#pragma region CommentStartDash

  TEST_CASE("HTMLTokenizer(CommentStartDash) - switches to CommentEnd when parsing HyphenMinus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!----");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CommentEnd);
  }

  TEST_CASE(
    "HTMLTokenizer(CommentStartDash) - switches to Data and emits comment when parsing GreaterThanSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!--->");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Comment);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
    REQUIRE(errors.back() == HTMLParseError::AbruptClosingOfEmptyComment);
  }

  TEST_CASE("HTMLTokenizer(CommentStartDash) - Emits comment and then EOF when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"-comment";
    expectedErrorCount = 1;
    inputStream.Append(U"<!---comment", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Comment);
      REQUIRE(errors.back() == HTMLParseError::EOFInComment);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(CommentStartDash) - emits HyphenMinus and reconsumes in Comment for anything else",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!---c");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::Comment);
  }

#pragma endregion

#pragma region Comment

  TEST_CASE("HTMLTokenizer(Comment) - switches to CommentLessThanSign when parsing LessThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"comment";
    inputStream.Append(U"<!--a<");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CommentLessThanSign);
  }

  TEST_CASE("HTMLTokenizer(Comment) - switches to CommentEndDash when parsing HyphenMinus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"com";
    inputStream.Append(U"<!--com-");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CommentEndDash);
  }

  TEST_CASE("HTMLTokenizer(Comment) - Replaces null character with U+FFFD when parsing NullCharacter",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    utf32_string input = U"<!--com";
    input.append(1uz, U'\0');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
    REQUIRE(tokenizer.GetState() == TokenizerState::Comment);
  }

  TEST_CASE("HTMLTokenizer(Comment) - Emits comment and then EOF when EOF reached", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"comment";
    expectedErrorCount = 1;
    inputStream.Append(U"<!--comment", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Comment);
      REQUIRE(errors.back() == HTMLParseError::EOFInComment);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(Comment) - appends to comment when parsing any other character",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"comment";
    inputStream.Append(U"<!--comment");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::Comment);
  }

#pragma endregion

#pragma region CommentLessThanSign

  TEST_CASE("HTMLTokenizer(CommentLessThanSign) - switches to CommentLessThanSignBang when parsing "
            "ExclamationMark",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--a<!");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CommentLessThanSignBang);
  }

  TEST_CASE(
    "HTMLTokenizer(CommentLessThanSign) - appends LessThanSign and remains in CommentLessThanSign when "
    "parsing LessThanSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--a<<");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CommentLessThanSign);
  }

  TEST_CASE("HTMLTokenizer(CommentLessThanSign) - reconsumes in Comment for anything else",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--a<b");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::Comment);
  }

#pragma endregion

#pragma region CommentLessThanSignBang

  TEST_CASE("HTMLTokenizer(CommentLessThanSignBang) - switches to CommentLessThanSignBangDash when parsing "
            "HyphenMinus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--a<!-");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CommentLessThanSignBangDash);
  }

  TEST_CASE("HTMLTokenizer(CommentLessThanSignBang) - reconsumes in Comment for anything else",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--a<!b");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::Comment);
  }

#pragma endregion

#pragma region CommentLessThanSignBangDash

  TEST_CASE("HTMLTokenizer(CommentLessThanSignBangDash) - switches to CommentLessThanSignBangDashDash when "
            "parsing HyphenMinus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--a<!--");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CommentLessThanSignBangDashDash);
  }

  TEST_CASE("HTMLTokenizer(CommentLessThanSignBangDash) - reconsumes in Comment for anything else",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--a<!-b");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::Comment);
  }

#pragma endregion

#pragma region CommentLessThanSignBangDashDash

  TEST_CASE("HTMLTokenizer(CommentLessThanSignBangDashDash) - emits comment when parsing GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"a<!";
    inputStream.Append(U"<!--a<!-->");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Comment);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE("HTMLTokenizer(CommentLessThanSignBangDashDash) - emits comment with parser error followed by "
            "EOF when parsing EOF",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"a<!";
    expectedErrorCount = 1;
    inputStream.Append(U"<!--a<!--", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Comment);
      REQUIRE(errors.back() == HTMLParseError::EOFInComment);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(CommentLessThanSignBangDashDash) - reconsumes in Comment with parser error for "
            "anything else",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!--a<!--b");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::Comment);
    REQUIRE(errors.back() == HTMLParseError::NestedComment);
  }

#pragma endregion

#pragma region CommentEndDash

  TEST_CASE("HTMLTokenizer(CommentEndDash) - switches to CommentEnd when parsing HyphenMinus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--com--");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CommentEnd);
  }

  TEST_CASE(
    "HTMLTokenizer(CommentEndDash) - emits comment with parser error followed by EOF when parsing EOF",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expectedErrorCount = 1;
    inputStream.Append(U"<!--com-", IsEOF(true));

    expected = U"com";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Comment);
      REQUIRE(errors.back() == HTMLParseError::EOFInComment);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(CommentEndDash) - reconsumes in Comment for anything else", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--com-a");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::Comment);
  }

#pragma endregion

#pragma region CommentEnd

  TEST_CASE("HTMLTokenizer(CommentEnd) - emits comment when parsing GreaterThanSign", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    expected = U"comment";
    inputStream.Append(U"<!--comment-->");
    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Comment);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE("HTMLTokenizer(CommentEnd) - switches to CommentEndBang when parsing Exclamation",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--comment--!");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CommentEndBang);
  }

  TEST_CASE("HTMLTokenizer(CommentEnd) - remains in CommentEnd when parsing HyphenMinus", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--comment---");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CommentEnd);
  }

  TEST_CASE("HTMLTokenizer(CommentEnd) - emits comment with parser error followed by EOF when parsing EOF",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expectedErrorCount = 1;
    inputStream.Append(U"<!--comment--", IsEOF(true));

    expected = U"comment";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Comment);
      REQUIRE(errors.back() == HTMLParseError::EOFInComment);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(CommentEnd) - reconsumes in Comment with parser error for anything else",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--comment--a");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::Comment);
  }

#pragma endregion

#pragma region CommentEndBang

  TEST_CASE("HTMLTokenizer(CommentEndBang) - switches to CommentEndDash when parsing HyphenMinus",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--comment--!-");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CommentEndDash);
  }

  TEST_CASE("HTMLTokenizer(CommentEndBang) - emits comment when parsing GreaterThanSign", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"comment";
    expectedErrorCount = 1;
    inputStream.Append(U"<!--comment--!>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Comment);
    REQUIRE(errors.back() == HTMLParseError::IncorrectlyClosedComment);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE(
    "HTMLTokenizer(CommentEndBang) - emits comment with parser error followed by EOF when parsing EOF",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expectedErrorCount = 1;
    inputStream.Append(U"<!--comment--!", IsEOF(true));

    expected = U"comment";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Comment);
      REQUIRE(errors.back() == HTMLParseError::EOFInComment);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(CommentEndBang) - reconsumes in Comment for anything else", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!--comment--!a");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::Comment);
  }

#pragma endregion

#pragma region DOCTYPE

  TEST_CASE("HTMLTokenizer(DOCTYPE) - switches to BeforeDOCTYPEName when parsing whitespace",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::DOCTYPE);

    expected = U"";
    inputStream.Append(U" ");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeDOCTYPEName);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPE) - emits DOCTYPE token with force-quirks when parsing GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(token->IsForceQuirks());
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
    REQUIRE(errors.back() == HTMLParseError::MissingDOCTYPEName);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPE) - emits EOF with force-quirks when EOF reached", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(DOCTYPE) - treats anything else as missing whitespace before DOCTYPE name",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPEa");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEName);
    REQUIRE(errors.back() == HTMLParseError::MissingWhitespaceBeforeDOCTYPEName);
  }

#pragma endregion

#pragma region BeforeDOCTYPEName

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPEName) - ignores whitespace", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE   \t\n\r  ");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeDOCTYPEName);
  }

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPEName) - switches to DOCTYPEName when parsing ASCII upper",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE A");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEName);
  }

  TEST_CASE(
    "HTMLTokenizer(BeforeDOCTYPEName) - emits ReplacementCharacter with parser error when parsing Null",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    utf32_string input = U"<!DOCTYPE";
    input.append(1uz, U'\0');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEName);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPEName) - emits DOCTYPE with force-quirks when parsing GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(token->IsForceQuirks());
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
    REQUIRE(errors.back() == HTMLParseError::MissingDOCTYPEName);
  }

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPEName) - emits DOCTYPE with force-quirks when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPEName) - treats anything else as start of DOCTYPE name",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPEa");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEName);
    REQUIRE(errors.back() == HTMLParseError::MissingWhitespaceBeforeDOCTYPEName);
  }

#pragma endregion

#pragma region DOCTYPEName

  TEST_CASE("HTMLTokenizer(DOCTYPEName) - switches to AfterDOCTYPEName when parsing whitespace",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"HTML5";
    inputStream.Append(U"<!DOCTYPE HTML5 ");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AfterDOCTYPEName);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPEName) - switches to Data and emits DOCTYPE when parsing GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html5";
    inputStream.Append(U"<!DOCTYPE HTML5>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(!token->IsForceQuirks());
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPEName) - appends to name when parsing ASCII upper", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML5");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEName);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPEName) - appends ReplacementCharacter with parser error when parsing Null",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    utf32_string input = U"<!DOCTYPE HT";
    input.append(1uz, U'\0');
    input.append(U"ML5");
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEName);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPEName) - emits DOCTYPE with force-quirks when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html5";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML5", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(DOCTYPEName) - treats anything else as part of DOCTYPE name", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML5a");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEName);
  }

#pragma endregion

#pragma region AfterDOCTYPEName

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEName) - ignores whitespace", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html5";
    inputStream.Append(U"<!DOCTYPE HTML5   \t\n\r  ");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AfterDOCTYPEName);
  }

  TEST_CASE(
    "HTMLTokenizer(AfterDOCTYPEName) - switches to Data and emits DOCTYPE when parsing GreaterThanSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html5";
    inputStream.Append(U"<!DOCTYPE HTML5>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(!token->IsForceQuirks());
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEName) - emits DOCTYPE with force-quirks when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html5";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML5", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEName) - switches to AfterDOCTYPEPublicKeyword when parsing 'PUBLIC'",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML PuBLiC");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AfterDOCTYPEPublicKeyword);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEName) - switches to AfterDOCTYPESystemKeyword when parsing 'SYSTEM'",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML SySTeM");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AfterDOCTYPESystemKeyword);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEName) - treats anything else as unexpected and switches to "
            "BogusDOCTYPE",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML a");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BogusDOCTYPE);
    REQUIRE(errors.back() == HTMLParseError::InvalidCharacterSequenceAfterDOCTYPEName);
  }

#pragma endregion

#pragma region AfterDOCTYPEPublicKeyword

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEPublicKeyword) - switches to BeforeDOCTYPEPublicIdentifier when "
            "parsing whitespace",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC ");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeDOCTYPEPublicIdentifier);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEPublicKeyword) - switches to DOCTYPEPublicIdentifierDoubleQuoted with "
            "parser error when parsing QuotationMark",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC\"");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEPublicIdentifierDoubleQuoted);
    REQUIRE(errors.back() == HTMLParseError::MissingWhitespaceAfterDOCTYPEPublicKeyword);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEPublicKeyword) - switches to DOCTYPEPublicIdentifierSingleQuoted with "
            "parser error when parsing Apostrophe",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC'");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEPublicIdentifierSingleQuoted);
    REQUIRE(errors.back() == HTMLParseError::MissingWhitespaceAfterDOCTYPEPublicKeyword);
  }

  TEST_CASE(
    "HTMLTokenizer(AfterDOCTYPEPublicKeyword) - emits DOCTYPE with force-quirks when parsing GreaterThanSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(token->IsForceQuirks());
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
    REQUIRE(errors.back() == HTMLParseError::MissingDOCTYPEPublicIdentifier);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEPublicKeyword) - emits DOCTYPE with force-quirks when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEPublicKeyword) - treats anything else as unexpected and switches to "
            "BogusDOCTYPE",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLICa");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BogusDOCTYPE);
    REQUIRE(errors.back() == HTMLParseError::MissingQuoteBeforeDOCTYPEPublicIdentifier);
  }

#pragma endregion

#pragma region BeforeDOCTYPEPublicIdentifier

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPEPublicIdentifier) - ignores whitespace", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC   \t\n\r  ");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeDOCTYPEPublicIdentifier);
  }

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPEPublicIdentifier) - switches to DOCTYPEPublicIdentifierDoubleQuoted "
            "when parsing QuotationMark",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEPublicIdentifierDoubleQuoted);
  }

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPEPublicIdentifier) - switches to DOCTYPEPublicIdentifierSingleQuoted "
            "when parsing Apostrophe",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC '");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEPublicIdentifierSingleQuoted);
  }

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPEPublicIdentifier) - emits DOCTYPE with force-quirks when parsing "
            "GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(token->IsForceQuirks());
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
    REQUIRE(errors.back() == HTMLParseError::MissingDOCTYPEPublicIdentifier);
  }

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPEPublicIdentifier) - emits DOCTYPE with force-quirks when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(BeforeDOCTYPEPublicIdentifier) - treats anything else as unexpected and switches to "
    "BogusDOCTYPE",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLICa");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BogusDOCTYPE);
    REQUIRE(errors.back() == HTMLParseError::MissingQuoteBeforeDOCTYPEPublicIdentifier);
  }

#pragma endregion

#pragma region DOCTYPEPublicIdentifierDoubleQuoted

  TEST_CASE(
    "HTMLTokenizer(DOCTYPEPublicIdentifierDoubleQuoted) - switches to AfterDOCTYPEPublicIdentifier when "
    "parsing QuotationMark",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"identifier\"");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AfterDOCTYPEPublicIdentifier);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPEPublicIdentifierDoubleQuoted) - replaces Null with U+FFFD",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    utf32_string input = U"<!DOCTYPE HTML PUBLIC \"iden";
    input.append(1uz, U'\0');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEPublicIdentifierDoubleQuoted);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE(
    "HTMLTokenizer(DOCTYPEPublicIdentifierDoubleQuoted) - emits DOCTYPE with force-quirks when parsing "
    "GreaterThanSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"identifier>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(token->IsForceQuirks());
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
    REQUIRE(errors.back() == HTMLParseError::AbruptDOCTYPEPublicIdentifier);
  }

  TEST_CASE(
    "HTMLTokenizer(DOCTYPEPublicIdentifierDoubleQuoted) - emits DOCTYPE with force-quirks when EOF reached",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"identifier", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(DOCTYPEPublicIdentifierDoubleQuoted) - appends to public identifier when parsing "
            "any character except QuotationMark, GreaterThanSign, Null, or EOF",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"identifier");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEPublicIdentifierDoubleQuoted);
  }

#pragma endregion

#pragma region DOCTYPEPublicIdentifierSingleQuoted

  TEST_CASE(
    "HTMLTokenizer(DOCTYPEPublicIdentifierSingleQuoted) - switches to AfterDOCTYPEPublicIdentifier when "
    "parsing Apostrophe",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC 'identifier'");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AfterDOCTYPEPublicIdentifier);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPEPublicIdentifierSingleQuoted) - replaces Null with U+FFFD",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    utf32_string input = U"<!DOCTYPE HTML PUBLIC 'iden";
    input.append(1uz, U'\0');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEPublicIdentifierSingleQuoted);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE(
    "HTMLTokenizer(DOCTYPEPublicIdentifierSingleQuoted) - emits DOCTYPE with force-quirks when parsing "
    "GreaterThanSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC 'identifier>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(token->IsForceQuirks());
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
    REQUIRE(errors.back() == HTMLParseError::AbruptDOCTYPEPublicIdentifier);
  }

  TEST_CASE(
    "HTMLTokenizer(DOCTYPEPublicIdentifierSingleQuoted) - emits DOCTYPE with force-quirks when EOF reached",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC 'identifier", IsEOF(true));

    expected = U"html";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(DOCTYPEPublicIdentifierSingleQuoted) - appends to public identifier when parsing "
            "any character except Apostrophe, GreaterThanSign, Null, or EOF",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC 'identifier");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPEPublicIdentifierSingleQuoted);
  }

#pragma endregion

#pragma region AfterDOCTYPEPublicIdentifier

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEPublicIdentifier) - switches to "
            "BetweenDOCTYPEPublicAndSystemIdentifiers when parsing whitespace",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\" ");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BetweenDOCTYPEPublicAndSystemIdentifiers);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEPublicIdentifier) - switches to Data and emits DOCTYPE when parsing "
            "GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\">");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(!token->IsForceQuirks());
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEPublicIdentifier) - switches to DOCTYPESystemIdentifierDoubleQuoted "
            "with parsing error when parsing QuotationMark",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\"\"");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPESystemIdentifierDoubleQuoted);
    REQUIRE(errors.back() == HTMLParseError::MissingWhitespaceBetweenDOCTYPEPublicAndSystemIdentifiers);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEPublicIdentifier) - switches to DOCTYPESystemIdentifierSingleQuoted "
            "with parsing error when parsing Apostrophe",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\"'");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPESystemIdentifierSingleQuoted);
    REQUIRE(errors.back() == HTMLParseError::MissingWhitespaceBetweenDOCTYPEPublicAndSystemIdentifiers);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPEPublicIdentifier) - emits DOCTYPE with force-quirks when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\"", IsEOF(true));

    expected = U"html";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(AfterDOCTYPEPublicIdentifier) - treats anything else as unexpected and switches to "
    "BogusDOCTYPE",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\"a");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BogusDOCTYPE);
    REQUIRE(errors.back() == HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier);
  }

#pragma endregion

#pragma region BetweenDOCTYPEPublicAndSystemIdentifiers

  TEST_CASE("HTMLTokenizer(BetweenDOCTYPEPublicAndSystemIdentifiers) - ignores whitespace",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\"   \t\n\r  ");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BetweenDOCTYPEPublicAndSystemIdentifiers);
  }

  TEST_CASE(
    "HTMLTokenizer(BetweenDOCTYPEPublicAndSystemIdentifiers) - emits DOCTYPE when parsing GreaterThanSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\">");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE("HTMLTokenizer(BetweenDOCTYPEPublicAndSystemIdentifiers) - switches to "
            "DOCTYPESystemIdentifierDoubleQuoted "
            "when parsing QuotationMark",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\" \"");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPESystemIdentifierDoubleQuoted);
  }

  TEST_CASE("HTMLTokenizer(BetweenDOCTYPEPublicAndSystemIdentifiers) - switches to "
            "DOCTYPESystemIdentifierSingleQuoted "
            "when parsing Apostrophe",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\" '");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPESystemIdentifierSingleQuoted);
  }

  TEST_CASE("HTMLTokenizer(BetweenDOCTYPEPublicAndSystemIdentifiers) - emits DOCTYPE with force-quirks when "
            "EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\"", IsEOF(true));

    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(BetweenDOCTYPEPublicAndSystemIdentifiers) - treats anything else as unexpected "
            "and switches to "
            "BogusDOCTYPE",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\"a");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BogusDOCTYPE);
    REQUIRE(errors.back() == HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier);
  }

#pragma endregion

#pragma region AfterDOCTYPESystemKeyword

  TEST_CASE("HTMLTokenizer(AfterDOCTYPESystemKeyword) - switches to BeforeDOCTYPESystemIdentifier when "
            "parsing whitespace",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM ");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeDOCTYPESystemIdentifier);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPESystemKeyword) - switches to DOCTYPESystemIdentifierDoubleQuoted with "
            "parser error when parsing QuotationMark",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM\"");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPESystemIdentifierDoubleQuoted);
    REQUIRE(errors.back() == HTMLParseError::MissingWhitespaceAfterDOCTYPESystemKeyword);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPESystemKeyword) - switches to DOCTYPESystemIdentifierSingleQuoted with "
            "parser error when parsing Apostrophe",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM'");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPESystemIdentifierSingleQuoted);
    REQUIRE(errors.back() == HTMLParseError::MissingWhitespaceAfterDOCTYPESystemKeyword);
  }

  TEST_CASE(
    "HTMLTokenizer(AfterDOCTYPESystemKeyword) - emits DOCTYPE with force-quirks when parsing GreaterThanSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(token->IsForceQuirks());
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
    REQUIRE(errors.back() == HTMLParseError::MissingDOCTYPESystemIdentifier);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPESystemKeyword) - emits DOCTYPE with force-quirks when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM", IsEOF(true));

    expected = U"html";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPESystemKeyword) - treats anything else as unexpected and switches to "
            "BogusDOCTYPE",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEMa");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BogusDOCTYPE);
    REQUIRE(errors.back() == HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier);
  }

#pragma endregion

#pragma region BeforeDOCTYPESystemIdentifier

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPESystemIdentifier) - ignores whitespace", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM   \t\n\r  ");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BeforeDOCTYPESystemIdentifier);
  }

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPESystemIdentifier) - switches to DOCTYPESystemIdentifierDoubleQuoted "
            "when parsing QuotationMark",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM \"");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPESystemIdentifierDoubleQuoted);
  }

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPESystemIdentifier) - switches to DOCTYPESystemIdentifierSingleQuoted "
            "when parsing Apostrophe",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM '");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPESystemIdentifierSingleQuoted);
  }

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPESystemIdentifier) - emits DOCTYPE with force-quirks when parsing "
            "GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(token->IsForceQuirks());
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
    REQUIRE(errors.back() == HTMLParseError::MissingDOCTYPESystemIdentifier);
  }

  TEST_CASE("HTMLTokenizer(BeforeDOCTYPESystemIdentifier) - emits DOCTYPE with force-quirks when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM", IsEOF(true));

    expected = U"html";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(BeforeDOCTYPESystemIdentifier) - treats anything else as unexpected and switches to "
    "BogusDOCTYPE",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEMa");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BogusDOCTYPE);
    REQUIRE(errors.back() == HTMLParseError::MissingQuoteBeforeDOCTYPESystemIdentifier);
  }

#pragma endregion

#pragma region DOCTYPESystemIdentifierDoubleQuoted

  TEST_CASE("HTMLTokenizer(DOCTYPESystemIdentifierDoubleQuoted) - switches to AfterDOCTYPESystemIdentifier "
            "when parsing QuotationMark",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM \"identifier\"");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AfterDOCTYPESystemIdentifier);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPESystemIdentifierDoubleQuoted) - replaces Null with U+FFFD",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    utf32_string input = U"<!DOCTYPE HTML SYSTEM \"iden";
    input.append(1uz, U'\0');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPESystemIdentifierDoubleQuoted);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPESystemIdentifierDoubleQuoted) - emits DOCTYPE with parser error when "
            "parsing GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM \"identifier>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
    REQUIRE(errors.back() == HTMLParseError::AbruptDOCTYPESystemIdentifier);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPESystemIdentifierDoubleQuoted) - emits DOCTYPE with force-quirks when EOF "
            "reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM \"identifier", IsEOF(true));

    expected = U"html";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(DOCTYPESystemIdentifierDoubleQuoted) - appends to system identifier when parsing "
            "any character except QuotationMark, GreaterThanSign, Null, or EOF",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM \"identifier");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPESystemIdentifierDoubleQuoted);
  }

#pragma endregion

#pragma region DOCTYPESystemIdentifierSingleQuoted

  TEST_CASE("HTMLTokenizer(DOCTYPESystemIdentifierSingleQuoted) - switches to AfterDOCTYPESystemIdentifier "
            "when parsing Apostrophe",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM 'identifier'");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AfterDOCTYPESystemIdentifier);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPESystemIdentifierSingleQuoted) - replaces Null with U+FFFD",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    utf32_string input = U"<!DOCTYPE HTML SYSTEM 'iden";
    input.append(1uz, U'\0');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPESystemIdentifierSingleQuoted);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPESystemIdentifierSingleQuoted) - emits DOCTYPE with parser error when "
            "parsing GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM 'identifier>");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
    REQUIRE(errors.back() == HTMLParseError::AbruptDOCTYPESystemIdentifier);
  }

  TEST_CASE("HTMLTokenizer(DOCTYPESystemIdentifierSingleQuoted) - emits DOCTYPE with force-quirks when EOF "
            "reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM 'identifier", IsEOF(true));
    expected = U"html";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }
    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(DOCTYPESystemIdentifierSingleQuoted) - appends to system identifier when parsing "
            "any character except Apostrophe, GreaterThanSign, Null, or EOF",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM 'identifier");
    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::DOCTYPESystemIdentifierSingleQuoted);
  }

#pragma endregion

#pragma region AfterDOCTYPESystemIdentifier

  TEST_CASE("HTMLTokenizer(AfterDOCTYPESystemIdentifier) - ignores whitespace", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM \"id\"   \t\n\r  ");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::AfterDOCTYPESystemIdentifier);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPESystemIdentifier) - switches to Data and emits DOCTYPE when parsing "
            "GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM \"id\">");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(!token->IsForceQuirks());
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE("HTMLTokenizer(AfterDOCTYPESystemIdentifier) - emits DOCTYPE with force-quirks when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM \"id\"", IsEOF(true));

    expected = U"html";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
      REQUIRE(token->IsForceQuirks());
      REQUIRE(errors.back() == HTMLParseError::EOFInDOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(AfterDOCTYPESystemIdentifier) - treats anything else as unexpected and switches to "
    "BogusDOCTYPE",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML SYSTEM \"id\"a");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BogusDOCTYPE);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedCharacterAfterDOCTYPESystemIdentifier);
  }

#pragma endregion

#pragma region BogusDOCTYPE

  TEST_CASE("HTMLTokenizer(BogusDOCTYPE) - emits DOCTYPE when parsing GreaterThanSign", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\" a random text >");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
  }

  TEST_CASE("HTMLTokenizer(BogusDOCTYPE) - emits DOCTYPE when EOF reached", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\" a random text", IsEOF(true));
    expectedErrorCount = 1;

    expected = U"html";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
    }
  }

  TEST_CASE("HTMLTokenizer(BogusDOCTYPE) - ignores Null with parse error", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"";
    expectedErrorCount = 2;
    utf32_string input = U"<!DOCTYPE HTML PUBLIC \"id\" a random text";
    input.append(1uz, U'\0');
    inputStream.Append(std::move(input));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::BogusDOCTYPE);
    REQUIRE(errors.back() == HTMLParseError::UnexpectedNullCharacter);
  }

  TEST_CASE("HTMLTokenizer(BogusDOCTYPE) - ignores characters until GreaterThanSign or EOF",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"html";
    expectedErrorCount = 1;
    inputStream.Append(U"<!DOCTYPE HTML PUBLIC \"id\" a random text >");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::DOCTYPE);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

#pragma endregion

#pragma region CDATASection

  TEST_CASE("HTMLTokenizer(CDATASection) - switches to CDATASectionBracket when parsing RightSquareBracket",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::CDATASection);

    expected = U"";
    inputStream.Append(U"]");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CDATASectionBracket);
  }

  TEST_CASE("HTMLTokenizer(CDATASection) - emits EndOfFile with parser error when EOF reached",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::CDATASection);

    expectedErrorCount = 1;
    inputStream.Append(U"A", IsEOF(true));

    expected = U"A";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::Character);
    }

    expected = U"";
    {
      NextTokenPtr token = tokenizer.NextToken();
      COMMON_TEST_CASES(HTMLToken::Type::EndOfFile);
      REQUIRE(errors.back() == HTMLParseError::EOFInCDATA);
    }
  }

  TEST_CASE(
    "HTMLTokenizer(CDATASection) - emits Character tokens for all characters except RightSquareBracket "
    "and EOF",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::CDATASection);

    expected = U"A";
    inputStream.Append(U"A");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::CDATASection);
  }

#pragma endregion

#pragma region CDATASectionBracket

  TEST_CASE(
    "HTMLTokenizer(CDATASectionBracket) - switches to CDATASectionEnd when parsing RightSquareBracket",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::CDATASectionBracket);

    expected = U"";
    inputStream.Append(U"]");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::CDATASectionEnd);
  }

  TEST_CASE("HTMLTokenizer(CDATASectionBracket) - emits Character token for RightSquareBracket and switches "
            "back to CDATASection when parsing any character except RightSquareBracket or EOF",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::CDATASectionBracket);

    expected = U"]A";
    inputStream.Append(U"A");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::CDATASection);
  }

#pragma endregion

#pragma region CDATASectionEnd

  TEST_CASE("HTMLTokenizer(CDATASectionEnd) - switches to Data when parsing GreaterThanSign",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::CDATASectionEnd);

    expected = U"";
    inputStream.Append(U">");

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(!token);
    REQUIRE(tokenizer.GetState() == TokenizerState::Data);
  }

  TEST_CASE(
    "HTMLTokenizer(CDATASectionEnd) - emits Character tokens for two RightSquareBrackets and "
    "switches back to CDATASection when parsing any character except RightSquareBracket or GreaterThanSign",
    "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::CDATASectionEnd);

    expected = U"]]A";
    inputStream.Append(U"A");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::CDATASection);
  }

  TEST_CASE("HTMLTokenizer(CDATASectionEnd) - emits Character token for one RightSquareBracket and remains "
            "CDATASectionEnd when parsing RightSquareBracket",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::CDATASectionEnd);

    expected = U"]";
    inputStream.Append(U"]");

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(tokenizer.GetState() == TokenizerState::CDATASectionEnd);
  }

#pragma endregion
}