#include "Krystal.HTML/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.HTML.Tests/TestDataParsers/HTMLTreeBuilderTestDataParser.hpp"
#include "Krystal.HTML/DOM/Internals/ElementFactory.hpp"
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

    void DoTreeConstructionTest(const TreeConstructionTest &test, size_t number, size_t total) noexcept
    {
      auto input = u8"--- TEST " + ToUTF8String(number + 1uz) + u8" OF " + ToUTF8String(total) + u8" ---\n"
                   + Krys::Text::ConvertToUTF8(utf32_stringview(test.Input));
      INFO(string(reinterpret_cast<const char *>(input.data()), input.size()));

      auto fragmentContext =
        u8"--- FRAGMENT CONTEXT ---\n#document-fragment\n" + test.FragmentContext.value_or(u8"none");
      INFO(string(reinterpret_cast<const char *>(fragmentContext.data()), fragmentContext.size()));

      auto expected = u8"--- EXPECTED OUTPUT ---\n" + test.Expected;
      INFO(string(reinterpret_cast<const char *>(expected.data()), expected.size()));

      DOMString output;
      if (test.FragmentContext.has_value())
      {
        auto document = CreateRef<Document>();
        TagName fragmentTagName = ParseTagName(*test.FragmentContext);
        QualifiedName qName = QualifiedName {Namespaces::HTML, DOMStringAtom::Null(),
                                             test.FragmentContext.value(), fragmentTagName, Namespace::HTML};
        auto element = ElementFactory::Create(*document, qName, DOMStringAtom::Null());
        auto result = HTMLDocumentParser::ParseFragment(
          *element, utf32_string(test.Input), false, test.ScriptingMode.value_or(ParserScriptingMode::Inert));

        auto documentFragment = CreateRef<DocumentFragment>(*document);

        output = u8"#document\n";
        for (auto &child : result)
        {
          DumpNode(*child, output, 0uz);
        }
        NormaliseData(output);
      }
      else
      {
        auto document = CreateRef<HTMLDocument>();
        auto parser = CreateParser(*document, utf32_string(test.Input));

        if (test.ScriptingMode.has_value())
        {
          parser->ScriptingMode(*test.ScriptingMode);
        }

        (void)parser->PumpTokenizer();

        output = Dump(*document);
      }

      auto actual = u8"\n--- ACTUAL OUTPUT ---\n" + output;
      INFO(string(reinterpret_cast<const char *>(actual.data()), actual.size()));

      bool equal = (output == test.Expected);
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

  PARSER_TEST_CASE("html-tree-builder/adoption-01.dat");
  PARSER_TEST_CASE("html-tree-builder/adoption-02.dat");

  PARSER_TEST_CASE("html-tree-builder/blocks-01.dat");

  PARSER_TEST_CASE("html-tree-builder/comments-01.dat");

  PARSER_TEST_CASE("html-tree-builder/doctype-01.dat");

  PARSER_TEST_CASE("html-tree-builder/entities-01.dat");
  PARSER_TEST_CASE("html-tree-builder/entities-02.dat");

  PARSER_TEST_CASE("html-tree-builder/formatting-elements-01.dat");

  PARSER_TEST_CASE("html-tree-builder/foster-parenting-01.dat");

  PARSER_TEST_CASE("html-tree-builder/html5test-com.dat");

  PARSER_TEST_CASE("html-tree-builder/inbody-01.dat");

  PARSER_TEST_CASE("html-tree-builder/inner-html-01.dat");

  PARSER_TEST_CASE("html-tree-builder/isindex-01.dat");

  PARSER_TEST_CASE("html-tree-builder/main-element-01.dat");

  PARSER_TEST_CASE("html-tree-builder/math-01.dat");

  PARSER_TEST_CASE("html-tree-builder/menuitem-element-01.dat");

  PARSER_TEST_CASE("html-tree-builder/namespace-sensitivity-01.dat");

  PARSER_TEST_CASE("html-tree-builder/noscript-01.dat");

  PARSER_TEST_CASE("html-tree-builder/plain-text-unsafe-01.dat");

  PARSER_TEST_CASE("html-tree-builder/quirks-01.dat");

  PARSER_TEST_CASE("html-tree-builder/ruby-01.dat");

  PARSER_TEST_CASE("html-tree-builder/script-data-01.dat");

  PARSER_TEST_CASE("html-tree-builder/search-element-01.dat");

  PARSER_TEST_CASE("html-tree-builder/svg-01.dat");

  PARSER_TEST_CASE("html-tree-builder/tables-01.dat");

  PARSER_TEST_CASE("html-tree-builder/template-01.dat");

  PARSER_TEST_CASE("html-tree-builder/tests-01.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-02.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-03.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-04.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-05.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-06.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-07.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-08.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-09.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-11.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-12.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-14.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-15.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-16.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-17.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-18.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-19.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-20.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-21.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-22.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-23.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-24.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-25.dat");
  PARSER_TEST_CASE("html-tree-builder/tests-26.dat");

  PARSER_TEST_CASE("html-tree-builder/tricky-01.dat");

  PARSER_TEST_CASE("html-tree-builder/void-in-phrasing-01.dat");

  PARSER_TEST_CASE("html-tree-builder/webkit-01.dat");
  PARSER_TEST_CASE("html-tree-builder/webkit-02.dat");
}