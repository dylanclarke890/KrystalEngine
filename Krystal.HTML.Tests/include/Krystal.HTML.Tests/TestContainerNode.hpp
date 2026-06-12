#pragma once

#include "Krystal.HTML/Infra/Namespaces.hpp"
#include "Krystal.HTML/DOM/ContainerNode.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/QualifiedName.hpp"

namespace Krys::Tests
{
  using namespace Krys::HTML;

  class TestContainerNode : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestContainerNode);

  public:
    TestContainerNode(Document &document) noexcept
        : Element(document, {Namespaces::HTML, DOMStringAtom::Null(), u8"test-container-node"}, NodeFlags::None)
    {
    }
  };
}