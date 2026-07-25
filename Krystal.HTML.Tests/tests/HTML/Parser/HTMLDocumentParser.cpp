#include "Krystal.HTML/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.HTML.Tests/TestParsers/HTMLTreeBuilderTestParser.hpp"
#include "Krystal.HTML/DOM/Internals/ElementFactory.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenizer.hpp"
#include <catch_all.hpp>
#include <filesystem>

namespace Krys::HTML::Tests
{
  namespace
  {
    Ref<HTMLDocumentParser> CreateParser(HTMLDocument &document, utf8_string &&input) noexcept
    {
      auto parser = CreateRef<HTMLDocumentParser>(document);
      parser->InputStream().Append(Krys::Move(input), IsEOF(true));
      return parser;
    }

    void DoHTMLTreeBuilderTest(const HTMLTreeBuilderTest &test, size_t number, size_t total) noexcept
    {
      DOMString output;
      if (test.FragmentContext.has_value())
      {
        auto document = CreateRef<Document>();
        TagName fragmentTagName = ParseTagName(*test.FragmentContext);
        QualifiedName qName = QualifiedName {Namespaces::HTML, DOMStringAtom::Null(),
                                             test.FragmentContext.value(), fragmentTagName, Namespace::HTML};
        auto element = ElementFactory::Create(*document, qName, DOMStringAtom::Null());
        auto result = HTMLDocumentParser::ParseFragment(
          *element, utf8_string(test.Input), false, test.ScriptingMode.value_or(ParserScriptingMode::Inert));

        output = u8"#document\n";
        for (auto &child : result)
        {
          SerializeNode(*child, output, 0uz);
        }
        NormaliseData(output);
      }
      else
      {
        auto document = CreateRef<HTMLDocument>();
        auto parser = CreateParser(*document, utf8_string(test.Input));

        if (test.ScriptingMode.has_value())
        {
          parser->ScriptingMode(*test.ScriptingMode);
        }

        (void)parser->PumpTokenizer();

        output = SerializeDocument(*document);
      }

      utf8_string str =
        u8"--- TEST " + ToUTF8(number + 1uz) + u8" OF " + ToUTF8(total) + u8" ---\n" + test.Input;
      UTF8_INFO(str);

      str = u8"--- FRAGMENT CONTEXT ---\n#document-fragment\n" + test.FragmentContext.value_or(u8"none");
      UTF8_INFO(str);

      str = u8"--- EXPECTED OUTPUT ---\n" + test.Expected;
      UTF8_INFO(str);

      str = u8"--- ACTUAL OUTPUT ---\n" + output;
      UTF8_INFO(str);

      bool equal = output == test.Expected;
      CHECK(equal);
    }

    void DoHTMLTreeBuilderTests(const List<HTMLTreeBuilderTest> &tests) noexcept
    {
      for (size_t i = 0uz; i < tests.size(); ++i)
      {
        DoHTMLTreeBuilderTest(tests[i], i, tests.size());
      }
    }
  }

#define EXECUTE_HTML_PARSER_TEST_CASE(testDataFile)                                                          \
  EXECUTE_TEST_CASE("HTMLDocumentParser", ParseHTMLTreeBuilderTests, DoHTMLTreeBuilderTests,                 \
                    "data/html-tree-builder/", testDataFile)

  EXECUTE_HTML_PARSER_TEST_CASE("adoption-01.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("adoption-02.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("blocks-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("comments-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("doctype-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("entities-01.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("entities-02.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("formatting-elements-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("foster-parenting-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("html5test-com.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("inbody-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("inner-html-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("isindex-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("main-element-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("math-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("menuitem-element-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("namespace-sensitivity-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("noscript-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("plain-text-unsafe-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("quirks-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("ruby-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("script-data-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("search-element-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("svg-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("tables-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("template-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("tests-01.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-02.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-03.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-04.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-05.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-06.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-07.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-08.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-09.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-11.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-12.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-14.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-15.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-16.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-17.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-18.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-19.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-20.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-21.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-22.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-23.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-24.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-25.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("tests-26.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("tricky-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("void-in-phrasing-01.dat");

  EXECUTE_HTML_PARSER_TEST_CASE("webkit-01.dat");
  EXECUTE_HTML_PARSER_TEST_CASE("webkit-02.dat");
}