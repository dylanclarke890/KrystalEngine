#include "Krystal.HTML/Node/NodeQueries.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeTraversal.hpp"

namespace Krys::HTML
{
  bool NodeQueries::HasSameRoot(const Node &a, const Node &b) noexcept
  {
    return &a.Root() == &b.Root();
  }

  bool NodeQueries::IsFollowing(const Node &a, const Node &b) noexcept
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
    while (current = NodeTraversal::Next(*current))
    {
      if (current == &b)
      {
        return true;
      }
    }

    return false;
  }

  bool NodeQueries::IsAncestorOf(const Node &a, const Node &b) noexcept
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

  bool NodeQueries::IsChildOf(const Node &parent, const Node &child) noexcept
  {
    if (!HasSameRoot(parent, child))
    {
      return false;
    }

    return child.ParentNode() == &parent;
  }

  bool NodeQueries::IsHostIncludingAncestorOf(Node &node, Node &other) noexcept
  {
    return false;
  }

  bool NodeQueries::IsConnectedInSameTreeScope(const Node &a, const Node &b) noexcept
  {
    return a.IsConnected() == b.IsConnected() && &a.GetTreeScope() == &b.GetTreeScope();
  }
}
