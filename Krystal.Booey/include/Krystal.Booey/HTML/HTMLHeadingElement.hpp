#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlheadingelement
  class HTMLHeadingElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLHeadingElement);

  public:
    HTMLHeadingElement(dom::Document &document) noexcept;

    /// @brief The heading's level: its number (1 for `h1`), increased by its computed heading offset, up
    /// to 9.
    /// @see https://html.spec.whatwg.org/#get-an-element's-computed-heading-level
    /// @see https://html.spec.whatwg.org/#heading-level
    KRYS_NODISCARD uint32 ComputedHeadingLevel() const noexcept;

    /// @brief The sum of the `headingoffset` values of @p element and its ancestors (crossing shadow
    /// boundaries), up to and including the nearest with `headingreset`.
    /// @see https://html.spec.whatwg.org/#get-an-element's-computed-heading-offset
    KRYS_NODISCARD static uint64 ComputedHeadingOffset(const dom::Element &element) noexcept;

#pragma region HTMLHeadingElement Obsolete members - https://html.spec.whatwg.org/#HTMLHeadingElement-partial

    /// @see https://html.spec.whatwg.org/#dom-hx-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hx-align
    void Align(dom::DOMString value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLHeadingElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLHeadingElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();