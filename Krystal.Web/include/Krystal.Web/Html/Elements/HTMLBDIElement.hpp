#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @brief A `<bdi>`, text isolated from its surroundings for bidirectional formatting. Without a `dir`
  /// attribute its directionality is auto. Its interface is `HTMLElement`.
  /// @see https://html.spec.whatwg.org/multipage/text-level-semantics.html#the-bdi-element
  class HTMLBDIElement final : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLBDIElement);

  public:
    explicit HTMLBDIElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLBDIElement)
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::web::HTMLElement>(target).IsHTMLBDIElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLBDIElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
