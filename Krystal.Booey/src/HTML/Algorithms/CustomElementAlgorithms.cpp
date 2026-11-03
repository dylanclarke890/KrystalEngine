#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
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

  // https://html.spec.whatwg.org/#look-up-a-custom-element-definition
  CustomElementInterface *CustomElementAlgorithms::LookupCustomElementDefinition(
    CustomElementRegistry *registry, dom::DOMStringAtom namespaceURI, dom::DOMStringAtom localName) noexcept
  {
    // 1. If registry is null, then return null.
    if (registry == nullptr)
    {
      return nullptr;
    }

    // 2. If namespace is not the HTML namespace, then return null.
    if (namespaceURI != infra::Namespaces::HTML)
    {
      return nullptr;
    }

    // 3. If registry's custom element definition set contains an item with name and local name both equal to
    //    localName, then return that item.
    // 4. If registry's custom element definition set contains an item with name equal to is and local name
    //    equal to localName, then return that item. (Skipped: there are no customized built-in elements.)
    // 5. Return null.
    return registry->FindInterface(localName);
  }

  // https://html.spec.whatwg.org/#look-up-a-custom-element-registry
  CustomElementRegistry *CustomElementAlgorithms::LookUpCustomElementRegistry(dom::Node &node) noexcept
  {
    // 1. If node is an Element object, then return node's custom element registry.
    // 2. If node is a ShadowRoot object, then return node's custom element registry.
    // 3. If node is a Document object, then return node's custom element registry.
    // 4. Return null.
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
