#include "Krystal.Web/Xml/XMLDocumentParser.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Web/Css/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Web/Css/CSSOM/StyleSheetList.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/CDATASection.hpp"
#include "Krystal.Web/Dom/Nodes/Comment.hpp"
#include "Krystal.Web/Dom/Nodes/DocumentFragment.hpp"
#include "Krystal.Web/Dom/Nodes/DocumentType.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/ProcessingInstruction.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Dom/Nodes/XMLDocument.hpp"
#include "Krystal.Web/Editing/Markup/Markup.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Infra/Namespaces.hpp"
#include "Krystal.Web/Svg/Elements/SVGCircleElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGSvgElement.hpp"
#include "Krystal.Web/Svg/SVGDocument.hpp"

namespace krys::web::tests
{
  namespace
  {
    /// @brief Loads @p markup of type @p mimeType from the strategy, and runs the loads it starts.
    void LoadDocument(LoadFixture &fixture, utf8_stringview markup, utf8_stringview mimeType,
                      utf8_stringview url = u8"https://example.com/document") noexcept
    {
      fixture.Strategy.AddText(url, markup, mimeType);
      fixture.LoadUrl(url);
      html::EventLoop::Current().RunUntilIdle();
    }

    template <typename T>
    KRYS_NODISCARD bool IsSVG(const Element &element) noexcept
    {
      auto *svgElement = DynamicDowncast<SVGElement>(element);
      return svgElement && Is<T>(*svgElement);
    }

    KRYS_NODISCARD std::string Serialize(const Node &node) noexcept
    {
      return Narrow(SerializeFragment(node, SerializedNodes::SubtreeIncludingNode));
    }
  }

  TEST_CASE("An SVG file loads as an SVG document", "[XML][XMLDocumentParser]")
  {
    LoadFixture fixture;
    LoadDocument(fixture,
                 u8"<?xml version=\"1.0\"?>\n"
                 u8"<svg xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" "
                 u8"width=\"20\" height=\"10\"><!-- a note --><circle id=\"c\" r=\"5\" xlink:title=\"dot\"/>"
                 u8"<desc>A &amp; B<![CDATA[ <raw> ]]></desc></svg>",
                 u8"image/svg+xml");

    auto &document = fixture.Document();
    REQUIRE(Is<SVGDocument>(document));
    CHECK(document.ContentType() == u8"image/svg+xml");
    CHECK(document.ReadyState() == html::DocumentReadyState::Complete);

    auto root = document.DocumentElement();
    REQUIRE(root);
    CHECK(IsSVG<SVGSvgElement>(*root));
    CHECK(root->NamespaceURI() == infra::namespaces::SVG);
    CHECK(Is<web::Comment>(root->FirstChild()));

    auto circle = document.GetElementById(u8"c");
    REQUIRE(circle);
    CHECK(IsSVG<SVGCircleElement>(*circle));
    CHECK(circle->GetAttributeNS(infra::namespaces::XLink, u8"title") == u8"dot");

    auto *desc = circle->NextSibling();
    REQUIRE(desc);
    REQUIRE(Is<Text>(desc->FirstChild()));
    CHECK(Downcast<Text>(*desc->FirstChild()).Data() == u8"A & B");
    REQUIRE(Is<CDATASection>(desc->LastChild()));
    CHECK(Downcast<CDATASection>(*desc->LastChild()).Data() == u8" <raw> ");
  }

  TEST_CASE("An XML document keeps its declaration and doctype",
            "[XML][XMLDocumentParser][html:parsing-xhtml-documents]")
  {
    LoadFixture fixture;
    LoadDocument(fixture,
                 u8"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                 u8"<!DOCTYPE note SYSTEM \"note.dtd\"><note a=\"1\"><?app go?><to>Té</to></note>",
                 u8"application/xml");

    auto &document = fixture.Document();
    REQUIRE(Is<XMLDocument>(document));
    CHECK_FALSE(Is<SVGDocument>(document));
    CHECK(document.ContentType() == u8"application/xml");
    CHECK(document.HasXMLDeclaration());
    CHECK(document.XMLVersion() == u8"1.0");
    CHECK(document.XMLEncoding() == u8"UTF-8");
    CHECK(document.XMLStandalone());

    auto *doctype = document.FirstChild();
    REQUIRE(Is<DocumentType>(doctype));
    CHECK(Downcast<DocumentType>(*doctype).Name() == u8"note");
    CHECK(Downcast<DocumentType>(*doctype).SystemId() == u8"note.dtd");

    auto root = document.DocumentElement();
    REQUIRE(root);
    CHECK(root->NamespaceURI() == null);
    CHECK(root->LocalName() == u8"note");
    REQUIRE(Is<web::ProcessingInstruction>(root->FirstChild()));
    CHECK(Downcast<web::ProcessingInstruction>(*root->FirstChild()).Target() == u8"app");

    CHECK(
      Serialize(document)
      == "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><!DOCTYPE note SYSTEM \"note.dtd\">"
         "<note a=\"1\"><?app go?><to>T\xc3\xa9</to></note>");
  }

