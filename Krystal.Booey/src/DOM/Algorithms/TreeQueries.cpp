#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include <ranges>

namespace krys::boo::dom
{
#pragma region Trees

  bool TreeQueries::IsParent(const Node &a, const Node &b) noexcept
  {
    return &a == b.ParentNode();
  }

  bool TreeQueries::IsChild(const Node &a, const Node &b) noexcept
  {
    return a.ParentNode() == &b;
  }

  const Node &TreeQueries::Root(const Node &node) noexcept
  {
    return *krys::boo::dom::Last(ConstInclusiveAncestorRange(node));
  }

  Node &TreeQueries::Root(Node &node) noexcept
  {
    return *krys::boo::dom::Last(InclusiveAncestorRange(node));
  }

  bool TreeQueries::IsDescendant(const Node &a, const Node &b) noexcept
  {
    return std::ranges::any_of(ConstAncestorRange(a), [&](const Node &n) { return &n == &b; });
  }

  bool TreeQueries::IsInclusiveDescendant(const Node &a, const Node &b) noexcept
  {
    return std::ranges::any_of(ConstInclusiveAncestorRange(a), [&](const Node &n) { return &n == &b; });
  }

  bool TreeQueries::IsAncestor(const Node &a, const Node &b) noexcept
  {
    return IsDescendant(b, a);
  }

  bool TreeQueries::IsInclusiveAncestor(const Node &a, const Node &b) noexcept
  {
    return IsInclusiveDescendant(b, a);
  }

  bool TreeQueries::IsSibling(const Node &a, const Node &b) noexcept
  {
    if (a.ParentNode() == nullptr || b.ParentNode() == nullptr || &a == &b)
    {
      return false;
    }

    return a.ParentNode() == b.ParentNode();
  }

  bool TreeQueries::IsInclusiveSibling(const Node &a, const Node &b) noexcept
  {
    if (&a == &b)
    {
      return true;
    }

    return IsSibling(a, b);
  }

  bool TreeQueries::IsPreceding(const Node &a, const Node &b) noexcept
  {
    if (&a == &b)
    {
      return false;
    }

    return std::ranges::any_of(ConstPrecedingRange(b), [&](const Node &current) { return &current == &a; });
  }

  bool TreeQueries::IsFollowing(const Node &a, const Node &b) noexcept
  {
    if (&a == &b)
    {
      return false;
    }

    return std::ranges::any_of(ConstPrecedingRange(a), [&](const Node &current) { return &current == &b; });
  }

  size_t TreeQueries::Index(const Node &node) noexcept
  {
    return Count(ConstPreviousSiblingRange(node));
  }

#pragma endregion

#pragma region Node Trees

  size_t TreeQueries::Length(const Node &node) noexcept
  {
    if (Is<DocumentType>(node) || Is<Attr>(node))
    {
      return 0uz;
    }

    if (auto *characterData = DynamicDowncast<CharacterData>(node))
    {
      return characterData->Length();
    }

    return Count(ConstChildNodeRange(Downcast<ContainerNode>(node)));
  }

  bool TreeQueries::IsEmpty(const Node &node) noexcept
  {
    return Length(node) == 0uz;
  }

#pragma endregion

#pragma region Document Trees

  bool TreeQueries::IsInDocumentTree(const Node &node) noexcept
  {
    return node.IsInDocumentTree();
  }

  const Element *TreeQueries::DocumentElement(const Document &document) noexcept
  {
    return TreeQueries::FirstElementChild(document);
  }

  Element *TreeQueries::DocumentElement(Document &document) noexcept
  {
    return TreeQueries::FirstElementChild(document);
  }

#pragma endregion

#pragma region Shadow Trees

  bool TreeQueries::IsInShadowTree(const Node &node) noexcept
  {
    return node.IsInShadowTree();
  }

  bool TreeQueries::IsHostIncludingInclusiveAncestorOf(const Node &a, const Node &b) noexcept
  {
    if (IsInclusiveAncestor(a, b))
    {
      return true;
    }

    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(Root(b)))
    {
      if (auto *host = shadowRoot->Host())
      {
        return IsHostIncludingInclusiveAncestorOf(a, *host);
      }
    }

