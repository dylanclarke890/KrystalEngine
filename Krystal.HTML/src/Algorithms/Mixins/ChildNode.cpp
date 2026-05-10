#include "Krystal.HTML/Algorithms/Mixins/ChildNode.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/Mixins/ParentNode.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Utils/SubtreeRanges.hpp"

namespace Krys::HTML::Mixins
{
  namespace
  {
    /// @brief Return the first node in the given range that isn't contained in the given list of nodes, or
    /// null if there are no such nodes.
    template <std::ranges::forward_range TRange>
    static RawPtr<Node> ViableSibling(TRange &&range, const List<NodeOrString> &nodes) noexcept
    {
      for (auto &item : range)
      {
        auto predicate = [&](auto &nodeOrString)
        {
          if (std::holds_alternative<Ref<Node>>(nodeOrString))
          {
            return std::get<Ref<Node>>(nodeOrString).get() == &item;
          }

          return false;
        };

        if (std::ranges::any_of(nodes, Krys::Move(predicate)))
        {
          return std::addressof(item);
        }
      }

      return nullptr;
    }
  }

  ExceptionOr<void> ChildNode::Before(Node &childNode, const List<NodeOrString> &nodes) noexcept
  {
    if (childNode.ParentNode() == nullptr)
    {
      return {};
    }
    auto &parent = *childNode.ParentNode();

    auto *viablePreviousSibling = ViableSibling(PreviousSiblingRange(childNode), nodes);

    auto node = Mixins::ParentNode::ConvertNodesIntoNode(nodes, childNode.NodeDocument());
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

    return {};
  }

  ExceptionOr<void> ChildNode::After(Node &childNode, const List<NodeOrString> &nodes) noexcept
  {
    if (childNode.ParentNode() == nullptr)
    {
      return {};
    }
    auto &parent = *childNode.ParentNode();

    auto *viableNextSibling = ViableSibling(NextSiblingRange(childNode), nodes);

    auto node = Mixins::ParentNode::ConvertNodesIntoNode(nodes, childNode.NodeDocument());
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

  ExceptionOr<void> ChildNode::ReplaceWith(Node &childNode, const List<NodeOrString> &nodes) noexcept
  {
    if (childNode.ParentNode() == nullptr)
    {
      return {};
    }
    auto &parent = *childNode.ParentNode();

    auto *viableNextSibling = ViableSibling(NextSiblingRange(childNode), nodes);

    auto node = Mixins::ParentNode::ConvertNodesIntoNode(nodes, childNode.NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (childNode.ParentNode() == &parent)
    {
      if (auto result = MutationAlgorithms::Replace(childNode, *node.Value(), parent); result.HasException())
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

  ExceptionOr<void> ChildNode::Remove(Node &childNode) noexcept
  {
    if (!childNode.ParentNode())
    {
      return {};
    }

    return MutationAlgorithms::Remove(childNode, SuppressObservers(false));
  }
}