#include "Krystal.HTML/Node/NodeTraversal.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include <cassert>

namespace Krys::HTML
{
  const Node &NodeTraversal::Root(const Node &node) noexcept
  {
    const Node *current = &node;
    while (current->ParentNode())
    {
      current = current->ParentNode();
    }

    return *current;
  }

  Node &NodeTraversal::Root(Node &node) noexcept
  {
    Node *current = &node;
    while (current->ParentNode())
    {
      current = current->ParentNode();
    }
    return *current;
  }

  template <bool CheckChildren = true, typename TNode>
  static RawPtr<Node> GetNext(const TNode &current) noexcept
  {
    if constexpr (CheckChildren)
    {
      if (current.FirstChild())
      {
        return current.FirstChild();
      }
    }

    if (current.NextSibling())
    {
      return current.NextSibling();
    }

    return NodeTraversal::NextAncestorSibling(current);
  }

  template <bool CheckChildren = true, typename TNode>
  static RawPtr<Node> GetNext(const TNode &current, RawPtr<const Node> stayWithin) noexcept
  {
    if constexpr (CheckChildren)
    {
      if (current.FirstChild())
      {
        return current.FirstChild();
      }
    }

    if (&current == stayWithin)
    {
      return nullptr;
    }

    if (current.NextSibling())
    {
      return current.NextSibling();
    }

    return NodeTraversal::NextAncestorSibling(current, stayWithin);
  }

  RawPtr<Node> NodeTraversal::Next(const Node &current) noexcept
  {
    return GetNext(current);
  }

  RawPtr<Node> NodeTraversal::Next(const ContainerNode &current) noexcept
  {
    return GetNext(current);
  }

  RawPtr<Node> NodeTraversal::Next(const Text &current) noexcept
  {
    return GetNext(current);
  }

  RawPtr<Node> NodeTraversal::Next(const Node &current, RawPtr<const Node> stayWithin) noexcept
  {
    return GetNext(current, stayWithin);
  }

  RawPtr<Node> NodeTraversal::Next(const ContainerNode &current, RawPtr<const Node> stayWithin) noexcept
  {
    return GetNext(current, stayWithin);
  }

  RawPtr<Node> NodeTraversal::Next(const Text &current, RawPtr<const Node> stayWithin) noexcept
  {
    return GetNext(current, stayWithin);
  }

  RawPtr<Node> NodeTraversal::NextSkippingChildren(const Node &current) noexcept
  {
    return GetNext<false>(current);
  }

  RawPtr<Node> NodeTraversal::NextSkippingChildren(const Node &current,
                                                   RawPtr<const Node> stayWithin) noexcept
  {
    return GetNext<false>(current, stayWithin);
  }

  RawPtr<Node> NodeTraversal::NextAncestorSibling(const Node &current) noexcept
  {
    assert(!current.NextSibling());

    const Node *ancestor = current.ParentNode();
    while (ancestor)
    {
      if (ancestor->NextSibling())
      {
        return ancestor->NextSibling();
      }

      ancestor = ancestor->ParentNode();
    }

    return nullptr;
  }

  RawPtr<Node> NodeTraversal::NextAncestorSibling(const Node &current, RawPtr<const Node> stayWithin) noexcept
  {
    assert(!current.NextSibling());
    assert(&current != stayWithin);

    const Node *ancestor = current.ParentNode();
    while (ancestor && ancestor != stayWithin)
    {
      if (ancestor->NextSibling())
      {
        return ancestor->NextSibling();
      }

      ancestor = ancestor->ParentNode();
    }

    return nullptr;
  }

  RawPtr<Node> NodeTraversal::DeepLastChild(Node &current) noexcept
  {
    RawPtr<Node> last = &current;
    while (last->LastChild())
    {
      last = last->LastChild();
    }

    return last;
  }

  RawPtr<Node> NodeTraversal::Last(const ContainerNode &current) noexcept
  {
    RawPtr<Node> node = current.LastChild();
    if (node == nullptr)
    {
      return nullptr;
    }

    while (node->LastChild())
    {
      node = node->LastChild();
    }
    return node;
  }

  RawPtr<Node> NodeTraversal::Previous(const Node &current) noexcept
  {
    if (RawPtr<Node> previous = current.PreviousSibling())
    {
      return DeepLastChild(*previous);
    }

    return current.ParentNode();
  }

