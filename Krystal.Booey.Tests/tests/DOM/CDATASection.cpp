#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
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
