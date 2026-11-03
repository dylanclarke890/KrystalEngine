#include "Krystal.Booey/DOM/Nodes/DOMImplementation.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentType.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/DOM/Nodes/XHTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/XMLDocument.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/SVG/SVGDocument.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE(
    "DOMImplementation.createDocumentType(): a doctype in the associated document, with a valid name",
    "[DOM][DOMImplementation][dom:interface-domimplementation][dom:dom-domimplementation-createdocumenttype]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto &implementation = document->Implementation();
    auto doctype =
      implementation.CreateDocumentType(u8"svg:svg", u8"-//W3C//DTD SVG 1.1//EN", u8"x.dtd").ReleaseValue();
    CHECK(doctype->Name() == u8"svg:svg");
    CHECK(doctype->PublicId() == u8"-//W3C//DTD SVG 1.1//EN");
    CHECK(doctype->SystemId() == u8"x.dtd");
    CHECK(&doctype->NodeDocument() == document.get());
    CHECK(doctype->ParentNode() == nullptr);

    for (auto *name : {u8"invalid name", u8"a>", u8"a\t"})
    {
      CHECK(implementation.CreateDocumentType(name, u8"", u8"") == ExceptionCode::InvalidCharacterError);
    }
    // Doctype names are more lenient than element names.
    CHECK_FALSE(implementation.CreateDocumentType(u8"1:a", u8"", u8"").HasException());
    CHECK_FALSE(implementation.CreateDocumentType(u8"", u8"", u8"").HasException());
  }

  TEST_CASE("DOMImplementation.createDocument(): an XML document with the doctype and document element given",
            "[DOM][DOMImplementation][dom:dom-domimplementation-createdocument]")
  {
    auto context = CreateRef<HTMLDocument>();
    auto &implementation = context->Implementation();

    SECTION("3. to 5. the doctype, then the document element")
    {
      auto doctype = implementation.CreateDocumentType(u8"root", u8"", u8"").ReleaseValue();
      auto document = implementation.CreateDocument(u8"urn:x", u8"p:root", doctype.get()).ReleaseValue();
      CHECK(document->FirstChild() == doctype.get());
      REQUIRE(document->DocumentElement() != nullptr);
      CHECK(document->DocumentElement()->NamespaceURI() == u8"urn:x");
      CHECK(document->DocumentElement()->Prefix() == u8"p");
      CHECK(document->DocumentElement()->LocalName() == u8"root");
      CHECK(&doctype->NodeDocument() == document.get());
      CHECK(document->ChildNodes()->Length() == 2u);
    }

    SECTION("3. no document element for the empty or null qualified name")
    {
      for (auto name : {DOMStringAtom::Empty(), DOMStringAtom::Null()})
      {
        auto document = implementation.CreateDocument(u8"urn:x", name).ReleaseValue();
        CHECK_FALSE(document->HasChildNodes());
      }
      CHECK_FALSE(implementation.CreateDocument(u8"urn:x").ReleaseValue()->HasChildNodes());
    }

    SECTION("3. the internal createElementNS steps' exceptions")
    {
      CHECK(implementation.CreateDocument(u8"urn:x", u8"1a").GetException().Code()
            == ExceptionCode::InvalidCharacterError);
      CHECK(implementation.CreateDocument(DOMStringAtom::Null(), u8"p:a").GetException().Code()
            == ExceptionCode::NamespaceError);
    }

    SECTION("1. and 7. the interface and content type follow the namespace")
    {
      auto xhtml = implementation.CreateDocument(infra::Namespaces::HTML, u8"html").ReleaseValue();
      CHECK(xhtml->ContentType() == u8"application/xhtml+xml");
      CHECK(Is<XHTMLDocument>(static_cast<Document &>(*xhtml)));
      CHECK(Is<html::HTMLElement>(*xhtml->DocumentElement()));

      auto svg = implementation.CreateDocument(infra::Namespaces::SVG, u8"svg").ReleaseValue();
      CHECK(svg->ContentType() == u8"image/svg+xml");
      CHECK(Is<svg::SVGDocument>(static_cast<Document &>(*svg)));

      auto xml = implementation.CreateDocument(u8"urn:x", u8"").ReleaseValue();
      CHECK(xml->ContentType() == u8"application/xml");
      CHECK_FALSE(Is<HTMLDocument>(static_cast<Document &>(*xml)));
    }
  }

  TEST_CASE("DOMImplementation.createHTMLDocument(): an HTML document with a doctype, head, body and title",
            "[DOM][DOMImplementation][dom:dom-domimplementation-createhtmldocument]")
  {
    auto context = CreateRef<HTMLDocument>();
    auto &implementation = context->Implementation();

    auto childNames = [](Node &parent)
    {
      DOMString names;
      for (auto *child = parent.FirstChild(); child; child = child->NextSibling())
      {
        names += (names.empty() ? u8"" : u8" ") + child->NodeName();
      }
      return names;
    };

    SECTION("with a title")
    {
      auto document = implementation.CreateHTMLDocument(u8"Hello").ReleaseValue();
      CHECK(Is<HTMLDocument>(*document));
      CHECK(document->ContentType() == u8"text/html");
      CHECK(childNames(*document) == u8"html HTML");
      CHECK(document->DocType()->Name() == u8"html");
      CHECK(document->DocType()->PublicId() == u8"");
      CHECK(document->DocType()->SystemId() == u8"");
      auto html = document->DocumentElement();
      CHECK(html->NamespaceURI() == infra::Namespaces::HTML);
      CHECK(childNames(*html) == u8"HEAD BODY");
      CHECK(childNames(*html->FirstChild()) == u8"TITLE");
      CHECK(html->FirstChild()->TextContent() == u8"Hello");
      CHECK(document->Title() == u8"Hello");
    }

    SECTION("with an empty title")
    {
      auto document = implementation.CreateHTMLDocument(DOMString()).ReleaseValue();
      auto *title = document->DocumentElement()->FirstChild()->FirstChild();
      REQUIRE(title != nullptr);
      CHECK(title->NodeName() == u8"TITLE");
      CHECK(childNames(*title) == u8"#text");
    }

    SECTION("without a title")
    {
      auto document = implementation.CreateHTMLDocument().ReleaseValue();
      CHECK_FALSE(document->DocumentElement()->FirstChild()->HasChildNodes());
    }
  }

  TEST_CASE("DOMImplementation.hasFeature(): always true",
            "[DOM][DOMImplementation][dom:dom-domimplementation-hasfeature]")
  {
    CHECK(CreateRef<Document>()->Implementation().HasFeature());
  }
}
