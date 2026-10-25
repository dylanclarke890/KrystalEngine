#pragma once

#include "Krystal.Booey/Platform/ScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeCheckedPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class RenderStyle;

  /// @brief The scrollable area of a layer whose box can scroll its overflow: its scroll position and
  /// extents, and its scrollbars.
  // FIXME: Marquees, touch, wheel and keyboard scrolling, animated scrolls, scroll snapping and anchoring,
  // composited scrolling, custom scroll corners and resizers, painting and hit testing of the overflow
  // controls, scroll events and scrolling into view come with input, animations, compositing and painting.
  class RenderLayerScrollableArea final : public ScrollableArea
  {
    KRYS_NON_COPY_MOVABLE(RenderLayerScrollableArea);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderLayerScrollableArea);

  private:
    bool _scrollDimensionsDirty {true};
    bool _inOverflowRelayout {false};
    bool _hasCompositedScrollableOverflow {false};
    bool _requiresScrollPositionReconciliation {false};

    // The width/height of our scrolled area.
    int32 _scrollWidth {0};
    int32 _scrollHeight {0};

    RenderLayer &_layer;
    ScrollPosition _scrollPosition;
    Maybe<ScrollPosition> _postLayoutScrollPosition;

    // For layers with overflow, we have a pair of scrollbars.
    RefPtr<Scrollbar> _hBar;
    RefPtr<Scrollbar> _vBar;

  public:
    explicit RenderLayerScrollableArea(RenderLayer &layer) noexcept;
    ~RenderLayerScrollableArea() noexcept override;

    KRYS_NODISCARD RenderLayer &Layer() noexcept
    {
      return _layer;
    }

    void Clear() noexcept;

    void RestoreScrollPosition() noexcept;

    void SetPostLayoutScrollPosition(Maybe<ScrollPosition> position) noexcept;
    void ApplyPostLayoutScrollPositionIfNeeded() noexcept;
    KRYS_NODISCARD bool HasPostLayoutScrollPosition() const noexcept
    {
      return static_cast<bool>(_postLayoutScrollPosition);
    }

    KRYS_NODISCARD int32 ScrollWidth() const noexcept;
    KRYS_NODISCARD int32 ScrollHeight() const noexcept;

    // Attempt to scroll the given ScrollOffset, returning the real target offset after it has
    // been adjusted by scroll snapping.
    ScrollOffset ScrollToOffset(const ScrollOffset &offset,
                                const ScrollPositionChangeOptions &options =
                                  ScrollPositionChangeOptions::CreateProgrammatic()) noexcept;

    /// @brief Scrolls by @p delta, passing what this box can't scroll on to the enclosing scrollable layer
    /// (or the view, if the layer has no overflow clip), setting @p scrolledArea to the last area scrolled.
    void ScrollByRecursively(const IntSize &delta, ScrollableArea **scrolledArea = nullptr) noexcept;

    /// @brief Scrolls so the absolute rect is aligned as asked.
    /// @return The rect in absolute coordinates after scrolling.
    LayoutRect ScrollRectToVisible(const LayoutRect &absoluteRect,
                                   const ScrollRectToVisibleOptions &options) noexcept;
    Maybe<LayoutRect> UpdateScrollPositionForScrollIntoView(const ScrollPositionChangeOptions &options,
                                                            const LayoutRect &revealRect,
                                                            const LayoutRect &localExposeRect) noexcept;

    void ScrollToXPosition(int32 x, const ScrollPositionChangeOptions &options) noexcept;
    void ScrollToYPosition(int32 y, const ScrollPositionChangeOptions &options) noexcept;
    void SetScrollPosition(const ScrollPosition &position,
                           const ScrollPositionChangeOptions &options) noexcept;

    KRYS_NODISCARD bool ScrollsOverflow() const noexcept;
    KRYS_NODISCARD bool HasScrollableHorizontalOverflow() const noexcept;
    KRYS_NODISCARD bool HasScrollableVerticalOverflow() const noexcept;
    KRYS_NODISCARD bool HasScrollbars() const noexcept
    {
      return HorizontalScrollbar() || VerticalScrollbar();
    }
    KRYS_NODISCARD bool HasHorizontalScrollbar() const noexcept
    {
      return HorizontalScrollbar();
    }
    KRYS_NODISCARD bool HasVerticalScrollbar() const noexcept
    {
      return VerticalScrollbar();
    }
    void SetHasHorizontalScrollbar(bool hasScrollbar) noexcept;
    void SetHasVerticalScrollbar(bool hasScrollbar) noexcept;

    KRYS_NODISCARD ScrollbarWidth ScrollbarWidthStyle() const noexcept final;

    KRYS_NODISCARD bool RequiresScrollPositionReconciliation() const noexcept
    {
      return _requiresScrollPositionReconciliation;
    }
    void SetRequiresScrollPositionReconciliation(bool requiresReconciliation = true) noexcept
    {
      _requiresScrollPositionReconciliation = requiresReconciliation;
    }

    // Returns true when the layer could do touch scrolling, but doesn't look at whether there is actually
    // scrollable overflow.
    KRYS_NODISCARD bool CanUseCompositedScrolling() const noexcept;
    KRYS_NODISCARD bool UsesCompositedScrolling() const noexcept;
    // Returns true when there is actually scrollable overflow (requires layout to be up-to-date).
    KRYS_NODISCARD bool HasCompositedScrollableOverflow() const noexcept
    {
      return _hasCompositedScrollableOverflow;
    }

    KRYS_NODISCARD int32 VerticalScrollbarWidth(
      OverlayScrollbarSizeRelevancy relevancy = OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize,
      bool isHorizontalWritingMode = true) const noexcept;
    KRYS_NODISCARD int32 HorizontalScrollbarHeight(
      OverlayScrollbarSizeRelevancy relevancy = OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize,
      bool isHorizontalWritingMode = true) const noexcept;

    KRYS_NODISCARD bool HasOverflowControls() const noexcept;

    void UpdateScrollInfoAfterLayout() noexcept;
    void UpdateScrollbarSteps() noexcept;

    void AvailableContentSizeChanged(AvailableSizeChangeReason reason) noexcept final;

    KRYS_NODISCARD bool CanShowNonOverlayScrollbars() const noexcept final;

    KRYS_NODISCARD ScrollPosition GetScrollPosition() const noexcept final
    {
      return _scrollPosition;
    }

    KRYS_NODISCARD Scrollbar *HorizontalScrollbar() const noexcept final
    {
      return _hBar.get();
    }
    KRYS_NODISCARD Scrollbar *VerticalScrollbar() const noexcept final
    {
      return _vBar.get();
    }
    KRYS_NODISCARD ScrollableArea *EnclosingScrollableArea() const noexcept final;

    KRYS_NODISCARD bool ShouldPlaceVerticalScrollbarOnLeft() const noexcept final;

    KRYS_NODISCARD bool IsRenderLayer() const noexcept final
    {
      return true;
    }
    KRYS_NODISCARD bool IsScrollCornerVisible() const noexcept final;
    KRYS_NODISCARD IntRect ScrollCornerRect() const noexcept final;
    void SetScrollOffset(const ScrollOffset &offset) noexcept final;

    KRYS_NODISCARD IntSize VisibleSize() const noexcept final;
    KRYS_NODISCARD IntSize ContentsSize() const noexcept final;
    KRYS_NODISCARD IntSize ReachableTotalContentsSize() const noexcept final;

    void UpdateScrollbarsAfterStyleChange(const RenderStyle *oldStyle) noexcept;
    void UpdateScrollbarsAfterLayout() noexcept;

    bool PositionOverflowControls(const IntSize &offsetFromRoot) noexcept;

    void UpdateAllScrollbarRelatedStyle() noexcept;

    KRYS_NODISCARD RenderLayer::OverflowControlRects OverflowControlsRects() const noexcept;

    KRYS_NODISCARD bool OverflowControlsIntersectRect(const IntRect &localRect) const noexcept;

    void ComputeHasCompositedScrollableOverflow(LayoutUpToDate layoutUpToDate) noexcept;

    // NOTE: This should only be called by the overridden setScrollOffset from ScrollableArea.
    void ScrollTo(const ScrollPosition &position) noexcept;

    /// @brief Fires `scrollend` at the element after a scroll it fired `scroll` for.
    void ScrollDidEnd() noexcept override;
    void UpdateAnchorPositionedAfterScroll() noexcept final;
    /// @brief Updates the compositing layers a scroll moved, from the enclosing compositing layer of the
    /// stacking context.
    void UpdateCompositingLayersAfterScroll() noexcept;

    /// @brief The backing's layers the scrollbars and scroll corner paint into, if they have their own.
    KRYS_NODISCARD GraphicsLayer *LayerForHorizontalScrollbar() const noexcept;
    KRYS_NODISCARD GraphicsLayer *LayerForVerticalScrollbar() const noexcept;
    KRYS_NODISCARD GraphicsLayer *LayerForScrollCorner() const noexcept;
    void InvalidateScrollbar(Scrollbar &scrollbar, const IntRect &rect) noexcept override;
    KRYS_NODISCARD bool IsScrollableOrRubberbandable() const noexcept override;

    /// @brief Paints the scrollbars and the scroll corner, at @p paintOffset.
    // FIXME: Overlay scrollbars' second pass and the resizer (paintResizer), once there are overlay
    // scrollbars and resizers.
    void PaintOverflowControls(GraphicsContext &context, EnumSet<PaintBehavior> paintBehavior,
                               const IntPoint &paintOffset, const IntRect &damageRect) noexcept;
    void PaintScrollCorner(GraphicsContext &context, const IntPoint &paintOffset,
                           const IntRect &damageRect) noexcept;
    /// @brief Paints the grip of a box the user can resize, in its bottom corner.
    void PaintResizer(GraphicsContext &context, const LayoutPoint &paintOffset,
                      const LayoutRect &damageRect) noexcept;
    /// @brief Whether @p hitTestLocation is over the resizer in one of @p layerFragments; sets
    /// @p pointInFragment to where.
    KRYS_NODISCARD bool HitTestResizerInFragments(const LayerFragments &layerFragments,
                                                  const HitTestLocation &hitTestLocation,
                                                  LayoutPoint &pointInFragment) const noexcept;
    /// @brief Whether @p localPoint (in the box's coordinates) is over a scrollbar, which @p result then
    /// names.
    bool HitTestOverflowControls(HitTestResult &result, const IntPoint &localPoint) noexcept;

    KRYS_NODISCARD IntPoint ConvertFromContainingViewToScrollbar(
      const Scrollbar &scrollbar, const IntPoint &parentPoint) const noexcept override;
    KRYS_NODISCARD OverscrollBehavior HorizontalOverscrollBehavior() const noexcept override;
    KRYS_NODISCARD OverscrollBehavior VerticalOverscrollBehavior() const noexcept override;

    KRYS_NODISCARD IntSize ScrollbarOffset(const Scrollbar &scrollbar) const noexcept;

  private:
    KRYS_NODISCARD bool HasHorizontalOverflow() const noexcept;
    KRYS_NODISCARD bool HasVerticalOverflow() const noexcept;

    KRYS_NODISCARD bool ShowsOverflowControls() const noexcept;

    KRYS_NODISCARD ScrollOffset ClampScrollOffset(const ScrollOffset &offset) const noexcept;

    void ComputeScrollDimensions() noexcept;
    void ComputeScrollOrigin() noexcept;

    KRYS_NODISCARD Ref<Scrollbar> CreateScrollbar(ScrollbarOrientation orientation) noexcept;
    void DestroyScrollbar(ScrollbarOrientation orientation) noexcept;

    void UpdateScrollbarPresenceAndState(Maybe<bool> hasHorizontalOverflow = null,
                                         Maybe<bool> hasVerticalOverflow = null) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::RenderLayerScrollableArea)
  KRYS_NODISCARD static bool IsType(const krys::boo::ScrollableArea &area) noexcept
  {
    return area.IsRenderLayer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
