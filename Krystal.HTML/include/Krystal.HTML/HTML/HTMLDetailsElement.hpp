#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmldetailselement
  class HTMLDetailsElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDetailsElement);

  public:
    HTMLDetailsElement(Document &document) noexcept;

#pragma region HTMLDetailsElement - https://html.spec.whatwg.org/#htmldetailselement

    /// @see https://html.spec.whatwg.org/#dom-details-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-details-name
    void Name(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-details-open
    KRYS_NODISCARD bool Open() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-details-open
    void Open(bool value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLDetailsElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLDetailsElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();