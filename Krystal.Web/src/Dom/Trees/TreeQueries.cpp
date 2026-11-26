#include "Krystal.Web/Dom/Trees/TreeQueries.hpp"
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
#include "Krystal.Web/Dom/Traversal/TreeTraversal.hpp"
#include <ranges>

namespace krys::web::dom
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
    return *krys::web::dom::Last(ConstInclusiveAncestorRange(node));
  }

  Node &TreeQueries::Root(Node &node) noexcept
  {
    return *krys::web::dom::Last(InclusiveAncestorRange(node));
  }

  bool TreeQueries::IsDescendant(const Node &a, const Node &b) noexcept
  {
    return std::ranges::any_of(ConstAncestorRange(a), [&](const Node &n) { return &n == &b; });
  }

  bool TreeQueries::IsDescendant(const Node &a, const Node *b) noexcept
  {
    return b && IsDescendant(a, *b);
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
    return TreeTraversal::FirstElementChild(document);
  }

  Element *TreeQueries::DocumentElement(Document &document) noexcept
  {
    return TreeTraversal::FirstElementChild(document);
  }

#pragma endregion

#pragma region Shadow Trees

  bool TreeQueries::IsInShadowTree(const Node &node) noexcept
  {
    return node.IsInShadowTree();
  }

  bool TreeQueries::IsHostIncludingInclusiveAncestorOf(const Node &a, const Node &b) noexcept
  {
    // A is an inclusive ancestor of B,
    if (IsInclusiveAncestor(a, b))
    {
      return true;
    }

    // or B's root has a non-null host and A is a host-including inclusive ancestor of B's root's host.
    // (A shadow root's host, or a template's contents' template element.)
    if (auto *fragment = DynamicDowncast<DocumentFragment>(Root(b)))
    {
      if (auto *host = fragment->Host())
      {
        return IsHostIncludingInclusiveAncestorOf(a, *host);
      }
    }

    return false;
  }

  bool TreeQueries::IsExclusiveDocumentFragmentNode(const Node &node) noexcept
  {
    return Is<DocumentFragment>(node) && !Is<ShadowRoot>(node);
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
      // A's root is a shadow root and A's root's host is a shadow-including inclusive descendant of B.
      if (auto *host = shadowRoot->Host())
      {
        return IsShadowIncludingInclusiveDescendant(*host, b);
      }
    }

    return false;
  }

  bool TreeQueries::IsShadowIncludingDescendant(const Node &a, const Node *b) noexcept
  {
    return b && IsShadowIncludingDescendant(a, *b);
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

    if (aShadowRoot->Mode() != ShadowRootMode::Open)
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

      return TreeTraversal::NextAncestorSibling(current);
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

      return TreeTraversal::NextAncestorSibling(current, stayWithin);
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

  Node *TreeQueries::NextShadowIncluding(const Node &current) noexcept
  {
    return NextShadowIncluding(current, nullptr);
  }

  Node *TreeQueries::NextShadowIncluding(const Node &current, const Node *stayWithin) noexcept
  {
    // A shadow host's shadow tree comes just after it, before its children.
    if (auto *host = DynamicDowncast<Element>(current))
    {
      if (auto *shadowRoot = TreeTraversal::GetShadowRoot(*host))
      {
        return shadowRoot;
      }
    }

    if (auto *child = current.FirstChild())
    {
      return child;
    }

    for (const Node *node = &current; node;)
    {
      if (node == stayWithin)
      {
        return nullptr;
      }

      if (auto *sibling = node->NextSibling())
      {
        return sibling;
      }

      // Climbing out of a shadow tree continues with the host's children, then what follows the host.
      if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(*node))
      {
        auto *host = shadowRoot->Host();
        if (auto *child = host->FirstChild())
        {
          return child;
        }
        node = host;
        continue;
      }

      node = node->ParentNode();
    }

    return nullptr;
  }

  Node *TreeQueries::Previous(const Node &current) noexcept
  {
    if (Node *previous = current.PreviousSibling())
    {
      return TreeTraversal::DeepLastChild(*previous);
    }

    return current.ParentNode();
  }

  Node *TreeQueries::Previous(const Node &current, const Node *stayWithin) noexcept
  {
    // Nothing within stayWithin comes before it, so its previous sibling's descendants are out of bounds.
    if (&current == stayWithin)
    {
      return nullptr;
    }

    if (Node *previous = current.PreviousSibling())
    {
      return TreeTraversal::DeepLastChild(*previous);
    }

    return current.ParentNode();
  }

#pragma endregion

#pragma region Helpers

  bool TreeQueries::IsExclusiveTextNode(const Node &node) noexcept
  {
    return Is<Text>(node) && !Is<CDATASection>(node);
  }

  bool TreeQueries::IsExclusiveTextNode(const Node *node) noexcept
  {
    return Is<Text>(node) && !Is<CDATASection>(node);
  }

  size_t TreeQueries::CountChildNodes(const Node &node) noexcept
  {
    auto *container = DynamicDowncast<ContainerNode>(node);
    return container ? Count(ConstChildNodeRange(*container)) : 0uz;
  }

#pragma endregion
}
