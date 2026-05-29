#pragma once

#include "Krystal.HTML/DOM/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/DOM/ContainerNode.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include <ranges>

namespace Krys::HTML::detail
{
#pragma region SubtreeRange

  template <typename T, typename TNode>
  concept SubtreeIteratorPolicy = requires(RawPtr<Node> current) {
    { T::IsInclusive } -> ConvertibleTo<bool>;
    { T::Advance(current) } -> SameType<RawPtr<TNode>>;
  };

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
      _node = IteratorPolicy::Advance(_node);
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
  requires SubtreeIteratorPolicy<IteratorPolicy, TNode>
  class SubtreeRange
  {
  public:
    using iterator = SubtreeIterator<IteratorPolicy, TNode>;

  private:
    typename iterator::pointer _begin;

  public:
    SubtreeRange(typename iterator::reference node) noexcept
    requires(IteratorPolicy::IsInclusive)
        : _begin(&node)
    {
    }

    SubtreeRange(typename iterator::pointer node) noexcept
    requires(IteratorPolicy::IsInclusive)
        : _begin(node)
    {
    }

    SubtreeRange(typename iterator::reference node) noexcept
    requires(!IteratorPolicy::IsInclusive)
        : _begin(IteratorPolicy::Advance(&node))
    {
    }

    SubtreeRange(typename iterator::pointer node) noexcept
    requires(!IteratorPolicy::IsInclusive)
        : _begin(IteratorPolicy::Advance(node))
    {
    }

    iterator begin() const noexcept
    {
      return iterator(_begin);
    }

    iterator end() const noexcept
    {
      return iterator();
    }
  };

#pragma endregion

#pragma region ScopedSubtreeRange

