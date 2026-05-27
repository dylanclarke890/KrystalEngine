#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
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
    return TreeTraversal::FirstElementChild(document);
  }

  RawPtr<Element> TreeQueries::DocumentElement(Document &document) noexcept
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

#pragma endregion
}
