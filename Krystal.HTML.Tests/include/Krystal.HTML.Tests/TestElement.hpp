#pragma once

#include "Krystal.HTML/Constants/Namespaces.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/QualifiedName.hpp"

namespace Krys::Tests
{
  using namespace Krys::HTML;

  class TestElement : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestElement);

  public:
    TestElement(Document &document) noexcept
        : Element(document, {Namespace::HTML, DOMStringAtom::Null(), u8"test-element"}, NodeFlags::None)
    {
    }
  };
}