  template <typename T, typename TNode>
  concept ScopedSubtreeIteratorPolicy = requires(RawPtr<TNode> current, RawPtr<Node> stayWithin) {
    { T::IsInclusive } -> ConvertibleTo<bool>;
    { T::ScopedAdvance(current, stayWithin) } -> SameType<RawPtr<TNode>>;
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
    RawPtr<match_constness_t<TNode, Node>> _root;

  public:
    ScopedSubtreeIterator() noexcept : _node(nullptr), _root(nullptr)
    {
    }

    ScopedSubtreeIterator(pointer node, RawPtr<match_constness_t<TNode, Node>> root) noexcept
        : _node(node), _root(root)
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
      _node = IteratorPolicy::ScopedAdvance(_node, _root);
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

  template <typename IteratorPolicy, typename TNode>
  requires ScopedSubtreeIteratorPolicy<IteratorPolicy, TNode>
  class ScopedSubtreeRange
  {
  public:
    using iterator = ScopedSubtreeIterator<IteratorPolicy, TNode>;

  private:
    RawPtr<match_constness_t<TNode, Node>> _root;
    typename iterator::pointer _begin;

  public:
    ScopedSubtreeRange() noexcept : _root(nullptr), _begin(nullptr)
    {
    }

    ScopedSubtreeRange(typename iterator::reference root) noexcept
    requires(IteratorPolicy::IsInclusive)
        : _root(&root), _begin(&root)
    {
    }

    ScopedSubtreeRange(typename iterator::pointer root) noexcept
    requires(IteratorPolicy::IsInclusive)
        : _root(root), _begin(root)
    {
    }

    ScopedSubtreeRange(typename iterator::reference root) noexcept
    requires(!IteratorPolicy::IsInclusive)
        : _root(&root), _begin(IteratorPolicy::ScopedAdvance(&root, &root))
    {
    }

    ScopedSubtreeRange(typename iterator::pointer root) noexcept
    requires(!IteratorPolicy::IsInclusive)
        : _root(root), _begin(IteratorPolicy::ScopedAdvance(root, root))
    {
    }

    ScopedSubtreeRange(match_constness_t<TNode, ContainerNode> &root) noexcept
    requires(!IteratorPolicy::IsInclusive && SameType<match_constness_t<TNode, Node>, TNode>)
        : _root(&root), _begin(root.FirstChild())
    {
    }

    ScopedSubtreeRange(RawPtr<match_constness_t<TNode, ContainerNode>> root) noexcept
    requires(!IteratorPolicy::IsInclusive && SameType<match_constness_t<TNode, Node>, TNode>)
        : _root(root), _begin(root->FirstChild())
    {
    }

    ScopedSubtreeRange(match_constness_t<TNode, ContainerNode> &root) noexcept
    requires(!IteratorPolicy::IsInclusive && SameType<match_constness_t<TNode, Element>, TNode>)
        : _root(&root), _begin(TreeTraversal::FirstElementChild(root))
    {
    }

    ScopedSubtreeRange(RawPtr<match_constness_t<TNode, ContainerNode>> root) noexcept
    requires(!IteratorPolicy::IsInclusive && SameType<match_constness_t<TNode, Element>, TNode>)
        : _root(root), _begin(TreeTraversal::FirstElementChild(*root))
    {
    }

    iterator begin() const noexcept
    {
      if (_begin != nullptr)
      {
        return iterator(_begin, _root);
      }
      else
      {
        return iterator();
      }
    }

    iterator end() const noexcept
    {
      return iterator();
    }
  };

#pragma endregion

#pragma region Iterator Policies

  template <typename TNode, bool Inclusive>
  struct NextSiblingIteratorPolicy
  {
    constexpr static bool IsInclusive = Inclusive;

    KRYS_NODISCARD static RawPtr<TNode> Advance(RawPtr<match_constness_t<TNode, Node>> current) noexcept
    {
      if constexpr (SameType<match_constness_t<TNode, HTMLElement>, TNode>)
      {
        return TreeTraversal::NextHTMLElementSibling(*current);
      }
      else if constexpr (SameType<match_constness_t<TNode, Element>, TNode>)
      {
        return TreeTraversal::NextElementSibling(*current);
      }
      else
      {
        return current->NextSibling();
      }
    }

    KRYS_NODISCARD static RawPtr<TNode> ScopedAdvance(RawPtr<match_constness_t<TNode, Node>> current,
                                                      RawPtr<const Node> root) noexcept
    {
      (void)root;
      return Advance(current);
    }
  };

  template <typename TNode, bool Inclusive>
  struct PreviousSiblingIteratorPolicy
  {
    constexpr static bool IsInclusive = Inclusive;

    KRYS_NODISCARD static RawPtr<TNode> Advance(RawPtr<match_constness_t<TNode, Node>> current) noexcept
    {
      if constexpr (SameType<match_constness_t<TNode, HTMLElement>, TNode>)
      {
        return TreeTraversal::PreviousHTMLElementSibling(*current);
      }
      else if constexpr (SameType<match_constness_t<TNode, Element>, TNode>)
      {
        return TreeTraversal::PreviousElementSibling(*current);
      }
      else
      {
        return current->PreviousSibling();
      }
    }

    KRYS_NODISCARD static RawPtr<TNode> ScopedAdvance(RawPtr<match_constness_t<TNode, Node>> current,
                                                      RawPtr<const Node> root) noexcept
    {
      (void)root;
      return Advance(current);
    }
  };

  template <typename TNode, bool Inclusive>
  struct AncestorIteratorPolicy
  {
    constexpr static bool IsInclusive = Inclusive;

    KRYS_NODISCARD static RawPtr<TNode> Advance(RawPtr<const Node> current) noexcept
    {
      return current->ParentNode();
    }
  };

  template <typename TNode, bool Inclusive>
  struct DescendantIteratorPolicy
  {
    constexpr static bool IsInclusive = Inclusive;

    KRYS_NODISCARD static RawPtr<TNode> ScopedAdvance(RawPtr<const Node> current,
                                                      RawPtr<const Node> stayWithin) noexcept
    {
      return TreeTraversal::Next(*current, stayWithin);
    }
  };

  template <typename TNode, bool Inclusive>
  struct ShadowIncludingDescendantIteratorPolicy
  {
    constexpr static bool IsInclusive = Inclusive;

    KRYS_NODISCARD static RawPtr<TNode> ScopedAdvance(RawPtr<const Node> current,
                                                      RawPtr<const Node> stayWithin) noexcept
    {
      return TreeTraversal::NextShadowIncluding(*current, stayWithin);
    }
  };

  template <typename TNode>
  struct PrecedingIteratorPolicy
  {
    constexpr static bool IsInclusive = false;

    KRYS_NODISCARD static RawPtr<TNode> Advance(RawPtr<const Node> current) noexcept
    {
      return TreeTraversal::Previous(*current);
    }
  };

  template <typename TNode>
  struct FollowingIteratorPolicy
  {
    constexpr static bool IsInclusive = false;

    KRYS_NODISCARD static RawPtr<TNode> Advance(RawPtr<const Node> current) noexcept
    {
      return TreeTraversal::Next(*current);
    }
  };

#pragma endregion
}

namespace Krys::HTML
{
  using NextSiblingRange = detail::SubtreeRange<detail::NextSiblingIteratorPolicy<Node, false>, Node>;
  using ConstNextSiblingRange =
    detail::SubtreeRange<detail::NextSiblingIteratorPolicy<const Node, false>, const Node>;

