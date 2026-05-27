#include "Krystal.HTML/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.HTML/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/ShadowRootAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/Constants/Namespaces.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/MutationObserver.hpp"
#include "Krystal.HTML/Factories/ElementFactory.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Comment.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ProcessingInstruction.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Node/XMLDocument.hpp"

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

      // TODO(impl): URL - Set the url on the cloned document to the url of the original document.
      // docCopy._baseURL = doc._baseURL;
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

  DOMStringAtom NodeAlgorithms::LocateNamespacePrefix(const Element &element,
                                                      DOMStringAtom namespaceURI) noexcept
  {
    if (element.NamespaceURI() == namespaceURI && element.Prefix() != DOMStringAtom::Null())
    {
      return element.Prefix();
    }

    for (auto &attribute : element._attributes)
    {
      if (attribute->Prefix() == NamespacePrefix::XMLNS && attribute->Value() == namespaceURI)
      {
        return attribute->LocalName();
      }
    }

    RawPtr<const Element> parentElement = element.ParentElement();
    if (parentElement != nullptr)
    {
      return LocateNamespacePrefix(*parentElement, namespaceURI);
    }

    return DOMStringAtom::Null();
  }

  DOMStringAtom NodeAlgorithms::LocateNamespace(const Node &node, DOMStringAtom prefix) noexcept
  {
    switch (node.NodeType())
    {
      case NodeType::ELEMENT_NODE:
      {
        if (prefix == NamespacePrefix::XML)
        {
          return Namespace::XML;
        }

        if (prefix == NamespacePrefix::XMLNS)
        {
          return Namespace::XMLNS;
        }

        auto &element = Downcast<Element>(node);
        if (element.NamespaceURI() != DOMStringAtom::Null() && element.Prefix() == prefix)
        {
          return element.NamespaceURI();
        }

        for (auto &attribute : element._attributes)
        {
          if (attribute->NamespaceURI() == Namespace::XMLNS && attribute->Prefix() == NamespacePrefix::XMLNS
              && attribute->LocalName() == prefix)
          {
            return attribute->Value().empty() ? DOMStringAtom::Null() : attribute->Value();
          }

          if (prefix == DOMStringAtom::Null() && attribute->NamespaceURI() == Namespace::XMLNS
              && attribute->Prefix() == DOMStringAtom::Null()
              && attribute->LocalName() == NamespacePrefix::XMLNS)
          {
            return attribute->Value().empty() ? DOMStringAtom::Null() : attribute->Value();
          }
        }

        RawPtr<const Element> parentElement = element.ParentElement();
        if (parentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return LocateNamespace(*parentElement, prefix);
      }
      case NodeType::DOCUMENT_NODE:
      {
        RefPtr<const Element> documentElement = Downcast<Document>(node).DocumentElement();
        if (documentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return LocateNamespace(*documentElement, prefix);
      }
      case NodeType::DOCUMENT_TYPE_NODE:
      case NodeType::DOCUMENT_FRAGMENT_NODE:
      {
        return DOMStringAtom::Null();
      }
      case NodeType::ATTRIBUTE_NODE:
      {
        RawPtr<const Element> ownerElement = Downcast<Attr>(node).OwnerElement();
        if (ownerElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return LocateNamespace(*ownerElement, prefix);
      }
      default:
      {
        RawPtr<const Element> parentElement = node.ParentElement();
        if (parentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return LocateNamespace(*parentElement, prefix);
      }
    }
  }

}