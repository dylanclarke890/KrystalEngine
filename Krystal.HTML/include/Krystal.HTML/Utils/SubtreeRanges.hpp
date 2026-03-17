#pragma once

#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include <ranges>

namespace Krys::HTML
{
  /// @brief Returns an iterator to the last node of a range.
  template <std::ranges::forward_range TRange>
  constexpr auto Last(TRange &&range) noexcept
  {
    auto it = std::ranges::begin(range);
    auto last = it;

    for (; it != std::ranges::end(range); ++it)
    {
      last = it;
    }

    return last;
  }

  /// @brief Returns an iterator to the last node of a range that satisfies a predicate.
  template <std::ranges::range TRange, typename Predicate>
  constexpr auto Last(TRange &&range, Predicate &&pred) noexcept
  {
    auto it = std::ranges::begin(range);
    auto last = it;

    for (; it != std::ranges::end(range); ++it)
    {
      if (pred(*it))
      {
        last = it;
      }
    }

    return last;
  }

  template <DerivedFrom<Node> TNode, std::ranges::forward_range TRange>
  constexpr auto FirstOfType(TRange &&range) noexcept
  {
    return std::ranges::find_if(range, [](auto &&node) { return Is<TNode>(node); });
  }

  template <DerivedFrom<Node> TNode, std::ranges::forward_range TRange>
  constexpr auto LastOfType(TRange &&range) noexcept
  {
    return Last(range, [](auto &&node) { return Is<TNode>(node); });
  }

  /// @brief Counts the number of nodes in a range.
  template <std::ranges::forward_range TRange>
  constexpr auto Count(TRange &&range) noexcept
  {
    if constexpr (std::ranges::sized_range<remove_cvref_t<TRange>>)
    {
      return std::ranges::size(range);
    }
    else
    {
      size_t count = 0;
      for (auto &&value : range)
      {
        (void)value;
        count++;
      }
      return count;
    }
  }

  /// @brief Counts the number of nodes in a range that satisfy a predicate.
  template <std::ranges::forward_range TRange, typename Predicate>
  constexpr auto Count(TRange &&range, Predicate &&pred) noexcept
  {
    size_t count = 0;
    for (auto &&value : range)
    {
      if (pred(value))
      {
        count++;
      }
    }
    return count;
  }
}

namespace Krys::HTML::detail
{
#pragma region Concepts

  template <typename T, typename TNode>
  concept HasAdvance = requires(RawPtr<TNode> current) {
    { T::Advance(current) } -> SameType<RawPtr<TNode>>;
  };

  template <typename T, typename TNode>
  concept HasScopedAdvance = requires(RawPtr<TNode> current, RawPtr<TNode> stayWithin) {
    { T::ScopedAdvance(current, stayWithin) } -> SameType<RawPtr<TNode>>;
  };

  template <typename T, typename TNode>
  concept HasAdvanceUnchecked = requires(RawPtr<TNode> current) {
    { T::AdvanceUnchecked(current) } -> SameType<RawPtr<TNode>>;
  };

  template <typename T, typename TNode>
  concept HasScopedAdvanceUnchecked = requires(RawPtr<TNode> current, RawPtr<TNode> stayWithin) {
    { T::ScopedAdvanceUnchecked(current, stayWithin) } -> SameType<RawPtr<TNode>>;
  };

  template <typename T, typename TNode>
  concept SubtreeIteratorPolicy = HasAdvance<T, TNode> || HasAdvanceUnchecked<T, TNode>;

  template <typename T, typename TNode>
  concept ScopedSubtreeIteratorPolicy = HasScopedAdvance<T, TNode> || HasScopedAdvanceUnchecked<T, TNode>;

#pragma endregion

#pragma region Iterators

  template <typename IteratorPolicy, typename TNode>
  requires(SubtreeIteratorPolicy<IteratorPolicy, TNode>)
  class SubtreeIterator
  {
  public:
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;
    using value_type = TNode;
    using difference_type = std::ptrdiff_t;
    using pointer = RawPtr<value_type>;
    using reference = value_type &;

