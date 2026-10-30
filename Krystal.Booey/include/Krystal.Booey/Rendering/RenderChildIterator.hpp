#pragma once

#include "Krystal.Booey/Rendering/RenderIterator.hpp"

namespace krys::boo
{
  /// @brief Iterates over the children of type @p T (which may be const) of a renderer.
  template <typename T>
  class RenderChildIterator : public RenderIterator<T>
  {
  public:
    explicit RenderChildIterator(const RenderElement &parent) noexcept : RenderIterator<T>(&parent)
    {
    }

    RenderChildIterator(const RenderElement &parent, T *current) noexcept
        : RenderIterator<T>(&parent, current)
    {
    }

    RenderChildIterator &operator++() noexcept
    {
      return static_cast<RenderChildIterator &>(RenderIterator<T>::TraverseNextSibling());
    }
  };

  template <typename T>
  using RenderChildConstIterator = RenderChildIterator<const T>;

  /// @brief The children of type @p T (which may be const) of a renderer.
  template <typename T>
  class RenderChildIteratorAdapter
  {
  private:
    RenderElementFor<T> *_parent;

  public:
    explicit RenderChildIteratorAdapter(RenderElementFor<T> &parent) noexcept : _parent(&parent)
    {
    }

    KRYS_NODISCARD RenderChildIterator<T> begin() const noexcept
    {
      return RenderChildIterator<T>(*_parent, RenderTraversal::FirstChild<T>(*_parent));
    }

    KRYS_NODISCARD RenderChildIterator<T> end() const noexcept
    {
      return RenderChildIterator<T>(*_parent);
    }

    KRYS_NODISCARD T *First() const noexcept
    {
      return RenderTraversal::FirstChild<T>(*_parent);
    }

    KRYS_NODISCARD T *Last() const noexcept
    {
      return RenderTraversal::LastChild<T>(*_parent);
    }
  };

  template <typename T>
  using RenderChildConstIteratorAdapter = RenderChildIteratorAdapter<const T>;

  template <typename T>
  KRYS_NODISCARD inline RenderChildIteratorAdapter<T> ChildrenOfType(RenderElement &parent) noexcept
  {
    return RenderChildIteratorAdapter<T>(parent);
  }

  template <typename T>
  KRYS_NODISCARD inline RenderChildConstIteratorAdapter<T>
    ChildrenOfType(const RenderElement &parent) noexcept
  {
    return RenderChildConstIteratorAdapter<T>(parent);
  }
}
