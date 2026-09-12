#pragma once

#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  class CustomElementRegistry;
  class Document;
  class HTMLElement;

  class CustomElementFactory
  {
  public:
    KRYS_NODISCARD static RefPtr<HTMLElement> TryCreate(Document &document, const QualifiedName &name,
                                                        dom::DOMStringAtom is, bool synchronousCustomElements,
                                                        CustomElementRegistry *registry) noexcept;
  };
}