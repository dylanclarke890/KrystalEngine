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
                        + ToUTF8(test.Name.empty() ? "Unnamed Test" : test.Name);
      UTF8_INFO(str);

      str = u8"\n--- INPUT ---\n" + Krys::Text::ConvertToUTF8(utf32_stringview(test.Html));
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

  HTML_TOKENIZER_TEST_CASE("afterattributename-01.dat");
  HTML_TOKENIZER_TEST_CASE("afterattributevaluequoted-01.dat");
  HTML_TOKENIZER_TEST_CASE("afterdoctypename-01.dat");
  HTML_TOKENIZER_TEST_CASE("afterdoctypepublicidentifier-01.dat");
  HTML_TOKENIZER_TEST_CASE("afterdoctypepublickeyword-01.dat");
  HTML_TOKENIZER_TEST_CASE("afterdoctypesystemidentifier-01.dat");
  HTML_TOKENIZER_TEST_CASE("afterdoctypesystemkeyword-01.dat");
  HTML_TOKENIZER_TEST_CASE("attributename-01.dat");
  HTML_TOKENIZER_TEST_CASE("attributevaluedoublequoted-01.dat");
  HTML_TOKENIZER_TEST_CASE("attributevaluesinglequoted-01.dat");
  HTML_TOKENIZER_TEST_CASE("attributevalueunquoted-01.dat");
  HTML_TOKENIZER_TEST_CASE("beforeattributename-01.dat");
  HTML_TOKENIZER_TEST_CASE("beforeattributevalue-01.dat");
  HTML_TOKENIZER_TEST_CASE("beforedoctypename-01.dat");
  HTML_TOKENIZER_TEST_CASE("beforedoctypepublicidentifier-01.dat");
  HTML_TOKENIZER_TEST_CASE("beforedoctypesystemidentifier-01.dat");
  HTML_TOKENIZER_TEST_CASE("betweendoctypepublicandsystemidentifiers-01.dat");
  HTML_TOKENIZER_TEST_CASE("boguscomment-01.dat");
  HTML_TOKENIZER_TEST_CASE("bogusdoctype-01.dat");
  HTML_TOKENIZER_TEST_CASE("cdatasection-01.dat");
  HTML_TOKENIZER_TEST_CASE("cdatasectionbracket-01.dat");
  HTML_TOKENIZER_TEST_CASE("cdatasectionend-01.dat");
  HTML_TOKENIZER_TEST_CASE("character-reference-01.dat");
  HTML_TOKENIZER_TEST_CASE("character-reference-02.dat");
  HTML_TOKENIZER_TEST_CASE("character-reference-03.dat");
  HTML_TOKENIZER_TEST_CASE("character-reference-04.dat");
  HTML_TOKENIZER_TEST_CASE("comment-01.dat");
  HTML_TOKENIZER_TEST_CASE("commentend-01.dat");
  HTML_TOKENIZER_TEST_CASE("commentendbang-01.dat");
  HTML_TOKENIZER_TEST_CASE("commentenddash-01.dat");
  HTML_TOKENIZER_TEST_CASE("commentlessthan-01.dat");
  HTML_TOKENIZER_TEST_CASE("commentlessthanbang-01.dat");
  HTML_TOKENIZER_TEST_CASE("commentlessthanbangdash-01.dat");
  HTML_TOKENIZER_TEST_CASE("commentlessthanbangdashdash-01.dat");
  HTML_TOKENIZER_TEST_CASE("commentstart-01.dat");
  HTML_TOKENIZER_TEST_CASE("commentstartdash-01.dat");
  HTML_TOKENIZER_TEST_CASE("data-01.dat");
  HTML_TOKENIZER_TEST_CASE("doctype-01.dat");
  HTML_TOKENIZER_TEST_CASE("doctypename-01.dat");
  HTML_TOKENIZER_TEST_CASE("doctypepublicidentifierdoublequoted-01.dat");
  HTML_TOKENIZER_TEST_CASE("doctypepublicidentifiersinglequoted-01.dat");
  HTML_TOKENIZER_TEST_CASE("doctypesystemidentifierdoublequoted-01.dat");
  HTML_TOKENIZER_TEST_CASE("doctypesystemidentifiersinglequoted-01.dat");
  HTML_TOKENIZER_TEST_CASE("endtagopen-01.dat");
  HTML_TOKENIZER_TEST_CASE("markupdeclarationopen-01.dat");
  HTML_TOKENIZER_TEST_CASE("plaintext-01.dat");
  HTML_TOKENIZER_TEST_CASE("rawtext-01.dat");
  HTML_TOKENIZER_TEST_CASE("rawtextendtagopen-01.dat");
  HTML_TOKENIZER_TEST_CASE("rawtextlessthan-01.dat");
  HTML_TOKENIZER_TEST_CASE("rcdata-01.dat");
  HTML_TOKENIZER_TEST_CASE("rcdataendtagname-01.dat");
  HTML_TOKENIZER_TEST_CASE("rcdataendtagopen-01.dat");
  HTML_TOKENIZER_TEST_CASE("rcdatalessthan-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdata-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescaped-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapeddash-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapeddashdash-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapedlessthan-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapeend-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdatadoubleescapestart-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdataendtagname-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdataendtagopen-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdataescaped-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdataescapeddash-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdataescapeddashdash-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdataescapedendtagname-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdataescapedendtagopen-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdataescapedlessthan-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdataescapestart-01.dat");
  HTML_TOKENIZER_TEST_CASE("scriptdatalessthan-01.dat");
  HTML_TOKENIZER_TEST_CASE("selfclosingstartag-01.dat");
  HTML_TOKENIZER_TEST_CASE("tagname-01.dat");
  HTML_TOKENIZER_TEST_CASE("tagopen-01.dat");
}