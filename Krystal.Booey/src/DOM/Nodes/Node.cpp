#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/DOM/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Internals/ElementRareData.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeAncestorIterator.hpp"
#include "Krystal.Booey/DOM/MutationObservers/TransientRegisteredObserver.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/CharacterData.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentType.hpp"
#include "Krystal.Booey/DOM/Nodes/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Nodes/PseudoElement.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomStateSet.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/Input/EventHandler.hpp"
#include "Krystal.Booey/Input/MouseEventTypes.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Booey/SVG/Elements/SVGUseElement.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"
#include <tuple>

namespace krys::boo
{
  Node::Node(Document &document, dom::NodeType type, dom::NodeFlags flags) noexcept
      : EventTarget(dom::EventTargetFlag::IsNode), _nodeType(type), _nodeDocument(&document),
        _parentNode(nullptr), _previousSibling(nullptr), _nextSibling(nullptr)
  {
    _nodeFlags = flags;
    if (type != dom::NodeType::DOCUMENT_NODE)
    {
      document.IncrementReferencingNodeCount();
    }
  }

  Node::~Node() noexcept
  {
    // willBeDeletedFrom: the document's accessibility cache lets go of the node's object. (A document's own
    // cache is gone by now.)
    if (!IsDocumentNode())
    {
      if (auto *cache = _nodeDocument->ExistingAXObjectCache())
      {
        cache->Remove(*this);
      }
    }

    // Before releasing the document, which what the rare data holds may refer to.
    _nodeRareData = nullptr;

    // Last, since it may delete the document.
    if (!IsDocumentNode())
    {
      _nodeDocument->DecrementReferencingNodeCount();
    }
  }

  void Node::InvalidateLiveCollectionCaches() noexcept
  {
    if (_nodeRareData)
    {
      _nodeRareData->InvalidateLiveCollectionCaches();
    }
  }

  void Node::InvalidateLiveCollectionCachesInAncestors() noexcept
  {
    if (auto &document = NodeDocument(); document.HasLiveCollectionCaches())
    {
      for (auto *node = this; node; node = node->ParentNode())
      {
        node->InvalidateLiveCollectionCaches();
      }
      document.InvalidateOwnerTreeLiveCollectionCaches();
    }
  }

  void Node::InvalidateLiveCollectionCachesForChildChange(const Node &child) noexcept
  {
    if (Is<Element>(child))
    {
      InvalidateLiveCollectionCachesInAncestors();
    }
    else if (_nodeRareData)
    {
      _nodeRareData->InvalidateChildNodeListCache();
    }
  }

  void Node::InvalidateLiveCollectionCachesForAttribute(const DOMStringAtom &localName) noexcept
  {
    if (auto &document = NodeDocument(); document.HasLiveCollectionCachesForAttribute(localName))
    {
      for (auto *node = this; node; node = node->ParentNode())
      {
        if (node->_nodeRareData)
        {
          node->_nodeRareData->InvalidateLiveCollectionCachesForAttribute(localName);
        }
      }
      document.InvalidateOwnerTreeLiveCollectionCachesForAttribute(localName);
    }
  }

  void Node::SetNodeDocument(Document &document) noexcept
  {
    krys_debug_assert(!IsDocumentNode());
    if (_nodeDocument == &document)
    {
      return;
    }
    // The old document counts them.
    InvalidateLiveCollectionCaches();
    // The new document is counted first: dropping the old one's count may delete it.
    document.IncrementReferencingNodeCount();
    std::exchange(_nodeDocument, &document)->DecrementReferencingNodeCount();
    for (const auto &listener : EventListeners())
    {
      document.AddListenerTypeIfNeeded(listener->Type());
    }
  }

  void Node::DidAddEventListener(const EventListener &listener) noexcept
  {
    EventTarget::DidAddEventListener(listener);
    NodeDocument().AddListenerTypeIfNeeded(listener.Type());
  }

  dom::DocumentOrShadowRoot &Node::TreeScope() const noexcept
  {
    if (_containingShadowRoot)
    {
      return *_containingShadowRoot;
    }
    return NodeDocument();
  }

  dom::DocumentOrShadowRoot &Node::TreeScopeForSVGReferences() const noexcept
  {
    if (auto *shadowRoot = dom::TreeQueries::ContainingShadowRoot(*this);
        shadowRoot && shadowRoot->Mode() == dom::ShadowRootMode::UserAgent)
    {
      if (auto *host = DynamicDowncast<SVGElement>(shadowRoot->Host()); host && Is<SVGUseElement>(*host))
      {
        return host->TreeScope();
      }
    }
    return TreeScope();
  }

