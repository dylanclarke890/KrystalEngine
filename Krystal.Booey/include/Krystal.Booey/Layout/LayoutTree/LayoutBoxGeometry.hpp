#pragma once

#include "Krystal.Booey/Layout/LayoutTree/LayoutGeometryRect.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  /// @brief A layout box's geometry in its formatting context: its border box's position relative to its
  /// containing block, and its content box size, margins, borders, padding and scrollbar space, all
  /// logical.
  class BoxGeometry
  {
  public:
    struct VerticalEdges
    {
      LayoutUnit Before;
      LayoutUnit After;

      VerticalEdges &operator+=(VerticalEdges other) noexcept
      {
        Before += other.Before;
        After += other.After;
        return *this;
      }
    };

    struct HorizontalEdges
    {
      LayoutUnit Start;
      LayoutUnit End;
    };

    struct Edges
    {
      HorizontalEdges Horizontal;
      VerticalEdges Vertical;

      KRYS_NODISCARD LayoutUnit Width() const noexcept
      {
        return Horizontal.Start + Horizontal.End;
      }

      KRYS_NODISCARD LayoutUnit Height() const noexcept
      {
        return Vertical.Before + Vertical.After;
      }
    };

  private:
    LayoutPoint _topLeft;
    LayoutUnit _contentBoxWidth;
    LayoutUnit _contentBoxHeight;
    Edges _margin;
    Edges _border;
    Edges _padding;
    LayoutUnit _verticalSpaceForScrollbar;
    LayoutUnit _horizontalSpaceForScrollbar;
#if KRYS_DEBUG
    bool _hasValidTop {false};
    bool _hasValidLeft {false};
    bool _hasValidHorizontalMargin {false};
    bool _hasValidVerticalMargin {false};
    bool _hasValidBorder {false};
    bool _hasValidPadding {false};
    bool _hasValidContentBoxHeight {false};
    bool _hasValidContentBoxWidth {false};
    bool _hasPrecomputedMarginBefore {false};
#endif

  public:
    BoxGeometry() noexcept = default;
    BoxGeometry(const BoxGeometry &) noexcept = default;
    BoxGeometry &operator=(const BoxGeometry &) noexcept = default;

    KRYS_NODISCARD static LayoutUnit BorderBoxTop(const BoxGeometry &box) noexcept
    {
      return box.Top();
    }
    KRYS_NODISCARD static LayoutUnit BorderBoxLeft(const BoxGeometry &box) noexcept
    {
      return box.Left();
    }
    KRYS_NODISCARD static LayoutPoint BorderBoxTopLeft(const BoxGeometry &box) noexcept
    {
      return box.TopLeft();
    }
    KRYS_NODISCARD static Rect BorderBoxRect(const BoxGeometry &box) noexcept
    {
      return {box.Top(), box.Left(), box.BorderBoxWidth(), box.BorderBoxHeight()};
    }
    KRYS_NODISCARD static Rect MarginBoxRect(const BoxGeometry &box) noexcept
    {
      return {box.Top() - box.MarginBefore(), box.Left() - box.MarginStart(), box.MarginBoxWidth(),
              box.MarginBoxHeight()};
    }

    KRYS_NODISCARD HorizontalEdges HorizontalMargin() const noexcept
    {
      krys_debug_assert(_hasValidHorizontalMargin);
      return _margin.Horizontal;
    }
    KRYS_NODISCARD VerticalEdges VerticalMargin() const noexcept
    {
      krys_debug_assert(_hasValidVerticalMargin);
      return _margin.Vertical;
    }
    KRYS_NODISCARD LayoutUnit MarginBefore() const noexcept
    {
      krys_debug_assert(_hasValidVerticalMargin);
      return _margin.Vertical.Before;
    }
    KRYS_NODISCARD LayoutUnit MarginStart() const noexcept
    {
      krys_debug_assert(_hasValidHorizontalMargin);
      return _margin.Horizontal.Start;
    }
    KRYS_NODISCARD LayoutUnit MarginAfter() const noexcept
    {
      krys_debug_assert(_hasValidVerticalMargin);
      return _margin.Vertical.After;
    }
    KRYS_NODISCARD LayoutUnit MarginEnd() const noexcept
    {
      krys_debug_assert(_hasValidHorizontalMargin);
      return _margin.Horizontal.End;
    }

    KRYS_NODISCARD LayoutUnit BorderBefore() const noexcept
    {
      krys_debug_assert(_hasValidBorder);
      return _border.Vertical.Before;
    }
    KRYS_NODISCARD LayoutUnit BorderAfter() const noexcept
    {
      krys_debug_assert(_hasValidBorder);
      return _border.Vertical.After;
    }
    KRYS_NODISCARD LayoutUnit BorderStart() const noexcept
    {
      krys_debug_assert(_hasValidBorder);
      return _border.Horizontal.Start;
    }
    KRYS_NODISCARD LayoutUnit BorderEnd() const noexcept
    {
      krys_debug_assert(_hasValidBorder);
      return _border.Horizontal.End;
    }
    KRYS_NODISCARD LayoutUnit VerticalBorder() const noexcept
    {
      return BorderBefore() + BorderAfter();
    }
    KRYS_NODISCARD LayoutUnit HorizontalBorder() const noexcept
    {
      return BorderStart() + BorderEnd();
    }

    KRYS_NODISCARD LayoutUnit PaddingBefore() const noexcept
    {
      krys_debug_assert(_hasValidPadding);
      return _padding.Vertical.Before;
    }
    KRYS_NODISCARD LayoutUnit PaddingAfter() const noexcept
    {
      krys_debug_assert(_hasValidPadding);
      return _padding.Vertical.After;
    }
    KRYS_NODISCARD LayoutUnit PaddingStart() const noexcept
    {
      krys_debug_assert(_hasValidPadding);
      return _padding.Horizontal.Start;
    }
    KRYS_NODISCARD LayoutUnit PaddingEnd() const noexcept
    {
      krys_debug_assert(_hasValidPadding);
      return _padding.Horizontal.End;
    }
    KRYS_NODISCARD LayoutUnit VerticalPadding() const noexcept
    {
      return PaddingBefore() + PaddingAfter();
    }
    KRYS_NODISCARD LayoutUnit HorizontalPadding() const noexcept
    {
      return PaddingStart() + PaddingEnd();
    }

    KRYS_NODISCARD LayoutUnit BorderAndPaddingStart() const noexcept
    {
      return BorderStart() + PaddingStart();
    }
    KRYS_NODISCARD LayoutUnit BorderAndPaddingEnd() const noexcept
    {
      return BorderEnd() + PaddingEnd();
    }
    KRYS_NODISCARD LayoutUnit BorderAndPaddingBefore() const noexcept
    {
      return BorderBefore() + PaddingBefore();
    }
    KRYS_NODISCARD LayoutUnit BorderAndPaddingAfter() const noexcept
    {
      return BorderAfter() + PaddingAfter();
    }
    KRYS_NODISCARD LayoutUnit HorizontalBorderAndPadding() const noexcept
    {
      return HorizontalBorder() + HorizontalPadding();
    }
    KRYS_NODISCARD LayoutUnit VerticalBorderAndPadding() const noexcept
    {
      return VerticalBorder() + VerticalPadding();
    }

    KRYS_NODISCARD LayoutUnit ContentBoxTop() const noexcept
    {
      return PaddingBoxTop() + PaddingBefore();
    }
    KRYS_NODISCARD LayoutUnit ContentBoxLeft() const noexcept
    {
      return PaddingBoxLeft() + PaddingStart();
    }
    KRYS_NODISCARD LayoutUnit ContentBoxBottom() const noexcept
    {
      return ContentBoxTop() + ContentBoxHeight();
    }
    KRYS_NODISCARD LayoutUnit ContentBoxRight() const noexcept
    {
      return ContentBoxLeft() + ContentBoxWidth();
    }
    KRYS_NODISCARD LayoutUnit ContentBoxHeight() const noexcept
    {
      krys_debug_assert(_hasValidContentBoxHeight);
      return _contentBoxHeight;
    }
    KRYS_NODISCARD LayoutUnit ContentBoxWidth() const noexcept
    {
      krys_debug_assert(_hasValidContentBoxWidth);
      return _contentBoxWidth;
    }
    KRYS_NODISCARD LayoutSize ContentBoxSize() const noexcept
    {
      return {ContentBoxWidth(), ContentBoxHeight()};
    }

    KRYS_NODISCARD LayoutUnit PaddingBoxTop() const noexcept
    {
      return BorderBefore();
    }
    KRYS_NODISCARD LayoutUnit PaddingBoxLeft() const noexcept
    {
      return BorderStart();
    }
    KRYS_NODISCARD LayoutUnit PaddingBoxBottom() const noexcept
    {
      return PaddingBoxTop() + PaddingBoxHeight();
    }
    KRYS_NODISCARD LayoutUnit PaddingBoxRight() const noexcept
    {
      return PaddingBoxLeft() + PaddingBoxWidth();
    }
    KRYS_NODISCARD LayoutUnit PaddingBoxHeight() const noexcept
    {
      return PaddingBefore() + ContentBoxHeight() + PaddingAfter();
    }
    KRYS_NODISCARD LayoutUnit PaddingBoxWidth() const noexcept
    {
      return PaddingStart() + ContentBoxWidth() + PaddingEnd();
    }

    KRYS_NODISCARD LayoutUnit BorderBoxHeight() const noexcept
    {
      return BorderBefore() + PaddingBoxHeight() + VerticalSpaceForScrollbar() + BorderAfter();
    }
    KRYS_NODISCARD LayoutUnit BorderBoxWidth() const noexcept
    {
      return BorderStart() + PaddingBoxWidth() + HorizontalSpaceForScrollbar() + BorderEnd();
    }
    KRYS_NODISCARD LayoutUnit MarginBoxHeight() const noexcept
    {
      return MarginBefore() + BorderBoxHeight() + MarginAfter();
    }
    KRYS_NODISCARD LayoutUnit MarginBoxWidth() const noexcept
    {
      return MarginStart() + BorderBoxWidth() + MarginEnd();
    }

    KRYS_NODISCARD LayoutUnit MarginBorderAndPaddingBefore() const noexcept
    {
      return MarginBefore() + BorderAndPaddingBefore();
    }
    KRYS_NODISCARD LayoutUnit MarginBorderAndPaddingAfter() const noexcept
    {
      return MarginAfter() + BorderAndPaddingAfter();
    }
    KRYS_NODISCARD LayoutUnit VerticalMarginBorderAndPadding() const noexcept
    {
      return MarginBorderAndPaddingBefore() + MarginBorderAndPaddingAfter();
    }
    KRYS_NODISCARD LayoutUnit MarginBorderAndPaddingStart() const noexcept
    {
      return MarginStart() + BorderAndPaddingStart();
    }
    KRYS_NODISCARD LayoutUnit MarginBorderAndPaddingEnd() const noexcept
    {
      return MarginEnd() + BorderAndPaddingEnd();
    }
    KRYS_NODISCARD LayoutUnit HorizontalMarginBorderAndPadding() const noexcept
    {
      return MarginBorderAndPaddingStart() + MarginBorderAndPaddingEnd();
    }

    KRYS_NODISCARD LayoutUnit VerticalSpaceForScrollbar() const noexcept
    {
      return _verticalSpaceForScrollbar;
    }
    KRYS_NODISCARD LayoutUnit HorizontalSpaceForScrollbar() const noexcept
    {
      return _horizontalSpaceForScrollbar;
    }

    KRYS_NODISCARD bool HasMarginBorderOrPadding() const noexcept
    {
      return HorizontalMarginBorderAndPadding() || VerticalMarginBorderAndPadding();
    }

    KRYS_NODISCARD Rect MarginBox() const noexcept;
    KRYS_NODISCARD Rect BorderBox() const noexcept;
    KRYS_NODISCARD Rect PaddingBox() const noexcept;
    KRYS_NODISCARD Rect ContentBox() const noexcept;

#if KRYS_DEBUG
    void SetHasPrecomputedMarginBefore() noexcept
    {
      _hasPrecomputedMarginBefore = true;
    }
#endif

    void SetTopLeft(const LayoutPoint &topLeft) noexcept
    {
#if KRYS_DEBUG
      _hasValidTop = true;
      _hasValidLeft = true;
#endif
      _topLeft = topLeft;
    }
    void SetTop(LayoutUnit top) noexcept
    {
#if KRYS_DEBUG
      _hasValidTop = true;
#endif
      _topLeft.SetY(top);
    }
    void SetLeft(LayoutUnit left) noexcept
    {
#if KRYS_DEBUG
      _hasValidLeft = true;
#endif
      _topLeft.SetX(left);
    }
    void MoveHorizontally(LayoutUnit offset) noexcept
    {
      _topLeft.Move(offset, 0_lu);
    }
    void MoveVertically(LayoutUnit offset) noexcept
    {
      _topLeft.Move(0_lu, offset);
    }
    void Move(const LayoutSize &size) noexcept
    {
      _topLeft.Move(size);
    }
    void MoveBy(LayoutPoint offset) noexcept
    {
      _topLeft.MoveBy(offset);
    }

    void SetContentBoxHeight(LayoutUnit height) noexcept
    {
#if KRYS_DEBUG
      _hasValidContentBoxHeight = true;
#endif
      _contentBoxHeight = height;
    }
    void SetContentBoxWidth(LayoutUnit width) noexcept
    {
#if KRYS_DEBUG
      _hasValidContentBoxWidth = true;
#endif
      _contentBoxWidth = width;
    }
    void SetContentBoxSize(const LayoutSize &size) noexcept
    {
      SetContentBoxWidth(size.Width());
      SetContentBoxHeight(size.Height());
    }

    void SetHorizontalMargin(HorizontalEdges margin) noexcept
    {
#if KRYS_DEBUG
      _hasValidHorizontalMargin = true;
#endif
      _margin.Horizontal = margin;
    }
    void SetMarginStart(LayoutUnit marginStart) noexcept
    {
#if KRYS_DEBUG
      _hasValidHorizontalMargin = true;
#endif
      _margin.Horizontal = {marginStart, _margin.Horizontal.End};
    }
    void SetMarginEnd(LayoutUnit marginEnd) noexcept
    {
#if KRYS_DEBUG
      _hasValidHorizontalMargin = true;
#endif
      _margin.Horizontal = {_margin.Horizontal.Start, marginEnd};
    }
    void SetVerticalMargin(VerticalEdges margin) noexcept
    {
#if KRYS_DEBUG
      _hasValidVerticalMargin = true;
      _hasPrecomputedMarginBefore = false;
#endif
      _margin.Vertical = margin;
    }

    void SetBorder(Edges border) noexcept
    {
#if KRYS_DEBUG
      _hasValidBorder = true;
#endif
      _border = border;
    }
    void SetHorizontalBorder(HorizontalEdges horizontalBorder) noexcept
    {
#if KRYS_DEBUG
      _hasValidBorder = true;
#endif
      _border.Horizontal = horizontalBorder;
    }
    void SetVerticalBorder(VerticalEdges verticalBorder) noexcept
    {
#if KRYS_DEBUG
      _hasValidBorder = true;
#endif
      _border.Vertical = verticalBorder;
    }

    void SetHorizontalPadding(HorizontalEdges horizontalPadding) noexcept
    {
#if KRYS_DEBUG
      _hasValidPadding = true;
#endif
      _padding.Horizontal = horizontalPadding;
    }
    void SetVerticalPadding(VerticalEdges verticalPadding) noexcept
    {
#if KRYS_DEBUG
      _hasValidPadding = true;
#endif
      _padding.Vertical = verticalPadding;
    }
    void SetPadding(Edges padding) noexcept
    {
#if KRYS_DEBUG
      _hasValidPadding = true;
#endif
      _padding = padding;
    }

    void SetVerticalSpaceForScrollbar(LayoutUnit scrollbarHeight) noexcept
    {
      _verticalSpaceForScrollbar = scrollbarHeight;
    }
    void SetHorizontalSpaceForScrollbar(LayoutUnit scrollbarWidth) noexcept
    {
      _horizontalSpaceForScrollbar = scrollbarWidth;
    }
    void SetSpaceForScrollbar(LayoutSize scrollbarSize) noexcept
    {
      SetVerticalSpaceForScrollbar(scrollbarSize.Height());
      SetHorizontalSpaceForScrollbar(scrollbarSize.Width());
    }

    void Reset() noexcept;

  private:
    KRYS_NODISCARD LayoutUnit Top() const noexcept
    {
      krys_debug_assert(_hasValidTop && (_hasPrecomputedMarginBefore || _hasValidVerticalMargin));
      return _topLeft.Y();
    }
    KRYS_NODISCARD LayoutUnit Left() const noexcept
    {
      krys_debug_assert(_hasValidLeft && _hasValidHorizontalMargin);
      return _topLeft.X();
    }
    KRYS_NODISCARD LayoutPoint TopLeft() const noexcept
    {
      krys_debug_assert(_hasValidTop && (_hasPrecomputedMarginBefore || _hasValidVerticalMargin));
      krys_debug_assert(_hasValidLeft && _hasValidHorizontalMargin);
      return _topLeft;
    }
  };
}
