#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Debug.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderBox;
  class RenderElement;

  /// @brief The state a layout pass carries down the render tree: offsets from the layout root, the clip, the
  /// layout delta and pagination.
  class RenderLayoutState
  {
    KRYS_NON_COPYABLE(RenderLayoutState);

  public:
    struct LineClamp
    {
      size_t MaximumLines {0};
      bool ShouldDiscardOverflow {false};
    };

  private:
    bool _clipped : 1 {false};
    bool _isPaginated : 1 {false};
    // If our page height has changed, this will force all blocks to relayout.
    bool _pageLogicalHeightChanged : 1 {false};
#if KRYS_DEBUG
    bool _layoutDeltaXSaturated : 1 {false};
    bool _layoutDeltaYSaturated : 1 {false};
#endif
    bool _marginTrimBlockStart : 1 {false};

    // FIXME: Distinguish between the layout clip rect and the paint clip rect which may be larger,
    // e.g., because of composited scrolling.
    LayoutRect _clipRect;

    // x/y offset from layout root. Includes in-flow positioning and scroll offsets.
    LayoutSize _paintOffset;
    // x/y offset from layout root. Does not include in-flow positioning or scroll offsets.
    LayoutSize _layoutOffset;
    // Transient offset from the final position of the object
    // used to ensure that repaints happen in the correct place.
    // This is a total delta accumulated from the root.
    LayoutSize _layoutDelta;

    // The current page height for the pagination model that encloses us.
    LayoutUnit _pageLogicalHeight;
    // The offset of the start of the first page in the nearest enclosing pagination model.
    LayoutSize _pageOffset;
    Maybe<LineClamp> _lineClamp;
#if KRYS_DEBUG
    RenderElement *_renderer {nullptr};
#endif

  public:
    RenderLayoutState() noexcept = default;
    RenderLayoutState(const LocalFrameViewLayoutContext::LayoutStateStack &layoutStateStack,
                      RenderBox &renderer, const LayoutSize &offset, LayoutUnit pageLogicalHeight,
                      bool pageLogicalHeightChanged, Maybe<LineClamp> lineClamp) noexcept;
    explicit RenderLayoutState(RenderElement &renderer) noexcept;

    KRYS_NODISCARD bool IsPaginated() const noexcept
    {
      return _isPaginated;
    }

    // The page logical offset is the object's offset from the top of the page in the page progression
    // direction (so an x-offset in vertical text and a y-offset for horizontal text).
    KRYS_NODISCARD LayoutUnit PageLogicalOffset(RenderBox *child,
                                                LayoutUnit childLogicalOffset) const noexcept;

    KRYS_NODISCARD LayoutUnit PageLogicalHeight() const noexcept
    {
      return _pageLogicalHeight;
    }
    KRYS_NODISCARD bool PageLogicalHeightChanged() const noexcept
    {
      return _pageLogicalHeightChanged;
    }

    KRYS_NODISCARD LayoutSize PaintOffset() const noexcept
    {
      return _paintOffset;
    }
    KRYS_NODISCARD LayoutSize LayoutOffset() const noexcept
    {
      return _layoutOffset;
    }

    KRYS_NODISCARD LayoutSize PageOffset() const noexcept
    {
      return _pageOffset;
    }

    KRYS_NODISCARD bool NeedsBlockDirectionLocationSetBeforeLayout() const noexcept
    {
      return _isPaginated && _pageLogicalHeight;
    }

#if KRYS_DEBUG
    KRYS_NODISCARD RenderElement *Renderer() const noexcept
    {
      return _renderer;
    }
#endif
    KRYS_NODISCARD LayoutRect ClipRect() const noexcept
    {
      return _clipRect;
    }
    KRYS_NODISCARD bool IsClipped() const noexcept
    {
      return _clipped;
    }

    void AddLayoutDelta(LayoutSize delta) noexcept;
    KRYS_NODISCARD LayoutSize LayoutDelta() const noexcept
    {
      return _layoutDelta;
    }
#if KRYS_DEBUG
    KRYS_NODISCARD bool LayoutDeltaMatches(LayoutSize delta) const noexcept;
#endif

    void SetLineClamp(Maybe<LineClamp> lineClamp) noexcept
    {
      _lineClamp = lineClamp;
    }
    KRYS_NODISCARD Maybe<LineClamp> GetLineClamp() const noexcept
    {
      return _lineClamp;
    }

    void SetMarginTrimBlockStart(bool marginTrimBlockStart) noexcept
    {
      _marginTrimBlockStart = marginTrimBlockStart;
    }
    KRYS_NODISCARD bool MarginTrimBlockStart() const noexcept
    {
      return _marginTrimBlockStart;
    }

  private:
    void ComputeOffsets(const RenderLayoutState &ancestor, RenderBox &renderer, LayoutSize offset) noexcept;
    void ComputeClipRect(const RenderLayoutState &ancestor, RenderBox &renderer) noexcept;
    // FIXME: webkit.org/b/179440 these functions should be part of the pagination
    // code/LocalFrameViewLayoutContext.
    void ComputePaginationInformation(const LocalFrameViewLayoutContext::LayoutStateStack &layoutStateStack,
                                      RenderBox &renderer, LayoutUnit pageLogicalHeight,
                                      bool pageLogicalHeightChanged) noexcept;
  };

  /// @brief Pushes a layout state for a box for the length of its scope.
  class LayoutStateMaintainer
  {
    KRYS_NON_COPYABLE(LayoutStateMaintainer);

  private:
    LocalFrameViewLayoutContext &_context;
    bool _paintOffsetCacheIsDisabled {false};
    bool _didPushLayoutState {false};

  public:
    explicit LayoutStateMaintainer(RenderBox &root, LayoutSize offset, bool disablePaintOffsetCache = false,
                                   LayoutUnit pageHeight = 0_lu, bool pageHeightChanged = false) noexcept;
    ~LayoutStateMaintainer() noexcept;
  };

  /// @brief Pushes the layout state of a subtree layout's root for the length of its scope.
  class SubtreeLayoutStateMaintainer
  {
    KRYS_NON_COPYABLE(SubtreeLayoutStateMaintainer);

  private:
    LocalFrameViewLayoutContext *_context {nullptr};
    bool _didDisablePaintOffsetCache {false};

  public:
    explicit SubtreeLayoutStateMaintainer(RenderElement *subtreeLayoutRoot) noexcept;
    ~SubtreeLayoutStateMaintainer() noexcept;
  };

  /// @brief Suspends the paint offset cache for the length of its scope.
  class LayoutStateDisabler
  {
    KRYS_NON_COPYABLE(LayoutStateDisabler);

  private:
    LocalFrameViewLayoutContext &_context;

  public:
    explicit LayoutStateDisabler(LocalFrameViewLayoutContext &context) noexcept;
    ~LayoutStateDisabler() noexcept;
  };

  /// @brief Stops a flex item's percentage heights from resolving for the length of its scope.
  class FlexPercentResolveDisabler
  {
    KRYS_NON_COPYABLE(FlexPercentResolveDisabler);

  private:
    LocalFrameViewLayoutContext &_layoutContext;
    const RenderBox &_flexItem;

  public:
    FlexPercentResolveDisabler(LocalFrameViewLayoutContext &layoutContext,
                               const RenderBox &flexItem) noexcept;
    ~FlexPercentResolveDisabler() noexcept;
  };

  /// @brief Has layout descend into the content that content-visibility skips, for the reasons given, for
  /// the length of its scope.
  class ContentVisibilityOverrideScope
  {
    KRYS_NON_COPY_MOVABLE(ContentVisibilityOverrideScope);

  public:
    enum class OverrideType : uint8
    {
      Hidden,
      Auto,
      RevealedWhenFound
    };

  private:
    LocalFrameViewLayoutContext &_layoutContext;

  public:
    ContentVisibilityOverrideScope(LocalFrameViewLayoutContext &layoutContext,
                                   EnumSet<OverrideType> overrideTypes) noexcept;
    ~ContentVisibilityOverrideScope() noexcept;
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::ContentVisibilityOverrideScope::OverrideType, 3uz);
