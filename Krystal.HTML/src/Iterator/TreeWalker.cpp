#include "Krystal.HTML/Iterator/TreeWalker.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/IteratorAlgorithms.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  ExceptionOr<RawPtr<Node>> TreeWalker::ParentNode() noexcept
  {
    RawPtr<Node> node = &CurrentNode();
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

  ExceptionOr<RawPtr<Node>> TreeWalker::FirstChild() noexcept
  {
    return IteratorAlgorithms::TraverseChildren(*this, TraverseFirstOrLast::First);
  }

  ExceptionOr<RawPtr<Node>> TreeWalker::LastChild() noexcept
  {
    return IteratorAlgorithms::TraverseChildren(*this, TraverseFirstOrLast::Last);
  }

  ExceptionOr<RawPtr<Node>> TreeWalker::PreviousSibling() noexcept
  {
    return IteratorAlgorithms::TraverseSiblings(*this, TraversalNextOrPrev::Previous);
  }

  ExceptionOr<RawPtr<Node>> TreeWalker::NextSibling() noexcept
  {
    return IteratorAlgorithms::TraverseSiblings(*this, TraversalNextOrPrev::Next);
  }

  ExceptionOr<RawPtr<Node>> TreeWalker::PreviousNode() noexcept
  {
    RawPtr<Node> node = &CurrentNode();
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

  ExceptionOr<RawPtr<Node>> TreeWalker::NextNode() noexcept
  {
    RawPtr<Node> node = &CurrentNode();

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

      RawPtr<Node> sibling = nullptr;
      RawPtr<Node> temporary = node;

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