  TEST_CASE("An XML document is UTF-8 whatever its declaration says", "[XML][XMLDocumentParser]")
  {
    for (auto encoding : {u8"ISO-8859-1", u8"windows-1252", u8"UTF-16", u8"Shift_JIS"})
    {
      CAPTURE(reinterpret_cast<const char *>(encoding));
      LoadFixture fixture;
      LoadDocument(fixture,
                   u8"<?xml version=\"1.0\" encoding=\"" + utf8_string(encoding) + u8"\"?><note>Té 日</note>",
                   u8"application/xml");

      auto &document = fixture.Document();
      // The declaration is kept as written.
      CHECK(document.XMLEncoding() == encoding);
      auto root = document.DocumentElement();
      REQUIRE(root);
      CHECK(root->LocalName() == u8"note");
      CHECK(root->TextContent() == u8"Té 日");
    }
  }

  TEST_CASE("A malformed XML document shows its errors", "[XML][XMLDocumentParser]")
  {
    LoadFixture fixture;
    LoadDocument(fixture, u8"<root><open></root>", u8"text/xml");

    auto &document = fixture.Document();
    CHECK(document.ReadyState() == html::DocumentReadyState::Complete);
    auto root = document.DocumentElement();
    REQUIRE(root);
    CHECK(root->LocalName() == u8"root");
    auto *error = root->FirstChild();
    REQUIRE(Is<Element>(error));
    CHECK(Downcast<Element>(*error).LocalName() == u8"parsererror");
    CHECK(Downcast<Element>(*error).NamespaceURI() == infra::namespaces::HTML);
    CHECK(error->TextContent().value_or(u8"").contains(u8"error on line 1"));
  }

  TEST_CASE("A malformed SVG document is shown inside an HTML document", "[XML][XMLDocumentParser]")
  {
    LoadFixture fixture;
    LoadDocument(fixture, u8"<svg xmlns=\"http://www.w3.org/2000/svg\"><rect></svg>", u8"image/svg+xml");

    auto root = fixture.Document().DocumentElement();
    REQUIRE(root);
    CHECK(root->LocalName() == u8"html");
    auto body = root->LastChild();
    REQUIRE(Is<Element>(body));
    CHECK(Downcast<Element>(*body).LocalName() == u8"body");
    REQUIRE(Is<Element>(body->FirstChild()));
    CHECK(Downcast<Element>(*body->FirstChild()).LocalName() == u8"parsererror");
    REQUIRE(Is<Element>(body->LastChild()));
    CHECK(IsSVG<SVGSvgElement>(Downcast<Element>(*body->LastChild())));
  }

  TEST_CASE("An XHTML document has HTML elements and HTML's named entities",
            "[XML][XMLDocumentParser][html:parsing-xhtml-documents]")
  {
    LoadFixture fixture;
    LoadDocument(
      fixture,
      u8"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" "
      u8"\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"
      u8"<html xmlns=\"http://www.w3.org/1999/xhtml\"><body><p id=\"p\">a&nbsp;b&amp;c</p></body></html>",
      u8"application/xhtml+xml");

    auto &document = fixture.Document();
    REQUIRE(document.ContentType() == u8"application/xhtml+xml");
    auto paragraph = document.GetElementById(u8"p");
    REQUIRE(paragraph);
    CHECK(Is<HTMLElement>(*paragraph));
    CHECK(paragraph->TextContent() == u8"a b&c");
  }

  TEST_CASE("An undefined entity in an XML document is an error",
            "[XML][XMLDocumentParser][html:parsing-xhtml-documents]")
  {
    LoadFixture fixture;
    LoadDocument(fixture, u8"<root>a&nbsp;b</root>", u8"application/xml");

    auto root = fixture.Document().DocumentElement();
    REQUIRE(root);
    REQUIRE(Is<Element>(root->FirstChild()));
    CHECK(Downcast<Element>(*root->FirstChild()).LocalName() == u8"parsererror");
  }

