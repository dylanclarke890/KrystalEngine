#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/ShadowRootAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/CharacterData.hpp"
#include "Krystal.HTML/DOM/ContainerNode.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.HTML/DOM/ProcessingInstruction.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  Node::Node(Document &document, HTML::NodeType type, NodeFlags flags) noexcept
      : EventTarget(EventTargetFlags::IsNode), _nodeType(type), _nodeDocument(ShareRefPtr(&document)),
        _parentNode(nullptr), _previousSibling(nullptr), _nextSibling(nullptr)
  {
    _flags = flags;
  }

#pragma region Node

  DOMString Node::BaseURI() const noexcept
  {
    // TODO(impl): URL - The baseURI getter steps are to return this’s node document’s document base URL,
    // serialized.
    // @see https://html.spec.whatwg.org/multipage/urls-and-fetching.html#document-base-url
    return {};
  }

  RawPtr<Document> Node::OwnerDocument() const noexcept
  {
    Document &document = NodeDocument();
    return &document == this ? nullptr : &document;
  }

  Node &Node::GetRootNode(const GetRootNodeOptions &options) noexcept
  {
    return options.Composed ? ShadowRootAlgorithms::ShadowIncludingRoot(*this) : TreeQueries::Root(*this);
  }

  RawPtr<Element> Node::ParentElement() const noexcept
  {
    return DynamicDowncast<Element>(ParentNode());
  }

  bool Node::HasChildNodes() const noexcept
  {
    return FirstChild() != nullptr;
  }

  Ref<NodeList> Node::ChildNodes() noexcept
  {
    if (_nodeRareData == nullptr)
    {
      _nodeRareData = CreateUnique<NodeRareData>();
    }

    return _nodeRareData->ChildNodes(*this);
  }

  RawPtr<Node> Node::FirstChild() const noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->FirstChild();
    }

    return nullptr;
  }

  RawPtr<Node> Node::LastChild() const noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->LastChild();
    }

    return nullptr;
  }

  ExceptionOr<void> Node::Normalize() noexcept
  {
    for (auto *node = TreeQueries::NextExclusiveTextNode(*this, this); node != nullptr;
         node = TreeQueries::NextExclusiveTextNode(*node, this))
    {
      auto length = node->Data().size();
      if (length == 0uz)
      {
        if (auto removeResult = MutationAlgorithms::Remove(*node); removeResult.HasException())
        {
          return removeResult.ReleaseException();
        }

        continue;
      }

      DOMString data {};

      RawPtr<const Node> current = node->NextSibling();
      while (TreeQueries::IsExclusiveTextNode(current))
      {
        data += *Downcast<Text>(current)->TextContent();
        current = current->NextSibling();
      }

      if (auto replaceResult = node->ReplaceData(length, 0uz, Krys::Move(data)); replaceResult.HasException())
      {
        return replaceResult.ReleaseException();
      }

      RawPtr<Node> currentNode = node->NextSibling();
      while (TreeQueries::IsExclusiveTextNode(currentNode))
      {
        auto &currentTextNode = Downcast<Text>(*currentNode);
        LiveRangeUpdater::NodeNormalized(*this, currentTextNode, length);

        length += currentTextNode.Data().size();
        currentNode = currentTextNode.NextSibling();
      }

      while (TreeQueries::IsExclusiveTextNode(node->NextSibling()))
      {
        if (auto removeResult = MutationAlgorithms::Remove(*node->NextSibling()); removeResult.HasException())
        {
          return removeResult.ReleaseException();
        }
      }
    }

    return {};
  }

  ExceptionOr<Ref<Node>> Node::CloneNode(bool subtree) noexcept
  {
    if (Is<ShadowRoot>(this))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    return NodeAlgorithms::CloneNode(*this, nullptr, subtree);
  }

  bool Node::IsEqualNode(RawPtr<const Node> otherNode) const noexcept
  {
    if (otherNode == nullptr)
    {
      return false;
    }

    return NodeAlgorithms::Equals(*this, *otherNode);
  }

  bool Node::IsSameNode(RawPtr<const Node> otherNode) const noexcept
  {
    return this == otherNode;
  }

  DocumentPosition Node::CompareDocumentPosition(const Node &other) const noexcept
  {
    if (this == &other)
    {
      return DocumentPosition::DOCUMENT_POSITION_EQUIVALENT;
    }

    RawPtr<const Node> node1 = &other;
    RawPtr<const Node> node2 = this;

    RawPtr<const Attr> attr1 = nullptr;
    RawPtr<const Attr> attr2 = nullptr;

    if (Is<Attr>(node1))
    {
      attr1 = Downcast<Attr>(node1);
      node1 = attr1->OwnerElement();
    }

    if (Is<Attr>(node2))
    {
      attr2 = Downcast<Attr>(node2);
      node2 = attr2->OwnerElement();

      if (attr1 != nullptr && node2 == node1)
      {
        auto &element1 = Downcast<Element>(*node1);
        auto &element2 = Downcast<Element>(*node2);

        for (auto &attribute : element2._attributes)
        {
          if (NodeAlgorithms::Equals(*attribute, *attr1))
          {
            return DocumentPosition::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC
                   | DocumentPosition::DOCUMENT_POSITION_PRECEDING;
          }

          if (NodeAlgorithms::Equals(*attribute, *attr2))
          {
            return DocumentPosition::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC
                   | DocumentPosition::DOCUMENT_POSITION_FOLLOWING;
          }
        }
      }
    }

    if (node1 == nullptr || node2 == nullptr || !TreeQueries::SameRoot(*node1, *node2))
    {
      auto orderingFlag = node1 < node2 ? DocumentPosition::DOCUMENT_POSITION_PRECEDING
                                        : DocumentPosition::DOCUMENT_POSITION_FOLLOWING;

      return DocumentPosition::DOCUMENT_POSITION_DISCONNECTED
             | DocumentPosition::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC | orderingFlag;
    }

    if ((TreeQueries::IsAncestor(*node1, *node2) && attr1 == nullptr) || (node1 == node2 && attr2 != nullptr))
    {
      return DocumentPosition::DOCUMENT_POSITION_CONTAINS | DocumentPosition::DOCUMENT_POSITION_PRECEDING;
    }

    if ((TreeQueries::IsDescendant(*node1, *node2) && attr2 == nullptr)
        || (node1 == node2 && attr1 != nullptr))
    {
      return DocumentPosition::DOCUMENT_POSITION_CONTAINED_BY | DocumentPosition::DOCUMENT_POSITION_FOLLOWING;
    }

    if (TreeQueries::IsPreceding(*node1, *node2))
    {
      return DocumentPosition::DOCUMENT_POSITION_PRECEDING;
    }

    return DocumentPosition::DOCUMENT_POSITION_FOLLOWING;
  }

  bool Node::Contains(RawPtr<const Node> other) const noexcept
  {
    if (other == nullptr)
    {
      return false;
    }

    return TreeQueries::IsInclusiveDescendant(*other, *this);
  }

  DOMStringAtom Node::LookupPrefix(DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == DOMStringAtom::Null() || namespaceURI == DOMStringAtom::Empty())
    {
      return DOMStringAtom::Null();
    }

    switch (NodeType())
    {
      case NodeType::ELEMENT_NODE:
      {
        return NodeAlgorithms::LocateNamespacePrefix(Downcast<Element>(*this), namespaceURI);
      }
      case NodeType::DOCUMENT_NODE:
      {
        auto *documentElement = TreeQueries::DocumentElement(Downcast<Document>(*this));
        if (documentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return NodeAlgorithms::LocateNamespacePrefix(*documentElement, namespaceURI);
      }
      case NodeType::DOCUMENT_TYPE_NODE:
      case NodeType::DOCUMENT_FRAGMENT_NODE:
      {
        return DOMStringAtom::Null();
      }
      case NodeType::ATTRIBUTE_NODE:
      {
        RawPtr<const Element> ownerElement = Downcast<Attr>(*this).OwnerElement();
        if (ownerElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return NodeAlgorithms::LocateNamespacePrefix(*ownerElement, namespaceURI);
      }
      default:
      {
        RawPtr<const Element> parentElement = ParentElement();
        if (parentElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return NodeAlgorithms::LocateNamespacePrefix(*parentElement, namespaceURI);
      }
    }
  }

  DOMStringAtom Node::LookupNamespaceURI(DOMStringAtom prefix) const noexcept
  {
    if (prefix == DOMStringAtom::Empty())
    {
      prefix = DOMStringAtom::Null();
    }

    return NodeAlgorithms::LocateNamespace(*this, prefix);
  }

  bool Node::IsDefaultNamespace(DOMStringAtom namespaceURI) const noexcept
  {
    if (namespaceURI == DOMStringAtom::Empty())
    {
      namespaceURI = DOMStringAtom::Null();
    }

    auto defaultNamespace = NodeAlgorithms::LocateNamespace(*this, DOMStringAtom::Null());
    return namespaceURI == defaultNamespace;
  }

  ExceptionOr<Node &> Node::InsertBefore(Node &newChild, RawPtr<Node> refChild) noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return MutationAlgorithms::PreInsert(newChild, *containerNode, refChild);
    }

    return Exception {ExceptionCode::HierarchyRequestError};
  }

  ExceptionOr<Node &> Node::ReplaceChild(Node &newChild, Node &oldChild) noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return MutationAlgorithms::Replace(oldChild, newChild, *containerNode);
    }

    return Exception {ExceptionCode::HierarchyRequestError};
  }

  ExceptionOr<Node &> Node::RemoveChild(Node &oldChild) noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return MutationAlgorithms::PreRemove(oldChild, *containerNode);
    }

    return Exception {ExceptionCode::NotFoundError};
  }

  ExceptionOr<Node &> Node::AppendChild(Node &newChild) noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return MutationAlgorithms::Append(newChild, *containerNode);
    }

    return Exception {ExceptionCode::HierarchyRequestError};
  }

#pragma endregion

  RawPtr<EventTarget> Node::GetParent(Event &event) const noexcept
  {
    if (SlotAlgorithms::IsAssigned(*this))
    {
      return SlotAlgorithms::GetAssignedSlot(*this);
    }

    return ParentNode();
  }

  List<Ref<RegisteredObserver>> &Node::RegisteredObservers() noexcept
  {
    if (_nodeRareData == nullptr)
    {
      _nodeRareData = CreateUnique<NodeRareData>();
    }

    return _nodeRareData->RegisteredObservers();
  }

  List<Ref<TransientRegisteredObserver>> &Node::TransientRegisteredObservers() noexcept
  {
    if (_nodeRareData == nullptr)
    {
      _nodeRareData = CreateUnique<NodeRareData>();
    }

    return _nodeRareData->TransientRegisteredObservers();
  }
}