    return false;
  }

#pragma endregion

#pragma region Shadow Roots

  const Node &TreeQueries::ShadowIncludingRoot(const Node &node) noexcept
  {
    auto &root = TreeQueries::Root(node);
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(root))
    {
      auto *host = shadowRoot->Host();
      return host ? ShadowIncludingRoot(*host) : root;
    }

    return root;
  }

  Node &TreeQueries::ShadowIncludingRoot(Node &node) noexcept
  {
    auto &root = TreeQueries::Root(node);
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(root))
    {
      auto *host = shadowRoot->Host();
      return host ? ShadowIncludingRoot(*host) : root;
    }

    return root;
  }

  bool TreeQueries::IsShadowIncludingDescendant(const Node &a, const Node &b) noexcept
  {
    if (TreeQueries::IsDescendant(a, b))
    {
      return true;
    }

    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(TreeQueries::Root(a)))
    {
      if (auto *host = shadowRoot->Host())
      {
        return IsShadowIncludingDescendant(*host, b);
      }
    }

    return false;
  }

  bool TreeQueries::IsShadowIncludingInclusiveDescendant(const Node &a, const Node &b) noexcept
  {
    if (&a == &b)
    {
      return true;
    }

    return IsShadowIncludingDescendant(a, b);
  }

  bool TreeQueries::IsShadowIncludingAncestor(const Node &a, const Node &b) noexcept
  {
    return IsShadowIncludingDescendant(b, a);
  }

  bool TreeQueries::IsShadowIncludingInclusiveAncestor(const Node &a, const Node &b) noexcept
  {
    return IsShadowIncludingInclusiveDescendant(b, a);
  }

  bool TreeQueries::IsClosedShadowHidden(const Node &a, const Node &b) noexcept
  {
    auto &aRoot = TreeQueries::Root(a);
    auto *aShadowRoot = DynamicDowncast<ShadowRoot>(aRoot);
    if (aShadowRoot == nullptr)
    {
      return false;
    }

    if (IsShadowIncludingInclusiveAncestor(aRoot, b))
    {
      return false;
    }

    if (aShadowRoot->Mode() == ShadowRootMode::Closed)
    {
      return true;
    }

    if (aShadowRoot->Host() && IsClosedShadowHidden(*aShadowRoot->Host(), b))
    {
      return true;
    }

    return false;
  }

  bool TreeQueries::IsShadowHost(const Node &node) noexcept
  {
    if (!Is<Element>(node))
    {
      return false;
    }

    return IsShadowHost(Downcast<Element>(node));
  }

  bool TreeQueries::IsShadowHost(const Element &node) noexcept
  {
    return node._shadowRoot != nullptr;
  }

#pragma endregion

