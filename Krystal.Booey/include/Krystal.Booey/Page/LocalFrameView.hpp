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
#include "Krystal.Booey/Graphics/Region.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Platform/ScrollableArea.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Rendering/Pagination.hpp"
#include "Krystal.Booey/Rendering/PaintPhase.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class GraphicsContext;
  class GraphicsLayer;
  class LocalFrame;
  class RenderView;

  /// @brief The view a frame's document is laid out in: its frame rect, the size content is laid out at, the
  /// media type, pagination, the sizes viewport units resolve against, the layout context, and its scroll
  /// position within the document (the scrolling half of a scroll view).
  /// @note FIXME: No obscured insets, visual viewport or auto-sizing, and views
  /// aren't widgets in a widget tree (there's no `Widget` or `ScrollView`): the visible content is the frame
  /// rect's size, less its scrollbars, at the scroll position.
  /// @brief Painted contents that moved: the pixels in `Rect` (in the view's coordinates) move by `Delta`,
  /// and what moves out of it is gone.
  struct ScrolledContents
  {
    IntSize Delta;
    IntRect Rect;
  };

  class LocalFrameView final : public RefCounted<LocalFrameView>,
                               public CanMakeWeakPtr<LocalFrameView>,
                               public ScrollableArea
  {
    KRYS_NON_COPY_MOVABLE(LocalFrameView);
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LocalFrameView);

  public:
    /// @brief How the user scrolled a view: themselves, or by something they did (moving the selection).
    enum class UserScrollType : uint8
    {
      Explicit,
      Implicit
    };

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
    /// @brief What needs painting again, in the view's coordinates, since the host last took it.
    Region _damage;
    /// @brief The painted contents the host moves before it paints the damage, as scrolling moved them.
    Maybe<ScrolledContents> _scrolledContents;
    /// @brief Renderers that paint a background fixed to the viewport (`background-attachment: fixed`): the
    /// contents can't be moved when the view scrolls. Keyed by address; renderers remove themselves.
    HashSet<const RenderElement *> _slowRepaintObjects;
    /// @brief Renderers fixed or sticky to the viewport, which stay put when the contents move. Keyed by
    /// address; renderers remove themselves.
    HashSet<const RenderLayerModelObject *> _viewportConstrainedObjects;
    Timer _delayedScrollToFocusedElementTimer;
    SelectionRevealMode _selectionRevealModeForFocusedElement {SelectionRevealMode::DoNotReveal};
    bool _shouldScrollToFocusedElement {false};
    /// @brief The node the view keeps in view as layout changes (the fragment's target, or the document for
    /// the top), until the view scrolls otherwise.
    WeakPtr<dom::ContainerNode> _maintainScrollPositionAnchor;
    WeakPtr<dom::ContainerNode> _scheduledMaintainScrollPositionAnchor;
    bool _scheduledToScrollToAnchor {false};
    /// @brief Tries the URL's text directives again while the page may still be loading their text.
    Timer _scrollToTextFragmentRetryTimer;
    Maybe<MonotonicTime::duration> _scrollToTextFragmentInitialAttemptTime;
    /// @brief The scroll position as the document entered the back/forward cache.
    ScrollPosition _cachedScrollPosition;
    /// @brief Whether the frame's loader is still to hear of the first layout.
    bool _firstLayoutCallbackPending {false};
    /// @brief How the user last scrolled the view, if they did.
    Maybe<UserScrollType> _lastUserScrollType;
    bool _wasEverScrolledExplicitlyByUser {false};
    /// @brief The text a text directive indicated, kept in view as layout changes, while its text stays the
    /// same.
    Maybe<dom::SimpleRange> _pendingTextFragmentIndicatorRange;
    utf8_string _pendingTextFragmentIndicatorText;
    /// @brief Where the pending range's ends were when it was last revealed.
    Maybe<IntRect> _lastRevealedTextFragmentRect;
    bool _skipScrollResetOfScrollToTextFragmentRange {false};
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
    /// @brief The viewport's size and zoom when `resize` was last scheduled, or at the first layout.
    IntSize _lastViewportSize;
    float _lastUsedZoomFactor {1};
    float _lastFrameScaleFactor {1};

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

    /// @brief The scale of the frame's content (the page's scale).
    KRYS_NODISCARD float FrameScaleFactor() const noexcept;

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

    void InvalidateScrollbar(Scrollbar &scrollbar, const IntRect &rect) noexcept override;
    void InvalidateScrollCorner(const IntRect &rect) noexcept;
    void PaintScrollCorner(GraphicsContext &context, const IntRect &cornerRect) noexcept;

    KRYS_NODISCARD const HashSet<const RenderLayerModelObject *> &ViewportConstrainedObjects() const noexcept
    {
      return _viewportConstrainedObjects;
    }

