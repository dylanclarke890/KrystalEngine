#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Enums/CustomElementState.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementInterface.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLMaybeFormAssociatedCustomElement.hpp"
#include "Krystal.Booey/HTML/HTMLUnknownElement.hpp"
#include "Krystal.Booey/HTML/Internals/HTMLElementFactory.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/MathML/Internals/MathMLElementFactory.hpp"
#include "Krystal.Booey/MathML/MathMLUnknownElement.hpp"
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

    // NOTE: customized built-in elements aren't supported, so a definition is only looked up by the local
    // name, and `is` doesn't make an element a custom element.
    auto *definition = html::CustomElementAlgorithms::LookupCustomElementDefinition(
      registry.value().get(), name.NamespaceURI(), name.LocalName());
    if (definition != nullptr)
    {
      if (synchronousCustomElements)
      {
        result = definition->ConstructElementWithFallback(document, *registry.value(), name);
      }
      else
      {
        auto element = definition->CreateElement(document);
        element->SetIsCustomElementUpgradeCandidate();
        element->EnqueueToUpgrade(*definition);
        result = element;
      }
    }
    else if (name.NamespaceURI() == infra::Namespaces::HTML)
    {
      // HTML's tag names are case-sensitive; createElement lowercases them in HTML documents.
      result = html::HTMLElementFactory::TryCreate(document, name.TagName());

      if (result == nullptr)
      {
        if (NameValidation::IsValidCustomElementName(name.LocalName().View()))
        {
          auto element = CreateRef<html::HTMLMaybeFormAssociatedCustomElement>(document, name.LocalName());
          element->SetIsCustomElementUpgradeCandidate();
          result = element;
        }
        else
        {
          result = CreateRefPtr<html::HTMLUnknownElement>(document);
        }
      }
    }
    else if (name.NamespaceURI() == infra::Namespaces::SVG)
    {
      // SVG's tag names are case-sensitive.
      result = svg::SVGElementFactory::TryCreate(document, name.TagName());

      if (result == nullptr)
      {
        result = CreateRefPtr<svg::SVGUnknownElement>(document);
      }
    }
    else if (name.NamespaceURI() == infra::Namespaces::MathML)
    {
      // MathML's tag names are case-sensitive.
      result = mathml::MathMLElementFactory::TryCreate(document, name.TagName());

      if (result == nullptr)
      {
        result = CreateRefPtr<mathml::MathMLUnknownElement>(document);
      }
    }

    if (result == nullptr)
    {
      result = AdoptRefPtr<Element>(new Element(document, name, NodeFlags::None));
    }

    result->_qualifiedName = name;
    result->_customElementRegistry = registry.value();
    if (registry.value() != nullptr && registry.value()->IsScoped())
    {
      registry.value()->AppendToScopedDocumentSet(document);
    }
    result->_is = is;

    return AdoptRef(*result.release());
  }
}