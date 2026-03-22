#include "Krystal.HTML/Algorithms/TreeMutationAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/SlotAssignmentAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/Algorithms/TreeMutationDispatcher.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"

namespace Krys::HTML
{
  ExceptionOr<void> TreeMutationAlgorithms::EnsurePreInsertValidity(Node &node, ContainerNode &parent,
                                                                    RawPtr<Node> child) noexcept
  {
    if (!parent.IsDocumentNode() && !parent.IsDocumentFragmentNode() && !parent.IsElementNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (TreeQueries::IsHostIncludingInclusiveAncestorOf(node, parent))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (child && child->ParentNode() != &parent)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    if (!node.IsDocumentFragmentNode() && !node.IsDocumentTypeNode() && !node.IsElementNode()
        && !node.IsCharacterDataNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if ((node.IsTextNode() && parent.IsDocumentNode())
        || (node.IsDocumentTypeNode() && !parent.IsDocumentNode()))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (parent.IsDocumentNode())
    {
      if (auto *documentFragment = DynamicDowncast<DocumentFragment>(node))
      {
        auto count = TreeQueries::ChildElementCount(*documentFragment);
        if (count > 1)
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }

        if (std::ranges::any_of(ConstChildNodeRange(*documentFragment),
                                [](auto &c) { return c.IsTextNode(); }))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }

        if (count && (TreeQueries::HasElementChild(parent) || TreeQueries::IsDocTypeOrDocTypeFollows(child)))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }
      }
      else if (node.IsElementNode())
      {
        if (TreeQueries::HasElementChild(parent) || TreeQueries::IsDocTypeOrDocTypeFollows(child))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }
      }
      else if (node.IsDocumentTypeNode())
      {
        if (std::ranges::any_of(ChildNodeRange(parent), [](auto &c) { return c.IsDocumentTypeNode(); }))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }

