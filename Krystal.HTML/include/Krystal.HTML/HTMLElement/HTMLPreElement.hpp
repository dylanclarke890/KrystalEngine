#pragma once

#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"

namespace Krys::HTML
{
  class HTMLPreElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLPreElement);

  public:
    HTMLPreElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLPreElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLPreElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();