  using InclusiveNextSiblingRange = detail::SubtreeRange<detail::NextSiblingIteratorPolicy<Node, true>, Node>;
  using ConstInclusiveNextSiblingRange =
    detail::SubtreeRange<detail::NextSiblingIteratorPolicy<const Node, true>, const Node>;

  using NextElementSiblingRange =
    detail::SubtreeRange<detail::NextSiblingIteratorPolicy<Element, false>, Element>;
  using ConstNextElementSiblingRange =
    detail::SubtreeRange<detail::NextSiblingIteratorPolicy<const Element, false>, const Element>;

  using InclusiveNextElementSiblingRange =
    detail::SubtreeRange<detail::NextSiblingIteratorPolicy<Element, true>, Element>;
  using ConstInclusiveNextElementSiblingRange =
    detail::SubtreeRange<detail::NextSiblingIteratorPolicy<const Element, true>, const Element>;

  using PreviousSiblingRange = detail::SubtreeRange<detail::PreviousSiblingIteratorPolicy<Node, false>, Node>;
  using ConstPreviousSiblingRange =
    detail::SubtreeRange<detail::PreviousSiblingIteratorPolicy<const Node, false>, const Node>;

  using InclusivePreviousSiblingRange =
    detail::SubtreeRange<detail::PreviousSiblingIteratorPolicy<Node, true>, Node>;
  using ConstInclusivePreviousSiblingRange =
    detail::SubtreeRange<detail::PreviousSiblingIteratorPolicy<const Node, true>, const Node>;

  using PreviousElementSiblingRange =
    detail::SubtreeRange<detail::PreviousSiblingIteratorPolicy<Element, false>, Element>;
  using ConstPreviousElementSiblingRange =
    detail::SubtreeRange<detail::PreviousSiblingIteratorPolicy<const Element, false>, const Element>;

  using InclusivePreviousElementSiblingRange =
    detail::SubtreeRange<detail::PreviousSiblingIteratorPolicy<Element, true>, Element>;
  using ConstInclusivePreviousElementSiblingRange =
    detail::SubtreeRange<detail::PreviousSiblingIteratorPolicy<const Element, true>, const Element>;

  using AncestorRange = detail::SubtreeRange<detail::AncestorIteratorPolicy<Node, false>, Node>;
  using ConstAncestorRange =
    detail::SubtreeRange<detail::AncestorIteratorPolicy<const Node, false>, const Node>;

  using InclusiveAncestorRange = detail::SubtreeRange<detail::AncestorIteratorPolicy<Node, true>, Node>;
  using ConstInclusiveAncestorRange =
    detail::SubtreeRange<detail::AncestorIteratorPolicy<const Node, true>, const Node>;

