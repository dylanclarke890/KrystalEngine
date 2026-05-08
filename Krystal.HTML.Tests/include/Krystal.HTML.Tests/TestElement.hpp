#pragma once

#include "Krystal.HTML/Node/Element.hpp"

namespace Krys::Tests
{
  using namespace Krys::HTML;

  class TestElement : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestElement);

  public:
    TestElement(Document &document) noexcept : Element(document, NodeFlag::None)
    {
    }
  };
}