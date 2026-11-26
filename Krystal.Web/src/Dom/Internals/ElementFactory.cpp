#include "Krystal.Web/Dom/Internals/ElementFactory.hpp"
#include "Krystal.Web/Dom/Algorithms/NameValidation.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Html/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementInterface.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLMaybeFormAssociatedCustomElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLUnknownElement.hpp"
#include "Krystal.Web/Html/Internals/HTMLElementFactory.hpp"
#include "Krystal.Web/Infra/Namespaces.hpp"
#include "Krystal.Web/MathML/Elements/MathMLUnknownElement.hpp"
#include "Krystal.Web/MathML/Internals/MathMLElementFactory.hpp"
#include "Krystal.Web/Svg/Elements/SVGElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGUnknownElement.hpp"
#include "Krystal.Web/Svg/Internals/SVGElementFactory.hpp"

namespace krys::web::dom
{
  Ref<Element> ElementFactory::Create(Document &document, const QualifiedName &name, DOMStringAtom is,
                                      bool synchronousCustomElements,
                                      CustomElementRegistryOrDefault registry) noexcept
  {
    // 1. Let result be null.
    RefPtr<Element> result = nullptr;

    // 2. If registry is "default", then set registry to the result of looking up a custom element registry
    //    given document.
    if (registry == DefaultCustomElementRegistry)
    {
      registry = document.CustomElementRegistry();
    }

    // 3. Let definition be the result of looking up a custom element definition given registry, namespace,
    //    localName, and is. (Customized built-in elements aren't supported, so it's looked up by the local
    //    name alone.)
    auto *definition = html::CustomElementAlgorithms::LookupCustomElementDefinition(
      registry.value(), name.NamespaceURI(), name.LocalName());
    // 4. (Skipped: customized built-in elements aren't supported, so definition's name is always its local
    //    name.)
    // 5. Otherwise, if definition is non-null:
    if (definition != nullptr)
    {
      // 5a. If synchronousCustomElements is true:
      //     (Constructing C is calling the definition's factory, which checks steps 5a.iv.D-G, with the
      //     element it's given; on failure, result is an HTMLUnknownElement whose state is "failed". There's
      //     no active custom element constructor map, as there are no JavaScript constructors.)
      if (synchronousCustomElements)
      {
        result = definition->ConstructElementWithFallback(document, *registry.value(), name);
      }
      // 5b. Otherwise:
      else
      {
        // 5b.i. Set result to the result of creating an element internal given document, HTMLElement,
        //       localName, the HTML namespace, prefix, "undefined", null, and registry.
        auto element = definition->CreateElement(document);
        element->SetIsCustomElementUpgradeCandidate();
        // 5b.ii. Enqueue a custom element upgrade reaction given result and definition.
        element->EnqueueToUpgrade(*definition);
        result = element;
      }
    }
    // 6. Otherwise:
    // 6a. Let interface be the element interface for localName and namespace.
    // 6b. Set result to the result of creating an element internal given document, interface, localName,
    //     namespace, prefix, "uncustomized", is, and registry.
    else if (name.NamespaceURI() == infra::namespaces::HTML)
    {
      // HTML's tag names are case-sensitive; createElement lowercases them in HTML documents.
      result = html::HTMLElementFactory::TryCreate(document, name.TagName());

      if (result == nullptr)
      {
        // 6c. If namespace is the HTML namespace, and either localName is a valid custom element name or is
        //     is non-null, then set result's custom element state to "undefined". (Customized built-in
        //     elements aren't supported, so is doesn't count.)
        if (NameValidation::IsValidCustomElementName(name.LocalName().View()))
        {
          auto element = CreateRef<HTMLMaybeFormAssociatedCustomElement>(document, name.LocalName());
          element->SetIsCustomElementUpgradeCandidate();
          result = element;
        }
        else
        {
          result = CreateRefPtr<HTMLUnknownElement>(document);
        }
      }
    }
    else if (name.NamespaceURI() == infra::namespaces::SVG)
    {
      // SVG's tag names are case-sensitive.
      result = SVGElementFactory::TryCreate(document, name.TagName());

      if (result == nullptr)
      {
        result = CreateRefPtr<SVGUnknownElement>(document);
      }
    }
    else if (name.NamespaceURI() == infra::namespaces::MathML)
    {
      // MathML's tag names are case-sensitive.
      result = MathMLElementFactory::TryCreate(document, name.TagName());

      if (result == nullptr)
      {
        result = CreateRefPtr<MathMLUnknownElement>(document);
      }
    }

    if (result == nullptr)
    {
      result = AdoptRefPtr<Element>(new Element(document, name, {}));
    }

    // Create an element internal's step 2, and steps 5a.iv.I-K: the element's namespace, prefix and local
    // name, custom element registry, and is value (null for a custom element).
    result->_qualifiedName = name;
    result->_customElementRegistry = ShareRefPtr(registry.value());
    if (registry.value() != nullptr && registry.value()->IsScoped())
    {
      registry.value()->AppendToScopedDocumentSet(document);
    }
    result->_is = definition != nullptr ? null : krys::move(is);

    // 7. Return result.
    return AdoptRef(*result.release());
  }
}
