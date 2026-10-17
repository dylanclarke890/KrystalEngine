#pragma once

#include "Krystal.Booey/Graphics/LayoutPoint.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  /// @brief A box being placed among floats: a float itself (intersecting with its margin box) or a
  /// formatting context root next to them (intersecting with its border box).
  class FloatAvoider
  {
  private:
    // These coordinate values are relative to the formatting root's border box.
    LayoutPoint _absoluteTopLeft;
    // Note that float avoider should work with no height value.
    LayoutUnit _borderBoxWidth;
    BoxGeometry::Edges _margin;
    BoxGeometry::HorizontalEdges _containingBlockAbsoluteContentBox;
    bool _isFloatingPositioned {true};
    bool _isStartAligned {true};

  public:
    FloatAvoider(LayoutPoint absoluteTopLeft, LayoutUnit borderBoxWidth, const BoxGeometry::Edges &margin,
                 BoxGeometry::HorizontalEdges containingBlockAbsoluteContentBox, bool isFloatingPositioned,
                 bool isStartAligned) noexcept;
    virtual ~FloatAvoider() noexcept = default;

    void SetInlineStart(LayoutUnit inlineStart) noexcept;
    void SetBlockStart(LayoutUnit blockStart) noexcept;
    void ResetInlineStart() noexcept
    {
      _absoluteTopLeft.SetX(InitialInlineStart());
    }

    KRYS_NODISCARD bool OverflowsContainingBlock() const noexcept;

    KRYS_NODISCARD LayoutUnit BlockStart() const noexcept
    {
      auto blockStart = _absoluteTopLeft.Y();
      if (IsFloatingBox())
      {
        blockStart -= MarginBefore();
      }
      return blockStart;
    }

    KRYS_NODISCARD LayoutUnit InlineStart() const noexcept
    {
      auto inlineStart = _absoluteTopLeft.X();
      if (IsFloatingBox())
      {
        inlineStart -= MarginStart();
      }
      return inlineStart;
    }

    KRYS_NODISCARD LayoutUnit InlineEnd() const noexcept
    {
      auto inlineEnd = InlineStart() + BorderBoxWidth();
      if (IsFloatingBox())
      {
        inlineEnd += MarginEnd();
      }
      return inlineEnd;
    }

    KRYS_NODISCARD bool IsStartAligned() const noexcept
    {
      return _isStartAligned;
    }

  private:
    KRYS_NODISCARD LayoutUnit BorderBoxWidth() const noexcept
    {
      return _borderBoxWidth;
    }
    KRYS_NODISCARD LayoutUnit InitialInlineStart() const noexcept;

    KRYS_NODISCARD LayoutUnit MarginBefore() const noexcept
    {
      return _margin.Vertical.Before;
    }
    KRYS_NODISCARD LayoutUnit MarginAfter() const noexcept
    {
      return _margin.Vertical.After;
    }
    KRYS_NODISCARD LayoutUnit MarginStart() const noexcept
    {
      return _margin.Horizontal.Start;
    }
    KRYS_NODISCARD LayoutUnit MarginEnd() const noexcept
    {
      return _margin.Horizontal.End;
    }
    KRYS_NODISCARD LayoutUnit MarginBoxWidth() const noexcept
    {
      return MarginStart() + BorderBoxWidth() + MarginEnd();
    }

    KRYS_NODISCARD bool IsFloatingBox() const noexcept
    {
      return _isFloatingPositioned;
    }
  };
}