  RawPtr<Node> NodeTraversal::Previous(const Node &current, RawPtr<const Node> stayWithin) noexcept
  {
    if (RawPtr<Node> previous = current.PreviousSibling())
    {
      return DeepLastChild(*previous);
    }

    if (&current == stayWithin)
    {
      return nullptr;
    }
    return current.ParentNode();
  }

  RawPtr<Node> NodeTraversal::PreviousSkippingChildren(const Node &current) noexcept
  {
    if (current.PreviousSibling())
    {
      return current.PreviousSibling();
    }

    for (RawPtr<Node> ancestor = current.ParentNode(); ancestor; ancestor = ancestor->ParentNode())
    {
      if (ancestor->PreviousSibling())
      {
        return ancestor->PreviousSibling();
      }
    }

    return nullptr;
  }

  RawPtr<Node> NodeTraversal::PreviousSkippingChildren(const Node &current,
                                                       RawPtr<const Node> stayWithin) noexcept
  {
    if (&current == stayWithin)
    {
      return nullptr;
    }

    if (current.PreviousSibling())
    {
      return current.PreviousSibling();
    }

    for (RawPtr<Node> ancestor = current.ParentNode(); ancestor != nullptr; ancestor = ancestor->ParentNode())
    {
      if (ancestor == stayWithin)
      {
        return nullptr;
      }
      if (ancestor->PreviousSibling())
      {
        return ancestor->PreviousSibling();
      }
    }
    return nullptr;
  }

  RawPtr<Node> NodeTraversal::NextPostOrder(const Node &current) noexcept
  {
    if (!current.NextSibling())
    {
      return current.ParentNode();
    }
    RawPtr<Node> next = current.NextSibling();
    while (next->FirstChild())
    {
      next = next->FirstChild();
    }
    return next;
  }

  RawPtr<Node> NodeTraversal::NextPostOrder(const Node &current, RawPtr<const Node> stayWithin) noexcept
  {
    if (&current == stayWithin)
    {
      return nullptr;
    }
    if (!current.NextSibling())
    {
      return current.ParentNode();
    }
    RawPtr<Node> next = current.NextSibling();
    while (next->FirstChild())
    {
      next = next->FirstChild();
    }
    return next;
  }

  RawPtr<Node> NodeTraversal::PreviousPostOrder(const Node &current) noexcept
  {
    if (current.LastChild())
    {
      return current.LastChild();
    }
    if (current.PreviousSibling())
    {
      return current.PreviousSibling();
    }

    return PreviousAncestorSiblingPostOrder(current);
  }

  RawPtr<Node> NodeTraversal::PreviousPostOrder(const Node &current, RawPtr<const Node> stayWithin) noexcept
  {
    if (current.LastChild())
    {
      return current.LastChild();
    }
    if (&current == stayWithin)
    {
      return nullptr;
    }
    if (current.PreviousSibling())
    {
      return current.PreviousSibling();
    }

    return PreviousAncestorSiblingPostOrder(current, stayWithin);
  }

  RawPtr<Node> NodeTraversal::PreviousSkippingChildrenPostOrder(const Node &current) noexcept
  {
    if (current.PreviousSibling())
    {
      return current.PreviousSibling();
    }
    return PreviousAncestorSiblingPostOrder(current);
  }

  RawPtr<Node> NodeTraversal::PreviousSkippingChildrenPostOrder(const Node &current,
                                                                RawPtr<const Node> stayWithin) noexcept
  {
    if (&current == stayWithin)
    {
      return nullptr;
    }
    if (current.PreviousSibling())
    {
      return current.PreviousSibling();
    }
    return PreviousAncestorSiblingPostOrder(current, stayWithin);
  }

  RawPtr<Node> NodeTraversal::PreviousAncestorSiblingPostOrder(const Node &current) noexcept
  {
    assert(!current.PreviousSibling());
    for (RawPtr<Node> ancestor = current.ParentNode(); ancestor != nullptr; ancestor = ancestor->ParentNode())
    {
      if (ancestor->PreviousSibling())
      {
        return ancestor->PreviousSibling();
      }
    }

    return nullptr;
  }

  RawPtr<Node> NodeTraversal::PreviousAncestorSiblingPostOrder(const Node &current,
                                                               RawPtr<const Node> stayWithin) noexcept
  {
    assert(!current.PreviousSibling());
    for (RawPtr<Node> ancestor = current.ParentNode(); ancestor != nullptr; ancestor = ancestor->ParentNode())
    {
      if (ancestor == stayWithin)
      {
        return nullptr;
      }
      if (ancestor->PreviousSibling())
      {
        return ancestor->PreviousSibling();
      }
    }
    return nullptr;
  }
}