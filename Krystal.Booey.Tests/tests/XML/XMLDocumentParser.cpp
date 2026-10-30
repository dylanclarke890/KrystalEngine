#include "Krystal.Booey/XML/Parser/XMLDocumentParser.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/LoadTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/StyleSheetList.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/XHTMLDocument.hpp"
#include "Krystal.Booey/DOM/XMLDocument.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/SVG/SVGCircleElement.hpp"
#include "Krystal.Booey/SVG/SVGDocument.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Loads @p markup of type @p mimeType from the strategy, and runs the loads it starts.
    void LoadDocument(LoadFixture &fixture, utf8_stringview markup, utf8_stringview mimeType,
                      utf8_stringview url = u8"https://example.com/document") noexcept
    {
      fixture.Strategy.AddText(url, markup, mimeType);
      fixture.LoadURL(url);
      html::EventLoop::Current().RunUntilIdle();
    }

    template <typename T>
    KRYS_NODISCARD bool IsSVG(const dom::Element &element) noexcept
    {
      auto *svgElement = DynamicDowncast<svg::SVGElement>(element);
      return svgElement && Is<T>(*svgElement);
    }

    KRYS_NODISCARD std::string Serialize(const dom::Node &node) noexcept
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
    REQUIRE(Is<svg::SVGDocument>(document));
    CHECK(document.ContentType() == u8"image/svg+xml");
    CHECK(document.ReadyState() == html::DocumentReadyState::Complete);

    auto root = document.DocumentElement();
    REQUIRE(root);
    CHECK(IsSVG<svg::SVGSvgElement>(*root));
    CHECK(root->NamespaceURI() == infra::Namespaces::SVG);
    CHECK(Is<dom::Comment>(root->FirstChild()));

    auto circle = document.GetElementById(u8"c");
    REQUIRE(circle);
    CHECK(IsSVG<svg::SVGCircleElement>(*circle));
    CHECK(circle->GetAttributeNS(dom::DOMStringAtom(infra::Namespaces::XLink), dom::DOMStringAtom(u8"title"))
          == u8"dot");

    auto *desc = circle->NextSibling();
    REQUIRE(desc);
    REQUIRE(Is<dom::Text>(desc->FirstChild()));
    CHECK(Downcast<dom::Text>(*desc->FirstChild()).Data() == u8"A & B");
    REQUIRE(Is<dom::CDATASection>(desc->LastChild()));
    CHECK(Downcast<dom::CDATASection>(*desc->LastChild()).Data() == u8" <raw> ");
  }

  TEST_CASE("An XML document keeps its declaration and doctype", "[XML][XMLDocumentParser]")
  {
    LoadFixture fixture;
    LoadDocument(fixture,
                 u8"<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
                 u8"<!DOCTYPE note SYSTEM \"note.dtd\"><note a=\"1\"><?app go?><to>Té</to></note>",
                 u8"application/xml");

    auto &document = fixture.Document();
    REQUIRE(Is<dom::XMLDocument>(document));
    CHECK_FALSE(Is<svg::SVGDocument>(document));
    CHECK(document.ContentType() == u8"application/xml");
    CHECK(document.HasXMLDeclaration());
    CHECK(document.XMLVersion() == u8"1.0");
    CHECK(document.XMLEncoding() == u8"UTF-8");
    CHECK(document.XMLStandalone());

    auto *doctype = document.FirstChild();
    REQUIRE(Is<dom::DocumentType>(doctype));
    CHECK(Downcast<dom::DocumentType>(*doctype).Name() == u8"note");
    CHECK(Downcast<dom::DocumentType>(*doctype).SystemId() == u8"note.dtd");

    auto root = document.DocumentElement();
    REQUIRE(root);
    CHECK(root->NamespaceURI() == dom::DOMStringAtom::Null());
    CHECK(root->LocalName() == u8"note");
    REQUIRE(Is<dom::ProcessingInstruction>(root->FirstChild()));
    CHECK(Downcast<dom::ProcessingInstruction>(*root->FirstChild()).Target() == u8"app");

    CHECK(
      Serialize(document)
      == "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><!DOCTYPE note SYSTEM \"note.dtd\">"
         "<note a=\"1\"><?app go?><to>T\xc3\xa9</to></note>");
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
    REQUIRE(Is<dom::Element>(error));
    CHECK(Downcast<dom::Element>(*error).LocalName() == u8"parsererror");
    CHECK(Downcast<dom::Element>(*error).NamespaceURI() == infra::Namespaces::HTML);
    CHECK(error->TextContent().value_or(u8"").find(u8"error on line 1") != utf8_string::npos);
  }

  TEST_CASE("A malformed SVG document is shown inside an HTML document", "[XML][XMLDocumentParser]")
  {
    LoadFixture fixture;
    LoadDocument(fixture, u8"<svg xmlns=\"http://www.w3.org/2000/svg\"><rect></svg>", u8"image/svg+xml");

    auto root = fixture.Document().DocumentElement();
    REQUIRE(root);
    CHECK(root->LocalName() == u8"html");
    auto body = root->LastChild();
    REQUIRE(Is<dom::Element>(body));
    CHECK(Downcast<dom::Element>(*body).LocalName() == u8"body");
    REQUIRE(Is<dom::Element>(body->FirstChild()));
    CHECK(Downcast<dom::Element>(*body->FirstChild()).LocalName() == u8"parsererror");
    REQUIRE(Is<dom::Element>(body->LastChild()));
    CHECK(IsSVG<svg::SVGSvgElement>(Downcast<dom::Element>(*body->LastChild())));
  }

  TEST_CASE("An XHTML document has HTML elements and HTML's named entities", "[XML][XMLDocumentParser]")
  {
    LoadFixture fixture;
    LoadDocument(
      fixture,
      u8"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" "
      u8"\"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"
      u8"<html xmlns=\"http://www.w3.org/1999/xhtml\"><body><p id=\"p\">a&nbsp;b&amp;c</p></body></html>",
      u8"application/xhtml+xml");

    auto &document = fixture.Document();
    REQUIRE(Is<dom::XHTMLDocument>(document));
    auto paragraph = document.GetElementById(u8"p");
    REQUIRE(paragraph);
    CHECK(Is<html::HTMLElement>(*paragraph));
    CHECK(paragraph->TextContent() == u8"a b&c");
  }

  TEST_CASE("An undefined entity in an XML document is an error", "[XML][XMLDocumentParser]")
  {
    LoadFixture fixture;
    LoadDocument(fixture, u8"<root>a&nbsp;b</root>", u8"application/xml");

    auto root = fixture.Document().DocumentElement();
    REQUIRE(root);
    REQUIRE(Is<dom::Element>(root->FirstChild()));
    CHECK(Downcast<dom::Element>(*root->FirstChild()).LocalName() == u8"parsererror");
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
    REQUIRE(Is<dom::ProcessingInstruction>(document.FirstChild()));
    Ref<dom::ProcessingInstruction> protectedInstruction =
      ShareRef(Downcast<dom::ProcessingInstruction>(*document.FirstChild()));
    auto &instruction = *protectedInstruction;
    CHECK(instruction.IsCSS());
    REQUIRE(instruction.Sheet());
    CHECK(instruction.Sheet()->Length() == 1);

    // The alternate sheet without a title loads nothing.
    REQUIRE(Is<dom::ProcessingInstruction>(document.FirstChild()->NextSibling()));
    CHECK_FALSE(Downcast<dom::ProcessingInstruction>(*document.FirstChild()->NextSibling()).Sheet());

    REQUIRE(document.StyleSheets().Length() == 1);
    CHECK(document.StyleSheets().Item(0) == instruction.Sheet());

    // Removing the instruction removes its sheet.
    instruction.Remove();
    CHECK_FALSE(instruction.Sheet());
    CHECK(document.StyleSheets().Length() == 0);
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
      REQUIRE(Is<dom::Element>(fragment->FirstChild()));
      auto &item = Downcast<dom::Element>(*fragment->FirstChild());
      CHECK(item.NamespaceURI() == u8"urn:a");
      CHECK(item.LocalName() == u8"item");
      REQUIRE(Is<dom::Element>(fragment->LastChild()));
      CHECK(Downcast<dom::Element>(*fragment->LastChild()).NamespaceURI() == dom::DOMStringAtom::Null());
    }

    SECTION("malformed")
    {
      auto result = CreateContextualFragment(*root, u8"<open>");
      REQUIRE(result.HasException());
      CHECK(result.ReleaseException().Code() == dom::ExceptionCode::SyntaxError);
    }
  }

  TEST_CASE("An SVG document shows the view its fragment names", "[XML][XMLDocumentParser]")
  {
    LoadFixture fixture;
    fixture.Strategy.AddText(u8"https://example.com/view.svg",
                             u8"<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 100 50\">"
                             u8"<view id=\"corner\" viewBox=\"0 0 10 5\"/></svg>",
                             u8"image/svg+xml");
    fixture.LoadURL(u8"https://example.com/view.svg#corner");
    html::EventLoop::Current().RunUntilIdle();

    auto root = fixture.Document().DocumentElement();
    REQUIRE(root);
    REQUIRE(IsSVG<svg::SVGSvgElement>(*root));
    auto &svg = Downcast<svg::SVGSvgElement>(Downcast<svg::SVGElement>(*root));
    CHECK(svg.CurrentViewBoxRect() == FloatRect(0, 0, 10, 5));
  }

  TEST_CASE("Processing instructions' pseudo-attributes are parsed as XML attributes",
            "[XML][XMLDocumentParser]")
  {
    auto attributes = xml::ParseAttributes(u8"href=\"a.css\" type='text/css' x:y=\"z\"");
    REQUIRE(attributes);
    CHECK(attributes->at(u8"href") == u8"a.css");
    CHECK(attributes->at(u8"type") == u8"text/css");
    CHECK(attributes->size() == 3);

    CHECK_FALSE(xml::ParseAttributes(u8"href=unquoted"));
  }
}
