#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/edits.html#htmlmodelement
  class HTMLModElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLModElement);

  public:
    HTMLModElement(Document &document) noexcept;

#pragma region HTMLModElement - https://html.spec.whatwg.org/multipage/edits.html#htmlmodelement

    /// @see https://html.spec.whatwg.org/multipage/edits.html#dom-mod-cite
    KRYS_NODISCARD dom::USVString Cite() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/edits.html#dom-mod-cite
    void Cite(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/edits.html#dom-mod-datetime
    KRYS_NODISCARD DOMString DateTime() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/edits.html#dom-mod-datetime
    void DateTime(DOMString value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLModElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLModElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
