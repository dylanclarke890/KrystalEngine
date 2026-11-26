#pragma once

#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Dom/Types/USVString.hpp"
#include "Krystal.Web/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Web/Graphics/Paths/Path.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Html/Mixins/UrlDecomposition.hpp"

namespace krys::web
{
  class HTMLImageElement;
  class HitTestResult;

  /// @see https://html.spec.whatwg.org/multipage/image-maps.html#htmlareaelement
  class HTMLAreaElement : public HTMLElement, public html::UrlDecomposition
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLAreaElement);

    enum class AreaShape : uint8
    {
      Default,
      Poly,
      Rect,
      Circle,
    };

  private:
    UniquePtr<DOMTokenList> _relList;
    // The region of the shape at the last size it was hit tested at.
    Maybe<Path> _region;
    List<double> _coords;
    LayoutSize _lastSize {-1, -1};
    AreaShape _shape {AreaShape::Rect};

  public:
    HTMLAreaElement(Document &document) noexcept;

    ~HTMLAreaElement() noexcept override;

    /// @brief Whether the area is the whole image (`shape=default`).
    KRYS_NODISCARD bool IsDefault() const noexcept
    {
      return _shape == AreaShape::Default;
    }

    /// @brief Hit tests the area's shape at @p location in an image of @p size, making the area the result's
    /// inner node and link if it's inside.
    bool MapMouseEvent(LayoutPoint location, const LayoutSize &size, HitTestResult &result) noexcept;

    /// @brief The area's shape in an image of @p elementSize, for its focus ring.
    KRYS_NODISCARD Path ComputePathForFocusRing(const LayoutSize &elementSize) const noexcept;

    /// @brief The bounds of the area's shape in absolute coordinates, over its image's @p renderer.
    KRYS_NODISCARD LayoutRect ComputeRect(const RenderObject *renderer) const noexcept;

    /// @brief The parent map's image.
    KRYS_NODISCARD RefPtr<HTMLImageElement> ImageElement() const noexcept;

    /// @brief Repaints the image, which paints the area's focus ring.
    void SetFocus(bool value,
                  dom::FocusVisibility visibility = dom::FocusVisibility::Invisible) noexcept override;

    /// @brief The image, which shows that the area is focused.
    KRYS_NODISCARD RefPtr<Element> FocusAppearanceUpdateTarget() noexcept override;

    /// @brief Follows the link, if it is one.
    /// @see https://html.spec.whatwg.org/#the-area-element:activation-behaviour
    void ActivationBehavior(Event &event) noexcept override;

    /// @brief Follows a focused link on Enter (as a simulated click), and on a click of the middle button.
    void DefaultEventHandler(Event &event) noexcept override;

#pragma region HTMLAreaElement - https://html.spec.whatwg.org/multipage/image-maps.html#htmlareaelement

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-alt
    KRYS_NODISCARD DOMString Alt() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-alt
    void Alt(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-coords
    KRYS_NODISCARD DOMString Coords() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-coords
    void Coords(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-shape
    KRYS_NODISCARD DOMString Shape() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-shape
    void Shape(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-target
    KRYS_NODISCARD DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-target
    void Target(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-download
    KRYS_NODISCARD DOMString Download() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-download
    void Download(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-ping
    KRYS_NODISCARD dom::USVString Ping() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-ping
    void Ping(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-rel
    KRYS_NODISCARD DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-rel
    void Rel(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-rellist
    KRYS_NODISCARD DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-referrerpolicy
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/image-maps.html#dom-area-referrerpolicy
    void ReferrerPolicy(DOMString value) noexcept;

#pragma endregion

#pragma region HTMLHyperlinkElementUtils - https://html.spec.whatwg.org/multipage/links.html#htmlhyperlinkelementutils

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-href
    KRYS_NODISCARD dom::USVString Href() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-href
    void Href(dom::USVString value) noexcept;

    /// @brief The `href` stringifier.
    /// @see https://html.spec.whatwg.org/multipage/links.html#dom-hyperlink-href
    KRYS_NODISCARD dom::USVString ToString() const noexcept;

#pragma endregion

#pragma region HTMLAreaElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLAreaElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-area-nohref
    KRYS_NODISCARD bool NoHref() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-area-nohref
    void NoHref(bool value) noexcept;

#pragma endregion

  protected:
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

  private:
    KRYS_NODISCARD net::Url FullUrl() const noexcept final;

    void SetFullUrl(const net::Url &url) noexcept final;

    /// @brief The area's shape over an image of @p size.
    KRYS_NODISCARD Path GetRegion(const LayoutSize &size) const noexcept;

    /// @brief Forgets the shape cached for hit testing, as the shape or coords attribute changed.
    void InvalidateCachedRegion() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLAreaElement)
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::web::HTMLElement>(target).IsHTMLAreaElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLAreaElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
