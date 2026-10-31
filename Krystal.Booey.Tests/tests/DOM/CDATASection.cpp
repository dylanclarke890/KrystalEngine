#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct CommonTestData
    {
      Ref<dom::Document> Document;
      Ref<CDATASection> Node;

      CommonTestData(dom::DOMString &&value)
          : Document(CreateRef<dom::Document>()), Node(CreateRef<CDATASection>(*Document, krys::move(value)))
      {
      }
    };
  }

  TEST_CASE("Create a CDATA section node: a Text node in the document with the data, that isn't exclusive",
            "[DOM][CDATASection][dom:interface-cdatasection][dom:create-a-cdata-section-node]")
  {
    CommonTestData data(u8"Hello, world!");
    CHECK(data.Node->Data() == u8"Hello, world!");
    CHECK(&data.Node->NodeDocument() == data.Document.get());
    CHECK(data.Node->ParentNode() == nullptr);
    CHECK_FALSE(TreeQueries::IsExclusiveTextNode(*data.Node));
    CHECK(CreateRef<CDATASection>(*data.Document)->Data() == u8"");

    // It's still a Text node to the methods it inherits.
    CHECK(data.Node->WholeText() == u8"Hello, world!");
  }

  TEST_CASE("Node.nodeName: a CDATASection's is #cdata-section", "[DOM][CDATASection][dom:dom-node-nodename]")
  {
    CommonTestData data(u8"Hello, world!");
    REQUIRE(data.Node->NodeName() == u8"#cdata-section");
  }

  TEST_CASE("Node.nodeType: a CDATASection is a CDATA_SECTION_NODE",
            "[DOM][CDATASection][dom:dom-node-nodetype][dom:dom-node-cdata_section_node]")
  {
    CommonTestData data(u8"Hello, world!");
    REQUIRE(data.Node->NodeType() == NodeType::CDATA_SECTION_NODE);
  }
}
