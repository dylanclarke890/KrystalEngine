#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  class Node;
}

namespace krys::boo
{
  class RenderElement;
  class RenderObject;

  /// @brief Where a node's new renderer goes in the render tree: a parent, and the sibling to insert it
  /// before.
  class RenderTreePosition
  {
  private:
    RenderElement *_parent;
    RenderObject *_nextSibling {nullptr};
    bool _hasValidNextSibling {false};

  public:
    explicit RenderTreePosition(RenderElement &parent) noexcept : _parent(&parent)
    {
    }

    RenderTreePosition(RenderElement &parent, RenderObject *nextSibling) noexcept
        : _parent(&parent), _nextSibling(nextSibling), _hasValidNextSibling(true)
    {
    }

    KRYS_NODISCARD RenderElement &Parent() const noexcept
    {
      return *_parent;
    }

    KRYS_NODISCARD RenderObject *NextSibling() const noexcept
    {
      krys_debug_assert(_hasValidNextSibling);
      return _nextSibling;
    }

    void ComputeNextSibling(const dom::Node &node) noexcept;

    void MoveToLastChild() noexcept
    {
      _nextSibling = nullptr;
      _hasValidNextSibling = true;
    }

    void InvalidateNextSibling() noexcept
    {
      _hasValidNextSibling = false;
    }

    void InvalidateNextSibling(const RenderObject &siblingRenderer) noexcept;

    KRYS_NODISCARD RenderObject *NextSiblingRenderer(const dom::Node &node) const noexcept;
  };
}
