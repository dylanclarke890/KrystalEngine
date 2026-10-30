#pragma once

#include "Krystal.Booey/Layout/LayoutTree/LayoutIterator.hpp"

namespace krys::boo::layout
{
  /// @brief Iterates over the descendants of type @p T of a layout box, in tree order.
  template <typename T>
  class LayoutDescendantIterator : public LayoutIterator<T>
  {
  public:
    explicit LayoutDescendantIterator(const ElementBox &root) noexcept : LayoutIterator<T>(&root)
    {
    }

    LayoutDescendantIterator(const ElementBox &root, const T *current) noexcept
        : LayoutIterator<T>(&root, current)
    {
    }

    LayoutDescendantIterator &operator++() noexcept
    {
      return static_cast<LayoutDescendantIterator &>(LayoutIterator<T>::TraverseNext());
    }
  };

  /// @brief The descendants of type @p T of a layout box.
  template <typename T>
  class LayoutDescendantIteratorAdapter
  {
  private:
    const ElementBox &_root;

  public:
    explicit LayoutDescendantIteratorAdapter(const ElementBox &root) noexcept : _root(root)
    {
    }

    KRYS_NODISCARD LayoutDescendantIterator<T> begin() noexcept
    {
      return LayoutDescendantIterator<T>(_root, Traversal::FirstWithin<T>(_root));
    }

    KRYS_NODISCARD LayoutDescendantIterator<T> end() noexcept
    {
      return LayoutDescendantIterator<T>(_root);
    }

    KRYS_NODISCARD LayoutDescendantIterator<T> At(const T &current) noexcept
    {
      return LayoutDescendantIterator<T>(_root, &current);
    }
  };

  template <typename T>
  KRYS_NODISCARD inline LayoutDescendantIteratorAdapter<T> DescendantsOfType(const ElementBox &root) noexcept
  {
    return LayoutDescendantIteratorAdapter<T>(root);
  }
}
