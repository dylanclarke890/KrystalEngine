#include "Krystal.Booey/DOM/TreeWalker.hpp"
#include "Krystal.Booey/DOM/Algorithms/IteratorAlgorithms.hpp"

namespace krys::boo::dom
{
  TreeWalker::TreeWalker(Node &root, dom::WhatToShow whatToShow, RefPtr<NodeFilter> &&filter) noexcept
      : TraversalBase(root, whatToShow, krys::move(filter)), _currentNode(ShareRef(root))
  {
  }

  ExceptionOr<Node *> TreeWalker::ParentNode() noexcept
  {
    // 1. Let node be this’s current.
    RefPtr<Node> node = _currentNode;

    // 2. While node is non-null and is not this’s root:
    while (node && node != &Root())
    {
      // 2a. Set node to node’s parent.
      node = ShareRefPtr(node->ParentNode());

      // 2b. If node is non-null and filtering node within this returns FILTER_ACCEPT, then set this’s current
      // to node
      //     and return node.
      if (node)
      {
        auto filterResult = IteratorAlgorithms::FilterNode(*this, *node);
        if (filterResult.HasException())
        {
          return filterResult.ReleaseException();
        }

        if (filterResult.Value() == FilterResult::FILTER_ACCEPT)
        {
          CurrentNode(*node);
          return node.get();
        }
      }
    }

    // 3. Return null.
    return nullptr;
  }

  ExceptionOr<Node *> TreeWalker::FirstChild() noexcept
  {
    return IteratorAlgorithms::TraverseChildren(*this, TraverseFirstOrLast::First);
  }

  ExceptionOr<Node *> TreeWalker::LastChild() noexcept
  {
    return IteratorAlgorithms::TraverseChildren(*this, TraverseFirstOrLast::Last);
  }

  ExceptionOr<Node *> TreeWalker::PreviousSibling() noexcept
  {
    return IteratorAlgorithms::TraverseSiblings(*this, TraversalNextOrPrev::Previous);
  }

  ExceptionOr<Node *> TreeWalker::NextSibling() noexcept
  {
    return IteratorAlgorithms::TraverseSiblings(*this, TraversalNextOrPrev::Next);
  }

  ExceptionOr<Node *> TreeWalker::PreviousNode() noexcept
  {
    // 1. Let node be this’s current.
    RefPtr<Node> node = _currentNode;

    // 2. While node is not this’s root:
    while (node != &Root())
    {
      // 2a. Let sibling be node’s previous sibling.
      RefPtr<Node> sibling = ShareRefPtr(node->PreviousSibling());

      // 2b. While sibling is non-null:
      while (sibling)
      {
        // 2b.i. Set node to sibling.
        node = sibling;

        // 2b.ii. Let result be the result of filtering node within this.
        auto result = IteratorAlgorithms::FilterNode(*this, *node);
        if (result.HasException())
        {
          return result.ReleaseException();
        }

        // 2b.iii. While result is not FILTER_REJECT and node has a child:
        while (result.Value() != FilterResult::FILTER_REJECT && node->LastChild())
        {
          // 2b.iii.A. Set node to node’s last child.
          node = ShareRefPtr(node->LastChild());

          // 2b.iii.B. Set result to the result of filtering node within this.
          result = IteratorAlgorithms::FilterNode(*this, *node);
          if (result.HasException())
          {
            return result.ReleaseException();
          }
        }

        // 2b.iv. If result is FILTER_ACCEPT, then set this’s current to node and return node.
        if (result.Value() == FilterResult::FILTER_ACCEPT)
        {
          CurrentNode(*node);
          return node.get();
        }

        // 2b.v. Set sibling to node’s previous sibling.
        sibling = ShareRefPtr(node->PreviousSibling());
      }

      // 2c. If node is this’s root or node’s parent is null, then return null.
      if (node == &Root() || !node->ParentNode())
      {
        return nullptr;
      }

      // 2d. Set node to node’s parent.
      node = ShareRefPtr(node->ParentNode());

      // 2e. If the return value of filtering node within this is FILTER_ACCEPT, then set this’s current to
      // node and
      //     return node.
      auto result = IteratorAlgorithms::FilterNode(*this, *node);
      if (result.HasException())
      {
        return result.ReleaseException();
      }

      if (result.Value() == FilterResult::FILTER_ACCEPT)
      {
        CurrentNode(*node);
        return node.get();
      }
    }

    // 3. Return null.
    return nullptr;
  }

  ExceptionOr<Node *> TreeWalker::NextNode() noexcept
  {
    // 1. Let node be this’s current.
    RefPtr<Node> node = _currentNode;

    // 2. Let result be FILTER_ACCEPT.
    FilterResult result = FilterResult::FILTER_ACCEPT;

    // 3. While true:
    while (true)
    {
      // 3a. While result is not FILTER_REJECT and node has a child:
      while (result != FilterResult::FILTER_REJECT && node->FirstChild())
      {
        // 3a.i. Set node to its first child.
        node = ShareRefPtr(node->FirstChild());

        // 3a.ii. Set result to the result of filtering node within this.
        auto filterResult = IteratorAlgorithms::FilterNode(*this, *node);
        if (filterResult.HasException())
        {
          return filterResult.ReleaseException();
        }
        result = filterResult.Value();

        // 3a.iii. If result is FILTER_ACCEPT, then set this’s current to node and return node.
        if (result == FilterResult::FILTER_ACCEPT)
        {
          CurrentNode(*node);
          return node.get();
        }
      }

      // 3b. Let sibling be null.
      Node *sibling = nullptr;

      // 3c. Let temporary be node.
      Node *temporary = node.get();

      // 3d. While temporary is non-null:
      while (temporary)
      {
        // 3d.i. If temporary is this’s root, then return null.
        if (temporary == &Root())
        {
          return nullptr;
        }

        // 3d.ii. Set sibling to temporary’s next sibling.
        sibling = temporary->NextSibling();

        // 3d.iii. If sibling is non-null, then break.
        if (sibling)
        {
          break;
        }

        // 3d.iv. Set temporary to temporary’s parent.
        temporary = temporary->ParentNode();
      }

      // 3e. If sibling is null, then return null.
      if (!sibling)
      {
        return nullptr;
      }

      // 3f. Set node to sibling.
      node = ShareRefPtr(sibling);

      // 3g. Set result to the result of filtering node within this.
      auto filterResult = IteratorAlgorithms::FilterNode(*this, *node);
      if (filterResult.HasException())
      {
        return filterResult.ReleaseException();
      }
      result = filterResult.Value();

      // 3h. If result is FILTER_ACCEPT, then set this’s current to node and return node.
      if (result == FilterResult::FILTER_ACCEPT)
      {
        CurrentNode(*node);
        return node.get();
      }
    }
  }
}