  private:
    pointer _node;

  public:
    SubtreeIterator() noexcept : _node(nullptr)
    {
    }

    SubtreeIterator(pointer node) noexcept : _node(node)
    {
    }

    reference operator*() const noexcept
    {
      return *_node;
    }

    pointer operator->() const noexcept
    {
      return _node;
    }

    SubtreeIterator &operator++() noexcept
    {
      if constexpr (HasAdvanceUnchecked<IteratorPolicy, TNode>)
      {
        _node = IteratorPolicy::AdvanceUnchecked(_node);
      }
      else if constexpr (HasAdvance<IteratorPolicy, TNode>)
      {
        _node = IteratorPolicy::Advance(_node);
      }
      else
      {
        static_assert(false, "Unknown Advance policy for SubtreeIterator");
      }

      return *this;
    }

    SubtreeIterator operator++(int) noexcept
    {
      SubtreeIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const SubtreeIterator &other) const noexcept
    {
      return _node == other._node;
    }

    bool operator!=(const SubtreeIterator &other) const noexcept
    {
      return _node != other._node;
    }
  };

  template <typename IteratorPolicy, typename TNode>
  requires(ScopedSubtreeIteratorPolicy<IteratorPolicy, TNode>)
  class ScopedSubtreeIterator
  {
  public:
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;
    using value_type = TNode;
    using difference_type = std::ptrdiff_t;
    using pointer = RawPtr<value_type>;
    using reference = value_type &;

  private:
    pointer _node;
    pointer _stayWithin;

  public:
    ScopedSubtreeIterator() noexcept : _node(nullptr), _stayWithin(nullptr)
    {
    }

    ScopedSubtreeIterator(pointer node, pointer stayWithin) noexcept : _node(node), _stayWithin(stayWithin)
    {
    }

    reference operator*() const noexcept
    {
      return *_node;
    }

    pointer operator->() const noexcept
    {
      return _node;
    }

    ScopedSubtreeIterator &operator++() noexcept
    {
      if constexpr (HasScopedAdvanceUnchecked<IteratorPolicy, TNode>)
      {
        _node = IteratorPolicy::ScopedAdvanceUnchecked(_node, _stayWithin);
      }
      else if constexpr (HasScopedAdvance<IteratorPolicy, TNode>)
      {
        _node = IteratorPolicy::ScopedAdvance(_node, _stayWithin);
      }
      else
      {
        static_assert(false, "Unknown Advance policy for SubtreeIterator");
      }

      return *this;
    }

    ScopedSubtreeIterator operator++(int) noexcept
    {
      ScopedSubtreeIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const ScopedSubtreeIterator &other) const noexcept
    {
      return _node == other._node;
    }

    bool operator!=(const ScopedSubtreeIterator &other) const noexcept
    {
      return _node != other._node;
    }
  };

#pragma endregion

#pragma region Iterator Policies

  template <typename TNode>
  struct NextSiblingIteratorPolicy
  {
    KRYS_NODISCARD static RawPtr<TNode> AdvanceUnchecked(RawPtr<Node> current) noexcept
    {
      if constexpr (SameType<Element, TNode>)
      {
        return TreeTraversal::NextElementSibling(*current);
      }
      else
      {
        return current->NextSibling();
      }
    }

    KRYS_NODISCARD static RawPtr<const TNode> AdvanceUnchecked(RawPtr<const Node> current) noexcept
    {
      if constexpr (SameType<Element, TNode>)
      {
        return TreeTraversal::NextElementSibling(*current);
      }
      else
      {
        return current->NextSibling();
      }
    }
  };

  template <typename TNode>
  struct PreviousSiblingIteratorPolicy
  {
    KRYS_NODISCARD static RawPtr<TNode> AdvanceUnchecked(RawPtr<Node> current) noexcept
    {
      if constexpr (SameType<Element, TNode>)
      {
        return TreeTraversal::PreviousElementSibling(*current);
      }
      else
      {
        return current->PreviousSibling();
      }
    }

