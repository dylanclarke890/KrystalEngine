#include "Krystal.Booey/DOM/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/NodeIterator.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/TreeWalker.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom
{
  ExceptionOr<FilterResult> IteratorAlgorithms::FilterNode(TraversalBase &traverser,
                                                           const Node &node) noexcept
  {
    // 1. If traverser’s is active is true, then throw an "InvalidStateError" DOMException.
    if (traverser.IsActive())
    {
      return Exception {ExceptionCode::InvalidStateError};
    }

    // 2. Let n be node’s nodeType attribute value − 1.
    auto n = static_cast<underlying_t<NodeType>>(node.NodeType()) - 1u;

    // 3. If the nth bit (where 0 is the least significant bit) of traverser’s whatToShow is not set, then
    // return
    //    FILTER_SKIP.
    auto whatToShow = static_cast<underlying_t<WhatToShow>>(traverser.WhatToShow());
    if ((whatToShow & (1u << n)) == 0u)
    {
      return FilterResult::FILTER_SKIP;
    }

    // 4. If traverser’s filter is null, then return FILTER_ACCEPT.
    if (!traverser.Filter())
    {
      return FilterResult::FILTER_ACCEPT;
    }

    // 5. Set traverser’s is active to true.
    traverser.IsActive(true);

    // 6. Let result be the return value of call a user object’s operation with traverser’s filter,
    // "acceptNode",
    //    and « node ». If this throws an exception, then set traverser’s is active to false and rethrow the
    //    exception.
    auto result = traverser.Filter()->AcceptNode(node);

    // 7. Set traverser’s is active to false.
    traverser.IsActive(false);

    // 8. Return result.
    return result;
  }

  void IteratorAlgorithms::PreRemove(NodeIterator &nodeIterator, const Node &toBeRemovedNode) noexcept
  {
    // 1. Set nodeIterator’s reference to the result of adjusting a node pointer given nodeIterator’s
    // reference,
    //    nodeIterator, and toBeRemovedNode.
    nodeIterator._reference = AdjustNodePointer(nodeIterator._reference, nodeIterator, toBeRemovedNode);

    // 2. If nodeIterator’s candidate reference is non-null, then set nodeIterator’s candidate reference to
    // the
    //    result of adjusting a node pointer given nodeIterator’s candidate reference, nodeIterator, and
    //    toBeRemovedNode.
    if (nodeIterator._candidateReference)
    {
      nodeIterator._candidateReference =
        AdjustNodePointer(*nodeIterator._candidateReference, nodeIterator, toBeRemovedNode);
    }
  }

  NodePointer IteratorAlgorithms::AdjustNodePointer(const NodePointer &nodePointer,
                                                    const NodeIterator &nodeIterator,
                                                    const Node &toBeRemovedNode) noexcept
  {
    // 1. If toBeRemovedNode is not an inclusive ancestor of nodePointer’s node, or toBeRemovedNode is an
    // inclusive
    //    ancestor of nodeIterator’s root, then return nodePointer.
    if (!TreeQueries::IsInclusiveAncestor(toBeRemovedNode, *nodePointer.Node)
        || TreeQueries::IsInclusiveAncestor(toBeRemovedNode, nodeIterator.Root()))
    {
      return nodePointer;
    }

    // 2. If nodePointer’s pointer before is true:
    if (nodePointer.PointerBefore)
    {
      // 2a. Let next be toBeRemovedNode’s first following node that is an inclusive descendant of
      // nodeIterator’s
      //     root and is not an inclusive descendant of toBeRemovedNode, if there is such a node; otherwise
      //     null.
      Node *next = TreeQueries::NextSkippingChildren(toBeRemovedNode, &nodeIterator.Root());

      // 2b. If next is non-null, then return (next, true).
      if (next)
      {
        return {ShareRef(*next), true};
      }
    }

    // 3. Let newNode be toBeRemovedNode’s parent, if toBeRemovedNode’s previous sibling is null; otherwise
    // the
    //    inclusive descendant of toBeRemovedNode’s previous sibling that appears last in tree order.
    Node *newNode = toBeRemovedNode.PreviousSibling()
                      ? TreeQueries::DeepLastChild(*toBeRemovedNode.PreviousSibling())
                      : toBeRemovedNode.ParentNode();
    krys_debug_assert(newNode);

    // 4. Return (newNode, false).
    return {ShareRef(*newNode), false};
  }

  ExceptionOr<RefPtr<Node>> IteratorAlgorithms::Traverse(NodeIterator &iterator,
                                                         TraversalNextOrPrev type) noexcept
  {
    // A traversal from inside the iterator's filter would replace the candidate reference of the traversal
    // that called the filter, so it throws before changing anything.
    if (iterator.IsActive())
    {
      return Exception {ExceptionCode::InvalidStateError};
    }

    // 1. Set iterator’s candidate reference to iterator’s reference.
    iterator._candidateReference = iterator._reference;

    // 2. Let result be null.
    RefPtr<Node> result;

    // 3. While true:
    while (true)
    {
      // The filter can remove nodes, which adjusts the candidate reference, so it's read afresh each time.
      NodePointer &candidate = *iterator._candidateReference;

      // 3a. If type is "next":
      if (type == TraversalNextOrPrev::Next)
      {
        // 3a.i. If iterator’s candidate reference’s pointer before is false:
        if (!candidate.PointerBefore)
        {
          // 3a.i.A. Let following be the first node following iterator’s candidate reference’s node in
          // iterator’s
          //         iterator collection. If there is no such node, then break.
          Node *following = TreeQueries::Next(*candidate.Node, &iterator.Root());
          if (!following)
          {
            break;
          }

          // 3a.i.B. Set iterator’s candidate reference to (following, false).
          candidate = {ShareRef(*following), false};
        }
        // 3a.ii. Otherwise, set iterator’s candidate reference to (iterator’s candidate reference’s node,
        // false).
        else
        {
          candidate.PointerBefore = false;
        }
      }
      // 3b. Otherwise:
      else
      {
        // 3b.i. If iterator’s candidate reference’s pointer before is true:
        if (candidate.PointerBefore)
        {
          // 3b.i.A. Let preceding be the first node preceding iterator’s candidate reference’s node in
          // iterator’s
          //         iterator collection. If there is no such node, then break.
          Node *preceding = TreeQueries::Previous(*candidate.Node, &iterator.Root());
          if (!preceding)
          {
            break;
          }

          // 3b.i.B. Set iterator’s candidate reference to (preceding, true).
          candidate = {ShareRef(*preceding), true};
        }
        // 3b.ii. Otherwise, set iterator’s candidate reference to (iterator’s candidate reference’s node,
        // true).
        else
        {
          candidate.PointerBefore = true;
        }
      }

      // 3c. Let node be iterator’s candidate reference’s node.
      Ref<Node> node = candidate.Node;

      // 3d. Let filterResult be the result of filtering node within iterator. If this throws an exception,
      // then set
      //     iterator’s candidate reference to null and rethrow that exception.
      auto filterResult = FilterNode(iterator, *node);
      if (filterResult.HasException())
      {
        iterator._candidateReference = null;
        return filterResult.ReleaseException();
      }

      // 3e. If filterResult is FILTER_ACCEPT:
      if (filterResult.Value() == FilterResult::FILTER_ACCEPT)
      {
        // 3e.i. Set iterator’s reference to iterator’s candidate reference.
        iterator._reference = *iterator._candidateReference;

        // 3e.ii. Set result to node.
        result = node;

        // 3e.iii. Break.
        break;
      }
    }

    // 4. Set iterator’s candidate reference to null.
    iterator._candidateReference = null;

    // 5. Return result.
    return result;
  }

  ExceptionOr<Node *> IteratorAlgorithms::TraverseChildren(TreeWalker &walker,
                                                           TraverseFirstOrLast type) noexcept
  {
    Node *node = &walker.CurrentNode();
    node = type == TraverseFirstOrLast::First ? node->FirstChild() : node->LastChild();

    while (node != nullptr)
    {
      auto result = FilterNode(walker, *node);
      if (result.HasException())
      {
        return result.ReleaseException();
      }

      if (result.Value() == FilterResult::FILTER_ACCEPT)
      {
        walker.CurrentNode(*node);
        return node;
      }
      else if (result.Value() == FilterResult::FILTER_SKIP)
      {
        auto child = type == TraverseFirstOrLast::First ? node->FirstChild() : node->LastChild();
        if (child)
        {
          node = child;
          continue;
        }
      }

      while (node != nullptr)
      {
        auto *sibling = type == TraverseFirstOrLast::First ? node->NextSibling() : node->PreviousSibling();
        if (sibling != nullptr)
        {
          node = sibling;
          break;
        }

        auto *parent = node != nullptr ? node->ParentNode() : nullptr;
        if (parent == nullptr || parent == &walker.Root() || parent == &walker.CurrentNode())
        {
          return nullptr;
        }

        node = parent;
      }
    }

    return nullptr;
  }

  ExceptionOr<Node *> IteratorAlgorithms::TraverseSiblings(TreeWalker &walker,
                                                           TraversalNextOrPrev type) noexcept
  {
    Node *node = &walker.CurrentNode();
    if (node == &walker.Root())
    {
      return nullptr;
    }

    while (true)
    {
      auto sibling = type == TraversalNextOrPrev::Next ? node->NextSibling() : node->PreviousSibling();
      while (sibling != nullptr)
      {
        node = sibling;

        auto result = FilterNode(walker, *node);
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        if (result.Value() == FilterResult::FILTER_ACCEPT)
        {
          walker.CurrentNode(*node);
          return node;
        }

        sibling = type == TraversalNextOrPrev::Next ? node->FirstChild() : node->LastChild();

        if (result.Value() == FilterResult::FILTER_REJECT || sibling == nullptr)
        {
          sibling = type == TraversalNextOrPrev::Next ? node->NextSibling() : node->PreviousSibling();
        }
      }

      node = node->ParentNode();
      if (node == nullptr || node == &walker.Root())
      {
        return nullptr;
      }

      auto result = FilterNode(walker, *node);
      if (result.HasException())
      {
        return result.ReleaseException();
      }

      if (result.Value() == FilterResult::FILTER_ACCEPT)
      {
        return nullptr;
      }
    }
  }
}
