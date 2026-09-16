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
    Node *node = &CurrentNode();
    while (node != nullptr && node != &Root())
    {
      node = node->ParentNode();
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
          return node;
        }
      }
    }

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
    Node *node = &CurrentNode();
    while (node != &Root())
    {
      auto sibling = node->PreviousSibling();
      while (sibling != nullptr)
      {
        node = sibling;

        auto result = IteratorAlgorithms::FilterNode(*this, *node);
        while (!result.HasException() && result.Value() != FilterResult::FILTER_REJECT
               && node->LastChild() != nullptr)
        {
          node = node->LastChild();
          result = IteratorAlgorithms::FilterNode(*this, *node);
        }

        if (result.HasException())
        {
          return result.ReleaseException();
        }

        if (result.Value() == FilterResult::FILTER_ACCEPT)
        {
          CurrentNode(*node);
          return node;
        }

        sibling = node->PreviousSibling();
      }

      if (node == &Root() || node->ParentNode() == nullptr)
      {
        return nullptr;
      }

      node = node->ParentNode();

      auto result = IteratorAlgorithms::FilterNode(*this, *node);
      if (result.HasException())
      {
        return result.ReleaseException();
      }

      if (result.Value() == FilterResult::FILTER_ACCEPT)
      {
        CurrentNode(*node);
        return node;
      }
    }

    return nullptr;
  }

  ExceptionOr<Node *> TreeWalker::NextNode() noexcept
  {
    Node *node = &CurrentNode();

    ExceptionOr<FilterResult> result = FilterResult::FILTER_ACCEPT;

    while (true)
    {
      while (!result.HasException() && result.Value() != FilterResult::FILTER_REJECT && node
             && node->FirstChild() != nullptr)
      {
        node = node->FirstChild();
        result = IteratorAlgorithms::FilterNode(*this, *node);

        if (!result.HasException() && result.Value() == FilterResult::FILTER_ACCEPT)
        {
          CurrentNode(*node);
          return node;
        }
      }

      if (result.HasException())
      {
        return result.ReleaseException();
      }

      Node *sibling = nullptr;
      Node *temporary = node;

      while (temporary != nullptr)
      {
        if (temporary == &Root())
        {
          return nullptr;
        }

        sibling = temporary->NextSibling();
        if (sibling != nullptr)
        {
          node = sibling;
          break;
        }

        temporary = temporary->ParentNode();
      }

      result = IteratorAlgorithms::FilterNode(*this, *node);
      if (result.HasException())
      {
        return result.ReleaseException();
      }

      if (result.Value() == FilterResult::FILTER_ACCEPT)
      {
        CurrentNode(*node);
        return node;
      }
    }
  }
}