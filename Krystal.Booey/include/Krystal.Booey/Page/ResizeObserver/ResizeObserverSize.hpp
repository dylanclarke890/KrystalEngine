#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The size of an observed box, in its writing mode's axes.
  /// @see https://drafts.csswg.org/resize-observer/#resizeobserversize
  class ResizeObserverSize : public RefCounted<ResizeObserverSize>
  {
  private:
    double _inlineSize {0};
    double _blockSize {0};

    ResizeObserverSize(double inlineSize, double blockSize) noexcept
        : _inlineSize(inlineSize), _blockSize(blockSize)
    {
    }

  public:
    KRYS_NODISCARD static Ref<ResizeObserverSize> Create(double inlineSize, double blockSize) noexcept
    {
      return AdoptRef(*new ResizeObserverSize(inlineSize, blockSize));
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserversize-inlinesize
    KRYS_NODISCARD double InlineSize() const noexcept
    {
      return _inlineSize;
    }

    /// @see https://drafts.csswg.org/resize-observer/#dom-resizeobserversize-blocksize
    KRYS_NODISCARD double BlockSize() const noexcept
    {
      return _blockSize;
    }
  };
}
