#include "Krystal.HTML/HTML/Parser/HTMLTokenizer.hpp"
#include "Krystal.HTML.Tests/TestParsers/HTMLTokenizerTestParser.hpp"

namespace Krys::HTML::Tests
{
  namespace
  {
    void DoHTMLTokenizerTest(const HTMLTokenizerTest &test, size_t number, size_t total) noexcept
    {
      HTMLInputStream inputStream;
      HTMLTokenizer tokenizer(inputStream, [allowed = test.CDATASectionAllowed]() { return allowed; });
      tokenizer.State(test.InitialState);
      if (test.AppendEOF)
      {
        inputStream.Append(utf32_string(test.Html), IsEOF(true));
      }
      else
      {
        inputStream.Append(utf32_string(test.Html), IsEOF(false));
      }

      DOMString tokens;
      while (true)
      {
        auto token = tokenizer.NextToken();
        if (!token)
        {
          break;
        }
        SerializeHTMLToken(*token, tokens);
      }

      NormaliseData(tokens);

      utf8_string str = u8"--- TEST " + ToUTF8(number + 1uz) + u8" OF " + ToUTF8(total) + u8" ---\n"
                        + Krys::Text::ConvertToUTF8(utf32_stringview(test.Html));
      UTF8_INFO(str);

      str = u8"--- EXPECTED TOKENS ---\n" + test.Tokens;
      UTF8_INFO(str);

      str = u8"\n--- ACTUAL TOKENS ---\n" + tokens;
      UTF8_INFO(str);

      auto initialState = string(magic_enum::enum_name(test.InitialState));
      str = u8"\n--- INITIAL STATE ---\n" + ToUTF8(initialState);

      auto expectedState = string(magic_enum::enum_name(test.ExpectedState));
      str = u8"\n--- EXPECTED STATE ---\n" + ToUTF8(expectedState);

      CHECK(tokenizer.State() == test.ExpectedState);

      size_t errorIndex = 0uz;

      for (const auto &error : tokenizer.ParseErrors())
      {
        if (errorIndex >= test.Errors.size())
        {
          break;
        }

        CHECK(error.Error == test.Errors[errorIndex].Error);
        CHECK(error.Location.Line == test.Errors[errorIndex].Location.Line);
        CHECK(error.Location.Column == test.Errors[errorIndex].Location.Column);
        ++errorIndex;
      }

      bool equal = tokens == test.Tokens;
      CHECK(equal);
    }

    void DoHTMLTokenizerTests(const List<HTMLTokenizerTest> &tests) noexcept
    {
      for (size_t i = 0uz; i < tests.size(); ++i)
      {
        DoHTMLTokenizerTest(tests[i], i, tests.size());
      }
    }
  }

#define HTML_TOKENIZER_TEST_CASE(datFile)                                                                    \
  TEST_CASE("HTMLTokenizer(" datFile ")", "[HTML][HTMLTokenizer]")                                           \
  {                                                                                                          \
    auto tests = ParseHTMLTokenizerTests("data/html-tokenizer/" datFile);                                    \
    REQUIRE(tests.has_value());                                                                              \
    DoHTMLTokenizerTests(*tests);                                                                            \
  }

  HTML_TOKENIZER_TEST_CASE("character-reference-01.dat");
}