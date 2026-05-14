#pragma once

#include "Krystal.HTML.Tests/TestNode.hpp"
#include "Krystal.HTML/Namespaces.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/QualifiedName.hpp"

namespace Krys::Tests
{
  using namespace Krys::HTML;

  class TestNode : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestNode);

  public:
    TestNode(Document &document) noexcept
        : Element(document, {Namespaces::HTML, DOMStringAtom::Null(), u8"test-element"})
    {
    }
  };
}