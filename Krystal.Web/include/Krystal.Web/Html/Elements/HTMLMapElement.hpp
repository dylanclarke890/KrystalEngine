#pragma once

#include "Krystal.Web/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  class HTMLImageElement;
  class HitTestResult;

  /// @see https://html.spec.whatwg.org/multipage/image-maps.html#htmlmapelement
  class HTMLMapElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMapElement);

  public:
    HTMLMapElement(Document &document) noexcept;

    /// @brief Hit tests the map's areas at @p location in an image of @p size: the first area whose shape
    /// contains it, or else the first default area, becomes the result's inner node and link.
    /// @return Whether an area was hit.
    bool MapMouseEvent(LayoutPoint location, const LayoutSize &size, HitTestResult &result) noexcept;

    /// @brief The first image in the map's tree that uses it.
    KRYS_NODISCARD RefPtr<HTMLImageElement> ImageElement() const noexcept;

#pragma region HTMLMapElement - https://html.spec.whatwg.org/multipage/image-maps.html#htmlmapelement

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-map-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-map-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-map-areas
    KRYS_NODISCARD Ref<HTMLCollection> Areas() noexcept;

#pragma endregion

  protected:
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLMapElement)
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::web::HTMLElement>(target).IsHTMLMapElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLMapElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
