#pragma once

#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  class ScrollableArea;
  class ScrollbarTheme;

  /// @brief A scrollbar of a scrollable area: its frame, its range (the visible and total sizes) and its
  /// position.
  // FIXME: Painting, mouse handling, autoscrolling, the widget tree (Widget), and custom scrollbars come with
  // painting and input.
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

    bool _enabled {true};

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
  };
}
