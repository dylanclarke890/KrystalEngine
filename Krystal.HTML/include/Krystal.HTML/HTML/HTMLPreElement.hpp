#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlpreelement
  class HTMLPreElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLPreElement);

  public:
    HTMLPreElement(Document &document) noexcept;

#pragma region HTMLPreElement Obsolete members - https://html.spec.whatwg.org/#HTMLPreElement-partial

    /// @see https://html.spec.whatwg.org/#dom-pre-width
    KRYS_NODISCARD int32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-pre-width
    void Width(int32 value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLPreElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLPreElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();