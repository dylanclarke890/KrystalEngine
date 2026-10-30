#pragma once

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief Walks a block's inline content: its inline-level renderers, descending into inline boxes but not
  /// into atomic inline-level boxes.
  class InlineWalker
  {
  private:
    RenderDescendantIterator<RenderObject> _iterator;

  public:
    explicit InlineWalker(const RenderBlockFlow &root, RenderObject *from = nullptr) noexcept
        : _iterator(root, from ? from : root.FirstChild())
    {
    }

    KRYS_NODISCARD RenderObject *Current() noexcept
    {
      return _iterator ? &*_iterator : nullptr;
    }

    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return !_iterator;
    }

    void Advance() noexcept
    {
      if (Is<RenderInline>(*_iterator))
      {
        _iterator.TraverseNext();
      }
      else
      {
        _iterator.TraverseNextSkippingChildren();
      }
    }
  };
}
