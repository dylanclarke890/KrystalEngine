#pragma once

#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmlbaseelement
  class HTMLBaseElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLBaseElement);

  public:
    HTMLBaseElement(Document &document) noexcept;

    // TODO(impl): URL and Target IDL attributes
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLBaseElement)
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLBaseElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
