#include "Krystal.HTML/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.HTML.Tests/DumpTree.hpp"
#include "Krystal.HTML.Tests/TreeConstructionTestParser.hpp"
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
      parser->InputStream().Append(utf32_string(input), IsEOF(true));
      return parser;
    }

    void DoTreeConstructionTest(const TreeConstructionTest &test) noexcept
    {
      auto document = CreateRef<HTMLDocument>();

      auto parser = CreateParser(*document, utf32_string(test.Input));
      (void)parser->PumpTokenizer();

      auto input = u8"Input: " + Krys::Text::ConvertToUTF8(utf32_stringview(test.Input));
      auto expectedDocument = u8"Expected Document:\n" + test.ExpectedDocument;

      INFO(string(reinterpret_cast<const char *>(input.data()), input.size()));
      INFO(string(reinterpret_cast<const char *>(expectedDocument.data()), expectedDocument.size()));

      auto dumpedDocument = Dump(*document);
      auto actualDocument = u8"Actual Document:\n" + dumpedDocument;
      INFO(string(reinterpret_cast<const char *>(actualDocument.data()), actualDocument.size()));

      CHECK(dumpedDocument == test.ExpectedDocument);
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