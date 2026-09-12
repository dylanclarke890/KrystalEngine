#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlmapelement
  class HTMLMapElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMapElement);

  public:
    HTMLMapElement(Document &document) noexcept;

#pragma region HTMLMapElement - https://html.spec.whatwg.org/#htmlmapelement

    /// @see https://html.spec.whatwg.org/#dom-map-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-map-name
    void Name(dom::DOMString &&value) noexcept;

    // TODO(HTMLMapElement, HTML): Implement Areas().
    /// @see https://html.spec.whatwg.org/#dom-map-areas
    Ref<HTMLCollection> Areas() noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLMapElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLMapElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();