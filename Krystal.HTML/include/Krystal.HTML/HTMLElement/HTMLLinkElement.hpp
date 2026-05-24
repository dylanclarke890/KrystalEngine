#pragma once

#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmllinkelement
  class HTMLLinkElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLinkElement);

  public:
    HTMLLinkElement(Document &document) noexcept;

    // TODO(impl): IDL attributes
  };
}
