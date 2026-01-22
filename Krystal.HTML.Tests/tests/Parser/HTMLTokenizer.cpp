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

#define SETUP_TEST()                                                                                         \
  HTMLInputStream inputStream;                                                                               \
  HTMLTokenizer tokenizer(inputStream);                                                                      \
  const auto &errors = tokenizer.GetParseErrors();                                                           \
  size_t expectedErrorCount = 0;                                                                             \
  auto expected = U"";

#define COMMON_TEST_CASES(tokenType)                                                                         \
  REQUIRE(token);                                                                                            \
  REQUIRE(token->GetType() == tokenType);                                                                    \
  REQUIRE(CompareDataBufferToString(token->GetDataBuffer(), expected));                                      \
  REQUIRE(errors.size() == expectedErrorCount)

#pragma region CharacterReference

  TEST_CASE("HTMLTokenizer(CharacterReference) - Non-character reference", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"&_";
    inputStream.Append(U"&_", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma region NamedCharacterReference

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - happy path", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"©";
    inputStream.Append(U"&copy;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - mixed case reference", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"⫌︀";
    inputStream.Append(U"&vsupnE;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - missing semicolon", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"À";
    expectedErrorCount = 1;
    inputStream.Append(U"&Agrave", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::MissingSemicolonAfterCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - no match", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"&nonentity";
    inputStream.Append(U"&nonentity", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - no match, ends in semicolon", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

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
    SETUP_TEST();

    expected = U"&co";
    inputStream.Append(U"&co", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

#pragma endregion

#pragma region DecimalCharacterReference

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - happy path", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"™";
    inputStream.Append(U"&#8482;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - lookup table", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"€";
    inputStream.Append(U"&#128;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - missing semicolon", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"™";
    expectedErrorCount = 1;
    inputStream.Append(U"&#8482", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::MissingSemicolonAfterCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - no numbers provided after #", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"&#;";
    expectedErrorCount = 1;
    inputStream.Append(U"&#;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::AbsenceOfDigitsInNumericCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - null character reference", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

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
    SETUP_TEST();

    expected = U"\xFFFD";
    expectedErrorCount = 1;
    inputStream.Append(U"&#1114112;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::CharacterReferenceOutsideUnicodeRange);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - surrogate", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"\xFFFD";
    expectedErrorCount = 1;
    inputStream.Append(U"&#55298;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::SurrogateCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - non character", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"\xFFFE";
    expectedErrorCount = 1;
    inputStream.Append(U"&#65534;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::NonCharacterCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - control character", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

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
    SETUP_TEST();

    expected = U"Œ";
    inputStream.Append(U"&#x152;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - lookup table", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"€";
    inputStream.Append(U"&#x80;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - mixed case hex digits", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"€";
    inputStream.Append(U"&#x20Ac;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - uppercase X", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"Œ";
    inputStream.Append(U"&#X152;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - missing semicolon", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

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
    SETUP_TEST();

    expected = U"&#X;";
    expectedErrorCount = 1;
    inputStream.Append(U"&#X;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors.back() == HTMLParseError::AbsenceOfDigitsInNumericCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - null character reference", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

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
    SETUP_TEST();

    expected = U"\xFFFD";
    expectedErrorCount = 1;
    inputStream.Append(U"&#x110000;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::CharacterReferenceOutsideUnicodeRange);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - surrogate", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"\xFFFD";
    expectedErrorCount = 1;
    inputStream.Append(U"&#xD800;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::SurrogateCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - non character", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"\xFFFE";
    expectedErrorCount = 1;
    inputStream.Append(U"&#xFFFE;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);

    REQUIRE(errors.back() == HTMLParseError::NonCharacterCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - control character", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

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
    SETUP_TEST();

    expected = U"©À€Œ&a;";
    expectedErrorCount = 2;
    inputStream.Append(U"&copy;&Agrave&#128;&#X152;&a;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES(HTMLToken::Type::Character);
    REQUIRE(errors[0] == HTMLParseError::MissingSemicolonAfterCharacterReference);
    REQUIRE(errors[1] == HTMLParseError::UnknownNamedCharacterReference);
  }

#pragma endregion
}