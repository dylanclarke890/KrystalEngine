#include "Krystal.HTML/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
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