#pragma once

#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/Core/ScrollTypes.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/DoublePoint.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Platform/ScrollableArea.hpp"
#include "Krystal.Booey/Rendering/Pagination.hpp"
#include "Krystal.Booey/Rendering/PaintPhase.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class GraphicsContext;
  class LocalFrame;
  class RenderView;

  /// @brief The view a frame's document is laid out in: its frame rect, the size content is laid out at, the
  /// media type, pagination, the sizes viewport units resolve against, the layout context, and its scroll
  /// position within the document (the scrolling half of a scroll view).
  /// @note FIXME: No obscured insets, visual viewport, auto-sizing, repaint rects or compositing, and views
  /// aren't widgets in a widget tree (there's no `Widget` or `ScrollView`): the visible content is the frame
  /// rect's size, less its scrollbars, at the scroll position.
  class LocalFrameView final : public RefCounted<LocalFrameView>,
                               public CanMakeWeakPtr<LocalFrameView>,
                               public ScrollableArea
  {
    KRYS_NON_COPY_MOVABLE(LocalFrameView);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LocalFrameView);

  public:
    struct OverrideViewportSize
    {
      Maybe<float> Width;
      Maybe<float> Height;

      friend bool operator==(const OverrideViewportSize &, const OverrideViewportSize &) = default;
    };

  private:
    WeakPtr<LocalFrame> _frame;
    IntRect _frameRect;
    IntSize _fixedLayoutSize;
    bool _useFixedLayout {false};
    bool _hasFlippedBlockRenderers {false};
    css::CSSOMStringAtom _mediaType;
    boo::Pagination _pagination;
    Maybe<OverrideViewportSize> _defaultViewportSizeOverride;
    Maybe<OverrideViewportSize> _smallViewportSizeOverride;
    Maybe<OverrideViewportSize> _largeViewportSizeOverride;
    LocalFrameViewLayoutContext _layoutContext;
    EnumSet<PaintBehavior> _paintBehavior;
    boo::Color _baseBackgroundColor {boo::Color::white};
    bool _isPainting {false};
    bool _isTransparent {false};
    bool _contentIsOpaque {false};
    ScrollPosition _scrollPosition;
    IntSize _contentsSize;
    bool _inUpdateScrollbars {false};
    RefPtr<Scrollbar> _horizontalScrollbar;
    RefPtr<Scrollbar> _verticalScrollbar;
    ScrollbarMode _horizontalScrollbarMode {ScrollbarMode::Auto};
    ScrollbarMode _verticalScrollbarMode {ScrollbarMode::Auto};
    bool _horizontalScrollbarLock {false};
    bool _verticalScrollbarLock {false};
    bool _canHaveScrollbars {true};
    uint32 _updateScrollbarsPass {0};
    IntSize _lastUsedSizeForLayout;

    explicit LocalFrameView(LocalFrame &frame) noexcept;

  public:
    ~LocalFrameView() noexcept;

    KRYS_NODISCARD static Ref<LocalFrameView> Create(LocalFrame &frame) noexcept;
    KRYS_NODISCARD static Ref<LocalFrameView> Create(LocalFrame &frame, const IntSize &initialSize) noexcept;

    KRYS_NODISCARD LocalFrame &Frame() const noexcept;

    /// @brief The render view of the frame's document, if it has one.
    KRYS_NODISCARD const RenderView *GetRenderView() const noexcept;

    KRYS_NODISCARD LocalFrameViewLayoutContext &LayoutContext() noexcept
    {
      return _layoutContext;
    }

    KRYS_NODISCARD const LocalFrameViewLayoutContext &LayoutContext() const noexcept
    {
      return _layoutContext;
    }

    /// @brief The view's rect in its parent's coordinates (the host's, for a top-level view).
    KRYS_NODISCARD const IntRect &FrameRect() const noexcept
    {
      return _frameRect;
    }

    void SetFrameRect(const IntRect &newRect) noexcept;

    KRYS_NODISCARD IntSize Size() const noexcept
    {
      return _frameRect.Size();
    }

    KRYS_NODISCARD int32 Width() const noexcept
    {
      return _frameRect.Width();
    }

    KRYS_NODISCARD int32 Height() const noexcept
    {
      return _frameRect.Height();
    }

    void Resize(const IntSize &size) noexcept;

    /// @brief The size content is laid out at: the fixed layout size if the view uses it, or else the visible
    /// content size.
    KRYS_NODISCARD IntSize GetLayoutSize() const noexcept;

    KRYS_NODISCARD int32 LayoutWidth() const noexcept
    {
      return GetLayoutSize().Width();
    }

    KRYS_NODISCARD int32 LayoutHeight() const noexcept
    {
      return GetLayoutSize().Height();
    }

    KRYS_NODISCARD IntSize FixedLayoutSize() const noexcept
    {
      return _fixedLayoutSize;
    }

    void SetFixedLayoutSize(const IntSize &newSize) noexcept;

    KRYS_NODISCARD bool UseFixedLayout() const noexcept
    {
      return _useFixedLayout;
    }

    KRYS_NODISCARD bool HasFlippedBlockRenderers() const noexcept
    {
      return _hasFlippedBlockRenderers;
    }
    void SetHasFlippedBlockRenderers(bool b) noexcept
    {
      _hasFlippedBlockRenderers = b;
    }

    void SetUseFixedLayout(bool enable) noexcept;

    /// @brief The part of the visible content (`VisibleContentRect`, the view's size less its scrollbars at
    /// the scroll position) that isn't covered by the host's interface: all of it.
    KRYS_NODISCARD IntRect UnobscuredContentRect(VisibleContentRectIncludesScrollbars scrollbarInclusion =
                                                   VisibleContentRectIncludesScrollbars::No) const noexcept;

    /// @brief The size of the visible content: the view's, less its scrollbars unless @p scrollbarInclusion
    /// says otherwise.
    KRYS_NODISCARD IntSize SizeForVisibleContent(VisibleContentRectIncludesScrollbars scrollbarInclusion =
                                                   VisibleContentRectIncludesScrollbars::No) const noexcept;

    /// @brief The document's scroll position relative to the view's origin: the scroll position (there are no
    /// headers, insets or left scrollbars).
    KRYS_NODISCARD IntPoint DocumentScrollPositionRelativeToViewOrigin() const noexcept
    {
      return _scrollPosition;
    }

    /// @brief The scale of the frame's content: none, as there's no page scaling.
    KRYS_NODISCARD float FrameScaleFactor() const noexcept
    {
      return 1;
    }

    /// @brief Whether the background is painted beyond the document, while rubber-banding: never.
    /// FIXME: Comes with scrolling the viewport.
    KRYS_NODISCARD bool HasExtendedBackgroundRectForPainting() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD IntRect ExtendedBackgroundRectForPainting() const noexcept
    {
      return {};
    }

