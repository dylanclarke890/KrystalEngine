#pragma once

#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Tree/TreeTraversal.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/TypeTraits.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML::detail
{
  template <typename T, typename TNode>
  concept HasAdvance = requires(RawPtr<TNode> current) {
    { T::Advance(current) } -> SameType<RawPtr<TNode>>;
  };

  template <typename T, typename TNode>
  concept HasAdvanceUnchecked = requires(RawPtr<TNode> current) {
    { T::AdvanceUnchecked(current) } -> SameType<RawPtr<TNode>>;
  };

  template <typename T, typename TNode>
  concept TreeIteratorPolicy = HasAdvance<T, TNode> || HasAdvanceUnchecked<T, TNode>;

  template <typename TAdvancePolicy, typename TNode>
  requires(TreeIteratorPolicy<TAdvancePolicy, TNode>)
  class TreeIterator
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
    TreeIterator() noexcept : _node(nullptr)
    {
    }

    TreeIterator(pointer node) noexcept : _node(node)
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

    TreeIterator &operator++() noexcept
    {
      if constexpr (HasAdvance<TAdvancePolicy, TNode>)
      {
        _node = TAdvancePolicy::Advance(_node);
      }
      else
      {
        _node = TAdvancePolicy::AdvanceUnchecked(_node);
      }

      return *this;
    }

    TreeIterator operator++(int) noexcept
    {
      TreeIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const TreeIterator &other) const noexcept
    {
      return _node == other._node;
    }

    bool operator!=(const TreeIterator &other) const noexcept
    {
      return _node != other._node;
    }
  };

  template <typename TAdvancePolicy, typename TNode>
  requires TreeIteratorPolicy<TAdvancePolicy, TNode>
  class SubtreeRange
  {
  public:
    using iterator = TreeIterator<TAdvancePolicy, TNode>;

  private:
    typename iterator::pointer _begin;

  public:
    iterator begin() const noexcept
    {
      return iterator(_begin);
    }

    iterator end() const noexcept
    {
      return iterator(nullptr);
    }

  protected:
    SubtreeRange(typename iterator::pointer node) noexcept : _begin(node)
    {
    }

    SubtreeRange(typename iterator::reference node) noexcept : _begin(&node)
    {
    }
  };

  struct ChildNodeIteratorPolicy
  {
    KRYS_NODISCARD static RawPtr<Node> AdvanceUnchecked(RawPtr<Node> current) noexcept
    {
      return current->NextSibling();
    }

    KRYS_NODISCARD static RawPtr<const Node> AdvanceUnchecked(RawPtr<const Node> current) noexcept
    {
      return current->NextSibling();
    }
  };

  template <typename TNode>
  requires(SameType<remove_const_t<TNode>, Node>)
  class ChildNodeRange : public SubtreeRange<ChildNodeIteratorPolicy, TNode>
  {
  public:
    ChildNodeRange(match_constness_t<TNode, ContainerNode> &parent) noexcept
        : SubtreeRange<ChildNodeIteratorPolicy, TNode>(parent.FirstChild())
    {
    }
  };

  struct ChildElementIteratorPolicy
  {
    KRYS_NODISCARD static RawPtr<Element> AdvanceUnchecked(RawPtr<Node> current) noexcept
    {
      return TreeTraversal::NextElementSibling(*current);
    }

    KRYS_NODISCARD static RawPtr<const Element> AdvanceUnchecked(RawPtr<const Node> current) noexcept
    {
      return TreeTraversal::NextElementSibling(*current);
    }
  };

  template <typename TNode>
  requires(SameType<remove_const_t<TNode>, Element>)
  class ChildElementRange : public SubtreeRange<ChildElementIteratorPolicy, TNode>
  {
  public:
    ChildElementRange(match_constness_t<TNode, ContainerNode> &parent) noexcept
        : SubtreeRange<ChildElementIteratorPolicy, TNode>(TreeTraversal::FirstElementChild(parent))
    {
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

  template <typename TNode>
  requires(SameType<remove_const_t<TNode>, Node>)
  class PrecedingRange : public SubtreeRange<PrecedingIteratorPolicy, TNode>
  {
  public:
    PrecedingRange(TNode &node) noexcept : SubtreeRange<PrecedingIteratorPolicy, TNode>(node)
    {
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

  struct DescendantIteratorPolicy
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

  template <typename TNode, bool Inclusive>
  class DescendantRange : public SubtreeRange<DescendantIteratorPolicy, TNode>
  {
  public:
    DescendantRange(match_constness_t<TNode, ContainerNode> &node) noexcept
    requires(Inclusive)
        : SubtreeRange<DescendantIteratorPolicy, TNode>(node)
    {
    }

    DescendantRange(match_constness_t<TNode, ContainerNode> &node) noexcept
    requires(!Inclusive)
        : SubtreeRange<DescendantIteratorPolicy, TNode>(node.FirstChild())
    {
    }
  };
}

namespace Krys::HTML
{
  using ChildNodeRange = ::Krys::HTML::detail::ChildNodeRange<Node>;
  using ConstChildNodeRange = ::Krys::HTML::detail::ChildNodeRange<const Node>;

  using ChildElementRange = ::Krys::HTML::detail::ChildElementRange<Element>;
  using ConstChildElementRange = ::Krys::HTML::detail::ChildElementRange<const Element>;

  using PrecedingRange = ::Krys::HTML::detail::PrecedingRange<Node>;
  using ConstPrecedingRange = ::Krys::HTML::detail::PrecedingRange<const Node>;

  using AncestorRange = ::Krys::HTML::detail::AncestorRange<Node, false>;
  using ConstAncestorRange = ::Krys::HTML::detail::AncestorRange<const Node, false>;

  using InclusiveAncestorRange = ::Krys::HTML::detail::AncestorRange<Node, true>;
  using ConstInclusiveAncestorRange = ::Krys::HTML::detail::AncestorRange<const Node, true>;

  using DescendantRange = ::Krys::HTML::detail::DescendantRange<Node, false>;
  using ConstDescendantRange = ::Krys::HTML::detail::DescendantRange<const Node, false>;

  using InclusiveDescendantRange = ::Krys::HTML::detail::DescendantRange<Node, true>;
  using ConstInclusiveDescendantRange = ::Krys::HTML::detail::DescendantRange<const Node, true>;
}