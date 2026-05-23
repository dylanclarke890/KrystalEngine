#include "Krystal.HTML/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.HTML/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/ShadowRootAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Factories/ElementFactory.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Comment.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ProcessingInstruction.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Node/XMLDocument.hpp"
#include "Krystal.HTML/Utils/SubtreeRanges.hpp"

namespace Krys::HTML
{

  ExceptionOr<void> NodeAlgorithms::StringReplaceAll(DOMString &&string, ContainerNode &parent) noexcept
  {
    if (string.empty())
    {
      return MutationAlgorithms::ReplaceAll(nullptr, parent);
    }
    else
    {
      Ref<Text> textNode = CreateRef<Text>(parent.NodeDocument(), Krys::Move(string));
      return MutationAlgorithms::ReplaceAll(textNode.get(), parent);
    }
  }

  ExceptionOr<Ref<Node>> NodeAlgorithms::CloneNode(Node &node, RawPtr<Document> document, bool subtree,
                                                   RawPtr<ContainerNode> parent,
                                                   RawPtr<CustomElementRegistry> fallbackRegistry) noexcept
  {
    if (document == nullptr)
    {
      document = &node.NodeDocument();
    }

    assert(!Is<Document>(node) || &node == document);

    auto copy = CloneSingleNode(node, *document, fallbackRegistry);
    ExtensibilityHooks::NodeCloned(node, *copy, subtree);

    if (parent != nullptr)
    {
      parent->AppendChild(*copy);
    }

    if (subtree)
    {
      if (auto *containerNode = DynamicDowncast<ContainerNode>(node))
      {
        for (auto &child : ChildNodeRange(*containerNode))
        {
          if (auto childCloneResult =
                CloneNode(child, document, subtree, Downcast<ContainerNode>(copy.get()), fallbackRegistry);
              childCloneResult.HasException())
          {
            return childCloneResult.ReleaseException();
          }
        }
      }
    }

    if (auto *element = DynamicDowncast<Element>(node))
    {
      auto *elementCopy = Downcast<Element>(copy.get());

      if (auto shadowRoot = element->ShadowRoot(); shadowRoot != nullptr && shadowRoot->Clonable())
      {
        assert(elementCopy->ShadowRoot() == nullptr);

        auto shadowRootRegistry = shadowRoot->CustomElementRegistry();
        if (CustomElementAlgorithms::IsGlobalCustomElementRegistry(shadowRootRegistry.get()))
        {
          shadowRootRegistry =
            ShareRefPtr(CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(*document));
        }

        if (auto attachShadowResult = ShadowRootAlgorithms::AttachShadowRoot(
              *elementCopy, shadowRoot->Mode(), Clonable(true), Serializable(shadowRoot->Serializable()),
              DelegatesFocus(shadowRoot->DelegatesFocus()), shadowRoot->SlotAssignment(),
              shadowRootRegistry.get());
            attachShadowResult.HasException())
        {
          return attachShadowResult.ReleaseException();
        }

        auto shadowRootCopy = elementCopy->ShadowRoot();
        shadowRootCopy->_declarative = shadowRoot->_declarative;
        shadowRootCopy->_keepCustomElementRegistryNull = shadowRoot->_keepCustomElementRegistryNull;

        for (auto &child : ChildNodeRange(*shadowRoot))
        {
          if (auto childCloneResult = CloneNode(child, document, subtree, shadowRootCopy.get());
              childCloneResult.HasException())
          {
            return childCloneResult.ReleaseException();
          }
        }
      }
    }

    return copy;
  }

  Ref<Node> NodeAlgorithms::CloneSingleNode(const Node &node, Document &document,
                                            RawPtr<CustomElementRegistry> fallbackRegistry) noexcept
  {
    RefPtr<Node> copy = nullptr;

    if (auto *element = DynamicDowncast<Element>(node))
    {
      auto registry = element->CustomElementRegistry();
      if (registry == nullptr)
      {
        registry = ShareRefPtr(fallbackRegistry);
      }

      if (CustomElementAlgorithms::IsGlobalCustomElementRegistry(registry.get()))
      {
        registry = ShareRefPtr(CustomElementAlgorithms::EffectiveGlobalCustomElementRegistry(document));
      }

      copy = ElementFactory::Create(document, element->_qualifiedName, element->_is, false, registry);

      for (auto &attr : element->_attributes)
      {
        auto attrCopy = AdoptRef<Attr>(*new Attr(document, attr->_name, DOMString(attr->Value())));
        ElementAlgorithms::AppendAttribute(*attr, Downcast<Element>(*copy));
      }
    }
    else if (Is<Document>(node))
    {
      auto &doc = Downcast<Document>(node);
      if (Is<HTMLDocument>(doc))
      {
        copy = CreateRef<HTMLDocument>();
      }
      else
      {
        copy = CreateRef<XMLDocument>();
      }
      auto &docCopy = Downcast<Document>(*copy);

      // TODO(feat): MINOR - Set copy's encoding and origin.
      docCopy._contentType = doc._contentType;
      docCopy._baseURL = doc._baseURL;
      docCopy._quirksMode = doc._quirksMode;
      docCopy._allowDeclarativeShadowRoots = doc._allowDeclarativeShadowRoots;
    }
    else if (Is<DocumentType>(node))
    {
      auto &docType = Downcast<const DocumentType>(node);
      copy = CreateRef<DocumentType>(document, docType.Name(), docType.PublicId(), docType.SystemId());
    }
    else if (Is<Attr>(node))
    {
      auto &attr = Downcast<const Attr>(node);
      copy = AdoptRef<Attr>(*new Attr(document, attr._name, DOMString(attr.Value())));
    }
    else if (Is<Text>(node))
    {
      auto &text = Downcast<const Text>(node);
      copy = CreateRef<Text>(document, DOMString(text.Data()));
    }
    else if (Is<Comment>(node))
    {
      auto &comment = Downcast<const Comment>(node);
      copy = CreateRef<Comment>(document, DOMString(comment.Data()));
    }
    else if (Is<ProcessingInstruction>(node))
    {
      auto &instruction = Downcast<const ProcessingInstruction>(node);
      copy = CreateRef<ProcessingInstruction>(document, DOMString(instruction.Target()),
                                              DOMString(instruction.Data()));
    }

    assert(copy);
    return copy;
  }
}