#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#the-marquee-element
  class HTMLMarqueeElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMarqueeElement);

  public:
    HTMLMarqueeElement(dom::Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-behavior
    KRYS_NODISCARD dom::DOMString Behavior() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-behavior
    void Behavior(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-bgcolor
    KRYS_NODISCARD dom::DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-bgcolor
    void BgColor(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-direction
    KRYS_NODISCARD dom::DOMString Direction() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-direction
    void Direction(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-height
    KRYS_NODISCARD dom::DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-height
    void Height(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-width
    KRYS_NODISCARD dom::DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-width
    void Width(dom::DOMString &&value) noexcept;

    // FIXME: hspace, vspace, loop, scrollAmount, scrollDelay, trueSpeed, start() and
    // stop(). Marquees don't scroll (the -webkit-marquee properties are dropped).

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLMarqueeElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLMarqueeElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
