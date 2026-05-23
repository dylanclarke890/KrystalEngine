#include "Krystal.HTML/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Factories/ElementFactory.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Utils/SubtreeRanges.hpp"

namespace Krys::HTML
{
  ExceptionOr<ElementCreationOptions>
    DocumentAlgorithms::FlattenElementCreationOptions(const ElementCreationOptionsOrString &options,
                                                      Document &document) noexcept
  {
    if (std::holds_alternative<ElementCreationOptions>(options))
    {
      auto &dict = std::get<ElementCreationOptions>(options);
      if (dict.CustomElementRegistry != nullptr && dict.Is != DOMStringAtom::Null())
      {
        return Exception {ExceptionCode::NotSupportedError};
      }

      if (dict.CustomElementRegistry != nullptr && !dict.CustomElementRegistry->IsScoped()
          && dict.CustomElementRegistry != document.CustomElementRegistry())
      {
        return Exception {ExceptionCode::NotSupportedError};
      }

      return dict;
    }

    return ElementCreationOptions {.CustomElementRegistry = document.CustomElementRegistry(),
                                   .Is = DOMStringAtom::Null()};
  }

  ExceptionOr<Ref<Element>>
    DocumentAlgorithms::InternalCreateElementNS(Document &document, DOMStringAtom namespaceUri,
                                                DOMStringAtom qualifiedName,
                                                const ElementCreationOptionsOrString &options) noexcept
  {
    auto name =
      NameValidation::ValidateAndExtract(namespaceUri, qualifiedName, ValidateAndExtractContext::Element);
    if (name.HasException())
    {
      return name.ReleaseException();
    }

    auto creationOptions = DocumentAlgorithms::FlattenElementCreationOptions(options, document);
    if (creationOptions.HasException())
    {
      return creationOptions.ReleaseException();
    }

    return ElementFactory::Create(
      document, {name.Value().NamespaceURI, name.Value().Prefix, name.Value().LocalName},
      creationOptions.Value().Is, true, creationOptions.Value().CustomElementRegistry);
  }

  ExceptionOr<void> DocumentAlgorithms::AdoptNode(Node &node, Document &document) noexcept
  {
    auto &oldDocument = node.NodeDocument();
    if (node.ParentNode() != nullptr)
    {
      if (auto result = MutationAlgorithms::Remove(node); result.HasException())
      {
        return result.ReleaseException();
      }
    }

    if (&document != &oldDocument)
    {
      auto documentEffectiveGlobalRegistry = document.CustomElementRegistry();

      for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(node))
      {
        inclusiveDescendant._ownerDocument = ShareRefPtr(&document);
        if (Is<ShadowRoot>(inclusiveDescendant))
        {
          auto &shadowRootInclusiveDescendant = Downcast<ShadowRoot>(inclusiveDescendant);
          auto registry = shadowRootInclusiveDescendant.CustomElementRegistry();
          if (registry == nullptr && !shadowRootInclusiveDescendant._keepCustomElementRegistryNull)
          {
            shadowRootInclusiveDescendant._customElementRegistry = documentEffectiveGlobalRegistry;
          }
          else if (CustomElementAlgorithms::IsGlobalCustomElementRegistry(registry.get()))
          {
            shadowRootInclusiveDescendant._customElementRegistry = documentEffectiveGlobalRegistry;
          }
        }
        else if (Is<Element>(inclusiveDescendant))
        {
          auto &elementInclusiveDescendant = Downcast<Element>(inclusiveDescendant);
          for (auto &attr : elementInclusiveDescendant._attributes)
          {
            attr->_ownerDocument = ShareRefPtr(&document);
          }

          auto registry = elementInclusiveDescendant.CustomElementRegistry();
          if (registry == nullptr || !registry->IsScoped())
          {
            elementInclusiveDescendant._customElementRegistry = documentEffectiveGlobalRegistry;
          }
        }
      }

      // TODO(impl): CUSTOM-ELEMENTS
      // For each inclusiveDescendant of node’s shadow-including inclusive descendants that is custom, in
      // shadow-including tree order: enqueue a custom element callback reaction with inclusiveDescendant,
      // callback name "adoptedCallback", and « oldDocument, document ».

      for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(node))
      {
        ExtensibilityHooks::NodeAdopted(inclusiveDescendant, oldDocument);
      }
    }

    return {};
  }
}