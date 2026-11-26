#include "Krystal.Web/Dom/Traversal/TreeTraversal.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Web/Dom/Internals/Iterators/SubtreeRanges.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/CDATASection.hpp"
#include "Krystal.Web/Dom/Nodes/CharacterData.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/DocumentType.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/Node.hpp"
#include "Krystal.Web/Dom/Nodes/PseudoElement.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
#include <ranges>

namespace krys::web::dom
{
#pragma region Shadow Roots

  ShadowRoot *TreeTraversal::GetShadowRoot(const Element &element) noexcept
  {
    return element._shadowRoot.get();
  }

  ShadowRoot *TreeTraversal::ContainingShadowRoot(const Node &node) noexcept
  {
    return DynamicDowncast<ShadowRoot>(const_cast<Node &>(TreeQueries::Root(node)));
  }

  Element *TreeTraversal::ShadowHost(const Node &node) noexcept
  {
    auto *shadowRoot = ContainingShadowRoot(node);
    return shadowRoot ? shadowRoot->Host() : nullptr;
  }

  Element *TreeTraversal::ParentOrShadowHostElement(const Node &node) noexcept
  {
    if (auto *parent = node.ParentElement())
    {
      return parent;
    }
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(node.ParentNode()))
    {
      return shadowRoot->Host();
    }
    return nullptr;
  }

#pragma endregion

