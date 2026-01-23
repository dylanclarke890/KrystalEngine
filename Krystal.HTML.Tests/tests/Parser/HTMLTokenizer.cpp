#include "Krystal.HTML/Parser/HTMLTokenizer.hpp"
#include <catch_all.hpp>

namespace
{
  using namespace Krys;
  using namespace Krys::HTML;

  bool CompareDataBufferToString(const HTMLToken::DataBuffer &buffer, utf32_stringview str) noexcept
  {
    if (buffer.size() != str.size())
    {
      return false;
    }
    for (size_t i = 0uz; i < buffer.size(); ++i)
    {
      if (buffer[i] != str[i])
      {
        return false;
      }
    }
    return true;
  }
}
namespace Krys::Tests
{
  using namespace Krys::HTML;

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
  REQUIRE(CompareDataBufferToString(token->GetDataBuffer(), expected));                                      \
  REQUIRE(errors.size() == expectedErrorCount);

#pragma region CharacterReference

  TEST_CASE("HTMLTokenizer(CharacterReference) - Non-character reference", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"&_";
    inputStream.Append(U"&_", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma region NamedCharacterReference

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - happy path", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"©";
    inputStream.Append(U"&copy;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - mixed case reference", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"⫌︀";
    inputStream.Append(U"&vsupnE;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - missing semicolon", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"À";
    expectedErrorCount = 1;
    inputStream.Append(U"&Agrave", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::MissingSemicolonAfterCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - no match", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"&nonentity";
    inputStream.Append(U"&nonentity", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - no match, ends in semicolon", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"&nonentity;";
    expectedErrorCount = 1;
    inputStream.Append(U"&nonentity;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::UnknownNamedCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - EOF in middle of otherwise valid reference",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"&co";
    inputStream.Append(U"&co", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  // TODO: test cases for when character references are consumed as part of attributes

#pragma endregion

#pragma region DecimalCharacterReference

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - happy path", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"™";
    inputStream.Append(U"&#8482;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - lookup table", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"€";
    inputStream.Append(U"&#128;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - missing semicolon", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"™";
    expectedErrorCount = 1;
    inputStream.Append(U"&#8482", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::MissingSemicolonAfterCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - no numbers provided after #", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"&#;";
    expectedErrorCount = 1;
    inputStream.Append(U"&#;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::AbsenceOfDigitsInNumericCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - null character reference", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"\xFFFD";
    expectedErrorCount = 1;
    inputStream.Append(U"&#0;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::NullCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - character reference outside unicode range",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"\xFFFD";
    expectedErrorCount = 1;
    inputStream.Append(U"&#1114112;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::CharacterReferenceOutsideUnicodeRange);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - surrogate", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"\xFFFD";
    expectedErrorCount = 1;
    inputStream.Append(U"&#55298;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::SurrogateCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - non character", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"\xFFFE";
    expectedErrorCount = 1;
    inputStream.Append(U"&#65534;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::NonCharacterCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - control character", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"\x0D";
    expectedErrorCount = 1;
    inputStream.Append(U"&#13;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::ControlCharacterReference);
  }

#pragma endregion

#pragma region HexadecimalCharacterReference

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - happy path", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"Œ";
    inputStream.Append(U"&#x152;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - lookup table", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"€";
    inputStream.Append(U"&#x80;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - mixed case hex digits", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"€";
    inputStream.Append(U"&#x20Ac;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - uppercase X", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"Œ";
    inputStream.Append(U"&#X152;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - missing semicolon", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"Œ";
    expectedErrorCount = 1;
    inputStream.Append(U"&#X152", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::MissingSemicolonAfterCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - no numbers provided after #X",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"&#X;";
    expectedErrorCount = 1;
    inputStream.Append(U"&#X;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::AbsenceOfDigitsInNumericCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - null character reference", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"\xFFFD";
    expectedErrorCount = 1;
    inputStream.Append(U"&#x00;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::NullCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - character reference outside unicode range",
            "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"\xFFFD";
    expectedErrorCount = 1;
    inputStream.Append(U"&#x110000;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::CharacterReferenceOutsideUnicodeRange);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - surrogate", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"\xFFFD";
    expectedErrorCount = 1;
    inputStream.Append(U"&#xD800;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::SurrogateCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - non character", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"\xFFFE";
    expectedErrorCount = 1;
    inputStream.Append(U"&#xFFFE;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::NonCharacterCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - control character", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"\x0D";
    expectedErrorCount = 1;
    inputStream.Append(U"&#x0D;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::ControlCharacterReference);
  }

#pragma endregion

  TEST_CASE("HTMLTokenizer(CharacterReference) - Multiple", "[HTML][Tokenizer]")
  {
    SETUP_TEST(TokenizerState::Data);

    expected = U"©À€Œ&a;";
    expectedErrorCount = 2;
    inputStream.Append(U"&copy;&Agrave&#128;&#X152;&a;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors[0] == HTMLParseError::MissingSemicolonAfterCharacterReference);
    REQUIRE(errors[1] == HTMLParseError::UnknownNamedCharacterReference);
  }

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
}