        if (child != nullptr
            && std::ranges::any_of(PrecedingRange(*child), [](auto &c) { return c.IsElementNode(); }))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }

        if (child == nullptr && TreeQueries::HasElementChild(parent))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }
      }
    }

    return {};
  }

  ExceptionOr<Node &> TreeMutationAlgorithms::PreInsert(Node &node, ContainerNode &parent,
                                                        RawPtr<Node> refChild) noexcept
  {
    if (auto result = EnsurePreInsertValidity(node, parent, refChild); result.HasException())
    {
      return result.ReleaseException();
    }

    if (&node == refChild)
    {
      refChild = node.NextSibling();
    }

    if (auto result = Insert(node, parent, refChild); result.HasException())
    {
      return result.ReleaseException();
    }

    return node;
  }

  ExceptionOr<void> TreeMutationAlgorithms::Insert(Node &node, ContainerNode &parent, RawPtr<Node> child,
                                                   SuppressObservers suppressObservers) noexcept
  {
    SmallNodeList nodes;
    if (node.IsDocumentFragmentNode())
    {
      TreeQueries::CollectChildNodes(Downcast<ContainerNode>(node), nodes);
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
      auto childIndex = TreeQueries::Index(*child);
      for (auto &range : parent.NodeDocument().LiveRanges())
      {
        if (range->StartContainer() == &parent && range->StartOffset() > childIndex)
        {
          range->_start.Offset += count;
        }

        if (range->EndContainer() == &parent && range->EndOffset() > childIndex)
        {
          range->_end.Offset += count;
        }
      }
    }

    auto *previousSibling = child != nullptr ? child->PreviousSibling() : parent.LastChild();
    auto &parentRoot = TreeQueries::Root(parent);
    auto *slotParent = DynamicDowncast<HTMLSlotElement>(parent);
    for (auto &target : nodes)
    {
      if (auto result = parent.OwnerDocument()->AdoptNode(*target); result.HasException())
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

      // TODO(fix): i'm not a fan of this tree scope business. think about how to make it better.
      target->SetTreeScopeRecursively(*parent.OwnerDocument());

      if (auto *shadowHost = DynamicDowncast<Element>(parent))
      {
        if (shadowHost->GetShadowRoot()
            && shadowHost->GetShadowRoot()->SlotAssignment() == SlotAssignmentMode::Named
            && SlotAssignmentAlgorithms::IsSlottable(*target))
        {
          SlotAssignmentAlgorithms::AssignSlot(*target);
        }
      }

      if (Is<ShadowRoot>(parentRoot) && slotParent && slotParent->AssignedNodes().empty())
      {
        SlotAssignmentAlgorithms::SignalSlotChange(*slotParent);
      }

      SlotAssignmentAlgorithms::AssignSlottablesForTree(Downcast<ContainerNode>(TreeQueries::Root(*target)));

      for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(*target))
      {
        TreeMutationDispatcher::Inserted(inclusiveDescendant);

        if (!inclusiveDescendant.IsConnected())
        {
          continue;
        }

        if (auto *element = DynamicDowncast<Element>(inclusiveDescendant))
        {
          // TODO(impl):
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
          // TODO(impl):
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

    TreeMutationDispatcher::ChildrenChanged(parent);

    List<Ref<Node>> staticNodeList;

    for (auto &target : nodes)
    {
      for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(*target))
      {
        staticNodeList.push_back(ShareRef(inclusiveDescendant));
      }
    }

    for (auto &node : staticNodeList)
    {
      if (node->IsConnected())
      {
        TreeMutationDispatcher::PostConnection(*node);
      }
    }

    return {};
  }

  ExceptionOr<Node &> TreeMutationAlgorithms::Append(Node &node, ContainerNode &parent) noexcept
  {
    return PreInsert(node, parent, nullptr);
  }

  ExceptionOr<void> TreeMutationAlgorithms::Move(Node &node, ContainerNode &newParent,
                                                 RawPtr<Node> child) noexcept
  {
    if (!TreeQueries::HasSameShadowIncludingRoot(newParent, node))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (TreeQueries::IsHostIncludingInclusiveAncestorOf(node, newParent))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (child != nullptr && child->ParentNode() != &newParent)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    if (!node.IsElementNode() && !node.IsCharacterDataNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (node.IsTextNode() && newParent.IsDocumentNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (newParent.IsDocumentNode() && node.IsElementNode())
    {
      if (TreeQueries::HasElementChild(newParent))
      {
        return Exception {ExceptionCode::HierarchyRequestError};
      }

      if (TreeQueries::IsDocTypeOrDocTypeFollows(child))
      {
        return Exception {ExceptionCode::HierarchyRequestError};
      }
    }

    assert(node.ParentNode() != nullptr);
    auto &oldParent = *node.ParentNode();

    TreeMutationDispatcher::LiveRangePreRemove(node);

    for (auto iterator : node.NodeDocument().NodeIterators())
    {
      if (&iterator->Root() == &node.NodeDocument())
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

    if (auto *assignedSlot = SlotAssignmentAlgorithms::GetAssignedSlot(node))
    {
      SlotAssignmentAlgorithms::AssignSlottables(*assignedSlot);
    }

    auto &oldParentRoot = TreeQueries::Root(oldParent);
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(oldParentRoot))
    {
      if (auto *slot = DynamicDowncast<HTMLSlotElement>(oldParent); slot && slot->AssignedNodes().empty())
      {
        SlotAssignmentAlgorithms::SignalSlotChange(*slot);
      }
    }

    if (Krys::HTML::HasNodeOfType<HTMLSlotElement>(ConstInclusiveDescendantRange(node)))
    {
      SlotAssignmentAlgorithms::AssignSlottablesForTree(oldParentRoot);
      SlotAssignmentAlgorithms::AssignSlottablesForTree(node);
    }

    if (child != nullptr)
    {
      auto childIndex = TreeQueries::Index(*child);
      for (auto range : newParent.NodeDocument().LiveRanges())
      {
        if (range->_start.Container == &newParent && range->_start.Offset > childIndex)
        {
          range->_start.Offset += 1;
        }

        if (range->_end.Container == &newParent && range->_end.Offset > childIndex)
        {
          range->_end.Offset += 1;
        }
      }
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

    SlotAssignmentAlgorithms::AssignSlottablesForTree(TreeQueries::Root(node));

    for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(node))
    {
      auto isSubtreeRoot = &inclusiveDescendant == &node;
      TreeMutationDispatcher::Moved(inclusiveDescendant, node, isSubtreeRoot, oldParent);

      // TODO(impl):If inclusiveDescendant is custom and newParent is connected, then enqueue a custom element
      // callback reaction with inclusiveDescendant, callback name "connectedMoveCallback", and « ».
    }

    TreeMutationDispatcher::QueueTreeMutationRecord(
      oldParent, {}, {ShareRef(node)}, ShareRefPtr(oldPreviousSibling), ShareRefPtr(oldNextSibling));

    TreeMutationDispatcher::QueueTreeMutationRecord(newParent, {ShareRef(node)}, {},
                                                    ShareRefPtr(newPreviousSibling), ShareRefPtr(child));

    return {};
  }

  ExceptionOr<Node &> TreeMutationAlgorithms::Replace(Node &child, Node &node, ContainerNode &parent) noexcept
  {
    if (!parent.IsDocumentNode() && !parent.IsDocumentFragmentNode() && !parent.IsElementNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (TreeQueries::IsHostIncludingInclusiveAncestorOf(node, parent))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (child.ParentNode() != &parent)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    if (!node.IsDocumentFragmentNode() && !node.IsDocumentTypeNode() && !node.IsElementNode()
        && !node.IsCharacterDataNode())
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if ((node.IsTextNode() && parent.IsDocumentNode())
        || (node.IsDocumentTypeNode() && !parent.IsDocumentNode()))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (parent.IsDocumentNode())
    {
      if (node.IsDocumentFragmentNode())
      {
        uint32 elementCount = 0;
        for (Node &fragmentChild : ChildNodeRange(Downcast<ContainerNode>(node)))
        {
          if (fragmentChild.IsElementNode())
          {
            if (++elementCount > 1)
            {
              return Exception {ExceptionCode::HierarchyRequestError};
            }
          }
          else if (fragmentChild.IsTextNode())
          {
            return Exception {ExceptionCode::HierarchyRequestError};
          }
        }

        if (elementCount == 1)
        {
          if (std::ranges::any_of(ConstChildElementRange(parent), [&](auto &e) { return &e != &child; }))
          {
            return Exception {ExceptionCode::HierarchyRequestError};
          }

          if (std::ranges::any_of(ConstFollowingRange(child), [](auto &e) { return e.IsDocumentTypeNode(); }))
          {
            return Exception {ExceptionCode::HierarchyRequestError};
          }
        }
      }
      else if (node.IsElementNode())
      {
        if (std::ranges::any_of(ConstChildElementRange(parent), [&](auto &e) { return &e != &child; }))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }

        if (std::ranges::any_of(ConstFollowingRange(child), [](auto &e) { return e.IsDocumentTypeNode(); }))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }
      }
      else if (node.IsDocumentFragmentNode())
      {
        if (std::ranges::any_of(ConstChildNodeRange(parent),
                                [&](auto &e) { return e.IsDocumentTypeNode() && &e != &child; }))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }

        if (std::ranges::any_of(ConstPrecedingRange(child), [](auto &e) { return e.IsElementNode(); }))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
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
    if (node.IsDocumentFragmentNode())
    {
      TreeQueries::CollectChildNodes(Downcast<ContainerNode>(node), nodes);
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

  ExceptionOr<void> TreeMutationAlgorithms::ReplaceAll(RawPtr<Node> node, ContainerNode &parent) noexcept
  {
    SmallNodeList removedNodes;
    TreeQueries::CollectChildNodes(parent, removedNodes);

    SmallNodeList addedNodes;
    if (node != nullptr)
    {
      if (auto *documentFragment = DynamicDowncast<DocumentFragment>(node))
      {
        TreeQueries::CollectChildNodes(*documentFragment, addedNodes);
      }
      else
      {
        addedNodes.push_back(ShareRef(*node));
      }
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

  ExceptionOr<Node &> TreeMutationAlgorithms::PreRemove(Node &node, ContainerNode &parent) noexcept
  {
    if (node.ParentNode() != &parent)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    if (auto result = Remove(node); result.HasException())
    {
      return result.ReleaseException();
    }

    return node;
  }

  ExceptionOr<void> TreeMutationAlgorithms::Remove(Node &node, SuppressObservers suppressObservers) noexcept
  {
    assert(node.ParentNode() != nullptr);
    auto &parent = *node.ParentNode();

    TreeMutationDispatcher::LiveRangePreRemove(node);

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

    if (auto *assignedSlot = SlotAssignmentAlgorithms::GetAssignedSlot(node))
    {
      SlotAssignmentAlgorithms::AssignSlottables(*assignedSlot);
    }

    auto &parentRoot = TreeQueries::Root(parent);
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(parentRoot))
    {
      if (auto *slot = DynamicDowncast<HTMLSlotElement>(parent); slot && slot->AssignedNodes().empty())
      {
        SlotAssignmentAlgorithms::SignalSlotChange(*slot);
      }
    }

    if (Krys::HTML::HasNodeOfType<HTMLSlotElement>(ConstInclusiveDescendantRange(node)))
    {
      SlotAssignmentAlgorithms::AssignSlottablesForTree(parentRoot);
      SlotAssignmentAlgorithms::AssignSlottablesForTree(node);
    }

    TreeMutationDispatcher::Removed(node, true, parent);

    // bool isParentConnected = parent.IsConnected();
    // TODO(impl): if node is custom and isParentConnected is true, then enqueue a custom element callback
    // reaction with node, callback name "disconnectedCallback", and « ».

    for (auto &descendant : InclusiveShadowIncludingDescendantRange(node))
    {
      TreeMutationDispatcher::Removed(descendant, false, parent);
      // TODO(impl): If descendant is custom and isParentConnected is true, then enqueue a custom element
      // callback reaction with descendant, callback name "disconnectedCallback", and « ».
    }

    // TODO(impl): For each inclusive ancestor inclusiveAncestor of parent, and then for each registered of
    // inclusiveAncestor’s registered observer list, if registered’s options["subtree"] is true, then append a
    // new transient registered observer whose observer is registered’s observer, options is registered’s
    // options, and source is registered to node’s registered observer list.

    if (!suppressObservers)
    {
      TreeMutationDispatcher::QueueTreeMutationRecord(
        parent, {}, {ShareRef(node)}, ShareRefPtr(oldPreviousSibling), ShareRefPtr(oldNextSibling));
    }

    TreeMutationDispatcher::ChildrenChanged(parent);

    return {};
  }

  ExceptionOr<Ref<Node>> TreeMutationAlgorithms::ConvertNodesIntoNode(const List<NodeOrString> &nodes,
                                                                      Document &document) noexcept
  {
    List<Ref<Node>> nodeList;
    for (auto &nodeOrString : nodes)
    {
      if (std::holds_alternative<DOMString>(nodeOrString))
      {
        DOMString copy = std::get<DOMString>(nodeOrString);
        nodeList.emplace_back(CreateRef<Text>(document, Krys::Move(copy)));
      }
      else
      {
        nodeList.push_back(std::get<Ref<Node>>(nodeOrString));
      }
    }

    if (nodeList.size() == 1)
    {
      return nodeList[0];
    }

    auto fragment = CreateRef<DocumentFragment>(document);
    for (auto &node : nodeList)
    {
      if (auto result = Append(*node, *fragment); result.HasException())
      {
        return result.ReleaseException();
      }
    }

    return AdoptRef<Node>(*fragment);
  }

  Ref<Node> TreeMutationAlgorithms::CloneNode(Node &node, RawPtr<Document> document, bool subtree,
                                              RawPtr<ContainerNode> parent,
                                              RawPtr<CustomElementRegistry> fallbackRegistry) noexcept
  {
    if (document == nullptr)
    {
      document = node.OwnerDocument();
    }

    assert(!node.IsDocumentNode() || &node == document);

    auto copy = CloneSingleNode(node, *document, fallbackRegistry);
    TreeMutationDispatcher::Cloned(node, *copy, subtree);

    if (parent != nullptr)
    {
      parent->AppendChild(*copy);
    }

    if (subtree)
    {
      assert(node.IsContainerNode());
      for (auto &child : ChildNodeRange(Downcast<ContainerNode>(node)))
      {
        CloneNode(child, document, subtree, Downcast<ContainerNode>(copy.get()), fallbackRegistry);
      }
    }

    if (auto *element = DynamicDowncast<Element>(node))
    {
      auto *elementCopy = DynamicDowncast<Element>(copy.get());
      if (auto *shadowRoot = element->GetShadowRoot(); shadowRoot && shadowRoot->Clonable())
      {
        assert(elementCopy->GetShadowRoot() && !elementCopy->GetShadowRoot()->Clonable());
        // TODO(IMPL):
      }
    }

    return copy;
  }

  Ref<Node> TreeMutationAlgorithms::CloneSingleNode(Node &node, Document &document,
                                                    RawPtr<CustomElementRegistry> fallbackRegistry) noexcept
  {
    RefPtr<Node> copy = nullptr;

    if (auto *element = DynamicDowncast<Element>(node))
    {
      // TODO(IMPL):
    }
    else
    {
      if (node.IsDocumentNode())
      {
        // TODO(IMPL):
      }
      else if (node.IsDocumentTypeNode())
      {
        // TODO(IMPL):
      }
      else if (node.IsAttributeNode())
      {
        // TODO(IMPL):
      }
      else if (node.IsTextNode() || node.IsCommentNode())
      {
        // TODO(IMPL):
      }
      else if (node.IsProcessingInstructionNode())
      {
        // TODO(IMPL):
      }
    }

    assert(copy);

    return copy;
  }
}
