#include "Krystal.Booey/DOM/Nodes/DocumentType.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct CommonTestData
    {
      Ref<dom::Document> Document;
      Ref<DocumentType> DocType;

      CommonTestData(const dom::DOMString &name, const dom::DOMString &publicId,
                     const dom::DOMString &systemId)
          : Document(CreateRef<dom::Document>()),
            DocType(*Document->Implementation().CreateDocumentType(name, publicId, systemId))
      {
      }
    };
  }

  TEST_CASE("Create a doctype: a doctype in the document, with the name and IDs, which default to empty",
            "[DOM][DocumentType][dom:interface-documenttype][dom:create-a-doctype][dom:concept-doctype]"
            "[dom:concept-doctype-name][dom:concept-doctype-publicid][dom:concept-doctype-systemid]")
  {
    CommonTestData data(u8"html", u8"-//W3C//DTD HTML 4.01//EN", u8"http://www.w3.org/TR/html4/strict.dtd");
    CHECK(&data.DocType->NodeDocument() == data.Document.get());
    CHECK(data.DocType->ParentNode() == nullptr);

    // The parser's doctype, without IDs.
    auto context = CreateRef<HTMLDocument>();
    auto parsed = Document::ParseHTMLUnsafe(*context, u8"<!DOCTYPE html><p>").ReleaseValue();
    auto doctype = parsed->DocType();
    REQUIRE(doctype != nullptr);
    CHECK(doctype->Name() == u8"html");
    CHECK(doctype->PublicId() == u8"");
    CHECK(doctype->SystemId() == u8"");
    CHECK(&doctype->NodeDocument() == parsed.get());
  }

  TEST_CASE("Node.nodeType: a DocumentType is a DOCUMENT_TYPE_NODE",
            "[DOM][DocumentType][dom:dom-node-nodetype][dom:dom-node-document_type_node]")
  {
    CommonTestData data(u8"html", u8"-//W3C//DTD HTML 4.01//EN", u8"http://www.w3.org/TR/html4/strict.dtd");
    REQUIRE(data.DocType->NodeType() == NodeType::DOCUMENT_TYPE_NODE);
  }

  TEST_CASE("DocumentType.name: the name", "[DOM][DocumentType][dom:dom-documenttype-name]")
  {
    CommonTestData data(u8"html", u8"-//W3C//DTD HTML 4.01//EN", u8"http://www.w3.org/TR/html4/strict.dtd");
    REQUIRE(data.DocType->Name() == u8"html");
  }

  TEST_CASE("DocumentType.publicId: the public ID", "[DOM][DocumentType][dom:dom-documenttype-publicid]")
  {
    CommonTestData data(u8"html", u8"-//W3C//DTD HTML 4.01//EN", u8"http://www.w3.org/TR/html4/strict.dtd");
    REQUIRE(data.DocType->PublicId() == u8"-//W3C//DTD HTML 4.01//EN");
  }

  TEST_CASE("DocumentType.systemId: the system ID", "[DOM][DocumentType][dom:dom-documenttype-systemid]")
  {
    CommonTestData data(u8"html", u8"-//W3C//DTD HTML 4.01//EN", u8"http://www.w3.org/TR/html4/strict.dtd");
    REQUIRE(data.DocType->SystemId() == u8"http://www.w3.org/TR/html4/strict.dtd");
  }

  TEST_CASE("Node.nodeName: a DocumentType's is its name", "[DOM][DocumentType][dom:dom-node-nodename]")
  {
    CommonTestData data(u8"html", u8"-//W3C//DTD HTML 4.01//EN", u8"http://www.w3.org/TR/html4/strict.dtd");
    REQUIRE(data.DocType->NodeName() == u8"html");
  }
}