  using DescendantRange = detail::ScopedSubtreeRange<detail::DescendantIteratorPolicy<Node, false>, Node>;
  using ConstDescendantRange =
    detail::ScopedSubtreeRange<detail::DescendantIteratorPolicy<const Node, false>, const Node>;

  using InclusiveDescendantRange =
    detail::ScopedSubtreeRange<detail::DescendantIteratorPolicy<Node, true>, Node>;
  using ConstInclusiveDescendantRange =
    detail::ScopedSubtreeRange<detail::DescendantIteratorPolicy<const Node, true>, const Node>;

  using ShadowIncludingDescendantRange =
    detail::ScopedSubtreeRange<detail::ShadowIncludingDescendantIteratorPolicy<Node, false>, Node>;
  using ConstShadowIncludingDescendantRange =
    detail::ScopedSubtreeRange<detail::ShadowIncludingDescendantIteratorPolicy<const Node, false>,
                               const Node>;

  using InclusiveShadowIncludingDescendantRange =
    detail::ScopedSubtreeRange<detail::ShadowIncludingDescendantIteratorPolicy<Node, true>, Node>;
  using ConstShadowIncludingInclusiveDescendantRange =
    detail::ScopedSubtreeRange<detail::ShadowIncludingDescendantIteratorPolicy<const Node, true>, const Node>;

  using PrecedingRange = detail::SubtreeRange<detail::PrecedingIteratorPolicy<Node>, Node>;
  using ConstPrecedingRange = detail::SubtreeRange<detail::PrecedingIteratorPolicy<const Node>, const Node>;

  using FollowingRange = detail::SubtreeRange<detail::FollowingIteratorPolicy<Node>, Node>;
  using ConstFollowingRange = detail::SubtreeRange<detail::FollowingIteratorPolicy<const Node>, const Node>;

  using ChildNodeRange = detail::ScopedSubtreeRange<detail::NextSiblingIteratorPolicy<Node, false>, Node>;
  using ConstChildNodeRange =
    detail::ScopedSubtreeRange<detail::NextSiblingIteratorPolicy<const Node, false>, const Node>;

  using ChildElementRange =
    detail::ScopedSubtreeRange<detail::NextSiblingIteratorPolicy<Element, false>, Element>;
  using ConstChildElementRange =
    detail::ScopedSubtreeRange<detail::NextSiblingIteratorPolicy<const Element, false>, const Element>;

  using ChildHTMLElementRange =
    detail::ScopedSubtreeRange<detail::NextSiblingIteratorPolicy<HTMLElement, false>, HTMLElement>;
  using ConstChildHTMLElementRange =
    detail::ScopedSubtreeRange<detail::NextSiblingIteratorPolicy<const HTMLElement, false>,
                               const HTMLElement>;

#pragma region Node Range Algorithms

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

  /// @brief Returns an iterator to the first node of a range that satisfies a predicate.
  template <std::ranges::forward_range TRange, typename Predicate>
  constexpr auto First(TRange &&range, Predicate &&pred) noexcept
  {
    return std::ranges::find_if(range, Krys::Move(pred));
  }

  /// @brief Returns an iterator to the last node of a range that satisfies a predicate.
  template <std::ranges::forward_range TRange, typename Predicate>
  constexpr auto Last(TRange &&range, Predicate &&pred) noexcept
  {
    auto it = std::ranges::begin(range);
    auto end = std::ranges::end(range);
    auto last = end;

    for (; it != end; ++it)
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
    return First(range, [](auto &&node) { return Is<TNode>(node); });
  }

  template <DerivedFrom<Node> TNode, std::ranges::forward_range TRange>
  constexpr auto LastOfType(TRange &&range) noexcept
  {
    return Last(range, [](auto &&node) { return Is<TNode>(node); });
  }

  template <DerivedFrom<Node> TNode, std::ranges::forward_range TRange>
  constexpr bool HasNodeOfType(TRange &&range) noexcept
  {
    return First(range, [](auto &&node) { return Is<TNode>(node); }) != std::ranges::end(range);
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

#pragma endregion
}