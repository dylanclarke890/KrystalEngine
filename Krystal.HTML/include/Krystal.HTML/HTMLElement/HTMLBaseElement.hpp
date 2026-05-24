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
