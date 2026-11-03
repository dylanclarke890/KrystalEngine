#include "Krystal.Booey/DOM/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Traversal/NodeIterator.hpp"
#include "Krystal.Booey/DOM/Traversal/TreeWalker.hpp"
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
    // 1. Let node be walker’s current.
    RefPtr<Node> node = ShareRefPtr(&walker.CurrentNode());

    // 2. Set node to node’s first child if type is "first"; otherwise to node’s last child.
    node = ShareRefPtr(type == TraverseFirstOrLast::First ? node->FirstChild() : node->LastChild());

    // 3. While node is non-null:
    while (node)
    {
      // 3a. Let result be the result of filtering node within walker.
      auto result = FilterNode(walker, *node);
      if (result.HasException())
      {
        return result.ReleaseException();
      }

      // 3b. If result is FILTER_ACCEPT, then set walker’s current to node and return node.
      if (result.Value() == FilterResult::FILTER_ACCEPT)
      {
        walker.CurrentNode(*node);
        return node.get();
      }

      // 3c. If result is FILTER_SKIP:
      if (result.Value() == FilterResult::FILTER_SKIP)
      {
        // 3c.i. Let child be node’s first child if type is "first"; otherwise node’s last child.
        Node *child = type == TraverseFirstOrLast::First ? node->FirstChild() : node->LastChild();

        // 3c.ii. If child is non-null, then set node to child and continue.
        if (child)
        {
          node = ShareRefPtr(child);
          continue;
        }
      }

      // 3d. While node is non-null:
      while (node)
      {
        // 3d.i. Let sibling be node’s next sibling if type is "first"; otherwise node’s previous sibling.
        Node *sibling = type == TraverseFirstOrLast::First ? node->NextSibling() : node->PreviousSibling();

        // 3d.ii. If sibling is non-null, then set node to sibling and break.
        if (sibling)
        {
          node = ShareRefPtr(sibling);
          break;
        }

        // 3d.iii. Let parent be node’s parent.
        Node *parent = node->ParentNode();

        // 3d.iv. If parent is null, walker’s root, or walker’s current, then return null.
        if (!parent || parent == &walker.Root() || parent == &walker.CurrentNode())
        {
          return nullptr;
        }

        // 3d.v. Set node to parent.
        node = ShareRefPtr(parent);
      }
    }

    // 4. Return null.
    return nullptr;
  }

  ExceptionOr<Node *> IteratorAlgorithms::TraverseSiblings(TreeWalker &walker,
                                                           TraversalNextOrPrev type) noexcept
  {
    // 1. Let node be walker’s current.
    RefPtr<Node> node = ShareRefPtr(&walker.CurrentNode());

    // 2. If node is root, then return null.
    if (node == &walker.Root())
    {
      return nullptr;
    }

    // 3. While true:
    while (true)
    {
      // 3a. Let sibling be node’s next sibling if type is "next"; otherwise node’s previous sibling.
      RefPtr<Node> sibling =
        ShareRefPtr(type == TraversalNextOrPrev::Next ? node->NextSibling() : node->PreviousSibling());

      // 3b. While sibling is non-null:
      while (sibling)
      {
        // 3b.i. Set node to sibling.
        node = sibling;

        // 3b.ii. Let result be the result of filtering node within walker.
        auto result = FilterNode(walker, *node);
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        // 3b.iii. If result is FILTER_ACCEPT, then set walker’s current to node and return node.
        if (result.Value() == FilterResult::FILTER_ACCEPT)
        {
          walker.CurrentNode(*node);
          return node.get();
        }

        // 3b.iv. Set sibling to node’s first child if type is "next"; otherwise to node’s last child.
        sibling = ShareRefPtr(type == TraversalNextOrPrev::Next ? node->FirstChild() : node->LastChild());

        // 3b.v. If result is FILTER_REJECT or sibling is null, then set sibling to node’s next sibling if
        // type is
        //       "next"; otherwise to node’s previous sibling.
        if (result.Value() == FilterResult::FILTER_REJECT || !sibling)
        {
          sibling =
            ShareRefPtr(type == TraversalNextOrPrev::Next ? node->NextSibling() : node->PreviousSibling());
        }
      }

      // 3c. Set node to node’s parent.
      node = ShareRefPtr(node->ParentNode());

      // 3d. If node is null or walker’s root, then return null.
      if (!node || node == &walker.Root())
      {
        return nullptr;
      }

      // 3e. If the return value of filtering node within walker is FILTER_ACCEPT, then return null.
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
