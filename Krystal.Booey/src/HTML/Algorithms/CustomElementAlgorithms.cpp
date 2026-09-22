#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::html
{
  bool CustomElementAlgorithms::IsGlobalCustomElementRegistry(const CustomElementRegistry *registry) noexcept
  {
    return registry != nullptr && !registry->IsScoped();
  }

  CustomElementRegistry *
    CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(dom::Document &document) noexcept
  {
    if (IsGlobalCustomElementRegistry(document._customElementRegistry.get()))
    {
      return document._customElementRegistry.get();
    }

    return nullptr;
  }

  CustomElementDefinition *CustomElementAlgorithms::LookupCustomElementDefinition(
    CustomElementRegistry *registry, dom::DOMStringAtom namespaceURI, dom::DOMStringAtom is) noexcept
  {
    if (registry == nullptr)
    {
      return nullptr;
    }

    if (namespaceURI != infra::Namespaces::HTML)
    {
      return nullptr;
    }

    // TODO(impl): CUSTOM-ELEMENTS
    (void)is;
    krys_not_implemented();
  }
}