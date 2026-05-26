#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.HTML/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/LiveRangeUpdater.hpp"
#include "Krystal.HTML/Algorithms/ShadowRootAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/Algorithms/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Types/SmallNodeList.hpp"

namespace Krys::HTML
{
  namespace
  {
    KRYS_NODISCARD bool HasElementChild(ContainerNode &node) noexcept
    {
      return HasNodeOfType<Element>(ChildNodeRange(node));
    }

    KRYS_NODISCARD bool IsDocTypeOrDocTypeFollows(RawPtr<Node> node) noexcept
    {
      if (node == nullptr)
      {
        return false;
      }

      if (Is<DocumentType>(node))
      {
        return true;
      }

      return HasNodeOfType<DocumentType>(ConstNextSiblingRange(*node));
    }

    SmallNodeList CollectChildNodes(ContainerNode &parent) noexcept
    {
      SmallNodeList nodes;

      for (Node &child : ChildNodeRange(parent))
      {
        nodes.emplace_back(ShareRef(child));
      }

      return nodes;
    }
  }

  ExceptionOr<void> MutationAlgorithms::EnsurePreInsertValidity(Node &node, ContainerNode &parent,
                                                                RawPtr<Node> child) noexcept
  {
    if (!IsOneOf<Document, DocumentFragment, Element>(parent))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    if (TreeQueries::IsHostIncludingInclusiveAncestorOf(node, parent))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    if (child != nullptr && child->ParentNode() != &parent)
    {
      return ExceptionCode::NotFoundError;
    }

    if (!IsOneOf<DocumentFragment, DocumentType, Element, CharacterData>(node))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    if ((Is<Text>(node) && Is<Document>(parent)) || (Is<DocumentType>(node) && !Is<Document>(parent)))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    if (Is<Document>(parent))
    {
      if (auto *documentFragment = DynamicDowncast<DocumentFragment>(node))
      {
        auto count = documentFragment->ChildElementCount();
        if (count > 1)
        {
          return ExceptionCode::HierarchyRequestError;
        }

        if (HasNodeOfType<Text>(ConstChildNodeRange(*documentFragment)))
        {
          return ExceptionCode::HierarchyRequestError;
        }

        if (count == 1 && (HasElementChild(parent) || IsDocTypeOrDocTypeFollows(child)))
        {
          return ExceptionCode::HierarchyRequestError;
        }
      }
      else if (Is<Element>(node))
      {
        if (HasElementChild(parent) || IsDocTypeOrDocTypeFollows(child))
        {
          return ExceptionCode::HierarchyRequestError;
        }
      }
      else if (Is<DocumentType>(node))
      {
        if (HasNodeOfType<DocumentType>(ConstChildNodeRange(parent)))
        {
          return ExceptionCode::HierarchyRequestError;
        }

        if (child != nullptr && HasNodeOfType<Element>(ConstPreviousSiblingRange(*child)))
        {
          return ExceptionCode::HierarchyRequestError;
        }

        if (child == nullptr && HasElementChild(parent))
        {
          return ExceptionCode::HierarchyRequestError;
        }
      }
    }

    return {};
  }

  ExceptionOr<Node &> MutationAlgorithms::PreInsert(Node &node, ContainerNode &parent,
                                                    RawPtr<Node> child) noexcept
  {
    if (auto result = EnsurePreInsertValidity(node, parent, child); result.HasException())
    {
      return result.ReleaseException();
    }

    auto referenceChild = child;
    if (&node == referenceChild)
    {
      referenceChild = node.NextSibling();
    }

    if (auto result = Insert(node, parent, referenceChild); result.HasException())
    {
      return result.ReleaseException();
    }

    return node;
  }

