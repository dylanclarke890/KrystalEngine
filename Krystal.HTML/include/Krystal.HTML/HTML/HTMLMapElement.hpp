#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlmapelement
  class HTMLMapElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMapElement);

  public:
    HTMLMapElement(Document &document) noexcept;

#pragma region HTMLMapElement - https://html.spec.whatwg.org/#htmlmapelement

    /// @see https://html.spec.whatwg.org/#dom-map-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-map-name
    void Name(DOMString &&value) noexcept;

    // TODO(HTMLMapElement, HTML): Implement Areas().
    /// @see https://html.spec.whatwg.org/#dom-map-areas
    Ref<HTMLCollection> Areas() noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLMapElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLMapElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();