#pragma once

#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmllinkelement
  class HTMLLinkElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLinkElement);

  public:
    HTMLLinkElement(Document &document) noexcept;

    // TODO(impl): IDL attributes
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLLinkElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLLinkElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();