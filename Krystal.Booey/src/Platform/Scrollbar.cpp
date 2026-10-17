#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include "Krystal.Booey/Platform/ScrollableArea.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"

namespace krys::boo
{
  Ref<Scrollbar> Scrollbar::CreateNativeScrollbar(ScrollableArea &scrollableArea,
                                                  ScrollbarOrientation orientation,
                                                  ScrollbarWidth width) noexcept
  {
    return AdoptRef(*new Scrollbar(scrollableArea, orientation, width));
  }

  int32 Scrollbar::PixelsPerLineStep(int32) noexcept
  {
    return PixelsPerLineStep();
  }

  int32 Scrollbar::MaxOverlapBetweenPages() noexcept
  {
    static int32 maxOverlapBetweenPages = ScrollbarTheme::Theme().MaxOverlapBetweenPages();
    return maxOverlapBetweenPages;
  }

  Scrollbar::Scrollbar(ScrollableArea &scrollableArea, ScrollbarOrientation orientation,
                       ScrollbarWidth widthStyle) noexcept
      : _scrollableArea(CreateWeakPtr(&scrollableArea)), _orientation(orientation), _widthStyle(widthStyle),
        _theme(ScrollbarTheme::Theme())
  {
    // FIXME: This is ugly and would not be necessary if we fix cross-platform code to actually query for
    // scrollbar thickness and use it when sizing scrollbars (rather than leaving one dimension of the
    // scrollbar alone when sizing).
    int32 thickness = Theme().ScrollbarThickness(widthStyle);
    SetFrameRect(IntRect(0, 0, thickness, thickness));

    _currentPos = static_cast<float>(OffsetForOrientation(scrollableArea.GetScrollOffset(), _orientation));
  }

  Scrollbar::~Scrollbar() noexcept = default;

  ScrollableArea &Scrollbar::GetScrollableArea() const noexcept
  {
    return *_scrollableArea;
  }

  int32 Scrollbar::OccupiedWidth() const noexcept
  {
    return IsOverlayScrollbar() ? 0 : Width();
  }

  int32 Scrollbar::OccupiedHeight() const noexcept
  {
    return IsOverlayScrollbar() ? 0 : Height();
  }

  void Scrollbar::OffsetDidChange() noexcept
  {
    float position =
      static_cast<float>(OffsetForOrientation(GetScrollableArea().GetScrollOffset(), _orientation));
    if (position == _currentPos)
    {
      return;
    }

    _currentPos = position;
    // FIXME: Move the thumb, and the pressed position when dragging it, once there's painting and input.
  }

  void Scrollbar::SetProportion(int32 visibleSize, int32 totalSize) noexcept
  {
    if (visibleSize == _visibleSize && totalSize == _totalSize)
    {
      return;
    }

    _visibleSize = visibleSize;
    _totalSize = totalSize;

    // FIXME: Resize the thumb, once there's painting.
  }

  void Scrollbar::SetSteps(int32 lineStep, int32 pageStep, int32 pixelsPerStep) noexcept
  {
    _lineStep = lineStep;
    _pageStep = pageStep;
    _pixelStep = 1.0f / static_cast<float>(pixelsPerStep);
  }

  void Scrollbar::SetFrameRect(const IntRect &rect) noexcept
  {
    _frameRect = rect;
    if (_scrollableArea)
    {
      _scrollableArea->ScrollbarFrameRectChanged(*this);
    }
  }

  void Scrollbar::SetEnabled(bool enabled) noexcept
  {
    if (_enabled == enabled)
    {
      return;
    }
    _enabled = enabled;
    // FIXME: Update the theme's and the scrollbars controller's enabled state and repaint, once there's
    // painting.
  }

  bool Scrollbar::IsOverlayScrollbar() const noexcept
  {
    return Theme().UsesOverlayScrollbars();
  }

  bool Scrollbar::ShouldParticipateInHitTesting() const noexcept
  {
    // Non-overlay scrollbars should always participate in hit testing.
    // FIXME: Ask the scrollbars controller about overlay scrollbars, once there is one.
    return true;
  }
}