#pragma region Compositing

    void WillRecalcStyle() noexcept;
    void SetIsInWindow(bool isInWindow) noexcept;

    bool FlushCompositingStateForThisFrame(const LocalFrame &rootFrameForFlush) noexcept;
    /// @brief Returns true if the flush was completed.
    bool FlushCompositingStateIncludingSubframes() noexcept;

    // Sometimes (for plug-ins) we need to eagerly go into compositing mode.
    void EnterCompositingMode() noexcept;
    KRYS_NODISCARD bool IsEnclosedInCompositingLayer() const noexcept;
    /// @brief Whether the view's contents scroll in a composited layer (the render view's).
    KRYS_NODISCARD bool UsesCompositedScrolling() const noexcept;

    void SetNeedsCompositingConfigurationUpdate() noexcept;
    void SetNeedsCompositingGeometryUpdate() noexcept;
    void SetDescendantsNeedUpdateBackingAndHierarchyTraversal() noexcept;

    KRYS_NODISCARD GraphicsLayer *LayerForHorizontalScrollbar() const noexcept;
    KRYS_NODISCARD GraphicsLayer *LayerForVerticalScrollbar() const noexcept;
    KRYS_NODISCARD GraphicsLayer *LayerForScrollCorner() const noexcept;
    void PositionScrollbarLayers() noexcept;

    KRYS_NODISCARD bool ManagesScrollbars() const noexcept
    {
      return true;
    }

    /// @brief The document's background colour (the root's and the body's, over the base background colour);
    /// invalid when there isn't enough information.
    KRYS_NODISCARD boo::Color DocumentBackgroundColor() const noexcept;

    KRYS_NODISCARD int32 InsetForLeftScrollbarSpace() const noexcept;
    KRYS_NODISCARD FloatPoint PositionForRootContentLayer() const noexcept;
    KRYS_NODISCARD LayoutRect
      FixedScrollableAreaBoundsInflatedForScrolling(const LayoutRect &uninflatedBounds) const noexcept;

    void UpdateCompositingLayersAfterScrolling() noexcept;
    void AddedOrRemovedScrollbar() noexcept;

    /// @brief The render view, for the updates that change it (its compositor).
    KRYS_NODISCARD RenderView *GetRenderViewForUpdate() const noexcept
    {
      return const_cast<RenderView *>(GetRenderView());
    }