    KRYS_NODISCARD static RawPtr<const TNode> AdvanceUnchecked(RawPtr<const Node> current) noexcept
    {
      if constexpr (SameType<Element, TNode>)
      {
        return TreeTraversal::PreviousElementSibling(*current);
      }
      else
      {
        return current->PreviousSibling();
      }
    }
  };

  struct AncestorIteratorPolicy
  {
    KRYS_NODISCARD static RawPtr<Node> AdvanceUnchecked(RawPtr<Node> current) noexcept
    {
      return current->ParentNode();
    }

    KRYS_NODISCARD static RawPtr<const Node> AdvanceUnchecked(RawPtr<const Node> current) noexcept
    {
      return current->ParentNode();
    }
  };

  struct DescendantIteratorPolicy
  {
    KRYS_NODISCARD static RawPtr<Node> ScopedAdvanceUnchecked(RawPtr<Node> current,
                                                              RawPtr<Node> stayWithin) noexcept
    {
      return TreeTraversal::Next(*current, stayWithin);
    }

    KRYS_NODISCARD static RawPtr<const Node> ScopedAdvanceUnchecked(RawPtr<const Node> current,
                                                                    RawPtr<const Node> stayWithin) noexcept
    {
      return TreeTraversal::Next(*current, stayWithin);
    }
  };

  struct PrecedingIteratorPolicy
  {
    KRYS_NODISCARD static RawPtr<Node> AdvanceUnchecked(RawPtr<Node> current) noexcept
    {
      return TreeTraversal::Previous(*current);
    }

    KRYS_NODISCARD static RawPtr<const Node> AdvanceUnchecked(RawPtr<const Node> current) noexcept
    {
      return TreeTraversal::Previous(*current);
    }
  };

  struct FollowingIteratorPolicy
  {
    KRYS_NODISCARD static RawPtr<Node> AdvanceUnchecked(RawPtr<Node> current) noexcept
    {
      return TreeTraversal::Next(*current);
    }

    KRYS_NODISCARD static RawPtr<const Node> AdvanceUnchecked(RawPtr<const Node> current) noexcept
    {
      return TreeTraversal::Next(*current);
    }
  };

#pragma endregion

#pragma region Ranges

  template <typename IteratorPolicy, typename TNode>
  requires SubtreeIteratorPolicy<IteratorPolicy, TNode>
  class SubtreeRange
  {
  public:
    using iterator = SubtreeIterator<IteratorPolicy, TNode>;

  private:
    typename iterator::pointer _begin;

  public:
    iterator begin() const noexcept
    {
      return iterator(_begin);
    }

    iterator end() const noexcept
    {
      return iterator();
    }

  protected:
    SubtreeRange(typename iterator::pointer node) noexcept : _begin(node)
    {
    }

    SubtreeRange(typename iterator::reference node) noexcept : _begin(&node)
    {
    }
  };

  template <typename IteratorPolicy, typename TNode>
  requires ScopedSubtreeIteratorPolicy<IteratorPolicy, TNode>
  class ScopedSubtreeRange
  {
  public:
    using iterator = ScopedSubtreeIterator<IteratorPolicy, TNode>;

  private:
    typename iterator::pointer _begin;
    typename iterator::pointer _stayWithin;

  public:
    iterator begin() const noexcept
    {
      return iterator(_begin, _stayWithin);
    }

    iterator end() const noexcept
    {
      return iterator();
    }

  protected:
    ScopedSubtreeRange(typename iterator::pointer node, typename iterator::pointer stayWithin) noexcept
        : _begin(node), _stayWithin(stayWithin)
    {
    }

    ScopedSubtreeRange(typename iterator::reference node, typename iterator::pointer stayWithin) noexcept
        : _begin(&node), _stayWithin(stayWithin)
    {
    }
  };

  template <typename TNode, bool Inclusive>
  requires(SameType<remove_const_t<TNode>, Node>)
  class NextSiblingRange : public SubtreeRange<NextSiblingIteratorPolicy<Node>, TNode>
  {
  public:
    NextSiblingRange(TNode &node) noexcept
    requires(Inclusive)
        : SubtreeRange<NextSiblingIteratorPolicy<Node>, TNode>(node)
    {
    }