  ExceptionOr<void> MutationAlgorithms::Insert(Node &node, ContainerNode &parent, RawPtr<Node> child,
                                               SuppressObservers suppressObservers) noexcept
  {
    SmallNodeList nodes;
    if (Is<DocumentFragment>(node))
    {
      nodes = CollectChildNodes(Downcast<ContainerNode>(node));
    }
    else
    {
      nodes.push_back(ShareRef<Node>(node));
    }

    auto count = nodes.size();
    if (count == 0)
    {
      return {};
    }

    if (auto *documentFragment = DynamicDowncast<DocumentFragment>(node))
    {
      while (auto *child = documentFragment->FirstChild())
      {
        auto result = Remove(*child, SuppressObservers(true));
        if (result.HasException())
        {
          return result.ReleaseException();
        }
      }

      TreeMutationDispatcher::QueueTreeMutationRecord(node, {}, nodes, nullptr, nullptr);
    }

    if (child != nullptr)
    {
      LiveRangeUpdater::InsertedBeforeNode(*child, count);
    }

    auto *previousSibling = child != nullptr ? child->PreviousSibling() : parent.LastChild();

    bool isParentNamedShadowHost =
      ShadowRootAlgorithms::IsShadowHost(parent)
      && Downcast<Element>(parent).ShadowRoot()->SlotAssignment() == SlotAssignmentMode::Named;
    bool isParentRootShadowRoot = Is<ShadowRoot>(TreeQueries::Root(parent));

    auto &parentRoot = TreeQueries::Root(parent);
    auto *slotParent = DynamicDowncast<HTMLSlotElement>(parent);
    for (auto &target : nodes)
    {
      if (auto result = parent.NodeDocument().AdoptNode(*target); result.HasException())
      {
        return result.ReleaseException();
      }

      target->SetParentNode(&parent);
      if (child == nullptr)
      {
        if (auto *lastChild = parent.LastChild())
        {
          target->SetPreviousSibling(lastChild);
          lastChild->SetNextSibling(target.get());
        }
        else
        {
          parent.SetFirstChild(target.get());
        }

        parent.SetLastChild(target.get());
      }
      else
      {
        if (auto *previousSibling = child->PreviousSibling())
        {
          previousSibling->SetNextSibling(target.get());
          target->SetPreviousSibling(previousSibling);

          child->SetPreviousSibling(target.get());
          target->SetNextSibling(child);
        }
        else
        {
          assert(parent.FirstChild() == child);
          parent.SetFirstChild(target.get());

          child->SetPreviousSibling(target.get());
          target->SetNextSibling(child);
        }
      }

      if (isParentNamedShadowHost && SlotAlgorithms::IsSlottable(*target))
      {
        SlotAlgorithms::AssignSlot(*target);
      }

      if (isParentRootShadowRoot && slotParent != nullptr && slotParent->_assignedNodes.empty())
      {
        SlotAlgorithms::SignalSlotChange(*slotParent);
      }

      SlotAlgorithms::AssignSlottablesForTree(Downcast<ContainerNode>(TreeQueries::Root(*target)));

      for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(*target))
      {
        ExtensibilityHooks::NodeInserted(inclusiveDescendant);

        if (!inclusiveDescendant.IsConnected())
        {
          continue;
        }

        if (auto *element = DynamicDowncast<Element>(inclusiveDescendant))
        {
          // TODO(impl): CUSTOM-ELEMENTS
          // If inclusiveDescendant is an element and inclusiveDescendant’s custom element registry is
          // non-null:
          // If inclusiveDescendant’s custom element registry’s is scoped is true, then append
          // inclusiveDescendant’s node document to inclusiveDescendant’s custom element registry’s scoped
          // document set.
          // If inclusiveDescendant is custom, then enqueue a custom element callback reaction with
          // inclusiveDescendant, callback name "connectedCallback", and « ».
          // Otherwise, try to upgrade inclusiveDescendant.
        }
        else if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(inclusiveDescendant))
        {
          // TODO(impl): CUSTOM-ELEMENTS
          // Otherwise, if inclusiveDescendant is a shadow root, inclusiveDescendant’s custom element
          // registry is non-null, and inclusiveDescendant’s custom element registry’s is scoped is true,
          // then append inclusiveDescendant’s node document to inclusiveDescendant’s custom element
          // registry’s scoped document set.
        }
      }
    }

    if (!suppressObservers)
    {
      TreeMutationDispatcher::QueueTreeMutationRecord(parent, nodes, {}, ShareRefPtr(previousSibling),
                                                      ShareRefPtr(child));
    }

    ExtensibilityHooks::NodeChildrenChanged(parent);

    SmallNodeList staticNodeList;
    for (auto &target : nodes)
    {
      for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(*target))
      {
        staticNodeList.push_back(ShareRef(inclusiveDescendant));
      }
    }

    for (auto &target : staticNodeList)
    {
      if (target->IsConnected())
      {
        ExtensibilityHooks::NodePostConnection(*target);
      }
    }

    return {};
  }

  ExceptionOr<Node &> MutationAlgorithms::Append(Node &node, ContainerNode &parent) noexcept
  {
    return PreInsert(node, parent, nullptr);
  }

  ExceptionOr<void> MutationAlgorithms::Move(Node &node, ContainerNode &newParent,
                                             RawPtr<Node> child) noexcept
  {
    if (!ShadowRootAlgorithms::HasSameShadowIncludingRoot(newParent, node))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    if (TreeQueries::IsHostIncludingInclusiveAncestorOf(node, newParent))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    if (child != nullptr && child->ParentNode() != &newParent)
    {
      return ExceptionCode::NotFoundError;
    }

    if (!IsOneOf<Element, CharacterData>(node))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    if (Is<Text>(node) && Is<Document>(newParent))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    if (Is<Document>(newParent) && Is<Element>(node))
    {
      if (HasElementChild(newParent))
      {
        return ExceptionCode::HierarchyRequestError;
      }

      if (IsDocTypeOrDocTypeFollows(child))
      {
        return ExceptionCode::HierarchyRequestError;
      }
    }

    assert(node.ParentNode() != nullptr);
    auto &oldParent = *node.ParentNode();

    LiveRangeUpdater::PreRemove(node);

    for (auto iterator : node.NodeDocument().NodeIterators())
    {
      if (&iterator->Root().NodeDocument() == &node.NodeDocument())
      {
        IteratorAlgorithms::PreRemove(*iterator, node);
      }
    }

    auto *oldPreviousSibling = node.PreviousSibling();
    auto *oldNextSibling = node.NextSibling();

    if (auto previousSibling = node.PreviousSibling())
    {
      previousSibling->SetNextSibling(node.NextSibling());
    }
    else
    {
      assert(oldParent.FirstChild() == &node);
      oldParent.SetFirstChild(node.NextSibling());
    }

    if (auto nextSibling = node.NextSibling())
    {
      nextSibling->SetPreviousSibling(node.PreviousSibling());
    }
    else
    {
      assert(oldParent.LastChild() == &node);
      oldParent.SetLastChild(node.PreviousSibling());
    }
    assert(oldParent.FirstChild() != &node);
    assert(oldParent.LastChild() != &node);
    node.SetParentNode(nullptr);
    node.SetPreviousSibling(nullptr);
    node.SetNextSibling(nullptr);

    if (auto *assignedSlot = SlotAlgorithms::GetAssignedSlot(node))
    {
      SlotAlgorithms::AssignSlottables(*assignedSlot);
    }

    auto &oldParentRoot = TreeQueries::Root(oldParent);
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(oldParentRoot))
    {
      if (auto *slot = DynamicDowncast<HTMLSlotElement>(oldParent); slot && slot->_assignedNodes.empty())
      {
        SlotAlgorithms::SignalSlotChange(*slot);
      }
    }

    if (Krys::HTML::HasNodeOfType<HTMLSlotElement>(ConstInclusiveDescendantRange(node)))
    {
      SlotAlgorithms::AssignSlottablesForTree(oldParentRoot);
      SlotAlgorithms::AssignSlottablesForTree(node);
    }

    if (child != nullptr)
    {
      LiveRangeUpdater::MovedBeforeNode(*child);
    }

    auto *newPreviousSibling = child != nullptr ? child->PreviousSibling() : newParent.LastChild();

    if (child == nullptr)
    {
      if (auto *lastChild = newParent.LastChild())
      {
        node.SetPreviousSibling(lastChild);
        lastChild->SetNextSibling(&node);
      }
      else
      {
        newParent.SetFirstChild(&node);
      }

      newParent.SetLastChild(&node);
    }
    else
    {
      if (auto *previousSibling = child->PreviousSibling())
      {
        previousSibling->SetNextSibling(&node);
        node.SetPreviousSibling(previousSibling);

        child->SetPreviousSibling(&node);
        node.SetNextSibling(child);
      }
      else
      {
        assert(newParent.FirstChild() == child);
        newParent.SetFirstChild(&node);

        child->SetPreviousSibling(&node);
        node.SetNextSibling(child);
      }
    }
    node.SetParentNode(&newParent);

    // TODO(impl): SLOTTABLES
    // If newParent is a shadow host whose shadow root’s slot assignment is "named" and node is a slottable,
    // then assign a slot for node.
    // If newParent’s root is a shadow root, and newParent is a slot whose assigned nodes is empty, then run
    // signal a slot change for newParent.

    SlotAlgorithms::AssignSlottablesForTree(TreeQueries::Root(node));

    for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(node))
    {
      auto isSubtreeRoot = &inclusiveDescendant == &node;
      ExtensibilityHooks::NodeMoved(inclusiveDescendant, isSubtreeRoot, oldParent);

      // TODO(impl): CUSTOM-ELEMENTS
      // If inclusiveDescendant is custom and newParent is connected, then enqueue a custom element
      // callback reaction with inclusiveDescendant, callback name "connectedMoveCallback", and « ».
    }

    TreeMutationDispatcher::QueueTreeMutationRecord(
      oldParent, {}, {ShareRef(node)}, ShareRefPtr(oldPreviousSibling), ShareRefPtr(oldNextSibling));

    TreeMutationDispatcher::QueueTreeMutationRecord(newParent, {ShareRef(node)}, {},
                                                    ShareRefPtr(newPreviousSibling), ShareRefPtr(child));

    return {};
  }

  ExceptionOr<Node &> MutationAlgorithms::Replace(Node &child, Node &node, ContainerNode &parent) noexcept
  {
    if (!IsOneOf<Document, DocumentFragment, Element>(parent))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    if (TreeQueries::IsHostIncludingInclusiveAncestorOf(node, parent))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    if (child.ParentNode() != &parent)
    {
      return ExceptionCode::NotFoundError;
    }

    if (!IsOneOf<DocumentFragment, DocumentType, Element, CharacterData>(node))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    if ((Is<Text>(node) && Is<Document>(parent)) || (Is<DocumentType>(node) && !Is<Document>(parent)))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    if (Is<Document>(parent))
    {
      if (auto *documentFragment = DynamicDowncast<DocumentFragment>(node))
      {
        auto count = documentFragment->ChildElementCount();
        if (count > 1)
        {
          return ExceptionCode::HierarchyRequestError;
        }

        if (HasNodeOfType<Text>(ConstChildNodeRange(*documentFragment)))
        {
          return ExceptionCode::HierarchyRequestError;
        }

        if (count == 1)
        {
          if (std::ranges::any_of(ConstChildElementRange(parent), [&](auto &e) { return &e != &child; }))
          {
            return ExceptionCode::HierarchyRequestError;
          }

          if (HasNodeOfType<DocumentType>(ConstFollowingRange(child)))
          {
            return ExceptionCode::HierarchyRequestError;
          }
        }
      }
      else if (Is<Element>(node))
      {
        if (std::ranges::any_of(ConstChildElementRange(parent), [&](auto &e) { return &e != &child; }))
        {
          return ExceptionCode::HierarchyRequestError;
        }

        if (HasNodeOfType<DocumentType>(ConstFollowingRange(child)))
        {
          return ExceptionCode::HierarchyRequestError;
        }
      }
      else if (Is<DocumentType>(node))
      {
        if (std::ranges::any_of(ConstChildNodeRange(parent),
                                [&](auto &e) { return Is<DocumentType>(e) && &e != &child; }))
        {
          return ExceptionCode::HierarchyRequestError;
        }

        if (HasNodeOfType<Element>(ConstPrecedingRange(child)))
        {
          return ExceptionCode::HierarchyRequestError;
        }
      }
    }

    auto referenceChild = child.NextSibling();
    if (referenceChild == &node)
    {
      referenceChild = node.NextSibling();
    }

    auto previousSibling = child.PreviousSibling();

    SmallNodeList removedNodes;
    if (child.ParentNode())
    {
      removedNodes.push_back(ShareRef(child));
      if (auto result = Remove(child, SuppressObservers(true)); result.HasException())
      {
        return result.ReleaseException();
      }
    }

    SmallNodeList nodes;
    if (Is<DocumentFragment>(node))
    {
      nodes = CollectChildNodes(Downcast<ContainerNode>(node));
    }
    else
    {
      nodes.push_back(ShareRef(node));
    }

    if (auto result = Insert(node, parent, referenceChild, SuppressObservers(true)); result.HasException())
    {
      return result.ReleaseException();
    }

    TreeMutationDispatcher::QueueTreeMutationRecord(parent, nodes, removedNodes, ShareRefPtr(previousSibling),
                                                    ShareRefPtr(referenceChild));

    return child;
  }

  ExceptionOr<void> MutationAlgorithms::ReplaceAll(RawPtr<Node> node, ContainerNode &parent) noexcept
  {
    SmallNodeList removedNodes = CollectChildNodes(parent);

    SmallNodeList addedNodes;
    if (auto *documentFragment = DynamicDowncast<DocumentFragment>(node))
    {
      addedNodes = CollectChildNodes(*documentFragment);
    }
    else if (node != nullptr)
    {
      addedNodes.push_back(ShareRef(*node));
    }

    while (auto *firstChild = parent.FirstChild())
    {
      if (auto result = Remove(*firstChild, SuppressObservers(true)); result.HasException())
      {
        return result.ReleaseException();
      }
    }

    if (node != nullptr)
    {
      if (auto result = Insert(*node, parent, nullptr, SuppressObservers(true)); result.HasException())
      {
        return result.ReleaseException();
      }
    }

    if (!addedNodes.empty() || !removedNodes.empty())
    {
      TreeMutationDispatcher::QueueTreeMutationRecord(parent, addedNodes, removedNodes, nullptr, nullptr);
    }

    return {};
  }

  ExceptionOr<Node &> MutationAlgorithms::PreRemove(Node &node, ContainerNode &parent) noexcept
  {
    if (node.ParentNode() != &parent)
    {
      return ExceptionCode::NotFoundError;
    }

    if (auto result = Remove(node); result.HasException())
    {
      return result.ReleaseException();
    }

    return node;
  }

  ExceptionOr<void> MutationAlgorithms::Remove(Node &node, SuppressObservers suppressObservers) noexcept
  {
    auto protectedNode = Protect(node);

    assert(node.ParentNode() != nullptr);
    auto &parent = *node.ParentNode();

    LiveRangeUpdater::PreRemove(node);

    for (auto iterator : node.NodeDocument().NodeIterators())
    {
      if (&iterator->Root() == &node.NodeDocument())
      {
        IteratorAlgorithms::PreRemove(*iterator, node);
      }
    }

    auto oldPreviousSibling = node.PreviousSibling();
    auto oldNextSibling = node.NextSibling();

    if (auto previousSibling = node.PreviousSibling())
    {
      previousSibling->SetNextSibling(node.NextSibling());
    }
    else
    {
      assert(parent.FirstChild() == &node);
      parent.SetFirstChild(node.NextSibling());
    }

    if (auto nextSibling = node.NextSibling())
    {
      nextSibling->SetPreviousSibling(node.PreviousSibling());
    }
    else
    {
      assert(parent.LastChild() == &node);
      parent.SetLastChild(node.PreviousSibling());
    }
    assert(parent.FirstChild() != &node);
    assert(parent.LastChild() != &node);
    node.SetParentNode(nullptr);
    node.SetPreviousSibling(nullptr);
    node.SetNextSibling(nullptr);

    if (auto *assignedSlot = SlotAlgorithms::GetAssignedSlot(node))
    {
      SlotAlgorithms::AssignSlottables(*assignedSlot);
    }

    auto &parentRoot = TreeQueries::Root(parent);
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(parentRoot))
    {
      if (auto *slot = DynamicDowncast<HTMLSlotElement>(parent); slot && slot->_assignedNodes.empty())
      {
        SlotAlgorithms::SignalSlotChange(*slot);
      }
    }

    if (Krys::HTML::HasNodeOfType<HTMLSlotElement>(ConstInclusiveDescendantRange(node)))
    {
      SlotAlgorithms::AssignSlottablesForTree(parentRoot);
      SlotAlgorithms::AssignSlottablesForTree(node);
    }

    ExtensibilityHooks::NodeRemoved(node, true, parent);

    // TODO(impl): CUSTOM-ELEMENTS
    // bool isParentConnected = parent.IsConnected();
    // if node is custom and isParentConnected is true, then enqueue a custom element callback
    // reaction with node, callback name "disconnectedCallback", and « ».

    for (auto &descendant : ShadowIncludingDescendantRange(node))
    {
      ExtensibilityHooks::NodeRemoved(descendant, false, parent);
      // TODO(impl): CUSTOM-ELEMENTS
      // If descendant is custom and isParentConnected is true, then enqueue a custom element callback
      // reaction with descendant, callback name "disconnectedCallback", and « ».
    }

    // TODO(impl): MUTATION-OBSERVERS
    // For each inclusive ancestor inclusiveAncestor of parent, and then for each registered of
    // inclusiveAncestor’s registered observer list, if registered’s options["subtree"] is true, then append a
    // new transient registered observer whose observer is registered’s observer, options is registered’s
    // options, and source is registered to node’s registered observer list.

    if (!suppressObservers)
    {
      TreeMutationDispatcher::QueueTreeMutationRecord(parent, {}, {Krys::Move(protectedNode)},
                                                      ShareRefPtr(oldPreviousSibling),
                                                      ShareRefPtr(oldNextSibling));
    }

    ExtensibilityHooks::NodeChildrenChanged(parent);

    return {};
  }
}
