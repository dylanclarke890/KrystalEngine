#pragma once

#include "Krystal.Booey/HTML/Enums/TagName.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
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