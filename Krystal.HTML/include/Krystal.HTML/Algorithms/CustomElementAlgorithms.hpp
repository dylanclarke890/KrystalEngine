#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;
  class CustomElementDefinition;
  class Document;

  class CustomElementAlgorithms
  {
  public:
    KRYS_NODISCARD static bool
      IsGlobalCustomElementRegistry(RawPtr<const CustomElementRegistry> registry) noexcept;

    KRYS_NODISCARD static RawPtr<CustomElementRegistry>
      EffectiveGlobalCustomElementRegistry(Document &document) noexcept;

    KRYS_NODISCARD static RawPtr<CustomElementDefinition>
      LookupCustomElementDefinition(RawPtr<CustomElementRegistry> registry, DOMStringAtom namespaceURI,
                                    DOMStringAtom is) noexcept;
  };
}