#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::html
{
  class CustomElementRegistry;
  class CustomElementDefinition;

  class CustomElementAlgorithms
  {
  public:
    KRYS_NODISCARD static bool IsGlobalCustomElementRegistry(const CustomElementRegistry *registry) noexcept;

    KRYS_NODISCARD static CustomElementRegistry *
      EffectiveGlobalCustomElementRegistry(dom::Document &document) noexcept;

    KRYS_NODISCARD static CustomElementDefinition *
      LookupCustomElementDefinition(CustomElementRegistry *registry, dom::DOMStringAtom namespaceURI,
                                    dom::DOMStringAtom is) noexcept;
  };
}