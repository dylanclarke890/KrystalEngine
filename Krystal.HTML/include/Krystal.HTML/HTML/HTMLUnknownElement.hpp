#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/HTML/HTMLTagName.hpp"

namespace Krys::HTML
{
  class HTMLUnknownElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLUnknownElement);

  public:
    HTMLUnknownElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLUnknownElement)
  static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsHTMLUnknownElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();