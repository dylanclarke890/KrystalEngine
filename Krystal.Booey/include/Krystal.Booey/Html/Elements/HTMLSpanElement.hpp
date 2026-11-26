#pragma once

#include "Krystal.Booey/Html/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#htmlspanelement
  class HTMLSpanElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSpanElement);

  public:
    HTMLSpanElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLSpanElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLSpanElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
