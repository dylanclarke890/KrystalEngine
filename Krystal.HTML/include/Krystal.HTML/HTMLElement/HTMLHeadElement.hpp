#pragma once

#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmlheadelement
  class HTMLHeadElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLHeadElement);

  public:
    HTMLHeadElement(Document &document) noexcept;
  };
}
