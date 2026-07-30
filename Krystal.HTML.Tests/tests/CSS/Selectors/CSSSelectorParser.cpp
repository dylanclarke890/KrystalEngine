#include "Krystal.HTML/CSS/Selectors/CSSSelectorParser.hpp"
#include "Krystal.HTML.Tests/TestParserUtils.hpp"
#include "Krystal.HTML/CSS/CSSImportRule.hpp"
#include "Krystal.HTML/CSS/CSSNamespaceRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/CSSStyleRule.hpp"
#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/MediaList.hpp"
#include "Krystal.HTML/CSS/StyleSheetContents.hpp"
#include <catch_all.hpp>

namespace Krys::HTML::Tests
{
  namespace
  {
    struct CSSSelectorParserTest
    {
      utf8_string Input;
      utf8_string Output;
      bool ExpectInvalid {false};
    };

    KRYS_NODISCARD List<CSSSelectorParserTest> ParseCSSSelectorParserTest(std::istream &stream) noexcept
    {
      List<CSSSelectorParserTest> tests;
      CSSSelectorParserTest currentTest;

      Maybe<utf8_string> output;

      auto finishTest = [&]()
      {
        if (!currentTest.Input.empty())
        {
          if (output.has_value())
          {
            NormaliseData(*output);
            currentTest.Output = Krys::Move(*output);
            output.reset();
          }
          else if (!currentTest.ExpectInvalid)
          {
            // If no output is specified for a valid test, assume the output is the same as the input.
            currentTest.Output = currentTest.Input;
          }
          tests.push_back(Krys::Move(currentTest));
          currentTest = {};
        }
      };

      auto parse = [&](utf8_string sectionName, utf8_string data)
      {
        if (sectionName == u8"input")
        {
          finishTest();
          NormaliseData(data);
          currentTest.Input = Krys::Move(data);
        }
        else if (sectionName == u8"output")
        {
          NormaliseData(data);
          output = Krys::Move(data);
        }
        else if (sectionName == u8"expect-invalid")
        {
          currentTest.ExpectInvalid = true;
        }
      };

      ParseTestData(stream, "$", ::Krys::Move(parse));
      finishTest();

      return tests;
    }

    void ExecuteCSSSelectorParserTest(const CSSSelectorParserTest &test, size_t number, size_t total) noexcept
    {
      auto result = CSSSelectorParser::ParseSelector(CSSOMString(test.Input), {}, nullptr, Null);

      utf8_string str = u8"--- TEST " + ToUTF8(number + 1uz) + u8" OF " + ToUTF8(total) + u8" ---\n";
      UTF8_INFO(str);

      str = u8"\n--- INPUT ---\n" + test.Input;
      UTF8_INFO(str);

      if (test.ExpectInvalid)
      {
        str = u8"\n--- EXPECTED OUTPUT ---\n" + ToUTF8("Invalid selector");
        bool isInvalid = !result.has_value();
        CHECK(isInvalid);
      }
      else
      {
        str = u8"--- EXPECTED OUTPUT ---\n" + test.Output;
        UTF8_INFO(str);

        auto output = result ? result->SelectorText() : u8"";
        str = u8"\n--- ACTUAL OUTPUT ---\n" + output;
        UTF8_INFO(str);

        bool equal = output == test.Output;
        CHECK(equal);
      }
    }

    void RunTest(string filename) noexcept
    {
      static string basedir = "data/css-selector-parser/";

      auto file = OpenTestDataFile(basedir + filename);
      REQUIRE(file.has_value());

      auto tests = ParseCSSSelectorParserTest(*file);
      REQUIRE(!tests.empty());

      ExecuteTests(tests, ExecuteCSSSelectorParserTest);
    }
  }

#define EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE(FileName)                                                      \
  TEST_CASE("CSSSelectorParser(" FileName ")", "[CSS][CSSSelectorParser]")                                   \
  {                                                                                                          \
    RunTest(FileName);                                                                                       \
  }

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("attribute-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("child-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("class-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("descendant-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("invalid-pseudo-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("invalid-pseudo-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("universal-01.dat");
}