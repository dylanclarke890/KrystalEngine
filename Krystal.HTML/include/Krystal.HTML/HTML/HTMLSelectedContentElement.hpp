#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlselectedcontentelement
  class HTMLSelectedContentElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSelectedContentElement);

  public:
    HTMLSelectedContentElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLSelectedContentElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLSelectedContentElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
