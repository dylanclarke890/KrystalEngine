#include "Krystal.HTML/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.HTML.Tests/TestParserUtils.hpp"
#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Comment.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/Internals/ElementFactory.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.HTML/HTML/Enums/ParserScriptingMode.hpp"
#include "Krystal.HTML/HTML/HTMLTemplateElement.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenizer.hpp"
#include "Krystal.HTML/SVG/Internals/SVGElementFactory.hpp"
#include "Krystal.HTML/SVG/SVGElement.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/Types/Pair.hpp"
#include "Krystal.Text/StringConversion.hpp"
#include <catch_all.hpp>
#include <filesystem>

namespace Krys::HTML::Tests
{
  namespace
  {
#pragma region Serialize

    void SerializeNode(Node &node, DOMString &output, size_t depth) noexcept
    {
      switch (node.NodeType())
      {
        case NodeType::ELEMENT_NODE:
        {
          auto &element = Downcast<Element>(node);

          Indent(output, depth);

          auto localName = DOMString(element.LocalName().View());
          auto namespaceName = [&] -> DOMString
          {
            if (element.NamespaceURI() == Namespaces::HTML)
            {
              return u8"";
            }
            else if (element.NamespaceURI() == Namespaces::SVG)
            {
              return u8"svg ";
            }
            else if (element.NamespaceURI() == Namespaces::MathML)
            {
              return u8"math ";
            }
            else
            {
              return u8"";
            }
          }();

          output += u8"<" + namespaceName + localName + u8">\n";

          List<Pair<DOMString, DOMString>> attributes;
          for (size_t i = 0; i < element.Attributes().Length(); ++i)
          {
            auto attr = element.Attributes().Item(i);
            auto localName = DOMString(attr->LocalName().View());
            auto namespaceName = [&] -> DOMString
            {
              if (attr->NamespaceURI() == Namespaces::HTML)
              {
                return u8"";
              }
              else if (attr->NamespaceURI() == Namespaces::XML)
              {
                return u8"xml ";
              }
              else if (attr->NamespaceURI() == Namespaces::XMLNS)
              {
                return u8"xmlns ";
              }
              else if (attr->NamespaceURI() == Namespaces::XLink)
              {
                return u8"xlink ";
              }
              else
              {
                return u8"";
              }
            }();

            auto &value = attr->Value();

            attributes.push_back({namespaceName + localName, value});
          }

          std::sort(attributes.begin(), attributes.end(),
                    [](const auto &a, const auto &b) { return a.first < b.first; });

          for (auto &[name, value] : attributes)
          {
            Indent(output, depth + 1uz);
            output += name + u8"=\"" + value + u8"\"\n";
          }

          if (element.NamespaceURI() == Namespaces::HTML && element.LocalName() == u8"template")
          {
            Indent(output, depth + 1uz);
            output += u8"content\n";

            for (auto &child : ChildNodeRange(*Downcast<HTMLTemplateElement>(element).Content()))
            {
              SerializeNode(child, output, depth + 2uz);
            }
          }
          break;
        }
        case NodeType::TEXT_NODE:
        {
          Indent(output, depth);
          output += u8'"' + Downcast<HTML::Text>(node).Data() + u8"\"\n";
          break;
        }
        case NodeType::COMMENT_NODE:
        {
          Indent(output, depth);
          output += u8"<!-- " + Downcast<Comment>(node).Data() + u8" -->\n";
          break;
        }
        case NodeType::DOCUMENT_TYPE_NODE:
        {
          auto &documentType = Downcast<DocumentType>(node);

          Krys::HTML::Tests::Indent(output, depth);

          output += u8"<!DOCTYPE ";
          output += documentType.Name();

          if (!documentType.PublicId().empty() || !documentType.SystemId().empty())
          {
            output += u8" \"";
            output += documentType.PublicId();
            output += u8"\" \"";
            output += documentType.SystemId();
            output += u8'"';
          }

          output += u8">\n";
          break;
        }
      }

      if (auto *containerNode = DynamicDowncast<ContainerNode>(node))
      {
        for (auto &child : ChildNodeRange(*containerNode))
        {
          SerializeNode(child, output, depth + 1uz);
        }
      }
    }

