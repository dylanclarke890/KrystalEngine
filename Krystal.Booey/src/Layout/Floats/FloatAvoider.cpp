#include "Krystal.Booey/Layout/Floats/FloatAvoider.hpp"
#include <algorithm>

namespace krys::boo::layout
{
  // Floating boxes intersect their margin box with the other floats in the context,
  // while other float avoiders (e.g. non-floating formatting context roots) intersect their border box.
  FloatAvoider::FloatAvoider(LayoutPoint absoluteTopLeft, LayoutUnit borderBoxWidth,
                             const BoxGeometry::Edges &margin,
                             BoxGeometry::HorizontalEdges containingBlockAbsoluteContentBox,
                             bool isFloatingPositioned, bool isStartAligned) noexcept
      : _absoluteTopLeft(absoluteTopLeft), _borderBoxWidth(borderBoxWidth), _margin(margin),
        _containingBlockAbsoluteContentBox(containingBlockAbsoluteContentBox),
        _isFloatingPositioned(isFloatingPositioned), _isStartAligned(isStartAligned)
  {
    _absoluteTopLeft.SetX(InitialInlineStart());
  }

  void FloatAvoider::SetInlineStart(LayoutUnit inlineStart) noexcept
  {
    if (IsStartAligned() && IsFloatingBox())
    {
      inlineStart += MarginStart();
    }

    if (!IsStartAligned())
    {
      inlineStart -= BorderBoxWidth();
      if (IsFloatingBox())
      {
        inlineStart -= MarginEnd();
      }
    }

    auto constrainedByContainingBlock = [&]
    {
      // Horizontal position is constrained by the containing block's content box.
      // Compute the horizontal position for the new floating by taking both the contining block and the
      // current left/right floats into account.
      if (IsStartAligned())
      {
        return std::max(_containingBlockAbsoluteContentBox.Start + MarginStart(), inlineStart);
      }

      // Make sure it does not overflow the containing block on the right.
      return std::min(inlineStart, _containingBlockAbsoluteContentBox.End - MarginBoxWidth() + MarginStart());
    }();

    _absoluteTopLeft.SetX(constrainedByContainingBlock);
  }

  void FloatAvoider::SetBlockStart(LayoutUnit blockStart) noexcept
  {
    if (IsFloatingBox())
    {
      blockStart += MarginBefore();
    }

    _absoluteTopLeft.SetY(blockStart);
  }

  LayoutUnit FloatAvoider::InitialInlineStart() const noexcept
  {
    if (IsStartAligned())
    {
      return _containingBlockAbsoluteContentBox.Start + MarginStart();
    }
    return _containingBlockAbsoluteContentBox.End - MarginEnd() - BorderBoxWidth();
  }

  bool FloatAvoider::OverflowsContainingBlock() const noexcept
  {
    auto left = _absoluteTopLeft.X() - MarginStart();
    if (_containingBlockAbsoluteContentBox.Start > left)
    {
      return true;
    }

    auto right = left + MarginBoxWidth();
    return _containingBlockAbsoluteContentBox.End < right;
  }
}
