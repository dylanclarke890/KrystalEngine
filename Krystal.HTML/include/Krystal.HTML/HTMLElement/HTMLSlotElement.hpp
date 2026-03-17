#pragma once

#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"

namespace Krys::HTML
{
  class HTMLSlotElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSlotElement);

  private:
    DOMString _name;

  public:
    HTMLSlotElement(Document &document) noexcept;

    KRYS_NODISCARD DOMString Name() const noexcept
    {
      return _name;
    }

    void Name(DOMString &&name) noexcept
    {
      _name = Krys::Move(name);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLSlotElement)
  static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsHTMLSlotElementNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();