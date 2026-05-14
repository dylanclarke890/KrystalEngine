#include "Krystal.HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Namespaces.hpp"

namespace Krys::HTML
{
  bool CustomElementAlgorithms::IsGlobalCustomElementRegistry(
    RawPtr<const CustomElementRegistry> registry) noexcept
  {
    return registry != nullptr && !registry->IsScoped();
  }

  RawPtr<CustomElementDefinition> CustomElementAlgorithms::LookupCustomElementDefinition(
    RawPtr<CustomElementRegistry> registry, DOMStringAtom namespaceURI, DOMStringAtom is) noexcept
  {
    if (registry == nullptr)
    {
      return nullptr;
    }

    if (namespaceURI != Namespaces::HTML)
    {
      return nullptr;
    }

    // TODO(impl): CUSTOM-ELEMENTS
    return nullptr;
  }
}