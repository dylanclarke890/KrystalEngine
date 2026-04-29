#include "Krystal.HTML/Algorithms/ParentNodeAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/HTMLCollection.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"

namespace Krys::HTML
{
  ExceptionOr<void> ParentNodeAlgorithms::Prepend(ContainerNode &parent,
                                                  const List<NodeOrString> &nodes) noexcept
  {
    auto node = MutationAlgorithms::ConvertNodesIntoNode(nodes, parent.NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = MutationAlgorithms::PreInsert(*node.Value(), parent, parent.FirstChild());
        result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> ParentNodeAlgorithms::Append(ContainerNode &parent,
                                                 const List<NodeOrString> &nodes) noexcept
  {
    auto node = MutationAlgorithms::ConvertNodesIntoNode(nodes, parent.NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = MutationAlgorithms::Append(*node.Value(), parent); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> ParentNodeAlgorithms::ReplaceChildren(ContainerNode &parent,
                                                          const List<NodeOrString> &nodes) noexcept
  {
    auto node = MutationAlgorithms::ConvertNodesIntoNode(nodes, parent.NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = MutationAlgorithms::EnsurePreInsertValidity(*node.Value(), parent, nullptr);
        result.HasException())
    {
      return result.ReleaseException();
    }

    return MutationAlgorithms::ReplaceAll(node.Value().get(), parent);
  }

  ExceptionOr<void> ParentNodeAlgorithms::MoveBefore(ContainerNode &parent, Node &node,
                                                     RawPtr<Node> refChild) noexcept
  {
    if (refChild == &node)
    {
      refChild = node.NextSibling();
    }

    return MutationAlgorithms::Move(node, parent, refChild);
  }
}