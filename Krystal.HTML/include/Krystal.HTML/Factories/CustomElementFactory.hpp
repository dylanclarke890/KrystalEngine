#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;
  class Document;
  class HTMLElement;

  class CustomElementFactory
  {
  public:
    KRYS_NODISCARD static RefPtr<HTMLElement> TryCreate(Document &document, const QualifiedName &name,
                                                        DOMStringAtom is, bool synchronousCustomElements,
                                                        RawPtr<CustomElementRegistry> registry) noexcept;
  };
}