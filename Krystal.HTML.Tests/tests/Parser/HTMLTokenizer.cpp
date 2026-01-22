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

#define COMMON_TEST_CASES()                                                                                  \
  REQUIRE(token);                                                                                            \
  REQUIRE(token->GetType() == HTMLToken::Type::Character);                                                   \
  REQUIRE(CompareDataBufferToString(token->GetDataBuffer(), expected));                                      \
  REQUIRE(errors.size() == expectedErrorCount)

  TEST_CASE("HTMLTokenizer(CharacterReference) - Non-character reference", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"&_";
    inputStream.Append(U"&_", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES();
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - happy path", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"©";
    inputStream.Append(U"&copy;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES();
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - mixed case", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"⫌︀";
    inputStream.Append(U"&vsupnE;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES();
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - missing semicolon", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"À";
    expectedErrorCount = 1;
    inputStream.Append(U"&Agrave", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    REQUIRE(token);
    REQUIRE(token->GetType() == HTMLToken::Type::Character);
    REQUIRE(CompareDataBufferToString(token->GetDataBuffer(), expected));
    REQUIRE(errors.size() == expectedErrorCount);
    REQUIRE(errors.back() == HTMLParseError::MissingSemicolonAfterCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - no match", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"&nonentity";
    inputStream.Append(U"&nonentity", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES();
  }

  TEST_CASE("HTMLTokenizer(NamedCharacterReference) - no match, ends in semicolon", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"&nonentity;";
    expectedErrorCount = 1;
    inputStream.Append(U"&nonentity;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES();

    REQUIRE(errors.back() == HTMLParseError::UnknownNamedCharacterReference);
  }

  TEST_CASE("HTMLTokenizer(DecimalCharacterReference) - happy path", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"™";
    inputStream.Append(U"&#8482;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES();
  }

  TEST_CASE("HTMLTokenizer(HexadecimalCharacterReference) - happy path", "[HTML][Tokenizer]")
  {
    SETUP_TEST();

    expected = U"Œ";
    inputStream.Append(U"&#x152;", IsEOF(true));

    NextTokenPtr token = tokenizer.NextToken();
    COMMON_TEST_CASES();
  }
}