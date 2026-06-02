#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlscriptelement
  class HTMLScriptElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLScriptElement);

  public:
    HTMLScriptElement(Document &document) noexcept : HTMLElement(document, DOMInterface::Script)
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLScriptElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLScriptElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();