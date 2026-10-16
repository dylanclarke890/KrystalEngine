#pragma once

#include "Krystal.Booey/Rendering/RenderIterator.hpp"

namespace krys::boo
{
  /// @brief Iterates over the descendants of type @p T (which may be const) of a renderer, in tree order.
  template <typename T>
  class RenderDescendantIterator : public RenderIterator<T>
  {
  public:
    explicit RenderDescendantIterator(const RenderElement &root) noexcept : RenderIterator<T>(&root)
    {
    }

    RenderDescendantIterator(const RenderElement &root, T *current) noexcept
        : RenderIterator<T>(&root, current)
    {
    }

    RenderDescendantIterator &operator++() noexcept
    {
      return static_cast<RenderDescendantIterator &>(RenderIterator<T>::TraverseNext());
    }
  };

  template <typename T>
  using RenderDescendantConstIterator = RenderDescendantIterator<const T>;

  /// @brief The descendants of type @p T (which may be const) of a renderer, in tree order.
  template <typename T>
  class RenderDescendantIteratorAdapter
  {
  private:
    RenderElementFor<T> *_root;

  public:
    explicit RenderDescendantIteratorAdapter(RenderElementFor<T> &root) noexcept : _root(&root)
    {
    }

    KRYS_NODISCARD RenderDescendantIterator<T> begin() const noexcept
    {
      return RenderDescendantIterator<T>(*_root, RenderTraversal::FirstWithin<T>(*_root));
    }

    KRYS_NODISCARD RenderDescendantIterator<T> end() const noexcept
    {
      return RenderDescendantIterator<T>(*_root);
    }

    KRYS_NODISCARD RenderDescendantIterator<T> at(T &current) const noexcept
    {
      return RenderDescendantIterator<T>(*_root, &current);
    }
  };

  template <typename T>
  using RenderDescendantConstIteratorAdapter = RenderDescendantIteratorAdapter<const T>;

  /// @brief Iterates over the descendants of type @p T (which may be const) of a renderer, children first.
  template <typename T>
  class RenderDescendantPostOrderIterator : public RenderPostOrderIterator<T>
  {
  public:
    explicit RenderDescendantPostOrderIterator(const RenderElement &root) noexcept
        : RenderPostOrderIterator<T>(&root)
    {
    }

    RenderDescendantPostOrderIterator(const RenderElement &root, T *current) noexcept
        : RenderPostOrderIterator<T>(&root, current)
    {
    }

    RenderDescendantPostOrderIterator &operator++() noexcept
    {
      return static_cast<RenderDescendantPostOrderIterator &>(RenderPostOrderIterator<T>::TraverseNext());
    }
  };

  template <typename T>
  using RenderDescendantPostOrderConstIterator = RenderDescendantPostOrderIterator<const T>;

  /// @brief The descendants of type @p T (which may be const) of a renderer, children first.
  template <typename T>
  class RenderDescendantPostOrderIteratorAdapter
  {
  private:
    RenderElementFor<T> *_root;

  public:
    explicit RenderDescendantPostOrderIteratorAdapter(RenderElementFor<T> &root) noexcept : _root(&root)
    {
    }

    KRYS_NODISCARD RenderDescendantPostOrderIterator<T> begin() const noexcept
    {
      return RenderDescendantPostOrderIterator<T>(*_root, RenderPostOrderTraversal::FirstWithin<T>(*_root));
    }

    KRYS_NODISCARD RenderDescendantPostOrderIterator<T> end() const noexcept
    {
      return RenderDescendantPostOrderIterator<T>(*_root);
    }

    KRYS_NODISCARD RenderDescendantPostOrderIterator<T> at(T &current) const noexcept
    {
      return RenderDescendantPostOrderIterator<T>(*_root, &current);
    }
  };

  template <typename T>
  using RenderDescendantPostOrderConstIteratorAdapter = RenderDescendantPostOrderIteratorAdapter<const T>;

  template <typename T>
  KRYS_NODISCARD inline RenderDescendantIteratorAdapter<T> DescendantsOfType(RenderElement &root) noexcept
  {
    return RenderDescendantIteratorAdapter<T>(root);
  }

  template <typename T>
  KRYS_NODISCARD inline RenderDescendantConstIteratorAdapter<T>
    DescendantsOfType(const RenderElement &root) noexcept
  {
    return RenderDescendantConstIteratorAdapter<T>(root);
  }

  template <typename T>
  KRYS_NODISCARD inline RenderDescendantPostOrderIteratorAdapter<T>
    DescendantsOfTypePostOrder(RenderElement &root) noexcept
  {
    return RenderDescendantPostOrderIteratorAdapter<T>(root);
  }

  template <typename T>
  KRYS_NODISCARD inline RenderDescendantPostOrderConstIteratorAdapter<T>
    DescendantsOfTypePostOrder(const RenderElement &root) noexcept
  {
    return RenderDescendantPostOrderConstIteratorAdapter<T>(root);
  }
}
