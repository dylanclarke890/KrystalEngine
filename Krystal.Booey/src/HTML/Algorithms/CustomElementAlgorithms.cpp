#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::html
{
  bool CustomElementAlgorithms::IsGlobalCustomElementRegistry(const CustomElementRegistry *registry) noexcept
  {
    return registry != nullptr && !registry->IsScoped();
  }

  CustomElementRegistry *
    CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(CustomElementRegistry *registry) noexcept
  {
    // 1. If registry is a global custom element registry, then return registry.
    if (IsGlobalCustomElementRegistry(registry))
    {
      return registry;
    }

    // 2. Return null.
    return nullptr;
  }

  CustomElementRegistry *
    CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(dom::Document &document) noexcept
  {
    return EffectiveGlobalCustomElementRegistry(document._customElementRegistry.get());
  }

  CustomElementInterface *CustomElementAlgorithms::LookupCustomElementDefinition(
    CustomElementRegistry *registry, dom::DOMStringAtom namespaceURI, dom::DOMStringAtom localName) noexcept
  {
    if (registry == nullptr)
    {
      return nullptr;
    }

    if (namespaceURI != infra::Namespaces::HTML)
    {
      return nullptr;
    }

    return registry->FindInterface(localName);
  }

  CustomElementRegistry *CustomElementAlgorithms::LookUpCustomElementRegistry(dom::Node &node) noexcept
  {
    if (auto *element = DynamicDowncast<dom::Element>(node))
    {
      return element->CustomElementRegistry().get();
    }

    if (auto *document = DynamicDowncast<dom::Document>(node))
    {
      return document->CustomElementRegistry().get();
    }

    if (auto *shadowRoot = DynamicDowncast<dom::ShadowRoot>(node))
    {
      return shadowRoot->CustomElementRegistry().get();
    }

    return nullptr;
  }
}
