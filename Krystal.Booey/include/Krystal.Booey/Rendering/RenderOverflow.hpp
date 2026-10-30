#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include <algorithm>

namespace krys::boo
{
  /// @brief Tracks content that spills out of a box.
  ///
  /// There are two types of overflow: layout overflow (which is expected to be reachable via scrolling
  /// mechanisms) and visual overflow (which is not expected to be reachable via scrolling mechanisms).
  ///
  /// Layout overflow examples include other boxes that spill out of our box, For example, in the inline case
  /// a tall image could spill out of a line box.
  ///
  /// Examples of visual overflow are shadows, text stroke, outline (and eventually border-image).
  ///
  /// This object is allocated only when some of these fields have non-default values in the owning box.
  class RenderOverflow : public CanMakeCheckedPtr<RenderOverflow>
  {
    KRYS_NON_COPY_MOVABLE(RenderOverflow);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderOverflow);

  private:
    LayoutRect _layoutOverflow;
    LayoutRect _visualOverflow;

    LayoutUnit _layoutClientAfterEdge;

  public:
    RenderOverflow(const LayoutRect &layoutRect, const LayoutRect &visualRect) noexcept
        : _layoutOverflow(layoutRect), _visualOverflow(visualRect)
    {
    }

    KRYS_NODISCARD LayoutRect LayoutOverflowRect() const noexcept
    {
      return _layoutOverflow;
    }
    KRYS_NODISCARD LayoutRect VisualOverflowRect() const noexcept
    {
      return _visualOverflow;
    }

    void Move(LayoutUnit dx, LayoutUnit dy) noexcept
    {
      _layoutOverflow.Move(dx, dy);
      _visualOverflow.Move(dx, dy);
    }

    void AddLayoutOverflow(const LayoutRect &rect) noexcept
    {
      LayoutUnit maxX = std::max(rect.MaxX(), _layoutOverflow.MaxX());
      LayoutUnit maxY = std::max(rect.MaxY(), _layoutOverflow.MaxY());
      LayoutUnit minX = std::min(rect.X(), _layoutOverflow.X());
      LayoutUnit minY = std::min(rect.Y(), _layoutOverflow.Y());
      // In case the width/height is larger than LayoutUnit can represent, fix the right/bottom edge and shift
      // the top/left ones
      _layoutOverflow.SetWidth(maxX - minX);
      _layoutOverflow.SetHeight(maxY - minY);
      _layoutOverflow.SetX(maxX - _layoutOverflow.Width());
      _layoutOverflow.SetY(maxY - _layoutOverflow.Height());
    }

    void AddVisualOverflow(const LayoutRect &rect) noexcept
    {
      LayoutUnit maxX = std::max(rect.MaxX(), _visualOverflow.MaxX());
      LayoutUnit maxY = std::max(rect.MaxY(), _visualOverflow.MaxY());
      _visualOverflow.SetX(std::min(rect.X(), _visualOverflow.X()));
      _visualOverflow.SetY(std::min(rect.Y(), _visualOverflow.Y()));
      _visualOverflow.SetWidth(maxX - _visualOverflow.X());
      _visualOverflow.SetHeight(maxY - _visualOverflow.Y());
    }

    void SetLayoutOverflow(const LayoutRect &rect) noexcept
    {
      _layoutOverflow = rect;
    }
    void SetVisualOverflow(const LayoutRect &rect) noexcept
    {
      _visualOverflow = rect;
    }

    KRYS_NODISCARD LayoutUnit LayoutClientAfterEdge() const noexcept
    {
      return _layoutClientAfterEdge;
    }
    void SetLayoutClientAfterEdge(LayoutUnit clientAfterEdge) noexcept
    {
      _layoutClientAfterEdge = clientAfterEdge;
    }
  };
}
