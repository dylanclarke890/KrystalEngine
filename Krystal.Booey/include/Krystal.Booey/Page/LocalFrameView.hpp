#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/Pagination.hpp"
#include "Krystal.Booey/Rendering/PaintPhase.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class GraphicsContext;
  class LocalFrame;
  class RenderView;

  /// @brief The view a frame's document is laid out in: its frame rect, the size content is laid out at, the
  /// media type, pagination, the sizes viewport units resolve against, and the layout context.
  /// @note FIXME: No scrolling, scrollbars, obscured insets, visual viewport, auto-sizing, repaint rects or
  /// compositing, and views aren't widgets in a widget tree: the visible content is the whole frame rect, at
  /// the origin.
  class LocalFrameView final : public RefCounted<LocalFrameView>, public CanMakeWeakPtr<LocalFrameView>
  {
    KRYS_NON_COPY_MOVABLE(LocalFrameView);

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

    /// @brief The part of the content that's visible: the view's size, at the origin (there's no scrolling).
    KRYS_NODISCARD IntRect VisibleContentRect() const noexcept;

    /// @brief The part of the visible content that isn't covered by the host's interface: all of it.
    KRYS_NODISCARD IntRect UnobscuredContentRect() const noexcept;

    /// @brief The size of the visible content.
    KRYS_NODISCARD IntSize SizeForVisibleContent() const noexcept
    {
      return VisibleContentRect().Size();
    }

    /// @brief The document's scroll position relative to the view's origin: none, as the view doesn't scroll.
    /// FIXME: Comes with scrolling the viewport.
    KRYS_NODISCARD IntPoint DocumentScrollPositionRelativeToViewOrigin() const noexcept
    {
      return {};
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

    // Painting

    /// @brief Paints the document's render view over @p dirtyRect, in the view's coordinates.
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
    KRYS_NODISCARD FloatRect DocumentToClientRect(FloatRect rect) const noexcept;

  private:
    void AvailableContentSizeChanged() noexcept;

    KRYS_NODISCARD FloatSize
      CalculateSizeForCSSViewportUnitsOverride(Maybe<OverrideViewportSize> override) const noexcept;
  };
}
