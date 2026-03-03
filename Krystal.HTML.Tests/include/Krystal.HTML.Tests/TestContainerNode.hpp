#pragma once

#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::Tests
{
  using namespace Krys::HTML;

  class TestContainerNode : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestContainerNode);

  public:
    TestContainerNode(Document &document) noexcept : Element(document, NodeFlag::None)
    {
    }
  };
}