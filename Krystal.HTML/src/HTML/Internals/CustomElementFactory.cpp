#include "Krystal.HTML/HTML/Internals/CustomElementFactory.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  RefPtr<HTMLElement> CustomElementFactory::TryCreate(Document &document, const QualifiedName &name,
                                                      DOMStringAtom is, bool synchronousCustomElements,
                                                      RawPtr<CustomElementRegistry> registry) noexcept
  {
    if (registry == nullptr)
    {
      registry = document.CustomElementRegistry().get();
    }

    auto definition = CustomElementAlgorithms::LookupCustomElementDefinition(registry, name.NamespaceURI, is);
    if (definition == nullptr)
    {
      return nullptr;
    }

    // TODO(impl): CUSTOM-ELEMENTS
    return nullptr;
  }
}