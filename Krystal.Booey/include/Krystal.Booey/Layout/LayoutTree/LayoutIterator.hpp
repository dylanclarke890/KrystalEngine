#pragma once

#include "Krystal.Booey/Layout/LayoutTree/LayoutInitialContainingBlock.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/TypeTraits.hpp"

namespace krys::boo::layout
{
  /// @brief Walks the boxes of type @p T in a layout subtree, staying within its root.
  template <typename T>
  class LayoutIterator
  {
  private:
    const ElementBox *_root;
    const T *_current;

  public:
    explicit LayoutIterator(const ElementBox *root) noexcept : _root(root), _current(nullptr)
    {
    }

    LayoutIterator(const ElementBox *root, const T *current) noexcept : _root(root), _current(current)
    {
    }

    KRYS_NODISCARD const T &Get() const noexcept
    {
      krys_debug_assert(_current);
      return *_current;
    }

    KRYS_NODISCARD const T &operator*() const noexcept
    {
      return Get();
    }

    KRYS_NODISCARD const T *operator->() const noexcept
    {
      return &Get();
    }

    KRYS_NODISCARD bool operator==(const LayoutIterator &other) const noexcept
    {
      krys_debug_assert(_root == other._root);
      return _current == other._current;
    }

    LayoutIterator &TraverseNext() noexcept;
    LayoutIterator &TraverseNextSkippingChildren() noexcept;
    LayoutIterator &TraverseNextSibling() noexcept;
  };

  // Similar to Is<>() but without the static_assert() making sure the check is necessary.
  template <typename T, typename U>
  KRYS_NODISCARD inline bool IsLayoutBoxOfType(const U &layoutBox) noexcept
  {
    if constexpr (SameType<T, U>)
    {
      return true;
    }
    else
    {
      return TypeCastTraits<const T, const U>::IsOfType(layoutBox);
    }
  }

  namespace LayoutBoxTraversal
  {
    template <typename U>
    requires DerivedFrom<remove_const_t<U>, ElementBox>
    KRYS_NODISCARD inline const Box *FirstChild(U &object) noexcept
    {
      return object.FirstChild();
    }

    KRYS_NODISCARD inline const Box *FirstChild(const Box &box) noexcept
    {
      if (auto *elementBox = DynamicDowncast<ElementBox>(box))
      {
        return elementBox->FirstChild();
      }
      return nullptr;
    }

    KRYS_NODISCARD inline const Box *NextAncestorSibling(const Box &current,
                                                         const ElementBox &stayWithin) noexcept
    {
      for (auto *ancestor = &current.Parent(); !Is<InitialContainingBlock>(*ancestor);
           ancestor = &ancestor->Parent())
      {
        if (ancestor == &stayWithin)
        {
          return nullptr;
        }
        if (auto *sibling = ancestor->NextSibling())
        {
          return sibling;
        }
      }
      return nullptr;
    }

    template <typename U>
    KRYS_NODISCARD inline const Box *Next(const U &current, const ElementBox &stayWithin) noexcept
    {
      if (auto *child = FirstChild(current))
      {
        return child;
      }

      if (static_cast<const Box *>(&current) == &stayWithin)
      {
        return nullptr;
      }

      if (auto *sibling = current.NextSibling())
      {
        return sibling;
      }

      return NextAncestorSibling(current, stayWithin);
    }

    template <typename U>
    KRYS_NODISCARD inline const Box *NextSkippingChildren(const U &current,
                                                          const ElementBox &stayWithin) noexcept
    {
      if (static_cast<const Box *>(&current) == &stayWithin)
      {
        return nullptr;
      }

      if (auto *sibling = current.NextSibling())
      {
        return sibling;
      }

      return NextAncestorSibling(current, stayWithin);
    }
  }

  // Traversal helpers
  namespace Traversal
  {
    template <typename T, typename U>
    KRYS_NODISCARD inline const T *FirstChild(U &current) noexcept
    {
      auto *object = LayoutBoxTraversal::FirstChild(current);
      while (object && !IsLayoutBoxOfType<T>(*object))
      {
        object = object->NextSibling();
      }
      return static_cast<const T *>(object);
    }

    template <typename T>
    KRYS_NODISCARD inline const T *NextSibling(const T &current) noexcept
    {
      auto *object = current.NextSibling();
      while (object && !IsLayoutBoxOfType<T>(*object))
      {
        object = object->NextSibling();
      }
      return static_cast<const T *>(object);
    }

    template <typename T, typename U>
    KRYS_NODISCARD inline const T *FirstWithin(const U &stayWithin) noexcept
    {
      auto *descendant = LayoutBoxTraversal::FirstChild(stayWithin);
      while (descendant && !IsLayoutBoxOfType<T>(*descendant))
      {
        descendant = LayoutBoxTraversal::Next(*descendant, stayWithin);
      }
      return static_cast<const T *>(descendant);
    }

    template <typename T, typename U>
    KRYS_NODISCARD inline const T *Next(const U &current, const ElementBox &stayWithin) noexcept
    {
      auto *descendant = LayoutBoxTraversal::Next(current, stayWithin);
      while (descendant && !IsLayoutBoxOfType<T>(*descendant))
      {
        descendant = LayoutBoxTraversal::Next(*descendant, stayWithin);
      }
      return static_cast<const T *>(descendant);
    }

    template <typename T, typename U>
    KRYS_NODISCARD inline const T *NextSkippingChildren(const U &current,
                                                        const ElementBox &stayWithin) noexcept
    {
      auto *descendant = LayoutBoxTraversal::NextSkippingChildren(current, stayWithin);
      while (descendant && !IsLayoutBoxOfType<T>(*descendant))
      {
        descendant = LayoutBoxTraversal::NextSkippingChildren(*descendant, stayWithin);
      }
      return static_cast<const T *>(descendant);
    }
  }

  template <typename T>
  inline LayoutIterator<T> &LayoutIterator<T>::TraverseNextSibling() noexcept
  {
    krys_debug_assert(_current);
    _current = Traversal::NextSibling<T>(*_current);
    return *this;
  }

  template <typename T>
  inline LayoutIterator<T> &LayoutIterator<T>::TraverseNext() noexcept
  {
    krys_debug_assert(_current);
    _current = Traversal::Next<T>(*_current, *_root);
    return *this;
  }

  template <typename T>
  inline LayoutIterator<T> &LayoutIterator<T>::TraverseNextSkippingChildren() noexcept
  {
    krys_debug_assert(_current);
    _current = Traversal::NextSkippingChildren<T>(*_current, *_root);
    return *this;
  }
}

#include "Krystal.Booey/Layout/LayoutTree/LayoutChildIterator.hpp"