#pragma region Tree Order Traversal

  namespace
  {
    template <bool CheckChildren = true, typename TNode>
    static Node *GetNext(const TNode &current) noexcept
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

      return TreeQueries::NextAncestorSibling(current);
    }

    template <bool CheckChildren = true, typename TNode>
    static Node *GetNext(const TNode &current, const Node *stayWithin) noexcept
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

      return TreeQueries::NextAncestorSibling(current, stayWithin);
    }
  }

  Node *TreeQueries::Next(const Node &current) noexcept
  {
    return GetNext(current);
  }

  Node *TreeQueries::Next(const ContainerNode &current) noexcept
  {
    return GetNext(current);
  }

  Node *TreeQueries::Next(const Node &current, const Node *stayWithin) noexcept
  {
    return GetNext(current, stayWithin);
  }

  Node *TreeQueries::Next(const ContainerNode &current, const Node *stayWithin) noexcept
  {
    return GetNext(current, stayWithin);
  }

  Node *TreeQueries::NextSkippingChildren(const Node &current) noexcept
  {
    return GetNext<false>(current);
  }

  Node *TreeQueries::NextSkippingChildren(const Node &current, const Node *stayWithin) noexcept
  {
    return GetNext<false>(current, stayWithin);
  }

  Node *TreeQueries::NextShadowIncluding(const Node &current) noexcept
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

    for (const Node *node = &current; node; node = node->ParentNode())
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

  Node *TreeQueries::NextShadowIncluding(const Node &current, const Node *stayWithin) noexcept
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

    for (const Node *node = &current; node; node = node->ParentNode())
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

  Node *TreeQueries::NextAncestorSibling(const Node &current) noexcept
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

  Node *TreeQueries::NextAncestorSibling(const Node &current, const Node *stayWithin) noexcept
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

  const Text *TreeQueries::NextExclusiveTextNode(const Node &current) noexcept
  {
    auto next = ConstFollowingRange(current);
    auto it = std::ranges::find_if(next, [](const auto &n) { return IsExclusiveTextNode(n); });
    return it == std::ranges::end(next) ? nullptr : Downcast<Text>(&*it);
  }

  Text *TreeQueries::NextExclusiveTextNode(Node &current) noexcept
  {
    auto next = FollowingRange(current);
    auto it = std::ranges::find_if(next, [](const auto &n) { return IsExclusiveTextNode(n); });
    return it == std::ranges::end(next) ? nullptr : Downcast<Text>(&*it);
  }

  Text *TreeQueries::NextExclusiveTextNode(const Node &current, const Node *stayWithin) noexcept
  {
    for (Node *next = Next(current, stayWithin); next; next = Next(*next, stayWithin))
    {
      if (IsExclusiveTextNode(*next))
      {
        return Downcast<Text>(next);
      }
    }

    return nullptr;
  }

  Node *TreeQueries::Previous(const Node &current) noexcept
  {
    if (Node *previous = current.PreviousSibling())
    {
      return DeepLastChild(*previous);
    }

    return current.ParentNode();
  }

  Node *TreeQueries::Previous(const Node &current, const Node *stayWithin) noexcept
  {
    if (Node *previous = current.PreviousSibling())
    {
      return DeepLastChild(*previous);
    }

    if (&current == stayWithin)
    {
      return nullptr;
    }

    return current.ParentNode();
  }

  Node *TreeQueries::PreviousSkippingChildren(const Node &current) noexcept
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

  Node *TreeQueries::PreviousSkippingChildren(const Node &current,
                                                     const Node *stayWithin) noexcept
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

  Node *TreeQueries::Last(const ContainerNode &current) noexcept
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

  Node *TreeQueries::DeepLastChild(Node &current) noexcept
  {
    Node *last = &current;
    while (last->LastChild())
    {
      last = last->LastChild();
    }

    return last;
  }

  const Element *TreeQueries::NextElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstNextSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  Element *TreeQueries::NextElementSibling(Node &current) noexcept
  {
    auto siblings = NextSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  const Element *TreeQueries::PreviousElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstPreviousSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  Element *TreeQueries::PreviousElementSibling(Node &current) noexcept
  {
    auto siblings = PreviousSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  const Element *TreeQueries::FirstElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = FirstOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  Element *TreeQueries::FirstElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = FirstOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  const Element *TreeQueries::LastElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = LastOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  Element *TreeQueries::LastElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = LastOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  const html::HTMLElement * TreeQueries::NextHTMLElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstNextSiblingRange(current);
    auto it = FirstOfType<html::HTMLElement>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<html::HTMLElement>(&*it);
  }

  html::HTMLElement *TreeQueries::NextHTMLElementSibling(Node &current) noexcept
  {
    auto siblings = NextSiblingRange(current);
    auto it = FirstOfType<html::HTMLElement>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<html::HTMLElement>(&*it);
  }

  const html::HTMLElement * TreeQueries::PreviousHTMLElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstPreviousSiblingRange(current);
    auto it = FirstOfType<html::HTMLElement>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<html::HTMLElement>(&*it);
  }

  html::HTMLElement *TreeQueries::PreviousHTMLElementSibling(Node &current) noexcept
  {
    auto siblings = PreviousSiblingRange(current);
    auto it = FirstOfType<html::HTMLElement>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<html::HTMLElement>(&*it);
  }

  const html::HTMLElement * TreeQueries::FirstHTMLElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = FirstOfType<html::HTMLElement>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<html::HTMLElement>(&*it);
  }

  html::HTMLElement *TreeQueries::FirstHTMLElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = FirstOfType<html::HTMLElement>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<html::HTMLElement>(&*it);
  }

  const html::HTMLElement * TreeQueries::LastHTMLElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = LastOfType<html::HTMLElement>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<html::HTMLElement>(&*it);
  }

  html::HTMLElement *TreeQueries::LastHTMLElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = LastOfType<html::HTMLElement>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<html::HTMLElement>(&*it);
  }

