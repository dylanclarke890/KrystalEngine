#pragma once

#include "Krystal.HTML/Constants/Namespaces.hpp"
#include "Krystal.HTML/DOM/ContainerNode.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/QualifiedName.hpp"

namespace Krys::Tests
{
  using namespace Krys::HTML;

  class TestContainerNode : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestContainerNode);

  public:
    TestContainerNode(Document &document) noexcept
        : Element(document, {Namespace::HTML, DOMStringAtom::Null(), u8"test-container-node"}, NodeFlags::None)
    {
    }
  };
}