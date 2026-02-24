#include "Krystal.HTML/Tree/TreeQueries.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Tree/TreeTraversal.hpp"

namespace Krys::HTML
{
  bool TreeQueries::HasSameRoot(const Node &a, const Node &b) noexcept
  {
    return &a.Root() == &b.Root();
  }

  bool TreeQueries::IsFollowing(const Node &a, const Node &b) noexcept
  {
    if (!HasSameRoot(a, b))
    {
      return false;
    }

    if (&a == &b)
    {
      return false;
    }

    RawPtr<const Node> current = &a;
    while (current = TreeTraversal::Next(*current))
    {
      if (current == &b)
      {
        return true;
      }
    }

    return false;
  }

  bool TreeQueries::IsAncestorOf(const Node &a, const Node &b) noexcept
  {
    if (!HasSameRoot(a, b))
    {
      return false;
    }

    RawPtr<const Node> current = &b;
    while (current)
    {
      if (current == &a)
      {
        return true;
      }
      current = current->ParentNode();
    }

    return false;
  }

  bool TreeQueries::IsChildOf(const Node &parent, const Node &child) noexcept
  {
    if (!HasSameRoot(parent, child))
    {
      return false;
    }

    return child.ParentNode() == &parent;
  }

  bool TreeQueries::IsHostIncludingAncestorOf(Node &node, Node &other) noexcept
  {
    return false;
  }

  bool TreeQueries::IsConnectedInSameTreeScope(const Node &a, const Node &b) noexcept
  {
    return a.IsConnected() == b.IsConnected() && &a.GetTreeScope() == &b.GetTreeScope();
  }

  bool TreeQueries::IsDocTypeOrDocTypeFollows(RawPtr<Node> node) noexcept
  {
    if (node == nullptr)
    {
      return false;
    }

    if (node->IsDocumentTypeNode())
    {
      return true;
    }

    RawPtr<Node> current = node;
    while (current = TreeTraversal::Next(*current))
    {
      if (current->IsDocumentTypeNode())
      {
        return true;
      }
    }

    return false;
  }
}
