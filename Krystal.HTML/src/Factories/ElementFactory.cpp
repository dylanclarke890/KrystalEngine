#include "Krystal.HTML/Factories/ElementFactory.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/CustomElement/CustomElementDefinition.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Factories/CustomElementFactory.hpp"
#include "Krystal.HTML/Factories/HTMLElementFactory.hpp"
#include "Krystal.HTML/Factories/MathMLElementFactory.hpp"
#include "Krystal.HTML/Factories/SVGElementFactory.hpp"
#include "Krystal.HTML/HTMLElement/HTMLElement.hpp"
#include "Krystal.HTML/MathMLElement/MathMLElement.hpp"
#include "Krystal.HTML/Namespaces.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/SVGElement/SVGElement.hpp"

namespace Krys::HTML
{
  Ref<Element> ElementFactory::Create(Document &document, const QualifiedName &name, DOMStringAtom is,
                                      bool synchronousCustomElements,
                                      CustomElementRegistryOrDefault registry) noexcept
  {
    RefPtr<Element> result = nullptr;

    if (registry == DefaultCustomElementRegistry)
    {
      registry = document.CustomElementRegistry();
    }

    if (name.NamespaceURI == Namespaces::HTML)
    {
      result = CustomElementFactory::TryCreate(document, name, is, synchronousCustomElements,
                                               registry.value().get());

      if (result == nullptr)
      {
        result = HTMLElementFactory::TryCreate(document, name.LocalName);
        if (NameValidation::IsValidCustomElementName(name.LocalName.View()) || is != DOMStringAtom::Null())
        {
          result->_customElementState = CustomElementState::Undefined;
        }
      }
    }
    else if (name.NamespaceURI == Namespaces::SVG)
    {
      result = SVGElementFactory::TryCreate(document, name);
    }
    else if (name.NamespaceURI == Namespaces::MathML)
    {
      result = MathMLElementFactory::TryCreate(document, name);
    }

    if (result == nullptr)
    {
      result = AdoptRefPtr<Element>(new Element(document, name, NodeFlag::None));
    }

    result->_qualifiedName = name;
    result->_customElementRegistry = registry.value();
    result->_is = is;

    return AdoptRef(*result.release());
  }
}