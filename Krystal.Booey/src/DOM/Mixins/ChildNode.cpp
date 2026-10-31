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
    KRYS_NODISCARD Node *FirstViableSibling(TRange &&range, const List<NodeOrString> &nodes) noexcept
    {
      for (auto &item : range)
      {
        auto isItem = [&](auto &nodeOrString)
        {
          auto *node = std::get_if<Ref<Node>>(&nodeOrString);
          return node && node->get() == &item;
        };

        if (std::ranges::none_of(nodes, isItem))
        {
          return std::addressof(item);
        }
      }

      return nullptr;
    }
  }

  ExceptionOr<void> ChildNode::Before(Node &childNode, const List<NodeOrString> &nodes) noexcept
  {
    // 1. Let parent be this's parent.
    // Converting the nodes can take this out of parent, so parent is kept alive.
    RefPtr<ContainerNode> parent = ShareRefPtr(childNode.ParentNode());

    // 2. If parent is null, then return.
    if (parent == nullptr)
    {
      return {};
    }

    // 3. Let viablePreviousSibling be this's first preceding sibling not in nodes; otherwise null.
    RefPtr<Node> viablePreviousSibling =
      ShareRefPtr(FirstViableSibling(PreviousSiblingRange(childNode), nodes));

    // 4. Let node be the result of converting nodes into a node, given nodes and this's node document.
    auto node = ParentNode::ConvertNodesIntoNode(nodes, childNode.NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    // 5. If viablePreviousSibling is null, then set it to parent's first child; otherwise to
    //    viablePreviousSibling's next sibling.
    viablePreviousSibling = ShareRefPtr(
      viablePreviousSibling == nullptr ? parent->FirstChild() : viablePreviousSibling->NextSibling());

    // 6. Pre-insert node into parent before viablePreviousSibling.
    if (auto result = MutationAlgorithms::PreInsert(*node.Value(), *parent, viablePreviousSibling.get());
        result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> ChildNode::After(Node &childNode, const List<NodeOrString> &nodes) noexcept
  {
    // 1. Let parent be this's parent.
    // Converting the nodes can take this out of parent, so parent is kept alive.
    RefPtr<ContainerNode> parent = ShareRefPtr(childNode.ParentNode());

    // 2. If parent is null, then return.
    if (parent == nullptr)
    {
      return {};
    }

    // 3. Let viableNextSibling be this's first following sibling not in nodes; otherwise null.
    RefPtr<Node> viableNextSibling = ShareRefPtr(FirstViableSibling(NextSiblingRange(childNode), nodes));

    // 4. Let node be the result of converting nodes into a node, given nodes and this's node document.
    auto node = ParentNode::ConvertNodesIntoNode(nodes, childNode.NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    // 5. Pre-insert node into parent before viableNextSibling.
    if (auto result = MutationAlgorithms::PreInsert(*node.Value(), *parent, viableNextSibling.get());
        result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> ChildNode::ReplaceWith(Node &childNode, const List<NodeOrString> &nodes) noexcept
  {
    // 1. Let parent be this's parent.
    // Converting the nodes can take this out of parent, so parent is kept alive.
    RefPtr<ContainerNode> parent = ShareRefPtr(childNode.ParentNode());

    // 2. If parent is null, then return.
    if (parent == nullptr)
    {
      return {};
    }

    // 3. Let viableNextSibling be this's first following sibling not in nodes; otherwise null.
    RefPtr<Node> viableNextSibling = ShareRefPtr(FirstViableSibling(NextSiblingRange(childNode), nodes));

    // 4. Let node be the result of converting nodes into a node, given nodes and this's node document.
    auto node = ParentNode::ConvertNodesIntoNode(nodes, childNode.NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    // 5. If this's parent is parent, replace this with node within parent.
    // This could have been inserted into node.
    if (childNode.ParentNode() == parent.get())
    {
      if (auto result = MutationAlgorithms::Replace(childNode, *node.Value(), *parent); result.HasException())
      {
        return result.ReleaseException();
      }
    }
    // 6. Otherwise, pre-insert node into parent before viableNextSibling.
    else
    {
      if (auto result = MutationAlgorithms::PreInsert(*node.Value(), *parent, viableNextSibling.get());
          result.HasException())
      {
        return result.ReleaseException();
      }
    }

    return {};
  }

  ExceptionOr<void> ChildNode::Remove(Node &node) noexcept
  {
    // 1. If this's parent is null, then return.
    if (node.ParentNode() == nullptr)
    {
      return {};
    }

    // 2. Remove this.
    return MutationAlgorithms::Remove(node, SuppressObservers(false));
  }
}