#include "Krystal.HTML/CSS/Parser/CSSTokenizer.hpp"
#include "Krystal.HTML.Tests/TestParsers/CSSTokenizerTestParser.hpp"
#include <catch_all.hpp>

namespace Krys::HTML::Tests
{
  namespace
  {
    void DoCSSTokenizerTest(const CSSTokenizerTest &test, size_t number, size_t total) noexcept
    {
      utf8_string str =
        u8"--- TEST " + ToUTF8(number + 1uz) + u8" OF " + ToUTF8(total) + u8" ---\n" + test.Css;
      UTF8_INFO(str);

      str = u8"--- EXPECTED TOKENS ---\n" + test.Tokens;
      UTF8_INFO(str);

      CSSInputStream inputStream(utf8_string(test.Css));
      CSSTokenizer tokenizer(inputStream);
      REQUIRE(tokenizer.PumpTokenizer());

      auto actual = SerializeCSSTokens(tokenizer.TokenRange());
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

#define EXECUTE_CSS_TOKENIZER_TEST_CASE(testDataFile)                                                        \
  EXECUTE_TEST_CASE("CSSTokenizer", ParseCSSTokenizerTests, DoCSSTokenizerTests, "data/css-tokenizer/",      \
                    testDataFile)

  EXECUTE_CSS_TOKENIZER_TEST_CASE("at-keyword-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("bad-string-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("bad-url-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("close-curly-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("close-paren-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("close-square-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("colon-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("comma-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("digit-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("dimension-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("escaped-code-point-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("full-stop-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-01.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-02.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-03.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-04.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-05.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-06.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-07.dat");
  EXECUTE_CSS_TOKENIZER_TEST_CASE("fuzz-08.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("hash-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("hyphen-minus-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("ident-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("ident-like-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("less-than-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("number-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("numeric-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("open-curly-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("open-paren-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("open-square-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("plus-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("reverse-solidus-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("semicolon-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("string-01.dat");

  EXECUTE_CSS_TOKENIZER_TEST_CASE("url-01.dat");
}