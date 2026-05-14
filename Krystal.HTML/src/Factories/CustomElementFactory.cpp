#include "Krystal.HTML/Factories/CustomElementFactory.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

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