#pragma region Tree Order Traversal

  Node *TreeTraversal::NextSkippingChildren(const Node &current) noexcept
  {
    if (current.NextSibling())
    {
      return current.NextSibling();
    }

    return NextAncestorSibling(current);
  }

  Node *TreeTraversal::NextSkippingChildren(const Node &current, const Node *stayWithin) noexcept
  {
    if (&current == stayWithin)
    {
      return nullptr;
    }

    if (current.NextSibling())
    {
      return current.NextSibling();
    }

    return NextAncestorSibling(current, stayWithin);
  }

  Node *TreeTraversal::NextAncestorSibling(const Node &current) noexcept
  {
    krys_debug_assert(!current.NextSibling());

    for (const Node &ancestor : ConstAncestorRange(current))
    {
      if (ancestor.NextSibling())
      {
        return ancestor.NextSibling();
      }
    }

    return nullptr;
  }

  Node *TreeTraversal::NextAncestorSibling(const Node &current, const Node *stayWithin) noexcept
  {
    krys_debug_assert(!current.NextSibling());
    krys_debug_assert(&current != stayWithin);

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

  const Text *TreeTraversal::NextExclusiveTextNode(const Node &current) noexcept
  {
    auto next = ConstFollowingRange(current);
    auto it = std::ranges::find_if(next, [](const auto &n) { return TreeQueries::IsExclusiveTextNode(n); });
    return it == std::ranges::end(next) ? nullptr : Downcast<Text>(&*it);
  }

  Text *TreeTraversal::NextExclusiveTextNode(Node &current) noexcept
  {
    auto next = FollowingRange(current);
    auto it = std::ranges::find_if(next, [](const auto &n) { return TreeQueries::IsExclusiveTextNode(n); });
    return it == std::ranges::end(next) ? nullptr : Downcast<Text>(&*it);
  }

  Text *TreeTraversal::NextExclusiveTextNode(const Node &current, const Node *stayWithin) noexcept
  {
    for (Node *next = TreeQueries::Next(current, stayWithin); next; next = TreeQueries::Next(*next, stayWithin))
    {
      if (TreeQueries::IsExclusiveTextNode(*next))
      {
        return Downcast<Text>(next);
      }
    }

    return nullptr;
  }

  Node *TreeTraversal::PreviousSkippingChildren(const Node &current) noexcept
  {
    if (current.PreviousSibling())
    {
      return current.PreviousSibling();
    }

    for (Node *ancestor = current.ParentNode(); ancestor; ancestor = ancestor->ParentNode())
    {
      if (ancestor->PreviousSibling())
      {
        return ancestor->PreviousSibling();
      }
    }

    return nullptr;
  }

  Node *TreeTraversal::PreviousSkippingChildren(const Node &current, const Node *stayWithin) noexcept
  {
    if (&current == stayWithin)
    {
      return nullptr;
    }

    if (current.PreviousSibling())
    {
      return current.PreviousSibling();
    }

    for (Node *ancestor = current.ParentNode(); ancestor != nullptr; ancestor = ancestor->ParentNode())
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

  namespace
  {
    KRYS_NODISCARD Element *ParentOrHostOfPseudo(const Node &node) noexcept
    {
      auto *pseudoElement = DynamicDowncast<PseudoElement>(node);
      return pseudoElement ? pseudoElement->HostElement() : node.ParentElement();
    }

    KRYS_NODISCARD bool IsPseudoElementOfType(const Node &node, PseudoElementType type) noexcept
    {
      auto *pseudoElement = DynamicDowncast<PseudoElement>(node);
      return pseudoElement && pseudoElement->GetPseudoElementType() == type;
    }

    KRYS_NODISCARD const Node *ParentOrHostNodeOfPseudo(const Node &node) noexcept
    {
      auto *pseudoElement = DynamicDowncast<PseudoElement>(node);
      return pseudoElement ? static_cast<const Node *>(pseudoElement->HostElement()) : node.ParentNode();
    }
  }

  Node *TreeTraversal::PseudoAwarePreviousSibling(const Node &node) noexcept
  {
    auto *parentOrHost = ParentOrHostOfPseudo(node);
    if (parentOrHost && !node.PreviousSibling())
    {
      if (IsPseudoElementOfType(node, PseudoElementType::After) && parentOrHost->LastChild())
      {
        return parentOrHost->LastChild();
      }
      if (!IsPseudoElementOfType(node, PseudoElementType::Before))
      {
        return parentOrHost->BeforePseudoElement();
      }
    }
    return node.PreviousSibling();
  }

  Node *TreeTraversal::PseudoAwareNextSibling(const Node &node) noexcept
  {
    auto *parentOrHost = ParentOrHostOfPseudo(node);
    if (parentOrHost && !node.NextSibling())
    {
      if (IsPseudoElementOfType(node, PseudoElementType::Before) && parentOrHost->FirstChild())
      {
        return parentOrHost->FirstChild();
      }
      if (!IsPseudoElementOfType(node, PseudoElementType::After))
      {
        return parentOrHost->AfterPseudoElement();
      }
    }
    return node.NextSibling();
  }

  Node *TreeTraversal::PseudoAwareFirstChild(const Node &node) noexcept
  {
    if (auto *currentElement = DynamicDowncast<Element>(node))
    {
      Node *first = currentElement->BeforePseudoElement();
      if (first)
      {
        return first;
      }
      first = currentElement->FirstChild();
      if (!first)
      {
        first = currentElement->AfterPseudoElement();
      }
      return first;
    }
    return node.FirstChild();
  }

  Node *TreeTraversal::PseudoAwareLastChild(const Node &node) noexcept
  {
    if (auto *currentElement = DynamicDowncast<Element>(node))
    {
      Node *last = currentElement->AfterPseudoElement();
      if (last)
      {
        return last;
      }
      last = currentElement->LastChild();
      if (!last)
      {
        last = currentElement->BeforePseudoElement();
      }
      return last;
    }
    return node.LastChild();
  }

  Node *TreeTraversal::PreviousIncludingPseudo(const Node &current, const Node *stayWithin) noexcept
  {
    if (&current == stayWithin)
    {
      return nullptr;
    }
    if (auto *previous = PseudoAwarePreviousSibling(current))
    {
      while (auto *lastChild = PseudoAwareLastChild(*previous))
      {
        previous = lastChild;
      }
      return previous;
    }
    return const_cast<Node *>(ParentOrHostNodeOfPseudo(current));
  }

  Node *TreeTraversal::NextIncludingPseudo(const Node &current, const Node *stayWithin) noexcept
  {
    if (auto *next = PseudoAwareFirstChild(current))
    {
      return next;
    }
    return NextIncludingPseudoSkippingChildren(current, stayWithin);
  }

  Node *TreeTraversal::NextIncludingPseudoSkippingChildren(const Node &current, const Node *stayWithin) noexcept
  {
    if (&current == stayWithin)
    {
      return nullptr;
    }
    if (auto *next = PseudoAwareNextSibling(current))
    {
      return next;
    }
    for (auto *ancestor = ParentOrHostNodeOfPseudo(current); ancestor; ancestor = ancestor->ParentNode())
    {
      if (ancestor == stayWithin)
      {
        return nullptr;
      }
      if (auto *next = PseudoAwareNextSibling(*ancestor))
      {
        return next;
      }
    }
    return nullptr;
  }

  Element *TreeTraversal::PreviousElementIncludingPseudo(const Node &current, const Node *stayWithin) noexcept
  {
    for (auto *node = PreviousIncludingPseudo(current, stayWithin); node;
         node = PreviousIncludingPseudo(*node, stayWithin))
    {
      if (auto *element = DynamicDowncast<Element>(*node))
      {
        return element;
      }
    }
    return nullptr;
  }

  Element *TreeTraversal::NextElementIncludingPseudo(const Node &current, const Node *stayWithin) noexcept
  {
    for (auto *node = NextIncludingPseudo(current, stayWithin); node;
         node = NextIncludingPseudo(*node, stayWithin))
    {
      if (auto *element = DynamicDowncast<Element>(*node))
      {
        return element;
      }
    }
    return nullptr;
  }

  Element *TreeTraversal::NextElementIncludingPseudoSkippingChildren(const Node &current,
                                                                   const Node *stayWithin) noexcept
  {
    for (auto *node = NextIncludingPseudoSkippingChildren(current, stayWithin); node;
         node = NextIncludingPseudoSkippingChildren(*node, stayWithin))
    {
      if (auto *element = DynamicDowncast<Element>(*node))
      {
        return element;
      }
    }
    return nullptr;
  }

  Element *TreeTraversal::PseudoAwarePreviousElementSibling(const Node &node) noexcept
  {
    for (auto *sibling = PseudoAwarePreviousSibling(node); sibling;
         sibling = PseudoAwarePreviousSibling(*sibling))
    {
      if (auto *element = DynamicDowncast<Element>(*sibling))
      {
        return element;
      }
    }
    return nullptr;
  }

  Node *TreeTraversal::Last(const ContainerNode &current) noexcept
  {
    Node *node = current.LastChild();
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

  Node *TreeTraversal::DeepLastChild(Node &current) noexcept
  {
    Node *last = &current;
    while (last->LastChild())
    {
      last = last->LastChild();
    }

    return last;
  }

  const Element *TreeTraversal::NextElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstNextSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  Element *TreeTraversal::NextElementSibling(Node &current) noexcept
  {
    auto siblings = NextSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  const Element *TreeTraversal::PreviousElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstPreviousSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  Element *TreeTraversal::PreviousElementSibling(Node &current) noexcept
  {
    auto siblings = PreviousSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  const Element *TreeTraversal::FirstElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = FirstOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  Element *TreeTraversal::FirstElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = FirstOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  const Element *TreeTraversal::LastElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = LastOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  Element *TreeTraversal::LastElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = LastOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  const HTMLElement *TreeTraversal::NextHTMLElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstNextSiblingRange(current);
    auto it = FirstOfType<HTMLElement>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  HTMLElement *TreeTraversal::NextHTMLElementSibling(Node &current) noexcept
  {
    auto siblings = NextSiblingRange(current);
    auto it = FirstOfType<HTMLElement>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  const HTMLElement *TreeTraversal::PreviousHTMLElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstPreviousSiblingRange(current);
    auto it = FirstOfType<HTMLElement>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  HTMLElement *TreeTraversal::PreviousHTMLElementSibling(Node &current) noexcept
  {
    auto siblings = PreviousSiblingRange(current);
    auto it = FirstOfType<HTMLElement>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  const HTMLElement *TreeTraversal::FirstHTMLElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = FirstOfType<HTMLElement>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  HTMLElement *TreeTraversal::FirstHTMLElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = FirstOfType<HTMLElement>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  const HTMLElement *TreeTraversal::LastHTMLElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = LastOfType<HTMLElement>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  HTMLElement *TreeTraversal::LastHTMLElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = LastOfType<HTMLElement>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<HTMLElement>(&*it);
  }

#pragma endregion

#pragma region Post Order Traversal

  Node *TreeTraversal::NextPostOrder(const Node &current) noexcept
  {
    if (!current.NextSibling())
    {
      return current.ParentNode();
    }

    Node *next = current.NextSibling();
    while (next->FirstChild())
    {
      next = next->FirstChild();
    }

    return next;
  }

  Node *TreeTraversal::NextPostOrder(const Node &current, const Node *stayWithin) noexcept
  {
    if (&current == stayWithin)
    {
      return nullptr;
    }

    if (!current.NextSibling())
    {
      return current.ParentNode();
    }

    Node *next = current.NextSibling();
    while (next->FirstChild())
    {
      next = next->FirstChild();
    }

    return next;
  }

  Node *TreeTraversal::PreviousPostOrder(const Node &current) noexcept
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

  Node *TreeTraversal::PreviousPostOrder(const Node &current, const Node *stayWithin) noexcept
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

  Node *TreeTraversal::PreviousSkippingChildrenPostOrder(const Node &current) noexcept
  {
    if (current.PreviousSibling())
    {
      return current.PreviousSibling();
    }

    return PreviousAncestorSiblingPostOrder(current);
  }

  Node *TreeTraversal::PreviousSkippingChildrenPostOrder(const Node &current, const Node *stayWithin) noexcept
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

  Node *TreeTraversal::PreviousAncestorSiblingPostOrder(const Node &current) noexcept
  {
    krys_debug_assert(!current.PreviousSibling());

    for (const auto &ancestor : ConstAncestorRange(current))
    {
      if (ancestor.PreviousSibling())
      {
        return ancestor.PreviousSibling();
      }
    }

    return nullptr;
  }

  Node *TreeTraversal::PreviousAncestorSiblingPostOrder(const Node &current, const Node *stayWithin) noexcept
  {
    krys_debug_assert(!current.PreviousSibling());

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

#pragma endregion

#pragma region Helpers

  bool TreeTraversal::SameRoot(const Node &a, const Node &b) noexcept
  {
    return &TreeQueries::Root(a) == &TreeQueries::Root(b);
  }

  ContainerNode *TreeTraversal::CommonAncestor(Node &a, Node &b) noexcept
  {
    auto *commonAncestor = &a;
    while (commonAncestor != nullptr)
    {
      if (TreeQueries::IsInclusiveAncestor(*commonAncestor, b))
      {
        return Downcast<ContainerNode>(commonAncestor);
      }

      commonAncestor = commonAncestor->ParentNode();
    }

    return nullptr;
  }

  const ContainerNode *TreeTraversal::CommonAncestor(const Node &a, const Node &b) noexcept
  {
    auto *commonAncestor = &a;
    while (commonAncestor != nullptr)
    {
      if (TreeQueries::IsInclusiveAncestor(*commonAncestor, b))
      {
        return Downcast<ContainerNode>(commonAncestor);
      }

      commonAncestor = commonAncestor->ParentNode();
    }

    return nullptr;
  }

  Node *TreeTraversal::ChildAt(ContainerNode &node, size_t index) noexcept
  {
    size_t i = 0;
    auto children = ChildNodeRange(node);
    auto it = std::ranges::find_if(children, [&](const Node &) { return i++ == index; });
    return it == std::ranges::end(children) ? nullptr : &*it;
  }

  const Node *TreeTraversal::ChildAt(const ContainerNode &node, size_t index) noexcept
  {
    size_t i = 0;
    auto children = ConstChildNodeRange(node);
    auto it = std::ranges::find_if(children, [&](const Node &) { return i++ == index; });
    return it == std::ranges::end(children) ? nullptr : &*it;
  }

#pragma endregion
}
