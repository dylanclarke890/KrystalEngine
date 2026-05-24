#pragma once

#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmlstyleelement
  class HTMLStyleElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLStyleElement);

  public:
    HTMLStyleElement(Document &document) noexcept;

    // TODO(impl): URL and Target IDL attributes
  };
}
