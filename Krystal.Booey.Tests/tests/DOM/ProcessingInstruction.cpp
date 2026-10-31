#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
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
      Ref<Document> Document;
      Ref<ProcessingInstruction> Node;

      CommonTestData(dom::DOMString &&target, dom::DOMString &&value)
          : Document(CreateRef<dom::Document>()),
            Node(*Document->CreateProcessingInstruction(krys::move(target), krys::move(value)))
      {
      }
    };
  }

  TEST_CASE("Node.nodeName: a ProcessingInstruction's is its target",
            "[DOM][ProcessingInstruction][dom:dom-node-nodename]")
  {
    CommonTestData data(u8"test-target", u8"Hello, world!");
    REQUIRE(data.Node->NodeName() == u8"test-target");
  }

  TEST_CASE("Node.nodeType: a ProcessingInstruction is a PROCESSING_INSTRUCTION_NODE",
            "[DOM][ProcessingInstruction][dom:dom-node-nodetype][dom:dom-node-processing_instruction_node]")
  {
    CommonTestData data(u8"test-target", u8"Hello, world!");
    REQUIRE(data.Node->NodeType() == NodeType::PROCESSING_INSTRUCTION_NODE);
  }
}
