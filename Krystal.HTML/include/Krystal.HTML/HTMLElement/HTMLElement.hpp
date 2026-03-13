#pragma once

#include "Krystal.HTML/Node/Element.hpp"

namespace Krys::HTML
{
  class HTMLElement : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLElement);

  protected:
    HTMLElement(Document &document, NodeFlag nodeFlags = NodeFlag::None) noexcept;

  public:
  };
}