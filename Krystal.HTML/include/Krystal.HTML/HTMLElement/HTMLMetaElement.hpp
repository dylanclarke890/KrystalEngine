#pragma once

#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmlmetaelement
  class HTMLMetaElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMetaElement);

  public:
    HTMLMetaElement(Document &document) noexcept;

    // TODO(impl): IDL attributes
  };
}