#pragma region Scrolling

    KRYS_NODISCARD bool IsScrollView() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD ScrollPosition GetScrollPosition() const noexcept override
    {
      return _scrollPosition;
    }

    KRYS_NODISCARD int32 ScrollX() const noexcept
    {
      return _scrollPosition.X();
    }

    KRYS_NODISCARD int32 ScrollY() const noexcept
    {
      return _scrollPosition.Y();
    }

    /// @brief The largest scroll position: the contents' size less the visible size, and not negative.
    KRYS_NODISCARD ScrollPosition MaximumScrollPosition() const noexcept override;

    /// @brief The size of the visible content: the view's size (there are no scrollbars).
    KRYS_NODISCARD IntSize VisibleSize() const noexcept override
    {
      return SizeForVisibleContent();
    }

    /// @brief The size of the document, as of the last layout.
    KRYS_NODISCARD IntSize ContentsSize() const noexcept override
    {
      return _contentsSize;
    }

    /// @brief Sets the size of the document, and moves the scroll position back within it.
    void SetContentsSize(const IntSize &newSize) noexcept;

    /// @brief Fits the contents size and the scroll origin to the document's rect, after a layout.
    void AdjustViewSize() noexcept;

    /// @brief The scroll position, as the contents (and the scrolling element's scroll APIs) see it.
    KRYS_NODISCARD ScrollPosition ContentsScrollPosition() const noexcept
    {
      return _scrollPosition;
    }

    void SetContentsScrollPosition(
      const ScrollPosition &position,
      const ScrollPositionChangeOptions &options = ScrollPositionChangeOptions::CreateProgrammatic()) noexcept
    {
      SetScrollPosition(position, options);
    }

    /// @brief Scrolls to @p scrollPosition, clamped to the scroll range unless @p options say otherwise.
    void SetScrollPosition(const ScrollPosition &scrollPosition,
                           const ScrollPositionChangeOptions &options =
                             ScrollPositionChangeOptions::CreateProgrammatic()) noexcept;
    void SetScrollOffsetWithOptions(Maybe<int32> x, Maybe<int32> y,
                                    const ScrollPositionChangeOptions &options) noexcept;
    void SetScrollOffsetWithOptions(const ScrollOffset &scrollOffset,
                                    const ScrollPositionChangeOptions &options) noexcept;

    KRYS_NODISCARD ScrollPosition
      AdjustScrollPositionWithinRange(const ScrollPosition &scrollPosition) const noexcept;

    /// @brief The scroll position fixed-position boxes are placed relative to: the scroll position (there's
    /// no visual viewport, page scale, header or rubber-banding).
    KRYS_NODISCARD LayoutPoint ScrollPositionForFixedPosition() const noexcept;
    KRYS_NODISCARD LayoutPoint ScrollPositionRespectingCustomFixedPosition() const noexcept
    {
      return ScrollPositionForFixedPosition();
    }

    /// @brief The rect fixed-position boxes are laid out in: the visible content.
    KRYS_NODISCARD LayoutRect RectForFixedPositionLayout() const noexcept;

    /// @brief Scrolls the view so @p absoluteRect (in the contents' coordinates) is visible as asked, unless
    /// it's fixed on both axes.
    void ScrollRectToVisibleInTopLevelView(const LayoutRect &absoluteRect, EnumSet<BoxAxis> isFixed,
                                           const ScrollRectToVisibleOptions &options) noexcept;

    KRYS_NODISCARD bool IsScrollCornerVisible() const noexcept override
    {
      return !ScrollCornerRect().IsEmpty();
    }

    /// @brief The corner between the scrollbars, in the view's coordinates (empty without both, or with
    /// overlay scrollbars).
    KRYS_NODISCARD IntRect ScrollCornerRect() const noexcept override;

    KRYS_NODISCARD Scrollbar *HorizontalScrollbar() const noexcept override
    {
      return _horizontalScrollbar.get();
    }

    KRYS_NODISCARD Scrollbar *VerticalScrollbar() const noexcept override
    {
      return _verticalScrollbar.get();
    }

    KRYS_NODISCARD ScrollbarMode HorizontalScrollbarMode() const noexcept override
    {
      return _horizontalScrollbarMode;
    }

    KRYS_NODISCARD ScrollbarMode VerticalScrollbarMode() const noexcept override
    {
      return _verticalScrollbarMode;
    }

    /// @brief Sets when the view shows each scrollbar (and so whether the user can scroll on that axis), and
    /// adds or removes the scrollbars to match.
    void SetScrollbarModes(ScrollbarMode horizontalMode, ScrollbarMode verticalMode,
                           bool horizontalLock = false, bool verticalLock = false) noexcept;
    void SetHorizontalScrollbarMode(ScrollbarMode mode, bool lock = false) noexcept
    {
      SetScrollbarModes(mode, VerticalScrollbarMode(), lock, _verticalScrollbarLock);
    }
    void SetVerticalScrollbarMode(ScrollbarMode mode, bool lock = false) noexcept
    {
      SetScrollbarModes(HorizontalScrollbarMode(), mode, _horizontalScrollbarLock, lock);
    }

    enum class ScrollbarModesCalculationStrategy : bool
    {
      RulesFromWebContentOnly,
      AnyRule
    };

    /// @brief The scrollbar modes the root's (or the body's) `overflow` asks for.
    void CalculateScrollbarModesForLayout(
      ScrollbarMode &hMode, ScrollbarMode &vMode,
      ScrollbarModesCalculationStrategy strategy = ScrollbarModesCalculationStrategy::AnyRule) const noexcept;

    /// @brief Gets the view ready for a layout of @p layoutRoot: its scrollbar modes, and marking the root
    /// (or the body) for layout if the size content is laid out at changed.
    void WillDoLayout(const RenderElement &layoutRoot) noexcept;

    /// @brief Whether the user can scroll the view: it has overflow, and `overflow` doesn't hide both axes.
    KRYS_NODISCARD bool IsScrollable() const noexcept;
    /// @brief Scrolls the view by a step of @p granularity in a direction relative to the document's writing
    /// mode.
    /// @return Whether it scrolled.
    bool LogicalScroll(ScrollLogicalDirection direction, ScrollGranularity granularity) noexcept;
    /// @brief Whether the root's writing mode is horizontal (its blocks stack vertically).
    KRYS_NODISCARD bool IsVerticalDocument() const noexcept;
    KRYS_NODISCARD bool IsFlippedDocument() const noexcept;
    KRYS_NODISCARD bool IsScrollableOrRubberbandable() const noexcept override
    {
      return IsScrollable();
    }

    /// @brief The view's scrollbar at @p windowPoint, if any.
    KRYS_NODISCARD Scrollbar *ScrollbarAtPoint(const IntPoint &windowPoint) const noexcept;

    KRYS_NODISCARD ScrollbarWidth ScrollbarWidthStyle() const noexcept override;
    KRYS_NODISCARD OverscrollBehavior HorizontalOverscrollBehavior() const noexcept override;
    KRYS_NODISCARD OverscrollBehavior VerticalOverscrollBehavior() const noexcept override;

    /// @brief The scrollable area enclosing the view: none, as there are no frames.
    KRYS_NODISCARD ScrollableArea *EnclosingScrollableArea() const noexcept override
    {
      return nullptr;
    }

    KRYS_NODISCARD bool ShouldPlaceVerticalScrollbarOnLeft() const noexcept override;

    /// @brief Fires `scrollend` at the document after a scroll it fired `scroll` for.
    void ScrollDidEnd() noexcept override;

    // Conversions between the view's coordinates (its frame's, at the origin, as there's no widget tree) and
    // the contents'.
    KRYS_NODISCARD IntPoint ViewToContents(const IntPoint &point) const noexcept;
    KRYS_NODISCARD IntPoint ContentsToView(const IntPoint &point) const noexcept;
    KRYS_NODISCARD FloatPoint ViewToContents(const FloatPoint &point) const noexcept;
    KRYS_NODISCARD FloatPoint ContentsToView(const FloatPoint &point) const noexcept;
    KRYS_NODISCARD IntRect ViewToContents(IntRect rect) const noexcept;
    KRYS_NODISCARD IntRect ContentsToView(IntRect rect) const noexcept;