#pragma endregion

#pragma region Post Order Traversal

  Node *TreeQueries::NextPostOrder(const Node &current) noexcept
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

  Node *TreeQueries::NextPostOrder(const Node &current, const Node *stayWithin) noexcept
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

  Node *TreeQueries::PreviousPostOrder(const Node &current) noexcept
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

  Node *TreeQueries::PreviousPostOrder(const Node &current, const Node *stayWithin) noexcept
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

  Node *TreeQueries::PreviousSkippingChildrenPostOrder(const Node &current) noexcept
  {
    if (current.PreviousSibling())
    {
      return current.PreviousSibling();
    }

    return PreviousAncestorSiblingPostOrder(current);
  }

  Node *TreeQueries::PreviousSkippingChildrenPostOrder(const Node &current,
                                                              const Node *stayWithin) noexcept
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

  Node *TreeQueries::PreviousAncestorSiblingPostOrder(const Node &current) noexcept
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

  Node *TreeQueries::PreviousAncestorSiblingPostOrder(const Node &current,
                                                             const Node *stayWithin) noexcept
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

#pragma endregion

#pragma region Helpers

  bool TreeQueries::SameRoot(const Node &a, const Node &b) noexcept
  {
    return &Root(a) == &Root(b);
  }

  ContainerNode *TreeQueries::CommonAncestor(Node &a, Node &b) noexcept
  {
    auto *commonAncestor = &a;
    while (commonAncestor != nullptr)
    {
      if (IsInclusiveAncestor(*commonAncestor, b))
      {
        return Downcast<ContainerNode>(commonAncestor);
      }

      commonAncestor = commonAncestor->ParentNode();
    }

    return nullptr;
  }

  const ContainerNode *TreeQueries::CommonAncestor(const Node &a, const Node &b) noexcept
  {
    auto *commonAncestor = &a;
    while (commonAncestor != nullptr)
    {
      if (IsInclusiveAncestor(*commonAncestor, b))
      {
        return Downcast<ContainerNode>(commonAncestor);
      }

      commonAncestor = commonAncestor->ParentNode();
    }

    return nullptr;
  }

  Node *TreeQueries::ChildAt(ContainerNode &node, size_t index) noexcept
  {
    size_t i = 0;
    auto children = ChildNodeRange(node);
    auto it = std::ranges::find_if(children, [&](const Node &) { return i++ == index; });
    return it == std::ranges::end(children) ? nullptr : &*it;
  }

  const Node *TreeQueries::ChildAt(const ContainerNode &node, size_t index) noexcept
  {
    size_t i = 0;
    auto children = ConstChildNodeRange(node);
    auto it = std::ranges::find_if(children, [&](const Node &) { return i++ == index; });
    return it == std::ranges::end(children) ? nullptr : &*it;
  }

  bool TreeQueries::IsExclusiveTextNode(const Node &node) noexcept
  {
    return Is<Text>(node) && !Is<CDATASection>(node);
  }

  bool TreeQueries::IsExclusiveTextNode(const Node *node) noexcept
  {
    return Is<Text>(node) && !Is<CDATASection>(node);
  }

#pragma endregion
}
