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
      auto input = u8"Input: " + Krys::Text::ConvertToUTF8(utf32_stringview(test.Input));
      INFO(string(reinterpret_cast<const char *>(input.data()), input.size()));

      auto expected = u8"Expected:\n" + test.Expected;
      INFO(string(reinterpret_cast<const char *>(expected.data()), expected.size()));

      auto document = CreateRef<HTMLDocument>();
      auto parser = CreateParser(*document, utf32_string(test.Input));
      REQUIRE_FALSE(parser->PumpTokenizer());

      auto dumped = Dump(*document);
      auto actual = u8"Actual:\n" + dumped;
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

  TEST_CASE("HTMLDocumentParser(basic test cases)", "[HTML][HTMLDocumentParser]")
  {
    auto tests = ParseTreeConstructionTests("data/html5-basic-tests.dat");
    REQUIRE(tests.has_value());
    DoTreeConstructionTests(*tests);
  }
}