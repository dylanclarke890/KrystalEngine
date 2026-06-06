#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/CDATASection.hpp"
#include "Krystal.HTML/DOM/CharacterData.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
#include <ranges>

namespace Krys::HTML
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
    return *Krys::HTML::Last(ConstInclusiveAncestorRange(node));
  }

  Node &TreeQueries::Root(Node &node) noexcept
  {
    return *Krys::HTML::Last(InclusiveAncestorRange(node));
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
    return Krys::HTML::Count(ConstPreviousSiblingRange(node));
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

  RawPtr<const Element> TreeQueries::DocumentElement(const Document &document) noexcept
  {
    return TreeQueries::FirstElementChild(document);
  }

  RawPtr<Element> TreeQueries::DocumentElement(Document &document) noexcept
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

      return TreeQueries::NextAncestorSibling(current);
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

      return TreeQueries::NextAncestorSibling(current, stayWithin);
    }
  }

  RawPtr<Node> TreeQueries::Next(const Node &current) noexcept
  {
    return GetNext(current);
  }

  RawPtr<Node> TreeQueries::Next(const ContainerNode &current) noexcept
  {
    return GetNext(current);
  }

  RawPtr<Node> TreeQueries::Next(const Node &current, RawPtr<const Node> stayWithin) noexcept
  {
    return GetNext(current, stayWithin);
  }

  RawPtr<Node> TreeQueries::Next(const ContainerNode &current, RawPtr<const Node> stayWithin) noexcept
  {
    return GetNext(current, stayWithin);
  }

  RawPtr<Node> TreeQueries::NextSkippingChildren(const Node &current) noexcept
  {
    return GetNext<false>(current);
  }

  RawPtr<Node> TreeQueries::NextSkippingChildren(const Node &current, RawPtr<const Node> stayWithin) noexcept
  {
    return GetNext<false>(current, stayWithin);
  }

  RawPtr<Node> TreeQueries::NextShadowIncluding(const Node &current) noexcept
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

  RawPtr<Node> TreeQueries::NextShadowIncluding(const Node &current, RawPtr<const Node> stayWithin) noexcept
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

  RawPtr<Node> TreeQueries::NextAncestorSibling(const Node &current) noexcept
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

  RawPtr<Node> TreeQueries::NextAncestorSibling(const Node &current, RawPtr<const Node> stayWithin) noexcept
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

  RawPtr<const Text> TreeQueries::NextExclusiveTextNode(const Node &current) noexcept
  {
    auto next = ConstFollowingRange(current);
    auto it = std::ranges::find_if(next, [](const auto &n) { return IsExclusiveTextNode(n); });
    return it == std::ranges::end(next) ? nullptr : Downcast<Text>(&*it);
  }

  RawPtr<Text> TreeQueries::NextExclusiveTextNode(Node &current) noexcept
  {
    auto next = FollowingRange(current);
    auto it = std::ranges::find_if(next, [](const auto &n) { return IsExclusiveTextNode(n); });
    return it == std::ranges::end(next) ? nullptr : Downcast<Text>(&*it);
  }

  RawPtr<Text> TreeQueries::NextExclusiveTextNode(const Node &current, RawPtr<const Node> stayWithin) noexcept
  {
    for (RawPtr<Node> next = Next(current, stayWithin); next; next = Next(*next, stayWithin))
    {
      if (IsExclusiveTextNode(*next))
      {
        return Downcast<Text>(next);
      }
    }

    return nullptr;
  }

  RawPtr<Node> TreeQueries::Previous(const Node &current) noexcept
  {
    if (RawPtr<Node> previous = current.PreviousSibling())
    {
      return DeepLastChild(*previous);
    }

    return current.ParentNode();
  }

  RawPtr<Node> TreeQueries::Previous(const Node &current, RawPtr<const Node> stayWithin) noexcept
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

  RawPtr<Node> TreeQueries::PreviousSkippingChildren(const Node &current) noexcept
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

  RawPtr<Node> TreeQueries::PreviousSkippingChildren(const Node &current,
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

  RawPtr<Node> TreeQueries::Last(const ContainerNode &current) noexcept
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

  RawPtr<Node> TreeQueries::DeepLastChild(Node &current) noexcept
  {
    RawPtr<Node> last = &current;
    while (last->LastChild())
    {
      last = last->LastChild();
    }

    return last;
  }

  RawPtr<const Element> TreeQueries::NextElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstNextSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<Element> TreeQueries::NextElementSibling(Node &current) noexcept
  {
    auto siblings = NextSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<const Element> TreeQueries::PreviousElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstPreviousSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<Element> TreeQueries::PreviousElementSibling(Node &current) noexcept
  {
    auto siblings = PreviousSiblingRange(current);
    auto it = FirstOfType<Element>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<const Element> TreeQueries::FirstElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = FirstOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<Element> TreeQueries::FirstElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = FirstOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<const Element> TreeQueries::LastElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = LastOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<Element> TreeQueries::LastElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = LastOfType<Element>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<Element>(&*it);
  }

  RawPtr<const HTMLElement> TreeQueries::NextHTMLElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstNextSiblingRange(current);
    auto it = FirstOfType<HTMLElement>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  RawPtr<HTMLElement> TreeQueries::NextHTMLElementSibling(Node &current) noexcept
  {
    auto siblings = NextSiblingRange(current);
    auto it = FirstOfType<HTMLElement>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  RawPtr<const HTMLElement> TreeQueries::PreviousHTMLElementSibling(const Node &current) noexcept
  {
    auto siblings = ConstPreviousSiblingRange(current);
    auto it = FirstOfType<HTMLElement>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  RawPtr<HTMLElement> TreeQueries::PreviousHTMLElementSibling(Node &current) noexcept
  {
    auto siblings = PreviousSiblingRange(current);
    auto it = FirstOfType<HTMLElement>(siblings);
    return it == std::ranges::end(siblings) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  RawPtr<const HTMLElement> TreeQueries::FirstHTMLElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = FirstOfType<HTMLElement>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  RawPtr<HTMLElement> TreeQueries::FirstHTMLElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = FirstOfType<HTMLElement>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  RawPtr<const HTMLElement> TreeQueries::LastHTMLElementChild(const ContainerNode &node) noexcept
  {
    auto children = ConstChildNodeRange(node);
    auto it = LastOfType<HTMLElement>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<HTMLElement>(&*it);
  }

  RawPtr<HTMLElement> TreeQueries::LastHTMLElementChild(ContainerNode &node) noexcept
  {
    auto children = ChildNodeRange(node);
    auto it = LastOfType<HTMLElement>(children);
    return it == std::ranges::end(children) ? nullptr : Downcast<HTMLElement>(&*it);
  }

#pragma endregion

#pragma region Post Order Traversal

  RawPtr<Node> TreeQueries::NextPostOrder(const Node &current) noexcept
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

  RawPtr<Node> TreeQueries::NextPostOrder(const Node &current, RawPtr<const Node> stayWithin) noexcept
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

  RawPtr<Node> TreeQueries::PreviousPostOrder(const Node &current) noexcept
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

  RawPtr<Node> TreeQueries::PreviousPostOrder(const Node &current, RawPtr<const Node> stayWithin) noexcept
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

  RawPtr<Node> TreeQueries::PreviousSkippingChildrenPostOrder(const Node &current) noexcept
  {
    if (current.PreviousSibling())
    {
      return current.PreviousSibling();
    }

    return PreviousAncestorSiblingPostOrder(current);
  }

  RawPtr<Node> TreeQueries::PreviousSkippingChildrenPostOrder(const Node &current,
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

  RawPtr<Node> TreeQueries::PreviousAncestorSiblingPostOrder(const Node &current) noexcept
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

  RawPtr<Node> TreeQueries::PreviousAncestorSiblingPostOrder(const Node &current,
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

#pragma endregion

#pragma region Helpers

  bool TreeQueries::SameRoot(const Node &a, const Node &b) noexcept
  {
    return &Root(a) == &Root(b);
  }

  RawPtr<ContainerNode> TreeQueries::CommonAncestor(Node &a, Node &b) noexcept
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

  RawPtr<const ContainerNode> TreeQueries::CommonAncestor(const Node &a, const Node &b) noexcept
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

  RawPtr<Node> TreeQueries::ChildAt(ContainerNode &node, size_t index) noexcept
  {
    size_t i = 0;
    auto children = ChildNodeRange(node);
    auto it = std::ranges::find_if(children, [&](const Node &) { return i++ == index; });
    return it == std::ranges::end(children) ? nullptr : &*it;
  }

  RawPtr<const Node> TreeQueries::ChildAt(const ContainerNode &node, size_t index) noexcept
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

  bool TreeQueries::IsExclusiveTextNode(RawPtr<const Node> node) noexcept
  {
    return Is<Text>(node) && !Is<CDATASection>(node);
  }

#pragma endregion
}
