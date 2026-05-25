#pragma once

#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmlheadelement
  class HTMLHeadElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLHeadElement);

  public:
    HTMLHeadElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLHeadElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLHeadElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();