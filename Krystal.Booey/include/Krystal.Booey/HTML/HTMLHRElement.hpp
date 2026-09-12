#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlhrelement
  class HTMLHRElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLHRElement);

  public:
    HTMLHRElement(Document &document) noexcept;

#pragma region HTMLHRElement Obsolete members - https://html.spec.whatwg.org/#HTMLHRElement-partial

    /// @see https://html.spec.whatwg.org/#dom-hr-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-color
    KRYS_NODISCARD dom::DOMString Color() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-color
    void Color(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-noshade
    KRYS_NODISCARD bool NoShade() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-noshade
    void NoShade(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-size
    KRYS_NODISCARD dom::DOMString Size() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-size
    void Size(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-width
    KRYS_NODISCARD dom::DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-width
    void Width(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLHRElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLHRElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLHRElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();