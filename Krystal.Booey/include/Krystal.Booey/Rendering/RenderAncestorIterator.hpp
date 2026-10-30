#pragma once

#include "Krystal.Booey/Rendering/RenderIterator.hpp"

namespace krys::boo
{
  /// @brief Iterates over the ancestors of type @p T (which may be const) of a renderer.
  template <typename T>
  class RenderAncestorIterator : public RenderIterator<T>
  {
  public:
    RenderAncestorIterator() noexcept : RenderIterator<T>(nullptr)
    {
    }

    explicit RenderAncestorIterator(T *current) noexcept : RenderIterator<T>(nullptr, current)
    {
    }

    RenderAncestorIterator &operator++() noexcept
    {
      return static_cast<RenderAncestorIterator &>(RenderIterator<T>::TraverseAncestor());
    }
  };

  template <typename T>
  using RenderAncestorConstIterator = RenderAncestorIterator<const T>;

  /// @brief The ancestors of type @p T (which may be const) of a renderer, from the first.
  template <typename T>
  class RenderAncestorIteratorAdapter
  {
  private:
    T *_first;

  public:
    explicit RenderAncestorIteratorAdapter(T *first) noexcept : _first(first)
    {
    }

    KRYS_NODISCARD RenderAncestorIterator<T> begin() const noexcept
    {
      return RenderAncestorIterator<T>(_first);
    }

    KRYS_NODISCARD RenderAncestorIterator<T> end() const noexcept
    {
      return RenderAncestorIterator<T>();
    }

    KRYS_NODISCARD T *First() const noexcept
    {
      return _first;
    }
  };

  template <typename T>
  using RenderAncestorConstIteratorAdapter = RenderAncestorIteratorAdapter<const T>;

  template <typename T>
  KRYS_NODISCARD inline RenderAncestorIteratorAdapter<T> AncestorsOfType(RenderObject &descendant) noexcept
  {
    T *first = RenderTraversal::FindAncestorOfType<T>(descendant);
    return RenderAncestorIteratorAdapter<T>(first);
  }

  template <typename T>
  KRYS_NODISCARD inline RenderAncestorConstIteratorAdapter<T>
    AncestorsOfType(const RenderObject &descendant) noexcept
  {
    const T *first = RenderTraversal::FindAncestorOfType<const T>(descendant);
    return RenderAncestorConstIteratorAdapter<T>(first);
  }

  /// @brief @p first if it's a @p T, then its ancestors of type @p T.
  template <typename T>
  KRYS_NODISCARD inline RenderAncestorIteratorAdapter<T> LineageOfType(RenderObject &first) noexcept
  {
    if (IsRendererOfType<T>(first))
    {
      return RenderAncestorIteratorAdapter<T>(static_cast<T *>(&first));
    }
    return AncestorsOfType<T>(first);
  }

  template <typename T>
  KRYS_NODISCARD inline RenderAncestorConstIteratorAdapter<T>
    LineageOfType(const RenderObject &first) noexcept
  {
    if (IsRendererOfType<T>(first))
    {
      return RenderAncestorConstIteratorAdapter<T>(static_cast<const T *>(&first));
    }
    return AncestorsOfType<T>(first);
  }
}
