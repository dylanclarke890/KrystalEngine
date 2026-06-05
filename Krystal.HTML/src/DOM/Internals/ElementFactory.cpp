#include "Krystal.HTML/DOM/Internals/ElementFactory.hpp"
#include "Krystal.HTML/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementDefinition.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/Internals/CustomElementFactory.hpp"
#include "Krystal.HTML/HTML/Internals/HTMLElementFactory.hpp"
#include "Krystal.HTML/Infra/Namespaces.hpp"
#include "Krystal.HTML/MathML/Internals/MathMLElementFactory.hpp"
#include "Krystal.HTML/MathML/MathMLElement.hpp"
#include "Krystal.HTML/SVG/Internals/SVGElementFactory.hpp"
#include "Krystal.HTML/SVG/SVGElement.hpp"

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

    if (name.NamespaceURI == Namespace::HTML)
    {
      result = CustomElementFactory::TryCreate(document, name, is, synchronousCustomElements,
                                               registry.value().get());

      if (result == nullptr)
      {
        result = HTMLElementFactory::Create(document, name.LocalName);
        if (NameValidation::IsValidCustomElementName(name.LocalName.View()) || is != DOMStringAtom::Null())
        {
          result->_customElementState = CustomElementState::Undefined;
        }
      }
    }
    else if (name.NamespaceURI == Namespace::SVG)
    {
      result = SVGElementFactory::TryCreate(document, name);
    }
    else if (name.NamespaceURI == Namespace::MathML)
    {
      result = MathMLElementFactory::TryCreate(document, name);
    }

    if (result == nullptr)
    {
      result = AdoptRefPtr<Element>(new Element(document, name, NodeFlags::None));
    }

    result->_qualifiedName = name;
    result->_customElementRegistry = registry.value();
    result->_is = is;

    return AdoptRef(*result.release());
  }
}