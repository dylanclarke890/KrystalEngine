#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlbaseelement
  class HTMLBaseElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLBaseElement);

  public:
    HTMLBaseElement(dom::Document &document) noexcept;

#pragma region HTMLBaseElement - https://html.spec.whatwg.org/#htmlbaseelement

    /// @see https://html.spec.whatwg.org/#dom-base-href
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> Href() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-base-href
    dom::ExceptionOr<void> Href(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-base-target
    KRYS_NODISCARD dom::DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-base-target
    void Target(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLBaseElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLBaseElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