    KRYS_NODISCARD static DOMString SerializeDocument(Document &document) noexcept
    {
      DOMString output = u8"#document\n";

      for (auto &child : ChildNodeRange(document))
      {
        SerializeNode(child, output, 0uz);
      }

      NormaliseData(output);
      return output;
    }

#pragma endregion

    struct HTMLTreeBuilderTest
    {
      utf8_string Input;
      DOMString Expected;
      Maybe<DOMString> FragmentContext;
      Maybe<ParserScriptingMode> ScriptingMode;
    };

    KRYS_NODISCARD static List<HTMLTreeBuilderTest> ParseHTMLTreeBuilderTests(std::istream &stream) noexcept
    {
      List<HTMLTreeBuilderTest> tests;
      HTMLTreeBuilderTest currentTest;

      auto parse = [&](utf8_string sectionName, utf8_string data)
      {
        if (sectionName == u8"data")
        {
          if (!currentTest.Input.empty())
          {
            tests.push_back(::Krys::Move(currentTest));
            currentTest = {};
          }

          currentTest.Input = data;
        }
        else if (sectionName == u8"expected")
        {
          currentTest.Expected = data;
        }
        else if (sectionName == u8"document")
        {
          currentTest.Expected = u8"#document\n" + data;
          NormaliseData(currentTest.Expected);
        }
        else if (sectionName == u8"document-fragment")
        {
          currentTest.FragmentContext = data;
        }
        else if (sectionName == u8"script-on")
        {
          if (!currentTest.FragmentContext.has_value())
          {
            currentTest.ScriptingMode = ParserScriptingMode::Normal;
          }
        }
        else if (sectionName == u8"script-off")
        {
          currentTest.ScriptingMode = ParserScriptingMode::Disabled;
        }
        else if (sectionName == u8"errors" || sectionName == u8"new-errors" || sectionName == u8"errors-new")
        {
        }
      };

      ParseTestData(stream, "#", ::Krys::Move(parse));
      if (!currentTest.Input.empty())
      {
        tests.push_back(::Krys::Move(currentTest));
      }

      return tests;
    }

    static Ref<HTMLDocumentParser> CreateParser(HTMLDocument &document, utf8_string &&input) noexcept
    {
      auto parser = CreateRef<HTMLDocumentParser>(document);
      parser->InputStream().Append(Krys::Move(input), IsEOF(true));
      return parser;
    }

    static void ExecuteHTMLTreeBuilderTest(const HTMLTreeBuilderTest &test, size_t number,
                                           size_t total) noexcept
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

    static void RunTest(string filename) noexcept
    {
      static string basedir = "data/html-tree-builder/";

      auto file = OpenTestDataFile(basedir + filename);
      REQUIRE(file.has_value());

      auto tests = ParseHTMLTreeBuilderTests(*file);
      REQUIRE(!tests.empty());

      ExecuteTests(tests, ExecuteHTMLTreeBuilderTest);
    }
  }
}

#pragma region Test Parser Tests

namespace Krys::HTML::Tests
{
  namespace
  {
    Ref<Element> CreateElement(Document &document, const DOMString &tagName)
    {
      auto element = document.CreateElement(tagName);
      REQUIRE(element.HasValue());
      return *element;
    }

    Ref<SVGElement> CreateSVGElement(Document &document, const DOMString &tagName) noexcept
    {
      auto element = document.CreateElementNS(Namespaces::SVG, tagName);
      REQUIRE(element.HasValue());
      return *element;
    }