  boo::RenderBox *Node::GetRenderBox() const noexcept
  {
    return DynamicDowncast<boo::RenderBox>(_renderer);
  }

  bool Node::IsInUserAgentShadowTree() const noexcept
  {
    if (!IsInShadowTree())
    {
      return false;
    }
    auto *shadowRoot = dom::TreeQueries::ContainingShadowRoot(*this);
    return shadowRoot && shadowRoot->Mode() == dom::ShadowRootMode::UserAgent;
  }

  text::TextDirection Node::EffectiveTextDirection() const noexcept
  {
    if (UsesEffectiveTextDirection())
    {
      return _nodeFlags.contains(dom::NodeFlag::EffectiveTextDirectionIsRTL) ? text::TextDirection::RTL
                                                                             : text::TextDirection::LTR;
    }

    // The document element's directionality, if it's an HTML element that keeps one.
    auto documentElement = NodeDocument().DocumentElement();
    if (documentElement && documentElement.get() != this && documentElement->IsHTMLElement()
        && documentElement->UsesEffectiveTextDirection())
    {
      return documentElement->EffectiveTextDirection();
    }
    return text::TextDirection::LTR;
  }

  void Node::SetEffectiveTextDirection(text::TextDirection direction) noexcept
  {
    if (direction == text::TextDirection::RTL)
    {
      _nodeFlags.Add(dom::NodeFlag::EffectiveTextDirectionIsRTL);
    }
    else
    {
      _nodeFlags.Remove(dom::NodeFlag::EffectiveTextDirectionIsRTL);
    }
  }

#pragma region Node

  DOMString Node::BaseURI() const noexcept
  {
    // 1. Let baseURL be the result of running this's node document's get the base URL algorithm.
    // 2. If baseURL is null, then set baseURL to this's node document's URL.
    // (A document's base URL is never null: it falls back to its URL.)
    // 3. Return baseURL, serialized.
    return NodeDocument().BaseUrl().String();
  }

  Document *Node::OwnerDocument() const noexcept
  {
    Document &document = NodeDocument();
    return &document == this ? nullptr : &document;
  }

  Node &Node::GetRootNode(const dom::GetRootNodeOptions &options) noexcept
  {
    return options.Composed ? dom::TreeQueries::ShadowIncludingRoot(*this) : dom::TreeQueries::Root(*this);
  }

  Element *Node::ParentElement() const noexcept
  {
    return DynamicDowncast<Element>(ParentNode());
  }

  bool Node::HasChildNodes() const noexcept
  {
    return FirstChild() != nullptr;
  }

  Ref<dom::NodeList> Node::ChildNodes() noexcept
  {
    return EnsureNodeRareData().ChildNodes(*this);
  }

  bool Node::HasEverPaintedImages() const noexcept
  {
    return _nodeRareData && _nodeRareData->HasEverPaintedImages();
  }

  void Node::SetHasEverPaintedImages(bool hasEverPaintedImages) noexcept
  {
    if (!hasEverPaintedImages && !_nodeRareData)
    {
      return;
    }
    EnsureNodeRareData().SetHasEverPaintedImages(hasEverPaintedImages);
  }

