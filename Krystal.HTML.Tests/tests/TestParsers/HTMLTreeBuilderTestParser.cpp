#include "Krystal.HTML.Tests/TestParsers/HTMLTreeBuilderTestParser.hpp"
#include "Krystal.HTML/SVG/Internals/SVGElementFactory.hpp"
#include "Krystal.HTML/SVG/SVGElement.hpp"
#include <catch_all.hpp>
#include <sstream>

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

    CHECK(tests[0].Input == U"<div></div>");
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

    CHECK(tests[0].Input == U"<!DOCTYPE html>");
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

    CHECK(tests[0].Input == U"<div foo=`bar`>");
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

    CHECK(tests[0].Input == U"<p>hello</p>");
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

    CHECK(tests[0].Input == U"<p>one</p>");
    CHECK(tests[0].Expected
          == u8"#document\n"
             u8"| <html>\n"
             u8"|   <head>\n"
             u8"|   <body>\n"
             u8"|     <p>\n"
             u8"|       \"one\"");

    CHECK(tests[1].Input == U"<p>two</p>");
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

    CHECK(tests[0].Input == U"<span></span>");
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

    CHECK(tests[0].Input == U"<div>\n  text\n</div>");
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

    CHECK(tests[0].Input == U"<div bar=\"ZZ&gt;YY\"></div>");
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
    CHECK(tests[0].Input == U"<li>Item</li>");
    CHECK(tests[0].FragmentContext.has_value());
    CHECK(tests[0].FragmentContext.value() == u8"ul");
  }
}
