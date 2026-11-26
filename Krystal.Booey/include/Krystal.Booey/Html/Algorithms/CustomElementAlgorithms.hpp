#pragma once

#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class Document;
  class Node;
}

namespace krys::boo::html
{
  class CustomElementInterface;
  class CustomElementRegistry;

  class CustomElementAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#is-a-global-custom-element-registry
    KRYS_NODISCARD static bool IsGlobalCustomElementRegistry(const CustomElementRegistry *registry) noexcept;

    /// @see https://dom.spec.whatwg.org/#effective-global-custom-element-registry
    KRYS_NODISCARD static CustomElementRegistry *
      EffectiveGlobalCustomElementRegistry(CustomElementRegistry *registry) noexcept;

    /// @brief @p document's custom element registry's effective global custom element registry.
    /// @see https://dom.spec.whatwg.org/#effective-global-custom-element-registry
    KRYS_NODISCARD static CustomElementRegistry *
      EffectiveGlobalCustomElementRegistry(Document &document) noexcept;

    /// @note Customized built-in elements aren't supported: a definition is found by its local name alone.
    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#look-up-a-custom-element-definition
    KRYS_NODISCARD static CustomElementInterface *
      LookupCustomElementDefinition(CustomElementRegistry *registry, const DOMStringAtom &namespaceURI,
                                    DOMStringAtom localName) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#look-up-a-custom-element-registry
    KRYS_NODISCARD static CustomElementRegistry *LookUpCustomElementRegistry(Node &node) noexcept;
  };
}
