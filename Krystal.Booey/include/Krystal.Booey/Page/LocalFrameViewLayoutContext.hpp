#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Graphics/LayoutSize.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Debug.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class LocalFrame;
  class LocalFrameView;
  class RenderBlock;
  class RenderBlockFlow;
  class RenderBox;
  class RenderLayoutState;
  class RenderElement;
  class RenderView;

  /// @brief Schedules and runs a view's layout: the style updates layout needs first, then the layout of the
  /// render tree. A scheduled layout runs on the layout timer, when the host next runs the event loop.
  class LocalFrameViewLayoutContext final
  {
    friend class LayoutStateMaintainer;
    friend class LayoutStateDisabler;
    friend class SubtreeLayoutStateMaintainer;

    KRYS_NON_COPY_MOVABLE(LocalFrameViewLayoutContext);

  public:
    enum class LayoutPhase : uint8
    {
      OutsideLayout,
      InPreLayout,
      InRenderTreeLayout,
      InViewSizeAdjust,
      InPostLayout
    };

    /// @brief The `text-box-trim` sides in effect for the block container being laid out: whether the first
    /// formatted line's start is trimmed, and the root of the last formatted line (whose end is trimmed).
    struct TextBoxTrim
    {
      bool TrimFirstFormattedLine {false};
      WeakPtr<const RenderBlockFlow> LastFormattedLineRoot;
    };

    /// @brief The blocks whose scroll info update is delayed until the outermost transaction (a flex
    /// container's layout) commits.
    struct UpdateScrollInfoAfterLayoutTransaction
    {
      int32 NestedCount {0};
      // Keyed by address: a block removes itself when it's destroyed.
      ListHashSet<RenderBlock *> Blocks;
    };

  private:
    enum class LayoutNestedState : uint8
    {
      NotInLayout,
      NotNested,
      Nested
    };

    LocalFrameView &_frameView;
    LayoutPhase _layoutPhase {LayoutPhase::OutsideLayout};
    LayoutNestedState _layoutNestedState {LayoutNestedState::NotInLayout};
    Timer _layoutTimer;
    bool _layoutSchedulingIsEnabled {true};
    bool _firstLayout {true};
    bool _setNeedsLayoutWasDeferred {false};
    uint32 _disableSetNeedsLayoutCount {0};
    WeakPtr<RenderElement> _subtreeLayoutRoot;
    bool _needsFullRepaint {true};
    bool _updateCompositingLayersIsPending {false};
    uint32 _paintOffsetCacheDisableCount {0};
    Maybe<TextBoxTrim> _textBoxTrim;
    Maybe<UpdateScrollInfoAfterLayoutTransaction> _updateScrollInfoAfterLayoutTransaction;
    // Keyed by address: flex layout removes each item it adds.
    HashSet<const RenderBox *> _percentHeightIgnoreList;
    List<AnchorScrollAdjuster> _anchorScrollAdjusters;

  public:
    explicit LocalFrameViewLayoutContext(LocalFrameView &frameView) noexcept;
    ~LocalFrameViewLayoutContext() noexcept;

    void Layout() noexcept;
    /// @brief Lays out between style updates, so container queries can read their containers' sizes.
    void InterleavedLayout() noexcept;

    bool UpdateCompositingLayersAfterStyleChange() noexcept;
    void UpdateCompositingLayersAfterLayout() noexcept;
    bool UpdateCompositingLayersAfterLayoutIfNeeded() noexcept;
    KRYS_NODISCARD bool NeedsLayout() const noexcept;

    // We rely on the side-effects of layout, like compositing updates, to update state in various subsystems
    // whose dependencies are poorly defined. This call triggers such updates.
    void SetNeedsLayoutAfterViewConfigurationChange() noexcept;

    void ScheduleLayout() noexcept;
    void UnscheduleLayout() noexcept;
    void ScheduleSubtreeLayout(RenderElement &layoutRoot) noexcept;

    void SetNeedsFullRepaint() noexcept
    {
      _needsFullRepaint = true;
    }
    KRYS_NODISCARD bool NeedsFullRepaint() const noexcept
    {
      return _needsFullRepaint;
    }

    KRYS_NODISCARD RenderLayoutState *LayoutState() const noexcept;
    // Returns true if layoutState should be used for its cached offset and clip.
    KRYS_NODISCARD bool IsPaintOffsetCacheEnabled() const noexcept
    {
      return !_paintOffsetCacheDisableCount && LayoutState();
    }
#if KRYS_DEBUG
    void CheckLayoutState() const noexcept;
#endif
    // layoutDelta is used transiently during layout to store how far an object has moved from its
    // last layout location, in order to repaint correctly.
    // If we're doing a full repaint m_layoutState will be 0, but in that case layoutDelta doesn't matter.
    KRYS_NODISCARD LayoutSize LayoutDelta() const noexcept;
    void AddLayoutDelta(const LayoutSize &delta) noexcept;
#if KRYS_DEBUG
    KRYS_NODISCARD bool LayoutDeltaMatches(const LayoutSize &delta) const noexcept;
#endif
    using LayoutStateStack = List<UniquePtr<RenderLayoutState>>;

    void DisablePercentHeightResolveFor(const RenderBox &flexItem) noexcept;
    void EnablePercentHeightResolveFor(const RenderBox &flexItem) noexcept;
    KRYS_NODISCARD bool IsPercentHeightResolveDisabledFor(const RenderBox &flexItem) const noexcept;

    KRYS_NODISCARD Maybe<TextBoxTrim> GetTextBoxTrim() const noexcept
    {
      return _textBoxTrim;
    }
    void SetTextBoxTrim(Maybe<TextBoxTrim> textBoxTrim) noexcept
    {
      _textBoxTrim = krys::move(textBoxTrim);
    }

    KRYS_NODISCARD UpdateScrollInfoAfterLayoutTransaction &
      GetUpdateScrollInfoAfterLayoutTransaction() noexcept
    {
      if (!_updateScrollInfoAfterLayoutTransaction)
      {
        _updateScrollInfoAfterLayoutTransaction.emplace();
      }
      return *_updateScrollInfoAfterLayoutTransaction;
    }
    KRYS_NODISCARD UpdateScrollInfoAfterLayoutTransaction *
      UpdateScrollInfoAfterLayoutTransactionIfExists() noexcept
    {
      return _updateScrollInfoAfterLayoutTransaction ? &*_updateScrollInfoAfterLayoutTransaction : nullptr;
    }

    /// @brief The renderer the next layout starts from, if it's a subtree layout.
    KRYS_NODISCARD RenderElement *SubtreeLayoutRoot() const noexcept;
    void ClearSubtreeLayoutRoot() noexcept
    {
      _subtreeLayoutRoot = nullptr;
    }
    void ConvertSubtreeLayoutToFullLayout() noexcept;

    void DisableSetNeedsLayout() noexcept;
    void EnableSetNeedsLayout() noexcept;

    KRYS_NODISCARD LayoutPhase GetLayoutPhase() const noexcept
    {
      return _layoutPhase;
    }

    KRYS_NODISCARD bool IsLayoutNested() const noexcept
    {
      return _layoutNestedState == LayoutNestedState::Nested;
    }

    KRYS_NODISCARD bool IsLayoutPending() const noexcept
    {
      return _layoutTimer.IsActive();
    }

    KRYS_NODISCARD bool IsInLayout() const noexcept
    {
      return _layoutPhase != LayoutPhase::OutsideLayout;
    }

    KRYS_NODISCARD bool IsInRenderTreeLayout() const noexcept
    {
      return _layoutPhase == LayoutPhase::InRenderTreeLayout;
    }

    /// @brief Whether the view may paint: not while its render tree is laid out or its size adjusted, nor in
    /// the post-layout tasks.
    /// FIXME: Upstream also paints while running the post-layout tasks asynchronously
    /// (`inAsynchronousTasks`), once they can be.
    KRYS_NODISCARD bool InPaintableState() const noexcept
    {
      return _layoutPhase != LayoutPhase::InRenderTreeLayout && _layoutPhase != LayoutPhase::InViewSizeAdjust
             && _layoutPhase != LayoutPhase::InPostLayout;
    }

    void ResetFirstLayoutFlag() noexcept
    {
      _firstLayout = true;
    }

    KRYS_NODISCARD bool DidFirstLayout() const noexcept
    {
      return !_firstLayout;
    }

    KRYS_NODISCARD LocalFrameView &View() const noexcept
    {
      return _frameView;
    }

    KRYS_NODISCARD LocalFrame &Frame() const noexcept;
    KRYS_NODISCARD dom::Document *Document() const noexcept;
    KRYS_NODISCARD RenderView *GetRenderView() const noexcept;

    KRYS_NODISCARD List<AnchorScrollAdjuster> &AnchorScrollAdjusters() noexcept
    {
      return _anchorScrollAdjusters;
    }
    KRYS_NODISCARD const AnchorScrollAdjuster *
      AnchorScrollAdjusterFor(const RenderBox &anchored) const noexcept;
    AnchorScrollAdjuster::Diff RegisterAnchorScrollAdjuster(AnchorScrollAdjuster &&) noexcept;
    void UnregisterAnchorScrollAdjusterFor(const RenderBox &anchored) noexcept;
    void InvalidateAnchorDependenciesForScroller(const RenderBox &scroller) noexcept;
    void RemoveScrollerFromAnchorScrollAdjusters(const RenderBox &scroller) noexcept;

  private:
    void LayoutTimerFired() noexcept;

    // These functions may only be accessed by LayoutStateMaintainer.
    // Subtree push
    void PushLayoutState(RenderElement &root) noexcept;
    bool PushLayoutState(RenderBox &renderer, const LayoutSize &offset, LayoutUnit pageHeight = 0_lu,
                         bool pageHeightChanged = false) noexcept;
    void PopLayoutState() noexcept;

    // Suspends the LayoutState optimization. Used under transforms that cannot be represented by
    // LayoutState (common in SVG) and when manipulating the render tree during layout in ways
    // that can trigger repaint of a non-child (e.g. when a list item moves its list marker around).
    // Note that even when disabled, LayoutState is still used to store layoutDelta.
    // These functions may only be accessed by LayoutStateMaintainer or LayoutStateDisabler.
    void DisablePaintOffsetCache() noexcept
    {
      _paintOffsetCacheDisableCount++;
    }
    void EnablePaintOffsetCache() noexcept
    {
      krys_debug_assert(_paintOffsetCacheDisableCount > 0);
      _paintOffsetCacheDisableCount--;
    }

    LayoutStateStack _layoutStateStack;

    KRYS_NODISCARD bool NeedsLayoutInternal() const noexcept;

    void PerformLayout() noexcept;
    KRYS_NODISCARD bool CanPerformLayout() const noexcept;

    KRYS_NODISCARD bool IsLayoutSchedulingEnabled() const noexcept
    {
      return _layoutSchedulingIsEnabled;
    }

    void UpdateStyleForLayout() noexcept;

    void SetSubtreeLayoutRoot(RenderElement &layoutRoot) noexcept;
  };
}
