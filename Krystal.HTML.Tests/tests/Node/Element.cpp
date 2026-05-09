#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML.Tests/TestElement.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ProcessingInstruction.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<TestElement> Node;

      CommonTestData() : Document(CreateRef<HTML::Document>()), Node(CreateRef<TestElement>(*Document))
      {
      }
    };
  }

  TEST_CASE("Element::NodeName", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE(data.Node->NodeName() == u8"");
  }

  TEST_CASE("Element::NodeType", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE(data.Node->NodeType() == NodeType::ELEMENT_NODE);
  }
}