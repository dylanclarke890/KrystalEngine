#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  class Document;
  class Node;
}

namespace krys::boo::html
{
  class CustomElementRegistry;
  class CustomElementInterface;

  class CustomElementAlgorithms
  {
  public:
    /// @see https://html.spec.whatwg.org/#is-a-global-custom-element-registry
    KRYS_NODISCARD static bool IsGlobalCustomElementRegistry(const CustomElementRegistry *registry) noexcept;

    /// @see https://html.spec.whatwg.org/#effective-global-custom-element-registry
    KRYS_NODISCARD static CustomElementRegistry *
      EffectiveGlobalCustomElementRegistry(dom::Document &document) noexcept;

    /// @note Customized built-in elements aren't supported: a definition is found by its local name alone.
    /// @see https://html.spec.whatwg.org/#look-up-a-custom-element-definition
    KRYS_NODISCARD static CustomElementInterface *
      LookupCustomElementDefinition(CustomElementRegistry *registry, dom::DOMStringAtom namespaceURI,
                                    dom::DOMStringAtom localName) noexcept;

    /// @see https://html.spec.whatwg.org/#look-up-a-custom-element-registry
    KRYS_NODISCARD static CustomElementRegistry *LookUpCustomElementRegistry(dom::Node &node) noexcept;
  };
}