  dom::NodeRareData &Node::EnsureNodeRareData() noexcept
  {
    if (_nodeRareData == nullptr)
    {
      _nodeRareData = IsElementNode() ? UniquePtr<dom::NodeRareData> {CreateUnique<dom::ElementRareData>()}
                                      : CreateUnique<dom::NodeRareData>();
    }
    return *_nodeRareData;
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
    html::CEReactionsScope ceReactions;
    // Run these steps for each descendant exclusive Text node node of this:
    RefPtr<Text> next = ShareRefPtr(dom::TreeQueries::NextExclusiveTextNode(*this, this));
    while (next != nullptr)
    {
      Ref<Text> node = next;

      // 1. Let length be node's length.
      auto length = node->Length();

      // 2. If length is zero, then remove node and continue with the next exclusive Text node, if any.
      if (length == 0uz)
      {
        next = ShareRefPtr(dom::TreeQueries::NextExclusiveTextNode(*node, this));
        if (auto removeResult = dom::MutationAlgorithms::Remove(*node); removeResult.HasException())
        {
          return removeResult.ReleaseException();
        }

        continue;
      }

      // 3. Let data be the concatenation of the data of node's contiguous exclusive Text nodes (excluding
      //    itself), in tree order.
      DOMString data {};
      const Node *current = node->NextSibling();
      while (dom::TreeQueries::IsExclusiveTextNode(current))
      {
        data += Downcast<Text>(current)->Data();
        current = current->NextSibling();
      }

      // 4. Replace data of node with length, 0, and data.
      if (auto replaceResult = node->ReplaceData(length, 0uz, krys::move(data)); replaceResult.HasException())
      {
        return replaceResult.ReleaseException();
      }

      // 5. Let currentNode be node's next sibling.
      Node *currentNode = node->NextSibling();

      // 6. While currentNode is an exclusive Text node:
      while (dom::TreeQueries::IsExclusiveTextNode(currentNode))
      {
        auto &currentTextNode = Downcast<Text>(*currentNode);

        // 6a. For each live range whose start node is currentNode: add length to its start offset and set its
        //     start node to node.
        // 6b. For each live range whose end node is currentNode: add length to its end offset and set its end
        //     node to node.
        // 6c. For each live range whose start node is currentNode's parent and start offset is currentNode's
        //     index: set its start node to node and its start offset to length.
        // 6d. For each live range whose end node is currentNode's parent and end offset is currentNode's
        // index:
        //     set its end node to node and its end offset to length.
        dom::LiveRangeUpdater::NodeNormalized(*node, currentTextNode, length);

        // 6e. Add currentNode's length to length.
        length += currentTextNode.Length();

        // 6f. Set currentNode to its next sibling.
        currentNode = currentTextNode.NextSibling();
      }

      // 7. Remove node's contiguous exclusive Text nodes (excluding itself), in tree order.
      while (dom::TreeQueries::IsExclusiveTextNode(node->NextSibling()))
      {
        if (auto removeResult = dom::MutationAlgorithms::Remove(*node->NextSibling());
            removeResult.HasException())
        {
          return removeResult.ReleaseException();
        }
      }

      next = ShareRefPtr(dom::TreeQueries::NextExclusiveTextNode(*node, this));
    }

    return {};
  }

