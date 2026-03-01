#pragma once

#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML.Tests/TestNode.hpp"

namespace Krys::Tests
{
  using namespace Krys::HTML;

  class TestNode : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestNode);

  public:
    TestNode(Document &document) noexcept : Node(document, NodeType::ELEMENT_NODE, NodeFlag::None)
    {
    }

    KRYS_NODISCARD DOMString NodeName() const noexcept override
    {
      return u8"TESTCONTAINER";
    }
  };
}