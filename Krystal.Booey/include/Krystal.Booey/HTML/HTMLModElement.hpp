#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlmodelement
  class HTMLModElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLModElement);

  public:
    HTMLModElement(dom::Document &document) noexcept;

#pragma region HTMLModElement - https://html.spec.whatwg.org/#htmlmodelement

    /// @see https://html.spec.whatwg.org/#dom-mod-cite
    KRYS_NODISCARD dom::ExceptionOr<dom::USVString> Cite() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-mod-cite
    dom::ExceptionOr<void> Cite(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-mod-datetime
    KRYS_NODISCARD dom::DOMString DateTime() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-mod-datetime
    void DateTime(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLModElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLModElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();