#pragma endregion

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

    /// @brief Scrolls by @p delta, clamped to the scroll range.
    void ScrollBy(const IntSize &delta) noexcept
    {
      SetScrollPosition(GetScrollPosition() + delta);
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

    /// @brief The size of the layout viewport: the initial containing block's.
    KRYS_NODISCARD LayoutSize BaseLayoutViewportSize() const noexcept;
    /// @brief The layout viewport, in document coordinates: the initial containing block's size at the scroll
    /// position.
    KRYS_NODISCARD LayoutRect LayoutViewportRect() const noexcept;

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

    /// @brief After a layout: tells the frame's loader of its first layout, once.
    void FireLayoutRelatedMilestonesIfNeeded() noexcept;

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
    void UpdateAnchorPositionedAfterScroll() noexcept override;

    /// @brief Called when what the viewport shows may have changed (scrolling, resizing, layout): resumes the
    /// image animations now visible.
    // FIXME: The visible viewport rect, once there's one.
    void ViewportContentsChanged() noexcept;
    void ResumeVisibleImageAnimations(const IntRect &visibleRect) noexcept;

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

    /// @brief Damages @p rect of the contents (in the contents' coordinates): the host paints it again.
    /// @see `ScrollView::repaintContentRectangle`, which tells the host window
    /// (`HostWindow::invalidateContentsAndRootView`).
    void RepaintContentRectangle(const IntRect &rect) noexcept;
    /// @brief Damages @p rect of the view (in the view's coordinates).
    void InvalidateRect(const IntRect &rect) noexcept;
    /// @brief Damages the whole view.
    void RepaintEntireView() noexcept;
    /// @brief Whether anything needs painting again.
    KRYS_NODISCARD bool HasDamage() const noexcept
    {
      return !_damage.IsEmpty();
    }
    /// @brief What needs painting again, in the view's coordinates; the view then has no damage.
    /// @note Take the scrolled contents first: the damage is where the contents are once they've moved.
    KRYS_NODISCARD Region TakeDamage() noexcept;
    /// @brief How the painted contents moved since the host last took them: the host moves its pixels in the
    /// rect to match before painting the damage. Null when nothing moved, and always when the page
    /// composites.
    /// @see `HostWindow::scroll`.
    KRYS_NODISCARD Maybe<ScrolledContents> TakeScrolledContents() noexcept;

    void AddSlowRepaintObject(const RenderElement &renderer) noexcept;
    void RemoveSlowRepaintObject(const RenderElement &renderer) noexcept;
    KRYS_NODISCARD bool HasSlowRepaintObject(const RenderElement &renderer) const noexcept;
    KRYS_NODISCARD bool HasSlowRepaintObjects() const noexcept
    {
      return !_slowRepaintObjects.empty();
    }
    KRYS_NODISCARD const HashSet<const RenderElement *> &SlowRepaintObjects() const noexcept
    {
      return _slowRepaintObjects;
    }
    void AddViewportConstrainedObject(const RenderLayerModelObject &object) noexcept;
    void RemoveViewportConstrainedObject(const RenderLayerModelObject &object) noexcept;
    KRYS_NODISCARD bool HasViewportConstrainedObjects() const noexcept
    {
      return !_viewportConstrainedObjects.empty();
    }
    /// @brief Whether scrolling repaints the whole view rather than moving the painted contents.
    KRYS_NODISCARD bool UseSlowRepaints() const noexcept;

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

    /// @brief Scrolls the document's focused element into view, on a zero-delay timer.
    void ScheduleScrollToFocusedElement(SelectionRevealMode selectionRevealMode) noexcept;
    /// @brief Brings the render tree's selection up to date at the next rendering update.
    void ScheduleSelectionUpdate() noexcept;
    /// @brief Cancels a scroll to the focused element that hasn't happened yet.
    void CancelScheduledScrolls() noexcept;

    /// @brief Keeps the scroll position, as the view's document enters the back/forward cache.
    void CacheCurrentScrollState() noexcept
    {
      _cachedScrollPosition = GetScrollPosition();
    }

    KRYS_NODISCARD const ScrollPosition &CachedScrollPosition() const noexcept
    {
      return _cachedScrollPosition;
    }

    /// @brief Whether the user scrolled the view (since the view was made, or its last user scroll type was
    /// cleared).
    KRYS_NODISCARD bool WasScrolledByUser() const noexcept
    {
      return _lastUserScrollType.has_value();
    }

    KRYS_NODISCARD bool WasEverScrolledExplicitlyByUser() const noexcept
    {
      return _wasEverScrolledExplicitlyByUser;
    }

    /// @brief Notes how the user last scrolled the view (null to clear it), which stops the view keeping the
    /// fragment or the focused element in view; a programmatic scroll isn't the user's.
    void SetLastUserScrollType(Maybe<UserScrollType> userScrollType) noexcept;

    /// @brief Scrolls to the fragment of @p url: makes the element it indicates the document's target and
    /// scrolls it into view (the top of the document for `#top` or an empty fragment), and focuses it.
    /// @return Whether there was something to scroll to.
    /// @see https://html.spec.whatwg.org/#scroll-to-the-fragment-identifier
    bool ScrollToFragment(const url::URL &url) noexcept;

    /// @brief Highlights the text the document's text directives indicate, makes its nearest element the
    /// document's target and scrolls it into view, trying again every 500ms for 3s (until the user scrolls)
    /// if it isn't there yet.
    /// @return Whether there was text to scroll to.
    /// @see https://wicg.github.io/scroll-to-text-fragment/#invoking-text-directives
    bool ScrollToTextFragment(bool isRetry = false) noexcept;

    /// @brief Scrolls to the element the fragment @p fragmentIdentifier indicates.
    /// @return Whether there was something to scroll to.
    bool ScrollToAnchorFragment(dom::DOMStringView fragmentIdentifier) noexcept;

    /// @brief Keeps @p anchorNode in view as layout changes (null stops it), scrolling to it now.
    void MaintainScrollPositionAtAnchor(dom::ContainerNode *anchorNode) noexcept;

    /// @brief The node the view keeps in view, if any.
    KRYS_NODISCARD dom::ContainerNode *MaintainedScrollPositionAnchor() const noexcept
    {
      return _maintainScrollPositionAnchor.get();
    }

    /// @brief Scrolls to the maintained anchor in a task, as layout may have moved it.
    void ScheduleScrollToAnchorAndTextFragment() noexcept;

    /// @brief @p value, in layout units, in CSS pixels: less the page zoom.
    KRYS_NODISCARD int32 MapFromLayoutToCSSUnits(LayoutUnit value) const noexcept;

    /// @brief @p value, in CSS pixels, in layout units: with the page zoom.
    KRYS_NODISCARD LayoutUnit MapFromCSSToLayoutUnits(int32 value) const noexcept;

    /// @brief The viewport's size, with its scrollbars, as `resize` reports changes to it.
    KRYS_NODISCARD IntSize SizeForResizeEvent() const noexcept;

    /// @brief After a layout: has the next rendering update fire `resize` at the window if the viewport's
    /// size or zoom changed since the last time.
    void ScheduleResizeEventIfNeeded() noexcept;

    /// @brief The viewport moved or resized: the window's visual viewport takes its new geometry.
    void LayoutOrVisualViewportChanged() noexcept;

    /// @brief Scrolls to the maintained anchor if a scroll to it is scheduled.
    void ScrollToAnchorAndTextFragmentNowIfNeeded() noexcept;

    /// @brief The scale from document coordinates to absolute ones: @p usedZoom (which already includes the
    /// page zoom) if given, or else the page zoom.
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
    KRYS_NODISCARD FloatRect AbsoluteToClientRect(FloatRect rect,
                                                  Maybe<float> usedZoom = null) const noexcept;
    /// @brief Converts an absolute rect to one relative to the layout viewport.
    KRYS_NODISCARD FloatRect AbsoluteToLayoutViewportRect(FloatRect rect) const noexcept;

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

    KRYS_NODISCARD IntRect VisibleContentRectInternal(
      VisibleContentRectIncludesScrollbars scrollbarInclusion) const noexcept override;
    void SetScrollOffset(const ScrollOffset &offset) noexcept override;
    /// @brief Moves the scroll position to @p newPosition, then updates what follows it (the layers, the
    /// scroll events).
    void ScrollTo(const ScrollPosition &newPosition) noexcept;
    void ScrollPositionChanged(const ScrollPosition &oldPosition, const ScrollPosition &newPosition) noexcept;
    /// @brief Moves the painted contents by @p scrollDelta (the change in the scroll position), or repaints
    /// them.
    void ScrollContents(const IntSize &scrollDelta) noexcept;
    /// @brief Moves the painted contents of @p rectToScroll by @p scrollDelta (the contents' move, the
    /// opposite of the scroll position's), repainting the viewport-constrained objects; false when they can't
    /// be moved.
    bool ScrollContentsFastPath(const IntSize &scrollDelta, const IntRect &rectToScroll) noexcept;
    void ScrollContentsSlowPath(const IntRect &updateRect) noexcept;
    void RepaintSlowRepaintObjects() noexcept;
    /// @brief The host window's half of the fast path: the contents of @p rectToScroll move by @p
    /// scrollDelta, with the damage, and what comes into view is damaged. Nothing, when the page composites.
    void ScrollHostContents(const IntSize &scrollDelta, const IntRect &rectToScroll) noexcept;
    /// @brief The part of the view whose painted contents move when it scrolls: the visible contents, without
    /// the scrollbars.
    KRYS_NODISCARD IntRect RectToCopyOnScroll() const noexcept;
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

  private:
    void ScrollToFocusedElementTimerFired() noexcept;
    void ScrollToFocusedElementInternal() noexcept;
    /// @brief Scrolls the maintained anchor into view: to its top and closest side.
    void ScrollToAnchor() noexcept;
    void ScrollToTextFragmentRetryTimerFired() noexcept;
    /// @brief Keeps @p range in view as layout changes, scrolling to it now.
    void MaintainScrollPositionAtScrollToTextFragmentRange(const dom::SimpleRange &range) noexcept;
    /// @brief Scrolls the pending text fragment range back into view, unless its text changed or the user
    /// scrolled.
    void ScrollToPendingTextFragmentRange() noexcept;
    /// @brief Forgets the pending text fragment range, unless a scroll to it is in progress.
    void CancelScheduledTextFragmentIndicatorTimer() noexcept;
  };
}
