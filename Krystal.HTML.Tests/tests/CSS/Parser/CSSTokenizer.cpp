#include "Krystal.HTML/CSS/Parser/CSSTokenizer.hpp"
#include "Krystal.HTML.Tests/TestParsers/CSSTokenizerTestParser.hpp"
#include <catch_all.hpp>

namespace Krys::HTML::Tests
{
  namespace
  {
    void DoCSSTokenizerTest(const CSSTokenizerTest &test, size_t number, size_t total) noexcept
    {
      CSSInputStream inputStream(utf32_string(test.Css));
      CSSTokenizer tokenizer(inputStream);

      DOMString tokens;
      while (true)
      {
        auto token = tokenizer.ConsumeToken();
        if (!token.has_value())
        {
          break;
        }

        SerializeCSSToken(*token, tokens);
      }

      if (tokens != test.Tokens)
      {
        UTF8Info(u8"--- TEST " + ToUTF8(number + 1uz) + u8" OF " + ToUTF8(total) + u8" ---\n"
                 + Krys::Text::ConvertToUTF8(utf32_stringview(test.Css)));

        UTF8Info(u8"--- EXPECTED TOKENS ---\n" + test.Tokens);

        UTF8Info(u8"\n--- ACTUAL TOKENS ---\n" + tokens);

        CHECK(false);
      }
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