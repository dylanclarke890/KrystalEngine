#pragma once

#include "Krystal.Booey/Dom/Algorithms/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/Html/Elements/HTMLElement.hpp"

namespace krys::boo::dom::detail
{
  /// @brief Steps to the next sibling that's an HTML element.
  template <typename TNode, bool Inclusive>
  struct NextHTMLElementSiblingIteratorPolicy
  {
    constexpr static bool IsInclusive = Inclusive;

    /// @brief The first child of @p parent that's an HTML element, where a children range starts.
    KRYS_NODISCARD static TNode *FirstChild(match_constness_t<TNode, ContainerNode> &parent) noexcept
    {
      return TreeQueries::FirstHTMLElementChild(parent);
    }

    KRYS_NODISCARD static TNode *Advance(match_constness_t<TNode, Node> *current) noexcept
    {
      return TreeQueries::NextHTMLElementSibling(*current);
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
      return TreeQueries::PreviousHTMLElementSibling(*current);
    }

    KRYS_NODISCARD static TNode *ScopedAdvance(match_constness_t<TNode, Node> *current,
                                               const Node *root) noexcept
    {
      (void)root;
      return Advance(current);
    }
  };
}

namespace krys::boo::dom
{
  /// @brief The HTML element children of a node.
  using ChildHTMLElementRange =
    detail::ScopedSubtreeRange<detail::NextHTMLElementSiblingIteratorPolicy<HTMLElement, false>, HTMLElement>;
  using ConstChildHTMLElementRange =
    detail::ScopedSubtreeRange<detail::NextHTMLElementSiblingIteratorPolicy<const HTMLElement, false>,
                               const HTMLElement>;
}
