#pragma once

#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"
#include "Krystal.HTML/HTMLElement/HTMLTagName.hpp"

namespace Krys::HTML
{
  class HTMLPreElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLPreElement);

  public:
    HTMLPreElement(Document &document, const QualifiedName &name) noexcept;
  };
}
