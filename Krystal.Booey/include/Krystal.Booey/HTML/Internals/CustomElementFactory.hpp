#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace boo::dom::dom
{
  class Document;
}

namespace krys::boo::html
{
  class CustomElementRegistry;
  class HTMLElement;

  class CustomElementFactory
  {
  public:
    KRYS_NODISCARD static RefPtr<HTMLElement> TryCreate(dom::Document &document,
                                                        const dom::QualifiedName &name, dom::DOMStringAtom is,
                                                        bool synchronousCustomElements,
                                                        CustomElementRegistry *registry) noexcept;
  };
}