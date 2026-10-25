#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/URLDecomposition.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class HitTestResult;
}

namespace krys::boo::html
{
  class HTMLImageElement;

  /// @see https://html.spec.whatwg.org/#htmlareaelement
  class HTMLAreaElement : public HTMLElement, public URLDecomposition
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLAreaElement);

    enum class AreaShape : uint8
    {
      Default,
      Poly,
      Rect,
      Circle
    };

  private:
    UniquePtr<dom::DOMTokenList> _relList;
    // The region of the shape at the last size it was hit tested at.
    Maybe<Path> _region;
    List<double> _coords;
    LayoutSize _lastSize {-1, -1};
    AreaShape _shape {AreaShape::Rect};

  public:
    HTMLAreaElement(dom::Document &document) noexcept;

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

    /// @brief The area's shape in absolute coordinates, over its image's @p renderer.
    KRYS_NODISCARD Path ComputePath(const RenderElement &renderer) const noexcept;
    /// @brief The bounds of the area's shape in absolute coordinates, over its image's @p renderer.
    KRYS_NODISCARD LayoutRect ComputeRect(const RenderObject *renderer) const noexcept;

    /// @brief The parent map's image.
    KRYS_NODISCARD RefPtr<HTMLImageElement> ImageElement() const noexcept;

    /// @brief Repaints the image, which paints the area's focus ring.
    void SetFocus(bool value,
                  dom::FocusVisibility visibility = dom::FocusVisibility::Invisible) noexcept override;

    /// @brief The image, which shows that the area is focused.
    KRYS_NODISCARD RefPtr<dom::Element> FocusAppearanceUpdateTarget() noexcept override;

    /// @brief Follows the link, if it is one.
    /// @see https://html.spec.whatwg.org/#the-area-element:activation-behaviour
    void ActivationBehavior(dom::Event &event) noexcept override;

    /// @brief Follows a focused link on Enter (as a simulated click), and on a click of the middle button.
    void DefaultEventHandler(dom::Event &event) noexcept override;

#pragma region HTMLAreaElement - https://html.spec.whatwg.org/#htmlareaelement

    /// @see https://html.spec.whatwg.org/#dom-area-alt
    KRYS_NODISCARD dom::DOMString Alt() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-alt
    void Alt(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-coords
    KRYS_NODISCARD dom::DOMString Coords() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-coords
    void Coords(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-shape
    KRYS_NODISCARD dom::DOMString Shape() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-shape
    void Shape(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-target
    KRYS_NODISCARD dom::DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-target
    void Target(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-download
    KRYS_NODISCARD dom::DOMString Download() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-download
    void Download(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-ping
    KRYS_NODISCARD dom::USVString Ping() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-ping
    void Ping(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-rel
    KRYS_NODISCARD dom::DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-rel
    void Rel(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-rellist
    KRYS_NODISCARD dom::DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-referrerpolicy
    KRYS_NODISCARD dom::DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-referrerpolicy
    void ReferrerPolicy(dom::DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLHyperlinkElementUtils - https://html.spec.whatwg.org/#htmlhyperlinkelementutils

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-href
    KRYS_NODISCARD dom::USVString Href() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-href
    void Href(dom::USVString &&value) noexcept;

    /// @brief The `href` stringifier.
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-href
    KRYS_NODISCARD dom::USVString ToString() const noexcept;

    using URLDecomposition::Protocol;

    /// @brief Changes the link's scheme, unless its URL is invalid.
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-protocol
    void Protocol(dom::DOMStringView value) noexcept;

#pragma endregion

#pragma region HTMLAreaElement Obsolete members - https://html.spec.whatwg.org/#HTMLAreaElement-partial

    /// @see https://html.spec.whatwg.org/#dom-area-nohref
    KRYS_NODISCARD bool NoHref() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-area-nohref
    void NoHref(bool value) noexcept;

#pragma endregion

  protected:
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

  private:
    KRYS_NODISCARD url::URL FullURL() const noexcept final;
    void SetFullURL(const url::URL &url) noexcept final;

    KRYS_NODISCARD Path GetRegion(const LayoutSize &size) const noexcept;
    void InvalidateCachedRegion() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLAreaElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLAreaElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLAreaElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();