    NextSiblingRange(TNode &node) noexcept
    requires(!Inclusive)
        : SubtreeRange<NextSiblingIteratorPolicy<Node>, TNode>(node.NextSibling())
    {
    }
  };

  template <typename TNode, bool Inclusive>
  requires(SameType<remove_const_t<TNode>, Element>)
  class NextElementSiblingRange : public SubtreeRange<NextSiblingIteratorPolicy<Element>, TNode>
  {
  public:
    NextElementSiblingRange(TNode &node) noexcept
    requires(Inclusive)
        : SubtreeRange<NextSiblingIteratorPolicy<Element>, TNode>(node)
    {
    }

    NextElementSiblingRange(TNode &node) noexcept
    requires(!Inclusive)
        : SubtreeRange<NextSiblingIteratorPolicy<Element>, TNode>(TreeTraversal::NextElementSibling(node))
    {
    }
  };

  template <typename TNode, bool Inclusive>
  requires(SameType<remove_const_t<TNode>, Node>)
  class PreviousSiblingRange : public SubtreeRange<PreviousSiblingIteratorPolicy<Node>, TNode>
  {
  public:
    PreviousSiblingRange(TNode &node) noexcept
    requires(Inclusive)
        : SubtreeRange<PreviousSiblingIteratorPolicy<Node>, TNode>(node)
    {
    }

    PreviousSiblingRange(TNode &node) noexcept
    requires(!Inclusive)
        : SubtreeRange<PreviousSiblingIteratorPolicy<Node>, TNode>(node.PreviousSibling())
    {
    }
  };

  template <typename TNode, bool Inclusive>
  requires(SameType<remove_const_t<TNode>, Element>)
  class PreviousElementSiblingRange : public SubtreeRange<PreviousSiblingIteratorPolicy<Element>, TNode>
  {
  public:
    PreviousElementSiblingRange(TNode &node) noexcept
    requires(Inclusive)
        : SubtreeRange<PreviousSiblingIteratorPolicy<Element>, TNode>(
            TreeTraversal::PreviousElementSibling(node))
    {
    }

    PreviousElementSiblingRange(TNode &node) noexcept
    requires(!Inclusive)
        : SubtreeRange<PreviousSiblingIteratorPolicy<Element>, TNode>(node)
    {
    }
  };

  template <typename TNode, bool Inclusive>
  requires(SameType<remove_const_t<TNode>, Node>)
  class AncestorRange : public SubtreeRange<AncestorIteratorPolicy, TNode>
  {
  public:
    AncestorRange(TNode &node) noexcept
    requires(Inclusive)
        : SubtreeRange<AncestorIteratorPolicy, TNode>(node)
    {
    }

    AncestorRange(TNode &node) noexcept
    requires(!Inclusive)
        : SubtreeRange<AncestorIteratorPolicy, TNode>(node.ParentNode())
    {
    }
  };

  template <typename TNode, bool Inclusive>
  class DescendantRange : public ScopedSubtreeRange<DescendantIteratorPolicy, TNode>
  {
  public:
    DescendantRange(match_constness_t<TNode, ContainerNode> &node) noexcept
    requires(Inclusive)
        : ScopedSubtreeRange<DescendantIteratorPolicy, TNode>(node, &node)
    {
    }

    DescendantRange(match_constness_t<TNode, ContainerNode> &node) noexcept
    requires(!Inclusive)
        : ScopedSubtreeRange<DescendantIteratorPolicy, TNode>(node.FirstChild(), &node)
    {
    }
  };

  template <typename TNode>
  requires(SameType<remove_const_t<TNode>, Node>)
  class ChildNodeRange : public SubtreeRange<NextSiblingIteratorPolicy<Node>, TNode>
  {
  public:
    ChildNodeRange(match_constness_t<TNode, ContainerNode> &parent) noexcept
        : SubtreeRange<NextSiblingIteratorPolicy<Node>, TNode>(parent.FirstChild())
    {
    }
  };

