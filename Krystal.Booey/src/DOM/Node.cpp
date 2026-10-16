#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeAncestorIterator.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"

namespace krys::boo::dom
{
  Node::Node(Document &document, dom::NodeType type, NodeFlags flags) noexcept
      : EventTarget(EventTargetFlags::IsNode), _nodeType(type), _nodeDocument(ShareRefPtr(&document)),
        _parentNode(nullptr), _previousSibling(nullptr), _nextSibling(nullptr)
  {
    _flags = flags;
  }

#pragma region Node

  DOMString Node::BaseURI() const noexcept
  {
    return NodeDocument().BaseURL().String();
  }

  Document *Node::OwnerDocument() const noexcept
  {
    Document &document = NodeDocument();
    return &document == this ? nullptr : &document;
  }

  Node &Node::GetRootNode(const GetRootNodeOptions &options) noexcept
  {
    return options.Composed ? TreeQueries::ShadowIncludingRoot(*this) : TreeQueries::Root(*this);
  }

  Element *Node::ParentElement() const noexcept
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

  Node *Node::FirstChild() const noexcept
  {
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return containerNode->FirstChild();
    }

    return nullptr;
  }

  Node *Node::LastChild() const noexcept
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

      const Node *current = node->NextSibling();
      while (TreeQueries::IsExclusiveTextNode(current))
      {
        data += *Downcast<Text>(current)->TextContent();
        current = current->NextSibling();
      }

      if (auto replaceResult = node->ReplaceData(length, 0uz, krys::move(data)); replaceResult.HasException())
      {
        return replaceResult.ReleaseException();
      }

      Node *currentNode = node->NextSibling();
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

  bool Node::IsEqualNode(const Node *otherNode) const noexcept
  {
    if (otherNode == nullptr)
    {
      return false;
    }

    return NodeAlgorithms::Equals(*this, *otherNode);
  }

  bool Node::IsSameNode(const Node *otherNode) const noexcept
  {
    return this == otherNode;
  }

  DocumentPosition Node::CompareDocumentPosition(const Node &other) const noexcept
  {
    if (this == &other)
    {
      return DocumentPosition::DOCUMENT_POSITION_EQUIVALENT;
    }

    const Node *node1 = &other;
    const Node *node2 = this;

    const Attr *attr1 = nullptr;
    const Attr *attr2 = nullptr;

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

        (void)element1;
        // TODO: are we missing part of the implementation here? element1 is not referenced
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

  bool Node::Contains(const Node *other) const noexcept
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
        const Element *ownerElement = Downcast<Attr>(*this).OwnerElement();
        if (ownerElement == nullptr)
        {
          return DOMStringAtom::Null();
        }

        return NodeAlgorithms::LocateNamespacePrefix(*ownerElement, namespaceURI);
      }
      default:
      {
        const Element *parentElement = ParentElement();
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

  ExceptionOr<Node &> Node::InsertBefore(Node &newChild, Node *refChild) noexcept
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

#pragma region Style

  const RenderStyle *Node::GetRenderStyle() const noexcept
  {
    auto *renderer = Renderer();
    if (!renderer)
    {
      return nullptr;
    }
    return &renderer->Style();
  }

  const RenderStyle *Node::ComputedStyle() noexcept
  {
    return ComputedStyle(null);
  }

  const RenderStyle *
    Node::ComputedStyle(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    auto composedParent = ShareRefPtr(ParentElementInComposedTree());
    return composedParent ? composedParent->ComputedStyle(pseudoElementIdentifier) : nullptr;
  }

  ContainerNode *Node::ParentInComposedTree() const noexcept
  {
    if (auto *slot = SlotAlgorithms::GetAssignedSlot(*this))
    {
      return slot;
    }
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(*this))
    {
      return shadowRoot->Host();
    }
    return ParentNode();
  }

  Element *Node::ParentElementInComposedTree() const noexcept
  {
    if (auto *slot = SlotAlgorithms::GetAssignedSlot(*this))
    {
      return slot;
    }
    if (auto *pseudoElement = DynamicDowncast<PseudoElement>(*this))
    {
      return pseudoElement->HostElement();
    }
    if (auto *parent = ParentNode())
    {
      if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(*parent))
      {
        return shadowRoot->Host();
      }
      if (auto *element = DynamicDowncast<Element>(*parent))
      {
        return element;
      }
    }
    return nullptr;
  }

  bool Node::IsComposedTreeDescendantOf(const Node &node) const noexcept
  {
    for (auto *currentAncestor = ParentElementInComposedTree(); currentAncestor;
         currentAncestor = currentAncestor->ParentElementInComposedTree())
    {
      if (currentAncestor == &node)
      {
        return true;
      }
    }
    return false;
  }

  void Node::AdjustStyleValidity(style::Validity validity, style::InvalidationMode mode) noexcept
  {
    if (validity > StyleValidity())
    {
      _styleValidity = validity;
    }

    switch (mode)
    {
      case style::InvalidationMode::Normal: break;
      case style::InvalidationMode::RecompositeLayer:
        SetNodeFlag(NodeFlags::StyleResolutionShouldRecompositeLayer);
        break;
      case style::InvalidationMode::RebuildRenderer:
      case style::InvalidationMode::InsertedIntoAncestor: SetNodeFlag(NodeFlags::HasInvalidRenderer); break;
    }
  }

  void Node::UpdateAncestorsForStyleRecalc() noexcept
  {
    MarkAncestorsForInvalidatedStyle();

    auto documentElement = NodeDocument().DocumentElement();
    if (!documentElement)
    {
      return;
    }
    if (!documentElement->ChildNeedsStyleRecalc() && !documentElement->NeedsStyleRecalc())
    {
      return;
    }

    auto document = ShareRef(NodeDocument());
    document->SetChildNeedsStyleRecalc();
    document->ScheduleStyleRecalc();
  }

  void Node::MarkAncestorsForInvalidatedStyle() noexcept
  {
    auto composedAncestors = ComposedTreeAncestors(*this);
    auto it = composedAncestors.begin();
    auto end = composedAncestors.end();
    if (it != end)
    {
      it->SetDirectChildNeedsStyleRecalc();

      for (; it != end; ++it)
      {
        // Iterator skips over shadow roots.
        if (auto *shadowRoot = TreeQueries::GetShadowRoot(*it))
        {
          shadowRoot->SetChildNeedsStyleRecalc();
        }
        if (it->ChildNeedsStyleRecalc())
        {
          break;
        }
        it->SetChildNeedsStyleRecalc();
      }
    }
  }

  void Node::InvalidateStyle(style::Validity validity, style::InvalidationMode mode) noexcept
  {
    // FIXME: Every connected node is in a rendered document; documents without a browsing context are styled
    // too.
    if (!IsConnected())
    {
      return;
    }

    // FIXME: This should eventually be an ASSERT.
    if (NodeDocument().InRenderTreeUpdate())
    {
      return;
    }

    if (validity != style::Validity::Valid)
    {
      SetNodeFlag(NodeFlags::IsComputedStyleInvalid);
    }

    bool markAncestors =
      StyleValidity() == style::Validity::Valid || mode == style::InvalidationMode::InsertedIntoAncestor;

    AdjustStyleValidity(validity, mode);

    if (markAncestors)
    {
      UpdateAncestorsForStyleRecalc();
    }
  }

#pragma endregion

  EventTarget *Node::GetParent(Event &event) const noexcept
  {
    (void)event;

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