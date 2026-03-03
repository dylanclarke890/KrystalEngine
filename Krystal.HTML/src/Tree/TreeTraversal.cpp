#include "Krystal.HTML/Tree/TreeTraversal.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Tree/TreeQueries.hpp"
#include <cassert>

namespace Krys::HTML
{
  const Node &TreeTraversal::Root(const Node &node) noexcept
  {
    const Node *current = &node;
    while (current->ParentNode())
    {
      current = current->ParentNode();
    }

    return *current;
  }

  Node &TreeTraversal::Root(Node &node) noexcept
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

    return TreeTraversal::NextAncestorSibling(current);
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

    return TreeTraversal::NextAncestorSibling(current, stayWithin);
  }

  RawPtr<Node> TreeTraversal::Next(const Node &current) noexcept
  {
    return GetNext(current);
  }

  RawPtr<Node> TreeTraversal::Next(const ContainerNode &current) noexcept
  {
    return GetNext(current);
  }

  RawPtr<Node> TreeTraversal::Next(const Text &current) noexcept
  {
    return GetNext(current);
  }

  RawPtr<Node> TreeTraversal::Next(const Node &current, RawPtr<const Node> stayWithin) noexcept
  {
    return GetNext(current, stayWithin);
  }

  RawPtr<Node> TreeTraversal::Next(const ContainerNode &current, RawPtr<const Node> stayWithin) noexcept
  {
    return GetNext(current, stayWithin);
  }

  RawPtr<Node> TreeTraversal::Next(const Text &current, RawPtr<const Node> stayWithin) noexcept
  {
    return GetNext(current, stayWithin);
  }

  RawPtr<Node> TreeTraversal::NextSkippingChildren(const Node &current) noexcept
  {
    return GetNext<false>(current);
  }

  RawPtr<Node> TreeTraversal::NextSkippingChildren(const Node &current,
                                                   RawPtr<const Node> stayWithin) noexcept
  {
    return GetNext<false>(current, stayWithin);
  }

  RawPtr<Node> TreeTraversal::NextAncestorSibling(const Node &current) noexcept
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

  RawPtr<Node> TreeTraversal::NextAncestorSibling(const Node &current, RawPtr<const Node> stayWithin) noexcept
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

  RawPtr<Node> TreeTraversal::DeepLastChild(Node &current) noexcept
  {
    RawPtr<Node> last = &current;
    while (last->LastChild())
    {
      last = last->LastChild();
    }

    return last;
  }

  RawPtr<Node> TreeTraversal::Last(const ContainerNode &current) noexcept
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

  RawPtr<Node> TreeTraversal::Previous(const Node &current) noexcept
  {
    if (RawPtr<Node> previous = current.PreviousSibling())
    {
      return DeepLastChild(*previous);
    }

    return current.ParentNode();
  }

  RawPtr<Node> TreeTraversal::Previous(const Node &current, RawPtr<const Node> stayWithin) noexcept
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

  RawPtr<Node> TreeTraversal::PreviousSkippingChildren(const Node &current) noexcept
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

  RawPtr<Node> TreeTraversal::PreviousSkippingChildren(const Node &current,
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

  RawPtr<Node> TreeTraversal::NextPostOrder(const Node &current) noexcept
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

  RawPtr<Node> TreeTraversal::NextPostOrder(const Node &current, RawPtr<const Node> stayWithin) noexcept
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

  RawPtr<Node> TreeTraversal::PreviousPostOrder(const Node &current) noexcept
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

  RawPtr<Node> TreeTraversal::PreviousPostOrder(const Node &current, RawPtr<const Node> stayWithin) noexcept
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

  RawPtr<Node> TreeTraversal::PreviousSkippingChildrenPostOrder(const Node &current) noexcept
  {
    if (current.PreviousSibling())
    {
      return current.PreviousSibling();
    }
    return PreviousAncestorSiblingPostOrder(current);
  }

  RawPtr<Node> TreeTraversal::PreviousSkippingChildrenPostOrder(const Node &current,
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

  RawPtr<Node> TreeTraversal::PreviousAncestorSiblingPostOrder(const Node &current) noexcept
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

  RawPtr<Node> TreeTraversal::PreviousAncestorSiblingPostOrder(const Node &current,
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

  RawPtr<Element> TreeTraversal::NextElementSibling(const Node &current) noexcept
  {
    for (RawPtr<Node> next = current.NextSibling(); next; next = next->NextSibling())
    {
      if (RawPtr<Element> element = DynamicDowncast<Element>(*next))
      {
        return element;
      }
    }

    return nullptr;
  }

  RawPtr<Element> TreeTraversal::PreviousElementSibling(const Node &current) noexcept
  {
    for (RawPtr<Node> prev = current.PreviousSibling(); prev; prev = prev->PreviousSibling())
    {
      if (RawPtr<Element> element = DynamicDowncast<Element>(*prev))
      {
        return element;
      }
    }

    return nullptr;
  }

  RawPtr<Element> TreeTraversal::FirstElementChild(const ContainerNode &node) noexcept
  {
    for (auto child = ShareRefPtr(node.FirstChild()); child; child = ShareRefPtr(child->NextSibling()))
    {
      if (auto *element = DynamicDowncast<Element>(*child))
      {
        return element;
      }
    }

    return nullptr;
  }

  RawPtr<Element> TreeTraversal::LastElementChild(const ContainerNode &node) noexcept
  {
    RawPtr<Element> lastElement = nullptr;
    for (auto child = ShareRefPtr(node.LastChild()); child; child = ShareRefPtr(child->PreviousSibling()))
    {
      if (auto *element = DynamicDowncast<Element>(*child))
      {
        return element;
      }
    }

    return nullptr;
  }

  RawPtr<Text> TreeTraversal::NextExclusiveTextNode(const Node &current) noexcept
  {
    for (RawPtr<Node> next = Next(current); next; next = Next(*next))
    {
      if (TreeQueries::IsExclusiveTextNode(*next))
      {
        return Downcast<Text>(next);
      }
    }

    return nullptr;
  }

  RawPtr<Text> TreeTraversal::NextExclusiveTextNode(const Node &current,
                                                    RawPtr<const Node> stayWithin) noexcept
  {
    for (RawPtr<Node> next = Next(current, stayWithin); next; next = Next(*next, stayWithin))
    {
      if (TreeQueries::IsExclusiveTextNode(*next))
      {
        return Downcast<Text>(next);
      }
    }

    return nullptr;
  }
}