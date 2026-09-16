#pragma once

#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::dom::tests
{
  class TestElement : public dom::Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestElement);

  public:
    TestElement(dom::Document &document) noexcept
        : Element(document, {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"test-element"},
                  dom::NodeFlags::None)
    {
    }
  };
}