  ExceptionOr<Ref<Node>> Node::CloneNode(bool subtree) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. If this is a shadow root, then throw a "NotSupportedError" DOMException.
    if (Is<ShadowRoot>(this))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    // 2. Return the result of cloning a node given this with subtree set to subtree.
    return dom::NodeAlgorithms::CloneNode(*this, nullptr, subtree);
  }

  bool Node::IsEqualNode(const Node *otherNode) const noexcept
  {
    if (otherNode == nullptr)
    {
      return false;
    }

    return dom::NodeAlgorithms::Equals(*this, *otherNode);
  }

  bool Node::IsSameNode(const Node *otherNode) const noexcept
  {
    return this == otherNode;
  }

  EnumSet<dom::DocumentPosition> Node::CompareDocumentPosition(const Node &other) const noexcept
  {
    // 1. If this is other, then return zero.
    if (this == &other)
    {
      return {};
    }

    // 2. Let node1 be other and node2 be this.
    const Node *node1 = &other;
    const Node *node2 = this;

    // 3. Let attr1 and attr2 be null.
    const Attr *attr1 = nullptr;
    const Attr *attr2 = nullptr;

    // 4. If node1 is an attribute, then set attr1 to node1 and node1 to attr1's element.
    if (Is<Attr>(node1))
    {
      attr1 = Downcast<Attr>(node1);
      node1 = attr1->OwnerElement();
    }

    // 5. If node2 is an attribute:
    if (Is<Attr>(node2))
    {
      // 5a. Set attr2 to node2 and node2 to attr2's element.
      attr2 = Downcast<Attr>(node2);
      node2 = attr2->OwnerElement();

      // 5b. If attr1 and node1 are non-null, and node2 is node1:
      if (attr1 != nullptr && node1 != nullptr && node2 == node1)
      {
        // 5b.i. For each attr of node2's attribute list:
        for (auto &attr : Downcast<Element>(*node2)._attributes)
        {
          // 5b.i.A. If attr equals attr1, then return the result of adding
          // DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC
          //         and DOCUMENT_POSITION_PRECEDING.
          if (dom::NodeAlgorithms::Equals(*attr, *attr1))
          {
            return {dom::DocumentPosition::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC,
                    dom::DocumentPosition::DOCUMENT_POSITION_PRECEDING};
          }

          // 5b.i.B. If attr equals attr2, then return the result of adding
          // DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC
          //         and DOCUMENT_POSITION_FOLLOWING.
          if (dom::NodeAlgorithms::Equals(*attr, *attr2))
          {
            return {dom::DocumentPosition::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC,
                    dom::DocumentPosition::DOCUMENT_POSITION_FOLLOWING};
          }
        }
      }
    }

    // 6. If node1 or node2 is null, or node1's root is not node2's root, then return the result of adding
    //    DOCUMENT_POSITION_DISCONNECTED, DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC, and either
    //    DOCUMENT_POSITION_PRECEDING or DOCUMENT_POSITION_FOLLOWING, with the constraint that this is to be
    //    consistent, together.
    if (node1 == nullptr || node2 == nullptr || !dom::TreeQueries::SameRoot(*node1, *node2))
    {
      // The nodes' addresses order them consistently, whichever is asked about the other.
      auto orderingFlag = &other < this ? dom::DocumentPosition::DOCUMENT_POSITION_PRECEDING
                                        : dom::DocumentPosition::DOCUMENT_POSITION_FOLLOWING;

      return EnumSet<dom::DocumentPosition> {dom::DocumentPosition::DOCUMENT_POSITION_DISCONNECTED,
                                             dom::DocumentPosition::DOCUMENT_POSITION_IMPLEMENTATION_SPECIFIC}
             | orderingFlag;
    }

    // 7. If node1 is an ancestor of node2 and attr1 is null, or node1 is node2 and attr2 is non-null, then
    // return
    //    the result of adding DOCUMENT_POSITION_CONTAINS to DOCUMENT_POSITION_PRECEDING.
    if ((dom::TreeQueries::IsAncestor(*node1, *node2) && attr1 == nullptr)
        || (node1 == node2 && attr2 != nullptr))
    {
      return {dom::DocumentPosition::DOCUMENT_POSITION_CONTAINS,
              dom::DocumentPosition::DOCUMENT_POSITION_PRECEDING};
    }

    // 8. If node1 is a descendant of node2 and attr2 is null, or node1 is node2 and attr1 is non-null, then
    //    return the result of adding DOCUMENT_POSITION_CONTAINED_BY to DOCUMENT_POSITION_FOLLOWING.
    if ((dom::TreeQueries::IsDescendant(*node1, *node2) && attr2 == nullptr)
        || (node1 == node2 && attr1 != nullptr))
    {
      return {dom::DocumentPosition::DOCUMENT_POSITION_CONTAINED_BY,
              dom::DocumentPosition::DOCUMENT_POSITION_FOLLOWING};
    }

    // 9. If node1 is preceding node2, then return DOCUMENT_POSITION_PRECEDING.
    if (dom::TreeQueries::IsPreceding(*node1, *node2))
    {
      return dom::DocumentPosition::DOCUMENT_POSITION_PRECEDING;
    }

    // 10. Return DOCUMENT_POSITION_FOLLOWING.
    return dom::DocumentPosition::DOCUMENT_POSITION_FOLLOWING;
  }

  bool Node::Contains(const Node *other) const noexcept
  {
    if (other == nullptr)
    {
      return false;
    }

    return dom::TreeQueries::IsInclusiveDescendant(*other, *this);
  }

  DOMStringAtom Node::LookupPrefix(const DOMStringAtom &namespaceURI) const noexcept
  {
    // 1. If namespace is null or the empty string, then return null.
    if (namespaceURI == null || namespaceURI == DOMStringAtom::Empty())
    {
      return null;
    }

    // 2. Switch on the interface this implements:
    switch (NodeType())
    {
      // Element: Return the result of locating a namespace prefix for this using namespace.
      case dom::NodeType::ELEMENT_NODE:
      {
        return dom::NodeAlgorithms::LocateNamespacePrefix(Downcast<Element>(*this), namespaceURI);
      }
      // Document:
      case dom::NodeType::DOCUMENT_NODE:
      {
        // 2a. If this's document element is null, then return null.
        auto *documentElement = dom::TreeQueries::DocumentElement(Downcast<Document>(*this));
        if (documentElement == nullptr)
        {
          return null;
        }

        // 2b. Return the result of locating a namespace prefix for this's document element using namespace.
        return dom::NodeAlgorithms::LocateNamespacePrefix(*documentElement, namespaceURI);
      }
      // DocumentType, DocumentFragment: Return null.
      case dom::NodeType::DOCUMENT_TYPE_NODE:
      case dom::NodeType::DOCUMENT_FRAGMENT_NODE:
      {
        return null;
      }
      // Attr:
      case dom::NodeType::ATTRIBUTE_NODE:
      {
        // 2a. If this's element is null, then return null.
        const Element *ownerElement = Downcast<Attr>(*this).OwnerElement();
        if (ownerElement == nullptr)
        {
          return null;
        }

        // 2b. Return the result of locating a namespace prefix for this's element using namespace.
        return dom::NodeAlgorithms::LocateNamespacePrefix(*ownerElement, namespaceURI);
      }
      // Otherwise:
      default:
      {
        // 2a. If this's parent element is null, then return null.
        const Element *parentElement = ParentElement();
        if (parentElement == nullptr)
        {
          return null;
        }

        // 2b. Return the result of locating a namespace prefix for this's parent element using namespace.
        return dom::NodeAlgorithms::LocateNamespacePrefix(*parentElement, namespaceURI);
      }
    }
  }

  DOMStringAtom Node::LookupNamespaceURI(DOMStringAtom prefix) const noexcept
  {
    // 1. If prefix is the empty string, then set it to null.
    if (prefix == DOMStringAtom::Empty())
    {
      prefix = null;
    }

    // 2. Return the result of running locate a namespace for this using prefix.
    return dom::NodeAlgorithms::LocateNamespace(*this, prefix);
  }

  bool Node::IsDefaultNamespace(DOMStringAtom namespaceURI) const noexcept
  {
    // 1. If namespace is the empty string, then set it to null.
    if (namespaceURI == DOMStringAtom::Empty())
    {
      namespaceURI = null;
    }

    // 2. Let defaultNamespace be the result of running locate a namespace for this using null.
    auto defaultNamespace = dom::NodeAlgorithms::LocateNamespace(*this, null);

    // 3. Return true if defaultNamespace is the same as namespace; otherwise false.
    return namespaceURI == defaultNamespace;
  }

  ExceptionOr<Node &> Node::InsertBefore(Node &newChild, Node *refChild) noexcept
  {
    html::CEReactionsScope ceReactions;
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return dom::MutationAlgorithms::PreInsert(newChild, *containerNode, refChild);
    }

    return Exception {ExceptionCode::HierarchyRequestError};
  }

  ExceptionOr<Node &> Node::ReplaceChild(Node &newChild, Node &oldChild) noexcept
  {
    html::CEReactionsScope ceReactions;
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return dom::MutationAlgorithms::Replace(oldChild, newChild, *containerNode);
    }

    return Exception {ExceptionCode::HierarchyRequestError};
  }

  ExceptionOr<Node &> Node::RemoveChild(Node &oldChild) noexcept
  {
    html::CEReactionsScope ceReactions;
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return dom::MutationAlgorithms::PreRemove(oldChild, *containerNode);
    }

    return Exception {ExceptionCode::NotFoundError};
  }

  ExceptionOr<Node &> Node::AppendChild(Node &newChild) noexcept
  {
    html::CEReactionsScope ceReactions;
    if (auto *containerNode = DynamicDowncast<ContainerNode>(*this))
    {
      return dom::MutationAlgorithms::Append(newChild, *containerNode);
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
    if (auto *slot = dom::SlotAlgorithms::GetAssignedSlot(*this))
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
    if (auto *slot = dom::SlotAlgorithms::GetAssignedSlot(*this))
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

#pragma region Tree Helpers

  uint32 Node::CountChildNodes() const noexcept
  {
    uint32 count = 0;
    for (auto *child = FirstChild(); child; child = child->NextSibling())
    {
      ++count;
    }
    return count;
  }

  Node *Node::TraverseToChildAt(uint32 index) const noexcept
  {
    auto *child = FirstChild();
    for (; child && index; --index)
    {
      child = child->NextSibling();
    }
    return child;
  }

  uint32 Node::ComputeNodeIndex() const noexcept
  {
    return static_cast<uint32>(dom::TreeQueries::Index(*this));
  }

  uint32 Node::Length() const noexcept
  {
    return static_cast<uint32>(dom::TreeQueries::Length(*this));
  }

  ContainerNode *Node::ParentOrShadowHostNode() const noexcept
  {
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(*this))
    {
      return shadowRoot->Host();
    }
    return ParentNode();
  }

  ContainerNode *Node::NonShadowBoundaryParentNode() const noexcept
  {
    auto *parent = ParentNode();
    return parent && !parent->IsShadowRootNode() ? parent : nullptr;
  }

  Node *Node::NonBoundaryShadowTreeRootNode() noexcept
  {
    krys_debug_assert(!IsShadowRootNode());
    Node *root = this;
    while (root)
    {
      if (root->IsShadowRootNode())
      {
        return root;
      }
      Node *parent = root->ParentNode();
      if (parent && parent->IsShadowRootNode())
      {
        return root;
      }
      root = parent;
    }
    return nullptr;
  }

  bool Node::IsDescendantOf(const Node *other) const noexcept
  {
    return other && dom::TreeQueries::IsDescendant(*this, *other);
  }

  bool Node::IsShadowIncludingDescendantOf(const Node *other) const noexcept
  {
    return other && dom::TreeQueries::IsShadowIncludingDescendant(*this, *other);
  }

  Node *Node::LastDescendant() const noexcept
  {
    auto *node = const_cast<Node *>(this);
    while (auto *lastChild = node->LastChild())
    {
      node = lastChild;
    }
    return node;
  }

#pragma endregion

#pragma region Editing

  bool Node::IsContentEditable() const noexcept
  {
    return ComputeEditability(UserSelectAllTreatment::Editable, ShouldUpdateStyle::Update)
           != Editability::ReadOnly;
  }

  bool Node::IsContentRichlyEditable() const noexcept
  {
    return ComputeEditability(UserSelectAllTreatment::NotEditable, ShouldUpdateStyle::Update)
           == Editability::CanEditRichly;
  }

  namespace
  {
    KRYS_NODISCARD Node::Editability
      ComputeEditabilityFromComputedStyle(const RenderStyle &style,
                                          Node::UserSelectAllTreatment treatment) noexcept
    {
      // `user-select: all` content is selected as one unit, so editing treats it as read-only.
      if (treatment == Node::UserSelectAllTreatment::NotEditable && style.UsedUserSelect() == UserSelect::All)
      {
        return Node::Editability::ReadOnly;
      }

      switch (style.UsedUserModify())
      {
        case UserModify::ReadOnly:               return Node::Editability::ReadOnly;
        case UserModify::ReadWrite:              return Node::Editability::CanEditRichly;
        case UserModify::ReadWritePlaintextOnly: return Node::Editability::CanEditPlainText;
      }
      krys_unreachable();
    }
  }

  Node::Editability Node::ComputeEditabilityWithStyle(const RenderStyle *incomingStyle,
                                                      UserSelectAllTreatment treatment,
                                                      ShouldUpdateStyle shouldUpdateStyle) const noexcept
  {
    if (!NodeDocument().HasLivingRenderTree() || IsPseudoElement())
    {
      return Editability::ReadOnly;
    }

    Ref document = ShareRef(NodeDocument());

    // In a shadow tree, the `contenteditable` attributes decide rather than style.
    if (IsInShadowTree())
    {
      return HTMLElement::EditabilityFromContentEditableAttr(*this);
    }

    // Style-based editability is always possible (no style sheet records whether it uses
    // `-internal-user-modify`), so style is brought up to date.
    if (shouldUpdateStyle == ShouldUpdateStyle::Update && document->NeedsStyleRecalc())
    {
      document->UpdateStyleIfNeeded();
    }

    // The style given, or the document's, or that of the node's element (a text node's is its parent's in the
    // composed tree), resolved for the purpose if the element has none.
    auto *style = [&]() -> const RenderStyle *
    {
      if (incomingStyle)
      {
        return incomingStyle;
      }
      if (IsDocumentNode())
      {
        return GetRenderStyle();
      }
      auto *element = DynamicDowncast<Element>(*this);
      if (!element)
      {
        element = ParentElementInComposedTree();
      }
      if (!element || !element->IsConnected())
      {
        return nullptr;
      }
      return const_cast<Element &>(*element).ResolveComputedStyle(
        Element::ResolveComputedStyleMode::Editability);
    }();

    if (!style)
    {
      return Editability::ReadOnly;
    }

    return ComputeEditabilityFromComputedStyle(*style, treatment);
  }

  Node::Editability Node::ComputeEditability(UserSelectAllTreatment treatment,
                                             ShouldUpdateStyle shouldUpdateStyle) const noexcept
  {
    return ComputeEditabilityWithStyle(nullptr, treatment, shouldUpdateStyle);
  }

  bool Node::IsRootEditableElement() const noexcept
  {
    return HasEditableStyle() && IsElementNode()
           && (!ParentNode() || !ParentNode()->HasEditableStyle() || !ParentNode()->IsElementNode()
               || NodeDocument().Body().get() == this);
  }

  Element *Node::RootEditableElement() const noexcept
  {
    Element *result = nullptr;
    auto body = NodeDocument().Body();
    for (auto *node = const_cast<Node *>(this); node && node->HasEditableStyle(); node = node->ParentNode())
    {
      if (auto *element = DynamicDowncast<Element>(*node))
      {
        result = element;
      }
      if (body.get() == node)
      {
        break;
      }
    }
    return result;
  }

  void Node::DefaultEventHandler(Event &event) noexcept
  {
    // A middle button press pans the nearest box that can scroll, unless it's on a link.
    if (event.Type() != event_names::MouseDown)
    {
      return;
    }
    auto *mouseEvent = DynamicDowncast<MouseEvent>(event);
    if (!mouseEvent || mouseEvent->Button() != static_cast<int16>(MouseButton::Middle))
    {
      return;
    }

    // From the target: this runs on the nodes above the link too.
    auto target = event.Target();
    for (auto *node = DynamicDowncast<Node>(target.get()); node; node = node->ParentOrShadowHostNode())
    {
      if (auto *element = DynamicDowncast<Element>(*node); element && css::IsLink(*element))
      {
        return;
      }
    }

    for (auto *renderer = Renderer(); renderer; renderer = renderer->Parent())
    {
      auto *renderBox = DynamicDowncast<RenderBox>(*renderer);
      if (renderBox && renderBox->CanBeScrolledAndHasScrollableArea())
      {
        if (auto *frame = NodeDocument().Frame())
        {
          frame->GetEventHandler().StartPanScrolling(*renderBox);
        }
        break;
      }
    }
  }

  bool Node::CanStartSelection() const noexcept
  {
    if (HasEditableStyle())
    {
      return true;
    }

    if (auto *renderer = Renderer())
    {
      const RenderStyle &style = renderer->Style();

      // A selection can start in `user-select: none` content, but on a draggable element, dragging wins.
      if (style.UserDrag() == UserDrag::Element && style.UsedUserSelect() == UserSelect::None)
      {
        return false;
      }
    }
    auto *parent = ParentOrShadowHostNode();
    return parent ? parent->CanStartSelection() : true;
  }

#pragma endregion

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
        _nodeFlags.Add(dom::NodeFlag::StyleResolutionShouldRecompositeLayer);
        break;
      case style::InvalidationMode::RebuildRenderer:
      case style::InvalidationMode::InsertedIntoAncestor:
        _nodeFlags.Add(dom::NodeFlag::HasInvalidRenderer);
        break;
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
    auto composedAncestors = dom::ComposedTreeAncestors(*this);
    auto it = composedAncestors.begin();
    auto end = composedAncestors.end();
    if (it != end)
    {
      it->SetDirectChildNeedsStyleRecalc();

      for (; it != end; ++it)
      {
        // The composed tree ancestors skip shadow roots, so a host marks its own.
        if (auto *shadowRoot = dom::TreeQueries::GetShadowRoot(*it))
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

    // The render tree update is applying style, and drops what it invalidates itself: anchored elements as an
    // anchor's renderer restyles, and elements whose animations it cancels as it tears renderers down.
    if (NodeDocument().InRenderTreeUpdate())
    {
      return;
    }

    if (validity != style::Validity::Valid)
    {
      _nodeFlags.Add(dom::NodeFlag::IsComputedStyleInvalid);
    }

    // An invalid node's ancestors are already marked, except for a node just inserted.
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

    if (dom::SlotAlgorithms::IsAssigned(*this))
    {
      return dom::SlotAlgorithms::GetAssignedSlot(*this);
    }

    return ParentNode();
  }

  List<Ref<dom::RegisteredObserver>> &Node::RegisteredObservers() noexcept
  {
    return EnsureNodeRareData().RegisteredObservers();
  }

  List<Ref<dom::TransientRegisteredObserver>> &Node::TransientRegisteredObservers() noexcept
  {
    return EnsureNodeRareData().TransientRegisteredObservers();
  }

  template <>
  ContainerNode *dom::Parent<dom::TreeType::Tree>(const Node &node) noexcept
  {
    return node.ParentNode();
  }

  template <>
  ContainerNode *dom::Parent<dom::TreeType::ShadowIncludingTree>(const Node &node) noexcept
  {
    return node.ParentOrShadowHostNode();
  }

  template <>
  ContainerNode *dom::Parent<dom::TreeType::ComposedTree>(const Node &node) noexcept
  {
    return node.ParentInComposedTree();
  }

  namespace
  {
    template <dom::TreeType treeType>
    KRYS_NODISCARD size_t Depth(const Node &node) noexcept
    {
      size_t depth = 0;
      const Node *ancestor = &node;
      while ((ancestor = dom::Parent<treeType>(*ancestor)))
      {
        ++depth;
      }
      return depth;
    }

    /// @brief The nearest common inclusive ancestor of two nodes, and the children of it that contain each
    /// (null for a node that is the common ancestor).
    struct AncestorAndChildren
    {
      const Node *CommonAncestor;
      const Node *DistinctAncestorA;
      const Node *DistinctAncestorB;
    };

    template <dom::TreeType treeType>
    KRYS_NODISCARD AncestorAndChildren CommonInclusiveAncestorAndChildren(const Node &a,
                                                                          const Node &b) noexcept
    {
      if (&a == &b)
      {
        return {&a, nullptr, nullptr};
      }

      // The deeper node climbs to the other's depth, then both climb together until they meet. The nodes
      // they were at just before are the common ancestor's children that hold them.
      auto [depthA, depthB] = std::make_tuple(Depth<treeType>(a), Depth<treeType>(b));
      auto [x, y, difference] = depthA >= depthB ? std::make_tuple(&a, &b, depthA - depthB)
                                                 : std::make_tuple(&b, &a, depthB - depthA);
      const Node *distinctAncestorA = nullptr;
      for (size_t i = 0; i < difference; ++i)
      {
        distinctAncestorA = x;
        x = dom::Parent<treeType>(*x);
      }
      const Node *distinctAncestorB = nullptr;
      while (x != y)
      {
        distinctAncestorA = x;
        distinctAncestorB = y;
        x = dom::Parent<treeType>(*x);
        y = dom::Parent<treeType>(*y);
      }
      if (depthA < depthB)
      {
        std::swap(distinctAncestorA, distinctAncestorB);
      }
      return {x, distinctAncestorA, distinctAncestorB};
    }

    KRYS_NODISCARD bool IsSiblingSubsequent(const Node &siblingA, const Node &siblingB) noexcept
    {
      krys_debug_assert(siblingA.ParentNode());
      krys_debug_assert(siblingA.ParentNode() == siblingB.ParentNode());
      krys_debug_assert(&siblingA != &siblingB);
      for (auto *sibling = &siblingA; sibling; sibling = sibling->NextSibling())
      {
        if (sibling == &siblingB)
        {
          return true;
        }
      }
      return false;
    }
  }

  template <dom::TreeType treeType>
  Node *dom::CommonInclusiveAncestor(const Node &a, const Node &b) noexcept
  {
    return const_cast<Node *>(CommonInclusiveAncestorAndChildren<treeType>(a, b).CommonAncestor);
  }

  template Node *dom::CommonInclusiveAncestor<dom::TreeType::Tree>(const Node &, const Node &) noexcept;

  template Node *dom::CommonInclusiveAncestor<dom::TreeType::ShadowIncludingTree>(const Node &,
                                                                                  const Node &) noexcept;

  template Node *dom::CommonInclusiveAncestor<dom::TreeType::ComposedTree>(const Node &,
                                                                           const Node &) noexcept;

  template <dom::TreeType treeType>
  std::partial_ordering dom::TreeOrder(const Node &a, const Node &b) noexcept
  {
    if (&a == &b)
    {
      return std::partial_ordering::equivalent;
    }
    auto result = CommonInclusiveAncestorAndChildren<treeType>(a, b);
    if (!result.CommonAncestor)
    {
      return std::partial_ordering::unordered;
    }
    if (!result.DistinctAncestorA)
    {
      return std::partial_ordering::less;
    }
    if (!result.DistinctAncestorB)
    {
      return std::partial_ordering::greater;
    }
    // Under a host, its shadow root comes before its children; a host has only one shadow root.
    bool isShadowRootA = Is<ShadowRoot>(*result.DistinctAncestorA);
    bool isShadowRootB = Is<ShadowRoot>(*result.DistinctAncestorB);
    if (isShadowRootA || isShadowRootB)
    {
      if (!isShadowRootB)
      {
        return std::partial_ordering::less;
      }
      if (!isShadowRootA)
      {
        return std::partial_ordering::greater;
      }
      krys_debug_assert(false);
      return std::partial_ordering::unordered;
    }
    return IsSiblingSubsequent(*result.DistinctAncestorA, *result.DistinctAncestorB)
             ? std::partial_ordering::less
             : std::partial_ordering::greater;
  }

  template std::partial_ordering dom::TreeOrder<dom::TreeType::Tree>(const Node &, const Node &) noexcept;

  template std::partial_ordering dom::TreeOrder<dom::TreeType::ShadowIncludingTree>(const Node &,
                                                                                    const Node &) noexcept;

  template std::partial_ordering dom::TreeOrder<dom::TreeType::ComposedTree>(const Node &,
                                                                             const Node &) noexcept;
}
