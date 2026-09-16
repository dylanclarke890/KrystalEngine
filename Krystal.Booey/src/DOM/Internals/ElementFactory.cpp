#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Enums/CustomElementState.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementDefinition.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLUnknownElement.hpp"
#include "Krystal.Booey/HTML/Internals/CustomElementFactory.hpp"
#include "Krystal.Booey/HTML/Internals/HTMLElementFactory.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/MathML/Internals/MathMLElementFactory.hpp"
#include "Krystal.Booey/MathML/MathMLElement.hpp"
#include "Krystal.Booey/SVG/Internals/SVGElementFactory.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGUnknownElement.hpp"

namespace krys::boo::dom
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

    auto elementName = html::ParseTagName(krys::text::ToASCIILower(name.LocalName().View()));
    if (name.NamespaceURI() == infra::Namespaces::HTML)
    {
      result = html::CustomElementFactory::TryCreate(document, name, is, synchronousCustomElements,
                                                     registry.value().get());

      if (result == nullptr)
      {
        result = html::HTMLElementFactory::TryCreate(document, elementName);
      }

      bool isValidCustomElementName = NameValidation::IsValidCustomElementName(name.LocalName().View());
      if (result == nullptr)
      {
        if (isValidCustomElementName)
        {
          result = CreateRefPtr<html::HTMLElement>(document);
        }
        else
        {
          result = CreateRefPtr<html::HTMLUnknownElement>(document);
        }
      }

      if (isValidCustomElementName || is != DOMStringAtom::Null())
      {
        result->_customElementState = CustomElementState::Undefined;
      }
    }
    else if (name.NamespaceURI() == infra::Namespaces::SVG)
    {
      result = svg::SVGElementFactory::TryCreate(document, elementName);

      if (result == nullptr)
      {
        result = CreateRefPtr<svg::SVGUnknownElement>(document);
      }
    }
    else if (name.NamespaceURI() == infra::Namespaces::MathML)
    {
      result = mathml::MathMLElementFactory::TryCreate(document, name);
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