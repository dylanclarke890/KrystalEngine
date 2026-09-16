#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Mixins/ParentNode.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom::mixins
{
  namespace
  {
    /// @brief Return the first node in the given range that isn't contained in the given list of nodes, or
    /// null if there are no such nodes.
    template <std::ranges::forward_range TRange>
    KRYS_NODISCARD static Node *FirstViableSibling(TRange &&range, const List<NodeOrString> &nodes) noexcept
    {
      for (auto &item : range)
      {
        auto predicate = [&](auto &nodeOrString)
        {
          if (std::holds_alternative<Ref<Node>>(nodeOrString))
          {
            return std::get<Ref<Node>>(nodeOrString) == &item;
          }

          return false;
        };

        if (std::ranges::any_of(nodes, krys::move(predicate)))
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
    auto *viablePreviousSibling = FirstViableSibling(PreviousSiblingRange(childNode), nodes);

    auto node = ParentNode::ConvertNodesIntoNode(nodes, childNode.NodeDocument());
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
    auto *viableNextSibling = FirstViableSibling(NextSiblingRange(childNode), nodes);

    auto node = ParentNode::ConvertNodesIntoNode(nodes, childNode.NodeDocument());
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
    auto *viableNextSibling = FirstViableSibling(NextSiblingRange(childNode), nodes);

    auto node = ParentNode::ConvertNodesIntoNode(nodes, childNode.NodeDocument());
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
    }
    else
    {
      if (auto result = MutationAlgorithms::PreInsert(*node.Value(), parent, viableNextSibling);
          result.HasException())
      {
        return result.ReleaseException();
      }
    }

    return {};
  }

  ExceptionOr<void> ChildNode::Remove(Node &node) noexcept
  {
    if (node.ParentNode() == nullptr)
    {
      return {};
    }

    return MutationAlgorithms::Remove(node, SuppressObservers(false));
  }
}