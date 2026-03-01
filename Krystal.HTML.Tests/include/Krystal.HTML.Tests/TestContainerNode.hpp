#pragma once

#include "Krystal.HTML/Node/ContainerNode.hpp"

namespace Krys::Tests
{
  using namespace Krys::HTML;

  class TestContainerNode : public ContainerNode
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestContainerNode);

  public:
    TestContainerNode(Document &document) noexcept
        : ContainerNode(document, NodeType::ELEMENT_NODE, NodeFlag::None)
    {
    }

    KRYS_NODISCARD DOMString NodeName() const noexcept override
    {
      return u8"TESTCONTAINER";
    }
  };
}