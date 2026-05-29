#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/DOM/Internals/QualifiedName.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;
  class Document;
  class HTMLElement;

  class HTMLElementFactory
  {
  public:
    KRYS_NODISCARD static RefPtr<HTMLElement> TryCreate(Document &document, DOMStringAtom localName) noexcept;
  };
}