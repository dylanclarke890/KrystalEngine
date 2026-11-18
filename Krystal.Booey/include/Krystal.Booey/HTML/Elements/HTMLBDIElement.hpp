#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @brief A `<bdi>`, text isolated from its surroundings for bidirectional formatting. Without a `dir`
  /// attribute its directionality is auto. Its interface is `HTMLElement`.
  /// @see https://html.spec.whatwg.org/#the-bdi-element
  class HTMLBDIElement final : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLBDIElement);

  public:
    explicit HTMLBDIElement(Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLBDIElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLBDIElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLBDIElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
