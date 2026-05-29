#include "Krystal.HTML/DOM/ProcessingInstruction.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<ProcessingInstruction> Node;

      CommonTestData(DOMString &&target, DOMString &&value)
          : Document(CreateRef<HTML::Document>()),
            Node(*Document->CreateProcessingInstruction(Krys::Move(target), Krys::Move(value)))
      {
      }
    };
  }

  TEST_CASE("ProcessingInstruction::NodeName", "[HTML][ProcessingInstruction]")
  {
    CommonTestData data(u8"test-target", u8"Hello, world!");
    REQUIRE(data.Node->NodeName() == u8"test-target");
  }

  TEST_CASE("ProcessingInstruction::NodeType", "[HTML][ProcessingInstruction]")
  {
    CommonTestData data(u8"test-target", u8"Hello, world!");
    REQUIRE(data.Node->NodeType() == NodeType::PROCESSING_INSTRUCTION_NODE);
  }
}
