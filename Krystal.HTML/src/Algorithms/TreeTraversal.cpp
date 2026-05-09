#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Utils/SubtreeRanges.hpp"
#include "Krystal.Lib/Ranges/Algorithm.hpp"
#include <cassert>

namespace Krys::HTML
{
  namespace
  {
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
  }

  const Node &TreeTraversal::Root(const Node &node) noexcept
  {
    return *Krys::HTML::Last(ConstInclusiveAncestorRange(node));
  }

  Node &TreeTraversal::Root(Node &node) noexcept
  {
    return *Krys::HTML::Last(InclusiveAncestorRange(node));
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

  RawPtr<Node> TreeTraversal::NextShadowIncluding(const Node &current) noexcept
  {
    if (auto *child = current.FirstChild())
    {
      return child;
    }

    if (auto *host = DynamicDowncast<Element>(current))
    {
      if (auto *shadowRoot = host->ShadowRoot().get())
      {
        return shadowRoot;
      }
    }

    for (RawPtr<const Node> node = &current; node; node = node->ParentNode())
    {
      if (auto *sibling = node->NextSibling())
      {
        return sibling;
      }

      // If climbing out of a shadow tree, continue from the host.
      if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(*node))
      {
        node = shadowRoot->Host();
        if (auto *sibling = node->NextSibling())
        {
          return sibling;
        }
      }
    }

    return nullptr;
  }

  RawPtr<Node> TreeTraversal::NextShadowIncluding(const Node &current, RawPtr<const Node> stayWithin) noexcept
  {
    if (auto *child = current.FirstChild())
    {
      return child;
    }

    if (auto *host = DynamicDowncast<Element>(current))
    {
      if (auto *shadowRoot = host->ShadowRoot().get())
      {
        return shadowRoot;
      }
    }

    for (RawPtr<const Node> node = &current; node; node = node->ParentNode())
    {
      if (node == stayWithin)
      {
        return nullptr;
      }

      if (auto *sibling = node->NextSibling())
      {
        return sibling;
      }

      // If climbing out of a shadow tree, continue from the host.
      if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(*node))
      {
        node = shadowRoot->Host();

        if (node == stayWithin)
        {
          return nullptr;
        }

        if (auto *sibling = node->NextSibling())
        {
          return sibling;
        }
      }
    }
    return nullptr;
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

    for (const Node &ancestor : ConstAncestorRange(current))
    {
      if (ancestor.NextSibling())
      {
        return ancestor.NextSibling();
      }
    }

    return nullptr;
  }

  RawPtr<Node> TreeTraversal::NextAncestorSibling(const Node &current, RawPtr<const Node> stayWithin) noexcept
  {
    assert(!current.NextSibling());
    assert(&current != stayWithin);

    for (const Node &ancestor : ConstAncestorRange(current))
    {
      if (&ancestor == stayWithin)
      {
        return nullptr;
      }

      if (ancestor.NextSibling())
      {
        return ancestor.NextSibling();
      }
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
    for (const auto &ancestor : ConstAncestorRange(current))
    {
      if (ancestor.PreviousSibling())
      {
        return ancestor.PreviousSibling();
      }
    }

    return nullptr;
  }

  RawPtr<Node> TreeTraversal::PreviousAncestorSiblingPostOrder(const Node &current,
                                                               RawPtr<const Node> stayWithin) noexcept
  {
    assert(!current.PreviousSibling());
    for (const auto &ancestor : ConstAncestorRange(current))
    {
      if (&ancestor == stayWithin)
      {
        return nullptr;
      }
      if (ancestor.PreviousSibling())
      {
        return ancestor.PreviousSibling();
      }
    }
    return nullptr;
  }

  RawPtr<const Element> TreeTraversal::NextElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstNextSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<Element> TreeTraversal::NextElementSibling(Node &current) noexcept
  {
    auto siblings = NextSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<const Element> TreeTraversal::PreviousElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstPreviousSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<Element> TreeTraversal::PreviousElementSibling(Node &current) noexcept
  {
    auto siblings = PreviousSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<const Element> TreeTraversal::FirstElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = FirstOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<Element> TreeTraversal::FirstElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = FirstOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<const Element> TreeTraversal::LastElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = LastOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<Element> TreeTraversal::LastElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = LastOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<const Text> TreeTraversal::NextExclusiveTextNode(const Node &current) noexcept
  {
    auto next = ConstFollowingRange(current);
    auto it = std::ranges::find_if(next, [](const auto &n) { return TreeQueries::IsExclusiveTextNode(n); });
    return it == std::ranges::end(next) ? nullptr : Downcast<Text>(&*it);
  }

  RawPtr<Text> TreeTraversal::NextExclusiveTextNode(Node &current) noexcept
  {
    auto next = FollowingRange(current);
    auto it = std::ranges::find_if(next, [](const auto &n) { return TreeQueries::IsExclusiveTextNode(n); });
    return it == std::ranges::end(next) ? nullptr : Downcast<Text>(&*it);
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