    Ref<DocumentType> CreateDocumentType(Document &document, const DOMString &name, const DOMString &publicId,
                                         const DOMString &systemId)
    {
      auto element = document.Implementation().CreateDocumentType(name, publicId, systemId);
      REQUIRE(element.HasValue());
      return *element;
    }

    List<HTMLTreeBuilderTest> Parse(const char *dat) noexcept
    {
      std::istringstream stream(dat);
      return ParseHTMLTreeBuilderTests(stream);
    }
  }

  TEST_CASE("DumpNode should serialize a complete HTML document correctly", "[HTML][TreeConstruction]")
  {
    Ref<Document> document = CreateRef<Document>();

    auto doctype = CreateDocumentType(*document, u8"html", u8"", u8"");
    REQUIRE_FALSE(document->AppendChild(*doctype).HasException());

    auto html = CreateElement(*document, u8"html");
    REQUIRE_FALSE(document->AppendChild(*html).HasException());

    auto head = CreateElement(*document, u8"head");
    REQUIRE_FALSE(html->AppendChild(*head).HasException());

    auto title = CreateElement(*document, u8"title");
    auto titleText = document->CreateTextNode(u8"Hello, World!");
    REQUIRE_FALSE(title->AppendChild(*titleText).HasException());
    REQUIRE_FALSE(head->AppendChild(*title).HasException());

    auto body = CreateElement(*document, u8"body");
    REQUIRE_FALSE(html->AppendChild(*body).HasException());

    auto div = CreateElement(*document, u8"div");
    REQUIRE_FALSE(body->AppendChild(*div).HasException());

    REQUIRE_FALSE(div->SetAttribute(u8"id", u8"main").HasException());
    REQUIRE_FALSE(div->SetAttribute(u8"class", u8"container").HasException());

    auto comment = document->CreateComment(u8" This is a comment ");
    REQUIRE_FALSE(div->AppendChild(*comment).HasException());

    auto divText = document->CreateTextNode(u8"Testing serializer");
    REQUIRE_FALSE(div->AppendChild(*divText).HasException());

    auto svg = CreateSVGElement(*document, u8"svg");
    REQUIRE_FALSE(body->AppendChild(*svg).HasException());

    DOMString output = SerializeDocument(*document);

    DOMString expectedOutput = u8R"(#document
| <!DOCTYPE html>
| <html>
|   <head>
|     <title>
|       "Hello, World!"
|   <body>
|     <div>
|       class="container"
|       id="main"
|       <!--  This is a comment  -->
|       "Testing serializer"
|     <svg svg>)";