  TEST_CASE("xml-stylesheet processing instructions load their sheets", "[XML][XMLDocumentParser]")
  {
    LoadFixture fixture;
    fixture.Strategy.AddText(u8"https://example.com/style.css", u8"rect { fill: green }");
    LoadDocument(fixture,
                 u8"<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>"
                 u8"<?xml-stylesheet href=\"alternate.css\" alternate=\"yes\"?>"
                 u8"<svg xmlns=\"http://www.w3.org/2000/svg\"><rect/></svg>",
                 u8"image/svg+xml");

    auto &document = fixture.Document();
    REQUIRE(Is<web::ProcessingInstruction>(document.FirstChild()));
    Ref<web::ProcessingInstruction> protectedInstruction =
      ShareRef(Downcast<web::ProcessingInstruction>(*document.FirstChild()));
    auto &instruction = *protectedInstruction;
    CHECK(instruction.IsCSS());
    REQUIRE(instruction.Sheet());
    CHECK(instruction.Sheet()->Length() == 1);

    // The alternate sheet without a title loads nothing.
    REQUIRE(Is<web::ProcessingInstruction>(document.FirstChild()->NextSibling()));
    CHECK_FALSE(Downcast<web::ProcessingInstruction>(*document.FirstChild()->NextSibling()).Sheet());

    REQUIRE(document.StyleSheets().Length() == 1);
    CHECK(document.StyleSheets().Item(0) == instruction.Sheet());

    // Removing the instruction removes its sheet.
    instruction.Remove();
    CHECK_FALSE(instruction.Sheet());
    CHECK(document.StyleSheets().Length() == 0);
  }

  TEST_CASE("xml-stylesheet processing instructions: where they load from, as in Chrome",
            "[XML][XMLDocumentParser][cssom:requirements-on-user-agents-implementing-the-xml-stylesheet-"
            "processing-instruction]")
  {
    LoadFixture fixture;
    fixture.Strategy.AddText(u8"https://example.com/style.css", u8"rect { fill: green }");
    auto instructionSheet = [&](const Node *node)
    {
      auto *instruction = DynamicDowncast<web::ProcessingInstruction>(node);
      return instruction ? instruction->Sheet() : nullptr;
    };

    SECTION("after the root element, outside the prolog, it still loads its sheet")
    {
      LoadDocument(fixture,
                   u8"<svg xmlns=\"http://www.w3.org/2000/svg\"><rect/></svg>"
                   u8"<?xml-stylesheet href=\"style.css\" type=\"text/css\"?>",
                   u8"image/svg+xml");
      CHECK(instructionSheet(fixture.Document().LastChild()));
      CHECK(fixture.Document().StyleSheets().Length() == 1);
    }

    SECTION("without an href, it loads the document itself as its sheet")
    {
      LoadDocument(
        fixture,
        u8"<?xml-stylesheet type=\"text/css\"?><svg xmlns=\"http://www.w3.org/2000/svg\"><rect/></svg>",
        u8"image/svg+xml");
      CHECK(instructionSheet(fixture.Document().FirstChild()));
      CHECK(fixture.Document().StyleSheets().Length() == 1);
    }

    SECTION("with a fragment href, it loads nothing")
    {
      LoadDocument(fixture,
                   u8"<?xml-stylesheet href=\"#sheet\" type=\"text/css\"?>"
                   u8"<svg xmlns=\"http://www.w3.org/2000/svg\"><rect/></svg>",
                   u8"image/svg+xml");
      CHECK_FALSE(instructionSheet(fixture.Document().FirstChild()));
      CHECK(fixture.Document().StyleSheets().Length() == 0);
    }
  }

  TEST_CASE("Markup inserted into an XML document is parsed as XML", "[XML][XMLDocumentParser]")
  {
    LoadFixture fixture;
    LoadDocument(fixture, u8"<root xmlns:a=\"urn:a\"><child/></root>", u8"application/xml");

    auto root = fixture.Document().DocumentElement();
    REQUIRE(root);

    SECTION("well formed")
    {
      auto result = CreateContextualFragment(*root, u8"<a:item>x</a:item><plain/>");
      REQUIRE_FALSE(result.HasException());
      auto fragment = result.ReleaseValue();
      REQUIRE(Is<Element>(fragment->FirstChild()));
      auto &item = Downcast<Element>(*fragment->FirstChild());
      CHECK(item.NamespaceURI() == u8"urn:a");
      CHECK(item.LocalName() == u8"item");
      REQUIRE(Is<Element>(fragment->LastChild()));
      CHECK(Downcast<Element>(*fragment->LastChild()).NamespaceURI() == null);
    }

    SECTION("malformed")
    {
      auto result = CreateContextualFragment(*root, u8"<open>");
      REQUIRE(result.HasException());
      CHECK(result.ReleaseException().Code() == ExceptionCode::SyntaxError);
    }
  }

  TEST_CASE("An SVG document shows the view its fragment names", "[XML][XMLDocumentParser]")
  {
    LoadFixture fixture;
    fixture.Strategy.AddText(u8"https://example.com/view.svg",
                             u8"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 50\">"
                             u8"<view id=\"corner\" viewBox=\"0 0 10 5\"/></svg>",
                             u8"image/svg+xml");
    fixture.LoadUrl(u8"https://example.com/view.svg#corner");
    html::EventLoop::Current().RunUntilIdle();

    auto root = fixture.Document().DocumentElement();
    REQUIRE(root);
    REQUIRE(IsSVG<SVGSvgElement>(*root));
    auto &svg = Downcast<SVGSvgElement>(Downcast<SVGElement>(*root));
    CHECK(svg.CurrentViewBoxRect() == RectF(0, 0, 10, 5));
  }
}
