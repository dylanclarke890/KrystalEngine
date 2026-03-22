#pragma once

#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class ChildNodeAlgorithms
  {
  public:
    KRYS_NODISCARD static ExceptionOr<void> Before(Node &childNode, const List<NodeOrString> &nodes) noexcept
    {
      if (childNode.ParentNode() == nullptr)
      {
        return {};
      }
      auto &parent = *childNode.ParentNode();

      auto *viablePreviousSibling = ViableSibling(PreviousSiblingRange(childNode), nodes);

      auto node = MutationAlgorithms::ConvertNodesIntoNode(nodes, childNode.NodeDocument());
      if (node.HasException())
      {
        return node.ReleaseException();
      }

      viablePreviousSibling =
        viablePreviousSibling == nullptr ? parent.FirstChild() : viablePreviousSibling->NextSibling();

      if (auto result = MutationAlgorithms::PreInsert(*node.Value(), parent, viablePreviousSibling);
          result.HasException())
      {
        return result.ReleaseException();
      }
    }

    KRYS_NODISCARD static ExceptionOr<void> After(Node &childNode, const List<NodeOrString> &nodes) noexcept
    {
      if (childNode.ParentNode() == nullptr)
      {
        return {};
      }
      auto &parent = *childNode.ParentNode();

      auto *viableNextSibling = ViableSibling(NextSiblingRange(childNode), nodes);

      auto node = MutationAlgorithms::ConvertNodesIntoNode(nodes, childNode.NodeDocument());
      if (node.HasException())
      {
        return node.ReleaseException();
      }

      if (auto result = MutationAlgorithms::PreInsert(*node.Value(), parent, viableNextSibling);
          result.HasException())
      {
        return result.ReleaseException();
      }

      return {};
    }

    KRYS_NODISCARD static ExceptionOr<void> ReplaceWith(Node &childNode,
                                                        const List<NodeOrString> &nodes) noexcept
    {
      if (childNode.ParentNode() == nullptr)
      {
        return {};
      }
      auto &parent = *childNode.ParentNode();

      auto *viableNextSibling = ViableSibling(NextSiblingRange(childNode), nodes);

      auto node = MutationAlgorithms::ConvertNodesIntoNode(nodes, childNode.NodeDocument());
      if (node.HasException())
      {
        return node.ReleaseException();
      }

      if (childNode.ParentNode() == &parent)
      {
        if (auto result = MutationAlgorithms::Replace(childNode, *node.Value(), parent);
            result.HasException())
        {
          return result.ReleaseException();
        }

        return {};
      }
      else
      {
        if (auto result = MutationAlgorithms::PreInsert(*node.Value(), parent, viableNextSibling);
            result.HasException())
        {
          return result.ReleaseException();
        }

        return {};
      }
    }

    KRYS_NODISCARD static ExceptionOr<void> Remove(Node &childNode) noexcept
    {
      if (!childNode.ParentNode())
      {
        return {};
      }

      return MutationAlgorithms::Remove(childNode, SuppressObservers(false));
    }

  private:
    /// @brief Return the first node in the given range that isn't contained in the given list of nodes, or
    /// null if there are no such nodes.
    template <std::ranges::forward_range TRange>
    static RawPtr<Node> ViableSibling(TRange &&range, const List<NodeOrString> &nodes) noexcept
    {
      for (auto &item : range)
      {
        if (std::ranges::find(nodes, item) == std::ranges::end(nodes))
        {
          return item;
        }
      }
      return nullptr;
    }
  };
}