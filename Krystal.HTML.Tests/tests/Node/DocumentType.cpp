#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  struct CommonTestData
  {
    Ref<Document> Document;
    Ref<DocumentType> DocType;

    CommonTestData(const DOMString &name, const DOMString &publicId, const DOMString &systemId)
        : Document(CreateRef<HTML::Document>()),
          DocType(CreateRef<DocumentType>(*Document, name, publicId, systemId))
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