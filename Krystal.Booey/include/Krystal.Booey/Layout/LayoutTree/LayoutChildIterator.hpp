#pragma once

#include "Krystal.Booey/Layout/LayoutTree/LayoutIterator.hpp"

namespace krys::boo::layout
{
  /// @brief Iterates over the children of type @p T of a layout box.
  template <typename T>
  class LayoutChildIterator : public LayoutIterator<T>
  {
  public:
    explicit LayoutChildIterator(const ElementBox &parent) noexcept : LayoutIterator<T>(&parent)
    {
    }

    LayoutChildIterator(const ElementBox &parent, const T *current) noexcept
        : LayoutIterator<T>(&parent, current)
    {
    }

    LayoutChildIterator &operator++() noexcept
    {
      return static_cast<LayoutChildIterator &>(LayoutIterator<T>::TraverseNextSibling());
    }
  };

  /// @brief The children of type @p T of a layout box.
  template <typename T>
  class LayoutChildIteratorAdapter
  {
  private:
    const ElementBox &_parent;

  public:
    explicit LayoutChildIteratorAdapter(const ElementBox &parent) noexcept : _parent(parent)
    {
    }

    KRYS_NODISCARD LayoutChildIterator<T> begin() const noexcept
    {
      return LayoutChildIterator<T>(_parent, Traversal::FirstChild<T>(_parent));
    }

    KRYS_NODISCARD LayoutChildIterator<T> end() const noexcept
    {
      return LayoutChildIterator<T>(_parent);
    }

    KRYS_NODISCARD const T *First() const noexcept
    {
      return Traversal::FirstChild<T>(_parent);
    }
  };

  template <typename T>
  KRYS_NODISCARD inline LayoutChildIteratorAdapter<T> ChildrenOfType(const ElementBox &parent) noexcept
  {
    return LayoutChildIteratorAdapter<T>(parent);
  }
}
