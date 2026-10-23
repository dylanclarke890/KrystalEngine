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
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include <tuple>

namespace krys::boo::dom
{
  Node::Node(Document &document, dom::NodeType type, NodeFlags flags) noexcept
      : EventTarget(EventTargetFlags::IsNode), _nodeType(type), _nodeDocument(&document),
        _parentNode(nullptr), _previousSibling(nullptr), _nextSibling(nullptr)
  {
    _flags = flags;
    if (type != NodeType::DOCUMENT_NODE)
    {
      document.IncrementReferencingNodeCount();
    }
  }

  Node::~Node() noexcept
  {
    // Last, since it may delete the document.
    if (!IsDocumentNode())
    {
      _nodeDocument->DecrementReferencingNodeCount();
    }
  }

  void Node::SetNodeDocument(Document &document) noexcept
  {
    krys_debug_assert(!IsDocumentNode());
    if (_nodeDocument == &document)
    {
      return;
    }
    // The new document is counted first: dropping the old one's count may delete it.
    document.IncrementReferencingNodeCount();
    std::exchange(_nodeDocument, &document)->DecrementReferencingNodeCount();
  }

  DocumentOrShadowRoot &Node::TreeScope() const noexcept
  {
    if (IsInShadowTree())
    {
      if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(TreeQueries::Root(const_cast<Node &>(*this))))
      {
        return *shadowRoot;
      }
    }
    return NodeDocument();
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
    auto *shadowRoot = TreeQueries::ContainingShadowRoot(*this);
    return shadowRoot && shadowRoot->Mode() == ShadowRootMode::UserAgent;
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
    return static_cast<uint32>(TreeQueries::Index(*this));
  }

  uint32 Node::Length() const noexcept
  {
    return static_cast<uint32>(TreeQueries::Length(*this));
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
    return other && TreeQueries::IsDescendant(*this, *other);
  }

  bool Node::IsShadowIncludingDescendantOf(const Node *other) const noexcept
  {
    return other && TreeQueries::IsShadowIncludingDescendant(*this, *other);
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
      // Ideally we'd call ASSERT(!needsStyleRecalc()) here, but
      // ContainerNode::setFocus() calls invalidateStyleForSubtree(), so the assertion
      // would fire in the middle of Document::setFocusedElement().

      // Elements with user-select: all style are considered atomic
      // therefore non editable.
      if (treatment == Node::UserSelectAllTreatment::NotEditable && style.UsedUserSelect() == UserSelect::All)
      {
        return Node::Editability::ReadOnly;
      }

      // FIXME: A page that's editable as a whole (`Page::isEditable`), once there's a page.
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

    if (IsInShadowTree())
    {
      return html::HTMLElement::EditabilityFromContentEditableAttr(*this);
    }

    // Style-based editability is always possible (no style sheet records whether it uses
    // `-webkit-user-modify`), so style is brought up to date.
    if (shouldUpdateStyle == ShouldUpdateStyle::Update && document->NeedsStyleRecalc())
    {
      document->UpdateStyleIfNeeded();
    }

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

  bool Node::CanStartSelection() const noexcept
  {
    if (HasEditableStyle())
    {
      return true;
    }

    // FIXME: A draggable element that can't be selected (`-webkit-user-drag: element` with `user-select:
    // none`) can't start one, once there's dragging.
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

  template <>
  ContainerNode *Parent<TreeType::Tree>(const Node &node) noexcept
  {
    return node.ParentNode();
  }

  template <>
  ContainerNode *Parent<TreeType::ShadowIncludingTree>(const Node &node) noexcept
  {
    return node.ParentOrShadowHostNode();
  }

  template <>
  ContainerNode *Parent<TreeType::ComposedTree>(const Node &node) noexcept
  {
    return node.ParentInComposedTree();
  }

  namespace
  {
    template <TreeType treeType>
    KRYS_NODISCARD size_t Depth(const Node &node) noexcept
    {
      size_t depth = 0;
      const Node *ancestor = &node;
      while ((ancestor = Parent<treeType>(*ancestor)))
      {
        ++depth;
      }
      return depth;
    }

    struct AncestorAndChildren
    {
      const Node *CommonAncestor;
      const Node *DistinctAncestorA;
      const Node *DistinctAncestorB;
    };

    template <TreeType treeType>
    KRYS_NODISCARD AncestorAndChildren CommonInclusiveAncestorAndChildren(const Node &a,
                                                                          const Node &b) noexcept
    {
      // This check isn't needed for correctness, but it is cheap and likely to be
      // common enough to be worth optimizing so we don't have to walk to the root.
      if (&a == &b)
      {
        return {&a, nullptr, nullptr};
      }
      // FIXME: Could optimize cases where nodes are both in the same shadow tree.
      // FIXME: Could optimize cases where nodes are in different documents to quickly return false.
      // FIXME: Could optimize cases where one node is connected and the other is not to quickly return false.
      auto [depthA, depthB] = std::make_tuple(Depth<treeType>(a), Depth<treeType>(b));
      auto [x, y, difference] = depthA >= depthB ? std::make_tuple(&a, &b, depthA - depthB)
                                                 : std::make_tuple(&b, &a, depthB - depthA);
      const Node *distinctAncestorA = nullptr;
      for (size_t i = 0; i < difference; ++i)
      {
        distinctAncestorA = x;
        x = Parent<treeType>(*x);
      }
      const Node *distinctAncestorB = nullptr;
      while (x != y)
      {
        distinctAncestorA = x;
        distinctAncestorB = y;
        x = Parent<treeType>(*x);
        y = Parent<treeType>(*y);
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

  template <TreeType treeType>
  Node *CommonInclusiveAncestor(const Node &a, const Node &b) noexcept
  {
    return const_cast<Node *>(CommonInclusiveAncestorAndChildren<treeType>(a, b).CommonAncestor);
  }

  template Node *CommonInclusiveAncestor<TreeType::Tree>(const Node &, const Node &) noexcept;
  template Node *CommonInclusiveAncestor<TreeType::ShadowIncludingTree>(const Node &, const Node &) noexcept;
  template Node *CommonInclusiveAncestor<TreeType::ComposedTree>(const Node &, const Node &) noexcept;

  template <TreeType treeType>
  std::partial_ordering TreeOrder(const Node &a, const Node &b) noexcept
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

  template std::partial_ordering TreeOrder<TreeType::Tree>(const Node &, const Node &) noexcept;
  template std::partial_ordering TreeOrder<TreeType::ShadowIncludingTree>(const Node &,
                                                                          const Node &) noexcept;
  template std::partial_ordering TreeOrder<TreeType::ComposedTree>(const Node &, const Node &) noexcept;
}