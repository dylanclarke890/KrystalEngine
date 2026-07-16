#include "Krystal.HTML/CSS/Parser/CSSTokenizer.hpp"
#include "Krystal.HTML.Tests/TestParsers/CSSTokenizerTestParser.hpp"
#include <catch_all.hpp>

namespace Krys::HTML::Tests
{
  namespace
  {
    KRYS_NODISCARD List<CSSToken> ParseCSSTokens(utf32_string&& css) noexcept
    {
      CSSInputStream inputStream(Krys::Move(css));
      CSSTokenizer tokenizer(inputStream);

      List<CSSToken> tokens;
      while (true)
      {
        auto token = tokenizer.ConsumeToken();
        if (!token.has_value())
        {
          break;
        }

        tokens.push_back(*token);
      }

      return tokens;
    }

    void DoCSSTokenizerTest(const CSSTokenizerTest &test, size_t number, size_t total) noexcept
    {
      auto tokens = ParseCSSTokens(utf32_string(test.Css));
      auto actual = SerializeCSSTokens(tokens);

      utf8_string str = u8"--- TEST " + ToUTF8(number + 1uz) + u8" OF " + ToUTF8(total) + u8" ---\n"
                        + Krys::Text::ConvertToUTF8(utf32_stringview(test.Css));
      UTF8_INFO(str);

      str = u8"--- EXPECTED TOKENS ---\n" + test.Tokens;
      UTF8_INFO(str);

      str = u8"\n--- ACTUAL TOKENS ---\n" + actual;
      UTF8_INFO(str);

      bool equal = actual == test.Tokens;
      CHECK(equal);
    }

    void DoCSSTokenizerTests(const List<CSSTokenizerTest> &tests) noexcept
    {
      for (size_t i = 0uz; i < tests.size(); ++i)
      {
        DoCSSTokenizerTest(tests[i], i, tests.size());
      }
    }
  }

#define CSS_TOKENIZER_TEST_CASE(datFile)                                                                     \
  TEST_CASE("CSSTokenizer(" datFile ")", "[HTML][CSSTokenizer]")                                             \
  {                                                                                                          \
    auto tests = ParseCSSTokenizerTests("data/css-tokenizer/" datFile);                                      \
    REQUIRE(tests.has_value());                                                                              \
    DoCSSTokenizerTests(*tests);                                                                             \
  }

  CSS_TOKENIZER_TEST_CASE("at-keyword-01.dat");
}