#pragma once

#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo
{
  class HitTestResult;
}

namespace krys::boo::html
{
  class HTMLImageElement;

  /// @see https://html.spec.whatwg.org/#htmlmapelement
  class HTMLMapElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMapElement);

  public:
    HTMLMapElement(dom::Document &document) noexcept;

    /// @brief Hit tests the map's areas at @p location in an image of @p size: the first area whose shape
    /// contains it, or else the first default area, becomes the result's inner node and link.
    /// @return Whether an area was hit.
    bool MapMouseEvent(LayoutPoint location, const LayoutSize &size, HitTestResult &result) noexcept;

    /// @brief The first image in the map's tree that uses it.
    KRYS_NODISCARD RefPtr<HTMLImageElement> ImageElement() const noexcept;

#pragma region HTMLMapElement - https://html.spec.whatwg.org/#htmlmapelement

    /// @see https://html.spec.whatwg.org/#dom-map-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-map-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-map-areas
    KRYS_NODISCARD Ref<dom::HTMLCollection> Areas() noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLMapElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLMapElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLMapElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();