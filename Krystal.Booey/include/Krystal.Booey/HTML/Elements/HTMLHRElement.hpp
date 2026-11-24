#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/#htmlhrelement
  class HTMLHRElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLHRElement);

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept final
    {
      return HasChildNodes() && HTMLElement::CanContainRangeEndPoint();
    }

    HTMLHRElement(Document &document) noexcept;

#pragma region HTMLHRElement Obsolete members - https://html.spec.whatwg.org/#HTMLHRElement-partial

    /// @see https://html.spec.whatwg.org/#dom-hr-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-align
    void Align(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-color
    KRYS_NODISCARD DOMString Color() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-color
    void Color(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-noshade
    KRYS_NODISCARD bool NoShade() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-noshade
    void NoShade(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-size
    KRYS_NODISCARD DOMString Size() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-size
    void Size(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hr-width
    void Width(DOMString value) noexcept;

#pragma endregion

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLHRElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLHRElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLHRElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
