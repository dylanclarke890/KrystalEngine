#include "Krystal.HTML/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/Factories/ElementFactory.hpp"
#include "Krystal.HTML/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

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

    return ElementFactory::CreateElement(
      document, {name.Value().NamespaceURI, name.Value().Prefix, name.Value().LocalName},
      creationOptions.Value().Is, true, creationOptions.Value().CustomElementRegistry);
  }

  ExceptionOr<void> DocumentAlgorithms::AdoptNode(Node &node, Document &document) noexcept
  {
    if (Is<Attr>(node))
    {
      auto &attr = Downcast<Attr>(node);
      RefPtr<Element> element = ShareRefPtr(attr.OwnerElement());
      if (element != nullptr)
      {
        if (auto result = element->RemoveAttributeNode(attr); result.HasException())
        {
          return result.ReleaseException();
        }
      }
    }
    else
    {
      if (auto oldParent = ShareRefPtr(node.ParentNode()))
      {
        if (auto result = oldParent->RemoveChild(node); result.HasException())
        {
          return {result.ReleaseException()};
        }
      }

      assert(!node.IsConnected());
      assert(!node.ParentNode());
    }

    return {};
  }
}