#pragma endregion

    // Painting

    /// @brief Paints the view's part of @p rect (in the view's coordinates): its visible contents, scrolled
    /// by the scroll position.
    // FIXME: The overhang areas, scrollbars and pan scroll icon, once there's rubber-banding, scrollbars and
    // pan scrolling.
    void Paint(GraphicsContext &context, const IntRect &rect) noexcept;

    /// @brief Paints the document's render view over @p dirtyRect, in the contents' coordinates.
    /// FIXME: The overlay scrollbars and painting a single node (`setNodeToDraw`) aren't ported.
    void PaintContents(GraphicsContext &, const IntRect &dirtyRect) noexcept;

    void SetPaintBehavior(EnumSet<PaintBehavior> behavior) noexcept
    {
      _paintBehavior = behavior;
    }
    KRYS_NODISCARD EnumSet<PaintBehavior> GetPaintBehavior() const noexcept
    {
      return _paintBehavior;
    }
    KRYS_NODISCARD bool IsPainting() const noexcept
    {
      return _isPainting;
    }

    KRYS_NODISCARD bool IsTransparent() const noexcept
    {
      return _isTransparent;
    }
    void SetTransparent(bool isTransparent) noexcept;

    /// @brief The colour painted under the document where nothing else is (white by default).
    KRYS_NODISCARD boo::Color BaseBackgroundColor() const noexcept
    {
      return _baseBackgroundColor;
    }
    void SetBaseBackgroundColor(const boo::Color &backgroundColor) noexcept;
    KRYS_NODISCARD bool HasOpaqueBackground() const noexcept
    {
      return !_isTransparent && _baseBackgroundColor.IsOpaque();
    }

    void SetContentIsOpaque(bool contentIsOpaque) noexcept
    {
      _contentIsOpaque = contentIsOpaque;
    }

    /// @brief The view's media type (`screen` by default).
    KRYS_NODISCARD const css::CSSOMStringAtom &MediaType() const noexcept
    {
      return _mediaType;
    }

    /// @brief Sets the view's media type, and re-evaluates the document's style sheets if it changed.
    void SetMediaType(const css::CSSOMStringAtom &mediaType) noexcept;

    KRYS_NODISCARD const boo::Pagination &GetPagination() const noexcept
    {
      return _pagination;
    }

    void SetPagination(const boo::Pagination &pagination) noexcept;

    void SetOverrideSizeForCSSDefaultViewportUnits(OverrideViewportSize size) noexcept;

    KRYS_NODISCARD Maybe<OverrideViewportSize> OverrideSizeForCSSDefaultViewportUnits() const noexcept
    {
      return _defaultViewportSizeOverride;
    }

    void SetSizeForCSSDefaultViewportUnits(FloatSize size) noexcept;
    void ClearSizeOverrideForCSSDefaultViewportUnits() noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSDefaultViewportUnits() const noexcept;

    void SetOverrideSizeForCSSSmallViewportUnits(OverrideViewportSize size) noexcept;

    KRYS_NODISCARD Maybe<OverrideViewportSize> OverrideSizeForCSSSmallViewportUnits() const noexcept
    {
      return _smallViewportSizeOverride;
    }

    void SetSizeForCSSSmallViewportUnits(FloatSize size) noexcept;
    void ClearSizeOverrideForCSSSmallViewportUnits() noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSSmallViewportUnits() const noexcept;

    void SetOverrideSizeForCSSLargeViewportUnits(OverrideViewportSize size) noexcept;

    KRYS_NODISCARD Maybe<OverrideViewportSize> OverrideSizeForCSSLargeViewportUnits() const noexcept
    {
      return _largeViewportSizeOverride;
    }

    void SetSizeForCSSLargeViewportUnits(FloatSize size) noexcept;
    void ClearSizeOverrideForCSSLargeViewportUnits() noexcept;
    KRYS_NODISCARD FloatSize SizeForCSSLargeViewportUnits() const noexcept;

    KRYS_NODISCARD FloatSize SizeForCSSDynamicViewportUnits() const noexcept;

    /// @brief Scrolls the layers that contain @p renderer so @p absoluteRect is visible as asked.
    /// @return Whether anything was asked to scroll.
    static bool ScrollRectToVisible(const LayoutRect &absoluteRect, const RenderObject &renderer,
                                    bool insideFixed, const ScrollRectToVisibleOptions &options) noexcept;

    /// @brief Scrolls the document's focused element into view.
    // FIXME: This scrolls right away rather than on a zero-delay timer, since there's no event loop.
    void ScheduleScrollToFocusedElement(SelectionRevealMode selectionRevealMode) noexcept;

    /// @brief The scale from document coordinates to absolute ones: @p usedZoom (which already includes the
    /// page zoom) if given, or else the page zoom.
    // FIXME: The frame scale factor, once frames can be scaled.
    KRYS_NODISCARD float DocumentToAbsoluteScaleFactor(Maybe<float> usedZoom = null) const noexcept;
    KRYS_NODISCARD float AbsoluteToDocumentScaleFactor(Maybe<float> usedZoom = null) const noexcept;
    KRYS_NODISCARD FloatRect AbsoluteToDocumentRect(FloatRect rect,
                                                    Maybe<float> usedZoom = null) const noexcept;
    /// @brief The offset from document coordinates to client (viewport) ones.
    KRYS_NODISCARD FloatSize DocumentToClientOffset() const noexcept;
    KRYS_NODISCARD DoublePoint AbsoluteToDocumentPoint(DoublePoint point,
                                                       Maybe<float> usedZoom = null) const noexcept;
    KRYS_NODISCARD DoublePoint DocumentToClientPoint(DoublePoint point) const noexcept;
    /// @brief Converts a point in the view's coordinates to the contents' (absolute) coordinates.
    KRYS_NODISCARD DoublePoint WindowToContents(const DoublePoint &point) const noexcept;
    KRYS_NODISCARD IntPoint WindowToContents(const IntPoint &point) const noexcept;
    KRYS_NODISCARD IntPoint ContentsToWindow(const IntPoint &point) const noexcept;
    KRYS_NODISCARD IntRect ContentsToWindow(const IntRect &rect) const noexcept;
    KRYS_NODISCARD FloatRect DocumentToClientRect(FloatRect rect) const noexcept;

    void AvailableContentSizeChanged(AvailableSizeChangeReason reason) noexcept override;

  private:
    bool SetHasHorizontalScrollbar(bool hasBar, bool *contentSizeAffected = nullptr) noexcept;
    bool SetHasVerticalScrollbar(bool hasBar, bool *contentSizeAffected = nullptr) noexcept;
    bool SetHasScrollbarInternal(RefPtr<Scrollbar> &scrollbar, ScrollbarOrientation orientation, bool hasBar,
                                 bool *contentSizeAffected) noexcept;
    KRYS_NODISCARD Ref<Scrollbar> CreateScrollbar(ScrollbarOrientation orientation) noexcept;
    void UpdateScrollbarSteps() noexcept;
    /// @brief Lays the view out again if adding or removing a scrollbar changed the size content is laid out
    /// at.
    void UpdateContentsSize() noexcept;
    void ApplyOverflowToViewport(const RenderElement &renderer, ScrollbarMode &hMode,
                                 ScrollbarMode &vMode) const noexcept;
    void AdjustScrollbarsForLayout(bool isFirstLayout) noexcept;
    void MarkRootOrBodyRendererDirty() const noexcept;
    void PaintScrollbars(GraphicsContext &context, const IntRect &rect) noexcept;
    /// @brief Fills the scrollbars' area and the scroll corner with the root's background colour, over the
    /// base background colour, so the page's background runs on under the scrollbars, which have no track.
    void PaintScrollbarsBackground(GraphicsContext &context) noexcept;
    void PaintScrollCorner(GraphicsContext &context, const IntRect &cornerRect) noexcept;

    KRYS_NODISCARD IntRect VisibleContentRectInternal(
      VisibleContentRectIncludesScrollbars scrollbarInclusion) const noexcept override;
    void SetScrollOffset(const ScrollOffset &offset) noexcept override;
    /// @brief Moves the scroll position to @p newPosition, then updates what follows it (the layers, the
    /// scroll events).
    void ScrollTo(const ScrollPosition &newPosition) noexcept;
    void ScrollPositionChanged(const ScrollPosition &oldPosition, const ScrollPosition &newPosition) noexcept;
    /// @brief Scrolls to @p desiredPosition, within the scroll range.
    // FIXME: Adding and removing scrollbars, once there are scrollbars.
    void UpdateScrollbars(const ScrollPosition &desiredPosition) noexcept;
    void ScheduleScrollEvent() noexcept;
    KRYS_NODISCARD LayoutRect GetPossiblyFixedRectToExpose(const LayoutRect &visibleRect,
                                                           const LayoutRect &exposeRect,
                                                           EnumSet<BoxAxis> isFixed,
                                                           const ScrollAlignment &alignX,
                                                           const ScrollAlignment &alignY) const noexcept;
    KRYS_NODISCARD LayoutRect GetPossiblyFixedRectToExpose(const LayoutRect &visibleRect,
                                                           const LayoutRect &exposeRect, bool isFixed,
                                                           const ScrollAlignment &alignX,
                                                           const ScrollAlignment &alignY) const noexcept;

    KRYS_NODISCARD FloatSize
      CalculateSizeForCSSViewportUnitsOverride(Maybe<OverrideViewportSize> override) const noexcept;
  };
}
