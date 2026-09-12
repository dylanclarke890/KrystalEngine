#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include <catch_all.hpp>

namespace krys::tests
{
  using namespace krys::boo;

  struct CommonTestData
  {
    Ref<Document> Document;
    Ref<DocumentType> DocType;

    CommonTestData(const dom::DOMString &name, const dom::DOMString &publicId, const dom::DOMString &systemId)
        : Document(CreateRef<HTML::Document>()),
          DocType(*Document->Implementation().CreateDocumentType(name, publicId, systemId))
    {
    }
  };

  TEST_CASE("DocumentType::NodeType", "[HTML][DocumentType]")
  {
    CommonTestData data(u8"html", u8"-//W3C//DTD HTML 4.01//EN", u8"http://www.w3.org/TR/html4/strict.dtd");
    REQUIRE(data.DocType->NodeType() == NodeType::DOCUMENT_TYPE_NODE);
  }

  TEST_CASE("DocumentType::Name", "[HTML][DocumentType]")
  {
    CommonTestData data(u8"html", u8"-//W3C//DTD HTML 4.01//EN", u8"http://www.w3.org/TR/html4/strict.dtd");
    REQUIRE(data.DocType->Name() == u8"html");
  }

  TEST_CASE("DocumentType::PublicId", "[HTML][DocumentType]")
  {
    CommonTestData data(u8"html", u8"-//W3C//DTD HTML 4.01//EN", u8"http://www.w3.org/TR/html4/strict.dtd");
    REQUIRE(data.DocType->PublicId() == u8"-//W3C//DTD HTML 4.01//EN");
  }

  TEST_CASE("DocumentType::SystemId", "[HTML][DocumentType]")
  {
    CommonTestData data(u8"html", u8"-//W3C//DTD HTML 4.01//EN", u8"http://www.w3.org/TR/html4/strict.dtd");
    REQUIRE(data.DocType->SystemId() == u8"http://www.w3.org/TR/html4/strict.dtd");
  }

  TEST_CASE("DocumentType::NodeName", "[HTML][DocumentType]")
  {
    CommonTestData data(u8"html", u8"-//W3C//DTD HTML 4.01//EN", u8"http://www.w3.org/TR/html4/strict.dtd");
    REQUIRE(data.DocType->NodeName() == u8"html");
  }
}