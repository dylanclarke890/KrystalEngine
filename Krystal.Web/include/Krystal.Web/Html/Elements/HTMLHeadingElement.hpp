#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/sections.html#htmlheadingelement
  class HTMLHeadingElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLHeadingElement);

  public:
    HTMLHeadingElement(Document &document) noexcept;

    /// @brief The heading's level: its number (1 for `h1`), increased by its computed heading offset, up
    /// to 9.
    /// @see https://html.spec.whatwg.org/#get-an-element's-computed-heading-level
    /// @see https://html.spec.whatwg.org/multipage/sections.html#heading-level
    KRYS_NODISCARD uint32 ComputedHeadingLevel() const noexcept;

    /// @brief The sum of the `headingoffset` values of @p element and its ancestors (crossing shadow
    /// boundaries), up to and including the nearest with `headingreset`.
    /// @see https://html.spec.whatwg.org/#get-an-element's-computed-heading-offset
    KRYS_NODISCARD static uint64 ComputedHeadingOffset(const Element &element) noexcept;

#pragma region HTMLHeadingElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLHeadingElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-hx-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-hx-align
    void Align(DOMString value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLHeadingElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLHeadingElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::web::HTMLElement>(target);
    return element && element->IsHTMLHeadingElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
