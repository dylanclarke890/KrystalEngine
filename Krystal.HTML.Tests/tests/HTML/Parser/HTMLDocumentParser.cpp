#include "Krystal.HTML/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.HTML.Tests/TreeConstruction.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenizer.hpp"
#include <catch_all.hpp>
#include <filesystem>

namespace Krys::HTML::Tests
{
  namespace
  {
    Ref<HTMLDocumentParser> CreateParser(HTMLDocument &document, utf32_string &&input) noexcept
    {
      auto parser = CreateRef<HTMLDocumentParser>(document);
      parser->InputStream().Append(Krys::Move(input), IsEOF(true));
      return parser;
    }

    utf8_string ToUTF8String(size_t number) noexcept
    {
      auto str = std::to_string(number);
      return utf8_string(str.begin(), str.end());
    }

    void ParseDocument(HTMLDocument &document, const TreeConstructionTest &test) noexcept
    {
      auto parser = CreateParser(document, utf32_string(test.Input));

      if (test.ScriptingMode.has_value())
      {
        parser->ScriptingMode(*test.ScriptingMode);
      }

      (void)parser->PumpTokenizer();
    }

    void DoTreeConstructionTest(const TreeConstructionTest &test, size_t number, size_t total) noexcept
    {
      auto input = u8"--- TEST " + ToUTF8String(number) + u8" OF " + ToUTF8String(total) + u8" ---\n"
                   + Krys::Text::ConvertToUTF8(utf32_stringview(test.Input));
      INFO(string(reinterpret_cast<const char *>(input.data()), input.size()));

      auto fragmentContext =
        u8"--- FRAGMENT CONTEXT ---\n#document-fragment\n" + test.FragmentContext.value_or(u8"none");
      INFO(string(reinterpret_cast<const char *>(fragmentContext.data()), fragmentContext.size()));

      auto expected = u8"--- EXPECTED OUTPUT ---\n" + test.Expected;
      INFO(string(reinterpret_cast<const char *>(expected.data()), expected.size()));

      auto document = CreateRef<HTMLDocument>();
      ParseDocument(*document, test);

      auto dumped = Dump(*document);
      auto actual = u8"\n--- ACTUAL OUTPUT ---\n" + dumped;
      INFO(string(reinterpret_cast<const char *>(actual.data()), actual.size()));

      bool equal = (dumped == test.Expected);
      CHECK(equal);
    }

    void DoTreeConstructionTests(const List<TreeConstructionTest> &tests) noexcept
    {
      for (size_t i = 0uz; i < tests.size(); ++i)
      {
        DoTreeConstructionTest(tests[i], i, tests.size());
      }
    }
  }

#define PARSER_TEST_CASE(datFile)                                                                            \
  TEST_CASE("HTMLDocumentParser(" datFile ")", "[HTML][HTMLDocumentParser]")                                 \
  {                                                                                                          \
    auto tests = ParseTreeConstructionTests("data/" datFile);                                                \
    REQUIRE(tests.has_value());                                                                              \
    DoTreeConstructionTests(*tests);                                                                         \
  }

  PARSER_TEST_CASE("tree-construction/adoption-01.dat");
  PARSER_TEST_CASE("tree-construction/adoption-02.dat");

  PARSER_TEST_CASE("tree-construction/blocks-01.dat");

  PARSER_TEST_CASE("tree-construction/comments-01.dat");

  PARSER_TEST_CASE("tree-construction/doctype-01.dat");

  PARSER_TEST_CASE("tree-construction/entities-01.dat");
  PARSER_TEST_CASE("tree-construction/entities-02.dat");

  PARSER_TEST_CASE("tree-construction/formatting-elements-01.dat");

  PARSER_TEST_CASE("tree-construction/foster-parenting-01.dat");

  PARSER_TEST_CASE("tree-construction/html5test-com.dat");

  PARSER_TEST_CASE("tree-construction/inbody-01.dat");

  PARSER_TEST_CASE("tree-construction/isindex-01.dat");

  PARSER_TEST_CASE("tree-construction/main-element-01.dat");

  PARSER_TEST_CASE("tree-construction/math-01.dat");

  PARSER_TEST_CASE("tree-construction/menuitem-element-01.dat");

  PARSER_TEST_CASE("tree-construction/namespace-sensitivity-01.dat");

  PARSER_TEST_CASE("tree-construction/noscript-01.dat");

  PARSER_TEST_CASE("tree-construction/plain-text-unsafe-01.dat");

  PARSER_TEST_CASE("tree-construction/quirks-01.dat");

  PARSER_TEST_CASE("tree-construction/ruby-01.dat");

  PARSER_TEST_CASE("tree-construction/script-data-01.dat");

  PARSER_TEST_CASE("tree-construction/search-element-01.dat");

  PARSER_TEST_CASE("tree-construction/svg-01.dat");

  PARSER_TEST_CASE("tree-construction/tables-01.dat");

  PARSER_TEST_CASE("tree-construction/template-01.dat");

  PARSER_TEST_CASE("tree-construction/tests-01.dat");

  PARSER_TEST_CASE("tree-construction/tricky-01.dat");

  PARSER_TEST_CASE("tree-construction/void-in-phrasing-01.dat");
}