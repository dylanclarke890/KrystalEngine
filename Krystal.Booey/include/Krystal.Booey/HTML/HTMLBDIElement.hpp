#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @brief A `<bdi>`, text isolated from its surroundings for bidirectional formatting. Without a `dir`
  /// attribute its directionality is auto. Its interface is `HTMLElement`.
  /// @see https://html.spec.whatwg.org/#the-bdi-element
  class HTMLBDIElement final : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLBDIElement);

  public:
    explicit HTMLBDIElement(dom::Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLBDIElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLBDIElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLBDIElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
