#pragma once

#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  class GraphicsContext;
  class ScrollableArea;
  class ScrollbarTheme;

  /// @brief A scrollbar of a scrollable area: its frame, its range (the visible and total sizes), its
  /// position, the parts the mouse is over and pressing, and painting through its theme.
  // FIXME: The
  // widget tree (Widget: the frame rect is in the scrollable area's coordinates) and custom scrollbars.
  class Scrollbar : public RefCounted<Scrollbar>
  {
    KRYS_NON_COPY_MOVABLE(Scrollbar);

  private:
    WeakPtr<ScrollableArea> _scrollableArea;
    ScrollbarOrientation _orientation;
    ScrollbarWidth _widthStyle;
    ScrollbarTheme &_theme;
    IntRect _frameRect;

    int32 _visibleSize {0};
    int32 _totalSize {0};
    float _currentPos {0};
    int32 _lineStep {0};
    int32 _pageStep {0};
    float _pixelStep {1};
    float _dragOrigin {0};
    ScrollbarPart _hoveredPart {NoPart};
    ScrollbarPart _pressedPart {NoPart};
    int32 _pressedPos {0};
    bool _draggingDocument {false};
    int32 _documentDragPos {0};
    bool _enabled {true};
    float _opacity {1};
    Timer _scrollTimer;

  public:
    KRYS_NODISCARD static Ref<Scrollbar> CreateNativeScrollbar(ScrollableArea &scrollableArea,
                                                               ScrollbarOrientation orientation,
                                                               ScrollbarWidth width) noexcept;

    Scrollbar(ScrollableArea &scrollableArea, ScrollbarOrientation orientation,
              ScrollbarWidth widthStyle) noexcept;
    ~Scrollbar() noexcept;

    // Called by the ScrollableArea when the scroll offset changes.
    void OffsetDidChange() noexcept;

    KRYS_NODISCARD const IntRect &FrameRect() const noexcept
    {
      return _frameRect;
    }
    void SetFrameRect(const IntRect &rect) noexcept;

    KRYS_NODISCARD int32 Width() const noexcept
    {
      return _frameRect.Width();
    }
    KRYS_NODISCARD int32 Height() const noexcept
    {
      return _frameRect.Height();
    }

    KRYS_NODISCARD int32 X() const noexcept
    {
      return _frameRect.X();
    }

    KRYS_NODISCARD int32 Y() const noexcept
    {
      return _frameRect.Y();
    }

    KRYS_NODISCARD IntPoint Location() const noexcept
    {
      return _frameRect.Location();
    }

    KRYS_NODISCARD static constexpr int32 PixelsPerLineStep() noexcept
    {
      return 40;
    }
    KRYS_NODISCARD static int32 PixelsPerLineStep(int32 viewWidthOrHeight) noexcept;
    KRYS_NODISCARD static float MinFractionToStepWhenPaging() noexcept
    {
      return 0.8f;
    }
    KRYS_NODISCARD static int32 MaxOverlapBetweenPages() noexcept;
    KRYS_NODISCARD static int32 PageStep(int32 viewWidthOrHeight, int32 contentWidthOrHeight) noexcept
    {
      return std::max(
        std::max<int32>(
          std::lround(static_cast<float>(viewWidthOrHeight) * Scrollbar::MinFractionToStepWhenPaging()),
          std::lround(static_cast<float>(contentWidthOrHeight - Scrollbar::MaxOverlapBetweenPages()))),
        1);
    }
    KRYS_NODISCARD static int32 PageStep(int32 viewWidthOrHeight) noexcept
    {
      return PageStep(viewWidthOrHeight, viewWidthOrHeight);
    }

    KRYS_NODISCARD static float PageStepDelta(int32 widthOrHeight) noexcept
    {
      return std::max(std::max(static_cast<float>(widthOrHeight) * Scrollbar::MinFractionToStepWhenPaging(),
                               static_cast<float>(widthOrHeight - Scrollbar::MaxOverlapBetweenPages())),
                      1.0f);
    }

    KRYS_NODISCARD ScrollableArea &GetScrollableArea() const noexcept;

    KRYS_NODISCARD bool IsCustomScrollbar() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD ScrollbarOrientation Orientation() const noexcept
    {
      return _orientation;
    }

    KRYS_NODISCARD int32 Value() const noexcept
    {
      return static_cast<int32>(std::lround(_currentPos));
    }
    KRYS_NODISCARD float CurrentPos() const noexcept
    {
      return _currentPos;
    }

    KRYS_NODISCARD int32 VisibleSize() const noexcept
    {
      return _visibleSize;
    }
    KRYS_NODISCARD int32 TotalSize() const noexcept
    {
      return _totalSize;
    }
    KRYS_NODISCARD int32 Maximum() const noexcept
    {
      return _totalSize - _visibleSize;
    }
    KRYS_NODISCARD ScrollbarWidth WidthStyle() const noexcept
    {
      return _widthStyle;
    }

    KRYS_NODISCARD int32 OccupiedWidth() const noexcept;
    KRYS_NODISCARD int32 OccupiedHeight() const noexcept;

    KRYS_NODISCARD int32 LineStep() const noexcept
    {
      return _lineStep;
    }
    KRYS_NODISCARD int32 PageStep() const noexcept
    {
      return _pageStep;
    }
    KRYS_NODISCARD float PixelStep() const noexcept
    {
      return _pixelStep;
    }

    void SetSteps(int32 lineStep, int32 pageStep, int32 pixelsPerStep = 1) noexcept;
    void SetProportion(int32 visibleSize, int32 totalSize) noexcept;

    KRYS_NODISCARD bool Enabled() const noexcept
    {
      return _enabled;
    }
    void SetEnabled(bool enabled) noexcept;

    KRYS_NODISCARD bool IsOverlayScrollbar() const noexcept;
    KRYS_NODISCARD bool ShouldParticipateInHitTesting() const noexcept;

    KRYS_NODISCARD ScrollbarTheme &Theme() const noexcept
    {
      return _theme;
    }

    void StyleChanged() noexcept
    {
    }

    KRYS_NODISCARD bool IsHiddenByStyle() const noexcept
    {
      return _widthStyle == ScrollbarWidth::None;
    }

    KRYS_NODISCARD float Opacity() const noexcept
    {
      return _opacity;
    }

    void SetOpacity(float opacity) noexcept
    {
      _opacity = opacity;
    }

    KRYS_NODISCARD int32 PressedPos() const noexcept
    {
      return _pressedPos;
    }

    void SetPressedPos(int32 p) noexcept
    {
      _pressedPos = p;
    }

    KRYS_NODISCARD ScrollbarPart PressedPart() const noexcept
    {
      return _pressedPart;
    }

    KRYS_NODISCARD ScrollbarPart HoveredPart() const noexcept
    {
      return _hoveredPart;
    }

    void SetHoveredPart(ScrollbarPart part) noexcept;
    void SetPressedPart(ScrollbarPart part) noexcept;

    /// @brief Paints the scrollbar where it intersects @p damageRect, through its theme.
    void Paint(GraphicsContext &context, const IntRect &damageRect) noexcept;

    // These methods are used for platform scrollbars to give :hover feedback. They will not get called when
    // the mouse went down in a scrollbar, since it is assumed the scrollbar will start grabbing all events in
    // that case anyway.
    bool MouseMoved(const PlatformMouseEvent &event) noexcept;
    void MouseEntered() noexcept;
    bool MouseExited() noexcept;

    // Used by some platform scrollbars to know when they've been released from capture.
    bool MouseUp(const PlatformMouseEvent &event) noexcept;
    bool MouseDown(const PlatformMouseEvent &event) noexcept;

    /// @brief Moves the thumb by where the mouse is (@p pos, along the scrollbar) from where it was pressed.
    void MoveThumb(int32 pos, bool draggingDocument = false) noexcept;

    /// @brief Converts a point in the window (the root view's coordinates) to the scrollbar's.
    KRYS_NODISCARD IntPoint ConvertFromContainingWindow(const IntPoint &windowPoint) const noexcept;

    /// @brief Repaints the scrollbar's @p rect.
    void InvalidateRect(const IntRect &rect) noexcept;
    void Invalidate() noexcept
    {
      InvalidateRect(IntRect(IntPoint(), _frameRect.Size()));
    }

    void UpdateScrollbarThickness() noexcept;

  private:
    void UpdateThumb() noexcept;
    void AutoscrollTimerFired() noexcept;
    void AutoscrollPressedPart(MonotonicTime::duration delay) noexcept;
    void StartTimerIfNeeded(MonotonicTime::duration delay) noexcept;
    void StopTimerIfNeeded() noexcept;
    KRYS_NODISCARD ScrollDirection PressedPartScrollDirection() const noexcept;
    KRYS_NODISCARD ScrollGranularity PressedPartScrollGranularity() const noexcept;
    KRYS_NODISCARD bool ThumbUnderMouse() const noexcept;
  };
}
