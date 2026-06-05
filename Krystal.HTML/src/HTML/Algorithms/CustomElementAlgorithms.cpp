#include "Krystal.HTML/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Infra/Namespaces.hpp"

namespace Krys::HTML
{
  bool CustomElementAlgorithms::IsGlobalCustomElementRegistry(
    RawPtr<const CustomElementRegistry> registry) noexcept
  {
    return registry != nullptr && !registry->IsScoped();
  }

  RawPtr<CustomElementRegistry>
    CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(Document &document) noexcept
  {
    if (IsGlobalCustomElementRegistry(document._customElementRegistry.get()))
    {
      return document._customElementRegistry.get();
    }

    return nullptr;
  }

  RawPtr<CustomElementDefinition> CustomElementAlgorithms::LookupCustomElementDefinition(
    RawPtr<CustomElementRegistry> registry, DOMStringAtom namespaceURI, DOMStringAtom is) noexcept
  {
    if (registry == nullptr)
    {
      return nullptr;
    }

    if (namespaceURI != Namespace::HTML)
    {
      return nullptr;
    }

    // TODO(impl): CUSTOM-ELEMENTS
    return nullptr;
  }
}