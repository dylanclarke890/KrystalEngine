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

    void DoTreeConstructionTest(const TreeConstructionTest &test) noexcept
    {
      auto input = u8"\n--- INPUT ---\n" + Krys::Text::ConvertToUTF8(utf32_stringview(test.Input));
      INFO(string(reinterpret_cast<const char *>(input.data()), input.size()));

      auto expected = u8"\n--- EXPECTED OUTPUT ---\n" + test.Expected;
      INFO(string(reinterpret_cast<const char *>(expected.data()), expected.size()));

      auto document = CreateRef<HTMLDocument>();
      auto parser = CreateParser(*document, utf32_string(test.Input));
      (void)parser->PumpTokenizer();

      auto dumped = Dump(*document);
      auto actual = u8"\n--- ACTUAL OUTPUT ---\n" + dumped;
      INFO(string(reinterpret_cast<const char *>(actual.data()), actual.size()));

      CHECK(dumped == test.Expected);
    }

    void DoTreeConstructionTests(const List<TreeConstructionTest> &tests) noexcept
    {
      for (const auto &test : tests)
      {
        DoTreeConstructionTest(test);
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

  PARSER_TEST_CASE("tree-construction/basic-01.dat");
  PARSER_TEST_CASE("tree-construction/basic-02.dat");
  PARSER_TEST_CASE("tree-construction/basic-03.dat");
  PARSER_TEST_CASE("tree-construction/inbody-01.dat");
  PARSER_TEST_CASE("tree-construction/isindex.dat");
  PARSER_TEST_CASE("tree-construction/comments-01.dat");
  PARSER_TEST_CASE("tree-construction/doctype-01.dat");
}