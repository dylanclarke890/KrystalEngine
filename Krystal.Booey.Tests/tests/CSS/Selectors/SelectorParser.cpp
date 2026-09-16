#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey.Tests/TestParserUtils.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/MediaList.hpp"
#include "Krystal.Booey/CSS/Rules/CSSOM/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/Rules/CSSOM/CSSNamespaceRule.hpp"
#include "Krystal.Booey/CSS/Rules/CSSOM/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/Rules/CSSOM/CSSStyleRule.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include <catch_all.hpp>

namespace krys::boo::css::tests
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
            krys::boo::tests::NormaliseData(*output);
            currentTest.Output = krys::move(*output);
            output.reset();
          }
          else if (!currentTest.ExpectInvalid)
          {
            // If no output is specified for a valid test, assume the output is the same as the input.
            currentTest.Output = currentTest.Input;
          }
          tests.push_back(krys::move(currentTest));
          currentTest = {};
        }
      };

      auto parse = [&](utf8_string sectionName, utf8_string data)
      {
        if (sectionName == u8"input")
        {
          finishTest();
          krys::boo::tests::NormaliseData(data);
          currentTest.Input = krys::move(data);
        }
        else if (sectionName == u8"output")
        {
          krys::boo::tests::NormaliseData(data);
          output = krys::move(data);
        }
        else if (sectionName == u8"expect-invalid")
        {
          currentTest.ExpectInvalid = true;
        }
      };

      krys::boo::tests::ParseTestData(stream, "$", ::krys::move(parse));
      finishTest();

      return tests;
    }

    void ExecuteCSSSelectorParserTest(const CSSSelectorParserTest &test, size_t number, size_t total) noexcept
    {
      auto result = SelectorParser::ParseSelector(CSSOMString(test.Input), {}, nullptr, null);

      utf8_string str = u8"--- TEST " + krys::boo::tests::ToUTF8(number + 1uz) + u8" OF "
                        + krys::boo::tests::ToUTF8(total) + u8" ---\n";
      UTF8_INFO(str);

      str = u8"\n--- INPUT ---\n" + test.Input;
      UTF8_INFO(str);

      if (test.ExpectInvalid)
      {
        str = u8"\n--- EXPECTED OUTPUT ---\n" + krys::boo::tests::ToUTF8("Invalid selector");
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

      auto file = krys::boo::tests::OpenTestDataFile(basedir + filename);
      REQUIRE(file.has_value());

      auto tests = ParseCSSSelectorParserTest(*file);
      REQUIRE(!tests.empty());

      krys::boo::tests::ExecuteTests(tests, ExecuteCSSSelectorParserTest);
    }
  }

#define EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE(FileName)                                                      \
  TEST_CASE("CSSSelectorParser(" FileName ")", "[CSS][CSSSelectorParser]")                                   \
  {                                                                                                          \
    RunTest(FileName);                                                                                       \
  }

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("an-plus-b-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("an-plus-b-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("attribute-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("child-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("class-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("descendant-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("focus-visible-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("has-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("has-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("heading-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("heading-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("id-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("invalid-pseudo-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("invalid-pseudo-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("not-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("not-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("part-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("part-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("sibling-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("slotted-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("slotted-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("state-01.dat");
  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("state-02.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("universal-01.dat");

  EXECUTE_CSS_SELECTOR_PARSER_TEST_CASE("where-01.dat");
}