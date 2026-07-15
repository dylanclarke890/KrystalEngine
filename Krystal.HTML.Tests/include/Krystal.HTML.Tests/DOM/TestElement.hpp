#pragma once

#include "Krystal.HTML/Infra/Namespaces.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/Types/QualifiedName.hpp"

namespace Krys::Tests
{
  using namespace Krys::HTML;

  class TestElement : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestElement);

  public:
    TestElement(Document &document) noexcept
        : Element(document, {Namespaces::HTML, DOMStringAtom::Null(), u8"test-element"}, NodeFlags::None)
    {
    }
  };
}