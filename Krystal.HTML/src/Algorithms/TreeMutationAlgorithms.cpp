#include "Krystal.HTML/Algorithms/TreeMutationAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
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
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

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
      if (node.IsDocumentFragmentNode())
      {
        uint32 elementCount = 0;
        for (Node &fragmentChild : ChildNodeRange(Downcast<ContainerNode>(node)))
        {
          if (fragmentChild.IsElementNode())
          {
            ++elementCount;
            if (elementCount > 1)
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
          if (TreeQueries::HasElementChild(parent))
          {
            return Exception {ExceptionCode::HierarchyRequestError};
          }

          if (TreeQueries::IsDocTypeOrDocTypeFollows(child))
          {
            return Exception {ExceptionCode::HierarchyRequestError};
          }
        }
      }
      else if (node.IsElementNode())
      {
        if (TreeQueries::HasElementChild(parent))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }

        if (TreeQueries::IsDocTypeOrDocTypeFollows(child))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }
      }
      else if (node.IsDocumentTypeNode())
      {
        if (std::ranges::any_of(
              ChildNodeRange(parent), [&](const Node &current)
              { return current.IsDocumentTypeNode() || (current.IsElementNode() && child == nullptr); }))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }

        if (child != nullptr
            && std::ranges::any_of(PrecedingRange(*child),
                                   [](const Node &current) { return current.IsElementNode(); }))
        {
          return Exception {ExceptionCode::HierarchyRequestError};
        }
      }
    }

    return {};
  }

  ExceptionOr<void> TreeMutationAlgorithms::PreInsert(Node &node, ContainerNode &parent,
                                                      RawPtr<Node> refChild) noexcept
  {
    if (auto result = EnsurePreInsertValidity(node, parent, refChild); result.HasException())
    {
      return {result.ReleaseException()};
    }

    if (&node == refChild)
    {
      refChild = node.NextSibling();
    }

    return Insert(node, parent, refChild);
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

    if (node.IsDocumentFragmentNode())
    {
      if (auto removeAllResult = RemoveAllChildren(Downcast<ContainerNode>(node), SuppressObservers(true));
          removeAllResult.HasException())
      {
        return {removeAllResult.ReleaseException()};
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

    auto *parentRoot = DynamicDowncast<ShadowRoot>(TreeQueries::Root(parent));
    auto *parentAsElement = DynamicDowncast<Element>(parent);
    auto *parentShadowRoot = parentAsElement ? parentAsElement->GetShadowRoot() : nullptr;
    auto *parentAsSlot = DynamicDowncast<HTMLSlotElement>(parent);

    auto *previousSibling = child ? child->PreviousSibling() : parent.LastChild();
    for (auto &target : nodes)
    {
      if (auto result = parent.OwnerDocument()->AdoptNode(*target); result.HasException())
      {
        return {result.ReleaseException()};
      }
      target->SetParentNode(&parent);
      target->SetTreeScopeRecursively(*parent.OwnerDocument());

      if (child == nullptr)
      {
        if (parent.LastChild())
        {
          target->SetPreviousSibling(parent.LastChild());
          parent.LastChild()->SetNextSibling(target.get());
        }
        else
        {
          parent.SetFirstChild(target.get());
        }

        parent.SetLastChild(target.get());
      }
      else
      {
        if (child->PreviousSibling())
        {
          child->PreviousSibling()->SetNextSibling(target.get());
          target->SetPreviousSibling(child->PreviousSibling());

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

      if (parentShadowRoot && parentShadowRoot->SlotAssignment() == SlotAssignmentMode::Named
          && SlotAssignmentAlgorithms::IsSlottable(*target))
      {
        SlotAssignmentAlgorithms::AssignSlot(*target);
      }

      if (parentShadowRoot && parentAsSlot && parentAsSlot->AssignedNodes().empty())
      {
        SlotAssignmentAlgorithms::SignalSlotChange(*parentAsSlot);
      }

      SlotAssignmentAlgorithms::AssignSlottablesForTree(Downcast<ContainerNode>(TreeQueries::Root(*target)));
      if (auto *targetContainer = DynamicDowncast<ContainerNode>(*target))
      {
        for (auto &inclusiveDescendant : InclusiveShadowIncludingDescendantRange(*targetContainer))
        {
          TreeMutationDispatcher::Inserted(inclusiveDescendant);

          if (!inclusiveDescendant.IsConnected())
          {
            continue;
          }

          if (auto *inclusiveDescendantAsElement = DynamicDowncast<Element>(inclusiveDescendant))
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
          else if (auto *inclusiveDescendantAsShadowRoot = DynamicDowncast<ShadowRoot>(inclusiveDescendant))
          {
            // Otherwise, if inclusiveDescendant is a shadow root, inclusiveDescendant’s custom element
            // registry is non-null, and inclusiveDescendant’s custom element registry’s is scoped is true,
            // then append inclusiveDescendant’s node document to inclusiveDescendant’s custom element
            // registry’s scoped document set.
          }
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

  ExceptionOr<void> TreeMutationAlgorithms::Append(Node &node, ContainerNode &parent) noexcept
  {
    return PreInsert(node, parent, nullptr);
  }

  ExceptionOr<void> TreeMutationAlgorithms::Move(Node &node, ContainerNode &newParent,
                                                 RawPtr<Node> refChild) noexcept
  {
    if (!TreeQueries::HasSameShadowIncludingRoot(newParent, node))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (TreeQueries::IsHostIncludingInclusiveAncestorOf(node, newParent))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (refChild != nullptr && refChild->ParentNode() != &newParent)
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

      if (refChild != nullptr && TreeQueries::IsDocTypeOrDocTypeFollows(refChild))
      {
        return Exception {ExceptionCode::HierarchyRequestError};
      }
    }

    auto *oldParent = node.ParentNode();
    assert(oldParent != nullptr);

    TreeMutationDispatcher::LiveRangePreRemove(node);

    // TODO(IMPL): For each NodeIterator object iterator whose root’s node document is node’s node document:
    // run the NodeIterator pre-remove steps given node and iterator.

    auto *oldPreviousSibling = node.PreviousSibling();
    auto *oldNextSibling = node.NextSibling();

    if (auto result = Remove(node, *oldParent); result.HasException())
    {
      return {result.ReleaseException()};
    }

    // TODO(IMPL):

    return {};
  }

  ExceptionOr<void> TreeMutationAlgorithms::PreRemove(Node &node, ContainerNode &parent) noexcept
  {
    if (node.ParentNode() != &parent)
    {
      return Exception {ExceptionCode::NotFoundError};
    }

    return Remove(node, parent);
  }

  ExceptionOr<void> TreeMutationAlgorithms::Remove(Node &node, ContainerNode &parent,
                                                   SuppressObservers suppressObservers) noexcept
  {
    if (auto previousSibling = ShareRefPtr(node.PreviousSibling()))
    {
      previousSibling->SetNextSibling(node.NextSibling());
    }
    else
    {
      assert(parent.FirstChild() == &node);
      parent.SetFirstChild(node.NextSibling());
    }

    if (auto nextSibling = ShareRefPtr(node.NextSibling()))
    {
      nextSibling->SetPreviousSibling(node.PreviousSibling());
    }
    else
    {
      assert(parent.LastChild() == &node);
      parent.SetLastChild(node.PreviousSibling());
    }

    node.SetParentNode(nullptr);
    node.SetPreviousSibling(nullptr);
    node.SetNextSibling(nullptr);

    assert(parent.FirstChild() != &node);
    assert(parent.LastChild() != &node);
    assert(!node.PreviousSibling());
    assert(!node.NextSibling());

    TreeMutationDispatcher::Removed(node, true, parent);

    // TODO(impl):

    return {};
  }

  ExceptionOr<void> TreeMutationAlgorithms::RemoveAllChildren(ContainerNode &parent,
                                                              SuppressObservers suppressObservers) noexcept
  {
    while (RawPtr<Node> child = parent.FirstChild())
    {
      auto result = Remove(*child, parent, suppressObservers);
      if (result.HasException())
      {
        return {result.ReleaseException()};
      }
    }

    return {};
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
