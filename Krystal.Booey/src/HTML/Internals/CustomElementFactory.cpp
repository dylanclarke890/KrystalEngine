#include "Krystal.Booey/HTML/Internals/CustomElementFactory.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::html
{
  RefPtr<HTMLElement> CustomElementFactory::TryCreate(dom::Document &document, const dom::QualifiedName &name,
                                                      dom::DOMStringAtom is, bool synchronousCustomElements,
                                                      CustomElementRegistry *registry) noexcept
  {
    if (registry == nullptr)
    {
      registry = document.CustomElementRegistry().get();
    }

    auto definition =
      CustomElementAlgorithms::LookupCustomElementDefinition(registry, name.NamespaceURI(), is);
    if (definition == nullptr)
    {
      return nullptr;
    }

    // TODO(impl): CUSTOM-ELEMENTS
    return nullptr;
  }
}