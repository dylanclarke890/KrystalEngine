#pragma once

#include "Krystal.HTML/HTML/Enums/TagName.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/Types/QualifiedName.hpp"
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
    KRYS_NODISCARD static RefPtr<HTMLElement> TryCreate(Document &document, TagName elementName) noexcept;
  };
}