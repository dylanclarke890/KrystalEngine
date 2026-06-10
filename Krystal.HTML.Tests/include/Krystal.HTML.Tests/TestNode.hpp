#pragma once

#include "Krystal.HTML.Tests/TestNode.hpp"
#include "Krystal.HTML/Infra/Namespaces.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/Types/QualifiedName.hpp"

namespace Krys::Tests
{
  using namespace Krys::HTML;

  class TestNode : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestNode);

  public:
    TestNode(Document &document) noexcept
        : Element(document, {Namespaces::HTML, DOMStringAtom::Null(), u8"test-element"}, NodeFlags::None)
    {
    }
  };
}