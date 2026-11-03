#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @brief The obsolete `param` element.
  /// @see https://html.spec.whatwg.org/#htmlparamelement
  class HTMLParamElement final : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLParamElement);

  public:
    explicit HTMLParamElement(dom::Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-param-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-param-name
    void Name(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-param-value
    KRYS_NODISCARD dom::DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-param-value
    void Value(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-param-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-param-type
    void Type(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-param-valuetype
    KRYS_NODISCARD dom::DOMString ValueType() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-param-valuetype
    void ValueType(dom::DOMString value) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLParamElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLParamElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
