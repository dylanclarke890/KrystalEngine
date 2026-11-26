#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmlheadelement
  class HTMLHeadElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLHeadElement);

  public:
    HTMLHeadElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLHeadElement)

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLHeadElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return Is<krys::web::HTMLElement>(target) && Downcast<krys::web::HTMLElement>(target).IsHTMLHeadElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
