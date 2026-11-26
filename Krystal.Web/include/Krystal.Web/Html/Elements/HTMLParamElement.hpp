#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @brief The obsolete `param` element.
  /// @see https://html.spec.whatwg.org/multipage/obsolete.html#htmlparamelement
  class HTMLParamElement final : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLParamElement);

  public:
    explicit HTMLParamElement(Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-param-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-param-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-param-value
    KRYS_NODISCARD DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-param-value
    void Value(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-param-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-param-type
    void Type(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-param-valuetype
    KRYS_NODISCARD DOMString ValueType() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-param-valuetype
    void ValueType(DOMString value) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLParamElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLParamElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
