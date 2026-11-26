#pragma once

#include "Krystal.Booey/Html/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmlheadelement
  class HTMLHeadElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLHeadElement);

  public:
    HTMLHeadElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLHeadElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLHeadElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return Is<krys::boo::HTMLElement>(target) && Downcast<krys::boo::HTMLElement>(target).IsHTMLHeadElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