    REQUIRE(output == expectedOutput);
  }

  TEST_CASE("ParseTreeConstructionTests - single test case with errors", "[HTML][TreeConstruction]")
  {
    auto tests = Parse("#data\n"
                       "<div></div>\n"
                       "#errors\n"
                       "(1,1): expected-doctype-but-got-start-tag\n"
                       "#document\n"
                       "| <html>\n"
                       "|   <head>\n"
                       "|   <body>\n"
                       "|     <div>");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == u8"<div></div>");
    CHECK(tests[0].Expected
          == u8"#document\n"
             u8"| <html>\n"
             u8"|   <head>\n"
             u8"|   <body>\n"
             u8"|     <div>");
  }

  TEST_CASE("ParseTreeConstructionTests - errors section may be empty", "[HTML][TreeConstruction]")
  {
    auto tests = Parse("#data\n"
                       "<!DOCTYPE html>\n"
                       "#errors\n"
                       "#document\n"
                       "| <!DOCTYPE html>\n"
                       "| <html>\n"
                       "|   <head>\n"
                       "|   <body>\n");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == u8"<!DOCTYPE html>");
    CHECK(tests[0].Expected
          == u8"#document\n"
             u8"| <!DOCTYPE html>\n"
             u8"| <html>\n"
             u8"|   <head>\n"
             u8"|   <body>");
  }

  TEST_CASE("ParseTreeConstructionTests - #new-errors section is skipped", "[HTML][TreeConstruction]")
  {
    auto tests = Parse("#data\n"
                       "<div foo=`bar`>\n"
                       "#errors\n"
                       "(1,10): equals-in-unquoted-attribute-value\n"
                       "#new-errors\n"
                       "(1:10) unexpected-character-in-unquoted-attribute-value\n"
                       "#document\n"
                       "| <html>\n"
                       "|   <head>\n"
                       "|   <body>\n"
                       "|     <div>\n"
                       "|       foo=\"`bar`\"\n");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == u8"<div foo=`bar`>");
    CHECK(tests[0].Expected
          == u8"#document\n"
             u8"| <html>\n"
             u8"|   <head>\n"
             u8"|   <body>\n"
             u8"|     <div>\n"
             u8"|       foo=\"`bar`\"");
  }

  TEST_CASE("ParseTreeConstructionTests - unknown # keyword is skipped", "[HTML][TreeConstruction]")
  {
    auto tests = Parse("#data\n"
                       "<p>hello</p>\n"
                       "#errors\n"
                       "#unknown-section\n"
                       "this line should be ignored\n"
                       "#document\n"
                       "| <html>\n"
                       "|   <head>\n"
                       "|   <body>\n"
                       "|     <p>\n"
                       "|       \"hello\"");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == u8"<p>hello</p>");
    CHECK(tests[0].Expected
          == u8"#document\n"
             u8"| <html>\n"
             u8"|   <head>\n"
             u8"|   <body>\n"
             u8"|     <p>\n"
             u8"|       \"hello\"");
  }

  TEST_CASE("ParseTreeConstructionTests - multiple test cases separated by blank lines",
            "[HTML][TreeConstruction]")
  {
    auto tests = Parse("#data\n"
                       "<p>one</p>\n"
                       "#errors\n"
                       "#document\n"
                       "| <html>\n"
                       "|   <head>\n"
                       "|   <body>\n"
                       "|     <p>\n"
                       "|       \"one\""
                       "\n"
                       "#data\n"
                       "<p>two</p>\n"
                       "#errors\n"
                       "#document\n"
                       "| <html>\n"
                       "|   <head>\n"
                       "|   <body>\n"
                       "|     <p>\n"
                       "|       \"two\"");

    REQUIRE(tests.size() == 2);

    CHECK(tests[0].Input == u8"<p>one</p>");
    CHECK(tests[0].Expected
          == u8"#document\n"
             u8"| <html>\n"
             u8"|   <head>\n"
             u8"|   <body>\n"
             u8"|     <p>\n"
             u8"|       \"one\"");

    CHECK(tests[1].Input == u8"<p>two</p>");
    CHECK(tests[1].Expected
          == u8"#document\n"
             u8"| <html>\n"
             u8"|   <head>\n"
             u8"|   <body>\n"
             u8"|     <p>\n"
             u8"|       \"two\"");
  }

  TEST_CASE("ParseTreeConstructionTests - last test case without trailing blank line",
            "[HTML][TreeConstruction]")
  {
    auto tests = Parse("#data\n"
                       "<span></span>\n"
                       "#errors\n"
                       "#document\n"
                       "| <html>\n"
                       "|   <head>\n"
                       "|   <body>\n"
                       "|     <span>");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == u8"<span></span>");
    CHECK(tests[0].Expected
          == u8"#document\n"
             u8"| <html>\n"
             u8"|   <head>\n"
             u8"|   <body>\n"
             u8"|     <span>");
  }

  TEST_CASE("ParseTreeConstructionTests - multi-line data is preserved", "[HTML][TreeConstruction]")
  {
    auto tests = Parse("#data\n"
                       "<div>\n"
                       "  text\n"
                       "</div>\n"
                       "#errors\n"
                       "#document\n"
                       "| <html>\n"
                       "|   <head>\n"
                       "|   <body>\n"
                       "|     <div>\n"
                       "|       \"\\n  text\\n\"\n");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == u8"<div>\n  text\n</div>");
  }

  TEST_CASE("ParseTreeConstructionTests - empty stream produces no tests", "[HTML][TreeConstruction]")
  {
    auto tests = Parse("");
    REQUIRE(tests.empty());
  }

  TEST_CASE("ParseTreeConstructionTests - #script-on/off sets ScriptingEnabled", "[HTML][TreeConstruction]")
  {
    SECTION("ScriptingEnabled is true when #script-on is present")
    {
      auto tests = Parse("#data\n"
                         "<div></div>\n"
                         "#script-on\n"
                         "#errors\n"
                         "(1,1): expected-doctype-but-got-start-tag\n"
                         "#document\n"
                         "| <html>\n"
                         "|   <head>\n"
                         "|   <body>\n"
                         "|     <div>\n");

      REQUIRE(tests.size() == 1);
      CHECK(tests[0].ScriptingMode.has_value());
      CHECK(tests[0].ScriptingMode.value() == ParserScriptingMode::Normal);
    }

    SECTION("ScriptingEnabled is false when #script-off is present")
    {
      auto tests = Parse("#data\n"
                         "<div></div>\n"
                         "#script-off\n"
                         "#errors\n"
                         "(1,1): expected-doctype-but-got-start-tag\n"
                         "#document\n"
                         "| <html>\n"
                         "|   <head>\n"
                         "|   <body>\n"
                         "|     <div>\n");

      REQUIRE(tests.size() == 1);
      CHECK(tests[0].ScriptingMode.has_value());
      CHECK(tests[0].ScriptingMode.value() == ParserScriptingMode::Disabled);
    }
  }

  TEST_CASE("ParseTreeConstructionTests - element attributes are captured in expected document",
            "[HTML][TreeConstruction]")
  {
    auto tests = Parse("#data\n"
                       "<div bar=\"ZZ&gt;YY\"></div>\n"
                       "#errors\n"
                       "(1,20): expected-doctype-but-got-start-tag\n"
                       "#document\n"
                       "| <html>\n"
                       "|   <head>\n"
                       "|   <body>\n"
                       "|     <div>\n"
                       "|       bar=\"ZZ>YY\"\n");

    REQUIRE(tests.size() == 1);

    CHECK(tests[0].Input == u8"<div bar=\"ZZ&gt;YY\"></div>");
    CHECK(tests[0].Expected
          == u8"#document\n"
             u8"| <html>\n"
             u8"|   <head>\n"
             u8"|   <body>\n"
             u8"|     <div>\n"
             u8"|       bar=\"ZZ>YY\"");
  }

  TEST_CASE("ParseTreeConstructionTests - #document-fragment sets fragment context",
            "[HTML][TreeConstruction]")
  {
    auto tests = Parse("#data\n"
                       "<li>Item</li>\n"
                       "#document-fragment\n"
                       "ul\n"
                       "#errors\n"
                       "(1,1): expected-doctype-but-got-start-tag\n"
                       "#document\n"
                       "| <html>\n"
                       "|   <head>\n"
                       "|   <body>\n"
                       "|     <ul>\n"
                       "|       <li>\n"
                       "|         \"Item\"\n");

    REQUIRE(tests.size() == 1);
    CHECK(tests[0].Input == u8"<li>Item</li>");
    CHECK(tests[0].FragmentContext.has_value());
    CHECK(tests[0].FragmentContext.value() == u8"ul");
  }
}

#pragma endregion

#define EXECUTE_HTML_PARSER_TEST_CASE(FileName)                                                              \
  TEST_CASE("HTMLDocumentParser(" FileName ")", "[HTML][HTMLDocumentParser]")                                \
  {                                                                                                          \
    ::Krys::HTML::Tests::RunTest(FileName);                                                                  \
  }

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