  template <typename TNode>
  requires(SameType<remove_const_t<TNode>, Element>)
  class ChildElementRange : public SubtreeRange<NextSiblingIteratorPolicy<Element>, TNode>
  {
  public:
    ChildElementRange(match_constness_t<TNode, ContainerNode> &parent) noexcept
        : SubtreeRange<NextSiblingIteratorPolicy<Element>, TNode>(TreeTraversal::FirstElementChild(parent))
    {
    }
  };

  template <typename TNode>
  requires(SameType<remove_const_t<TNode>, Node>)
  class PrecedingRange : public SubtreeRange<PrecedingIteratorPolicy, TNode>
  {
  public:
    PrecedingRange(TNode &node) noexcept : SubtreeRange<PrecedingIteratorPolicy, TNode>(node)
    {
    }
  };

  template <typename TNode>
  requires(SameType<remove_const_t<TNode>, Node>)
  class FollowingRange : public SubtreeRange<FollowingIteratorPolicy, TNode>
  {
  public:
    FollowingRange(TNode &node) noexcept : SubtreeRange<FollowingIteratorPolicy, TNode>(node)
    {
    }
  };

#pragma endregion
}

namespace Krys::HTML
{
  using NextSiblingRange = detail::NextSiblingRange<Node, false>;
  using InclusiveNextSiblingRange = detail::NextSiblingRange<Node, true>;
  using ConstNextSiblingRange = detail::NextSiblingRange<const Node, false>;
  using ConstInclusiveNextSiblingRange = detail::NextSiblingRange<const Node, true>;

  using NextElementSiblingRange = detail::NextElementSiblingRange<Element, false>;
  using ConstNextElementSiblingRange = detail::NextElementSiblingRange<const Element, false>;
  using InclusiveNextElementSiblingRange = detail::NextElementSiblingRange<Element, true>;
  using ConstInclusiveNextElementSiblingRange = detail::NextElementSiblingRange<const Element, true>;

  using PreviousSiblingRange = detail::PreviousSiblingRange<Node, false>;
  using ConstPreviousSiblingRange = detail::PreviousSiblingRange<const Node, false>;
  using InclusivePreviousSiblingRange = detail::PreviousSiblingRange<Node, true>;
  using ConstInclusivePreviousSiblingRange = detail::PreviousSiblingRange<const Node, true>;

  using PreviousElementSiblingRange = detail::PreviousElementSiblingRange<Element, false>;
  using ConstPreviousElementSiblingRange = detail::PreviousElementSiblingRange<const Element, false>;
  using InclusivePreviousElementSiblingRange = detail::PreviousElementSiblingRange<Element, true>;
  using ConstInclusivePreviousElementSiblingRange = detail::PreviousElementSiblingRange<const Element, true>;

  using AncestorRange = detail::AncestorRange<Node, false>;
  using InclusiveAncestorRange = detail::AncestorRange<Node, true>;
  using ConstAncestorRange = detail::AncestorRange<const Node, false>;
  using ConstInclusiveAncestorRange = detail::AncestorRange<const Node, true>;

  using DescendantRange = detail::DescendantRange<Node, false>;
  using InclusiveDescendantRange = detail::DescendantRange<Node, true>;
  using ConstDescendantRange = detail::DescendantRange<const Node, false>;
  using ConstInclusiveDescendantRange = detail::DescendantRange<const Node, true>;

  using ChildNodeRange = detail::ChildNodeRange<Node>;
  using ConstChildNodeRange = detail::ChildNodeRange<const Node>;
  using ChildElementRange = detail::ChildElementRange<Element>;
  using ConstChildElementRange = detail::ChildElementRange<const Element>;

  using PrecedingRange = detail::PrecedingRange<Node>;
  using ConstPrecedingRange = detail::PrecedingRange<const Node>;
  using FollowingRange = detail::FollowingRange<Node>;
  using ConstFollowingRange = detail::FollowingRange<const Node>;
}