#include "Krystal.Web/Dom/Nodes/CDATASection.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web/Dom/Algorithms/TreeQueries.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/Node.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"

namespace krys::web::dom::tests
{
  namespace
  {
    struct CDATASectionTestData
    {
      Ref<web::Document> Document;
      Ref<CDATASection> Node;

      CDATASectionTestData(DOMString &&value)
          : Document(CreateRef<web::Document>()), Node(CreateRef<CDATASection>(*Document, krys::move(value)))
      {
      }
    };
  }

  TEST_CASE("Create a CDATA section node: a Text node in the document with the data, that isn't exclusive",
            "[DOM][CDATASection][dom:interface-cdatasection][dom:create-a-cdata-section-node]")
  {
    CDATASectionTestData data(u8"Hello, world!");
    CHECK(data.Node->Data() == u8"Hello, world!");
    CHECK(&data.Node->NodeDocument() == data.Document.get());
    CHECK(data.Node->ParentNode() == nullptr);
    CHECK_FALSE(TreeQueries::IsExclusiveTextNode(*data.Node));
    CHECK(CreateRef<CDATASection>(*data.Document)->Data().empty());

    // It's still a Text node to the methods it inherits.
    CHECK(data.Node->WholeText() == u8"Hello, world!");
  }

  TEST_CASE("Node.nodeName: a CDATASection's is #cdata-section", "[DOM][CDATASection][dom:dom-node-nodename]")
  {
    CDATASectionTestData data(u8"Hello, world!");
    REQUIRE(data.Node->NodeName() == u8"#cdata-section");
  }

  TEST_CASE("Node.nodeType: a CDATASection is a CDATA_SECTION_NODE",
            "[DOM][CDATASection][dom:dom-node-nodetype][dom:dom-node-cdata_section_node]")
  {
    CDATASectionTestData data(u8"Hello, world!");
    REQUIRE(data.Node->NodeType() == NodeType::CDATA_SECTION_NODE);
  }
}
