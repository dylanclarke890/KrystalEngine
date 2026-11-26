#pragma once

#include "Krystal.Web/Dom/Internals/Iterators/SubtreeRanges.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Dom/Internals/TreeTraversal.hpp"

namespace krys::web::dom::detail
{
  /// @brief Steps to the next sibling that's an HTML element.
  template <typename TNode, bool Inclusive>
  struct NextHTMLElementSiblingIteratorPolicy
  {
    constexpr static bool IsInclusive = Inclusive;

    /// @brief The first child of @p parent that's an HTML element, where a children range starts.
    KRYS_NODISCARD static TNode *FirstChild(match_constness_t<TNode, ContainerNode> &parent) noexcept
    {
      return TreeTraversal::FirstHTMLElementChild(parent);
    }

    KRYS_NODISCARD static TNode *Advance(match_constness_t<TNode, Node> *current) noexcept
    {
      return TreeTraversal::NextHTMLElementSibling(*current);
    }

    KRYS_NODISCARD static TNode *ScopedAdvance(match_constness_t<TNode, Node> *current,
                                               const Node *root) noexcept
    {
      (void)root;
      return Advance(current);
    }
  };

  /// @brief Steps to the previous sibling that's an HTML element.
  template <typename TNode, bool Inclusive>
  struct PreviousHTMLElementSiblingIteratorPolicy
  {
    constexpr static bool IsInclusive = Inclusive;

    KRYS_NODISCARD static TNode *Advance(match_constness_t<TNode, Node> *current) noexcept
    {
      return TreeTraversal::PreviousHTMLElementSibling(*current);
    }

    KRYS_NODISCARD static TNode *ScopedAdvance(match_constness_t<TNode, Node> *current,
                                               const Node *root) noexcept
    {
      (void)root;
      return Advance(current);
    }
  };
}

namespace krys::web::dom
{
  /// @brief The HTML element children of a node.
  using ChildHTMLElementRange =
    detail::ScopedSubtreeRange<detail::NextHTMLElementSiblingIteratorPolicy<HTMLElement, false>, HTMLElement>;
  using ConstChildHTMLElementRange =
    detail::ScopedSubtreeRange<detail::NextHTMLElementSiblingIteratorPolicy<const HTMLElement, false>,
                               const HTMLElement>;
}
