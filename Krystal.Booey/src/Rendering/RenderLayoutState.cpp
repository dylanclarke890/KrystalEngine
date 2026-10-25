#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo
{
  RenderLayoutState::RenderLayoutState(KRYS_MAYBE_UNUSED RenderElement &renderer) noexcept
#if KRYS_DEBUG
      : _renderer(&renderer)
#endif
  {
    // FIXME: Start from the container's absolute position, clipped and scrolled by it when it clips its
    // overflow (localToAbsolute, cachedSizeForOverflowClip, scrollPosition), once there's geometry mapping
    // and scrolling.
    if (_isPaginated)
    {
      // This is just a flag for known page height (see
      // RenderBlockFlow::checkForPaginationLogicalHeightChange).
      _pageLogicalHeight = 1_lu;
    }
  }

  RenderLayoutState::RenderLayoutState(const LocalFrameViewLayoutContext::LayoutStateStack &layoutStateStack,
                                       RenderBox &renderer, const LayoutSize &offset,
                                       LayoutUnit pageLogicalHeight, bool pageLogicalHeightChanged,
                                       Maybe<LineClamp> lineClamp) noexcept
      : _lineClamp(lineClamp)
#if KRYS_DEBUG
        ,
        _renderer(&renderer)
#endif
  {
    if (!layoutStateStack.empty())
    {
      const auto &ancestor = *layoutStateStack.back();
      ComputeOffsets(ancestor, renderer, offset);
      ComputeClipRect(ancestor, renderer);
    }
    ComputePaginationInformation(layoutStateStack, renderer, pageLogicalHeight, pageLogicalHeightChanged);
  }

  void RenderLayoutState::ComputeOffsets(const RenderLayoutState &ancestor,
                                         KRYS_MAYBE_UNUSED RenderBox &renderer, LayoutSize offset) noexcept
  {
    // FIXME: Fixed positioned boxes start from the view's absolute position (localToAbsolute), once there's
    // geometry mapping.
    _paintOffset = ancestor.PaintOffset() + offset;

    // FIXME: Out-of-flow boxes inside an in-flow positioned inline move with it
    // (offsetForInFlowPositionedInline), once there's inline layout.

    _layoutOffset = _paintOffset;

    // FIXME: In-flow positioned boxes with a layer move by the layer's offset, and boxes that clip their
    // overflow by their scroll position, once there are layers and scrolling.

    _layoutDelta = ancestor.LayoutDelta();
#if KRYS_DEBUG
    _layoutDeltaXSaturated = ancestor._layoutDeltaXSaturated;
    _layoutDeltaYSaturated = ancestor._layoutDeltaYSaturated;
#endif
  }

  void RenderLayoutState::ComputeClipRect(const RenderLayoutState &ancestor, RenderBox &renderer) noexcept
  {
    _clipped = !renderer.IsFixedPositioned() && ancestor.IsClipped();
    if (_clipped)
    {
      _clipRect = ancestor.ClipRect();
    }
    if (!renderer.HasNonVisibleOverflow())
    {
      return;
    }

    // FIXME: Intersect with the box's overflow clip at its scroll position (scrollPosition,
    // cachedSizeForOverflowClip), once there's scrolling.
  }

  void RenderLayoutState::ComputePaginationInformation(
    const LocalFrameViewLayoutContext::LayoutStateStack &layoutStateStack, RenderBox &renderer,
    LayoutUnit pageLogicalHeight, bool pageLogicalHeightChanged) noexcept
  {
    const auto *ancestor = layoutStateStack.empty() ? nullptr : layoutStateStack.back().get();
    // If we establish a new page height, then cache the offset to the top of the first page.
    // We can compare this later on to figure out what part of the page we're actually on.
    if (pageLogicalHeight || renderer.IsRenderFragmentedFlow())
    {
      _pageLogicalHeight = pageLogicalHeight;
      bool isFlipped = renderer.Style().GetWritingMode().IsBlockFlipped();
      _pageOffset = LayoutSize(_layoutOffset.Width()
                                 + (!isFlipped ? renderer.BorderLeft() + renderer.PaddingLeft()
                                               : renderer.BorderRight() + renderer.PaddingRight()),
                               _layoutOffset.Height()
                                 + (!isFlipped ? renderer.BorderTop() + renderer.PaddingTop()
                                               : renderer.BorderBottom() + renderer.PaddingBottom()));
      _pageLogicalHeightChanged = pageLogicalHeightChanged;
      _isPaginated = true;
    }
    else if (ancestor)
    {
      // If we don't establish a new page height, then propagate the old page height and offset down.
      _pageLogicalHeight = ancestor->PageLogicalHeight();
      _pageLogicalHeightChanged = ancestor->PageLogicalHeightChanged();
      _pageOffset = ancestor->PageOffset();

      // FIXME: Disable pagination for objects we don't support (isUnsplittableForPagination), and paginate
      // inside fragmented flows (enclosingFragmentedFlow), once there's fragmentation.
      _isPaginated = static_cast<bool>(_pageLogicalHeight);
    }
  }

  LayoutUnit RenderLayoutState::PageLogicalOffset(RenderBox *child,
                                                  LayoutUnit childLogicalOffset) const noexcept
  {
    if (child->IsHorizontalWritingMode())
    {
      return _layoutOffset.Height() + childLogicalOffset - _pageOffset.Height();
    }
    return _layoutOffset.Width() + childLogicalOffset - _pageOffset.Width();
  }

  void RenderLayoutState::AddLayoutDelta(LayoutSize delta) noexcept
  {
    _layoutDelta += delta;
#if KRYS_DEBUG
    _layoutDeltaXSaturated |=
      _layoutDelta.Width() == LayoutUnit::Max() || _layoutDelta.Width() == LayoutUnit::Min();
    _layoutDeltaYSaturated |=
      _layoutDelta.Height() == LayoutUnit::Max() || _layoutDelta.Height() == LayoutUnit::Min();
#endif
  }

#if KRYS_DEBUG
  bool RenderLayoutState::LayoutDeltaMatches(LayoutSize delta) const noexcept
  {
    return (delta.Width() == _layoutDelta.Width() || _layoutDeltaXSaturated)
           && (delta.Height() == _layoutDelta.Height() || _layoutDeltaYSaturated);
  }
#endif

  LayoutStateMaintainer::LayoutStateMaintainer(RenderBox &root, LayoutSize offset,
                                               bool disablePaintOffsetCache, LayoutUnit pageHeight,
                                               bool pageHeightChanged) noexcept
      : _context(root.View().FrameView().LayoutContext()),
        _paintOffsetCacheIsDisabled(disablePaintOffsetCache)
  {
    _didPushLayoutState = _context.PushLayoutState(root, offset, pageHeight, pageHeightChanged);
    if (_didPushLayoutState && _paintOffsetCacheIsDisabled)
    {
      _context.DisablePaintOffsetCache();
    }
  }

  LayoutStateMaintainer::~LayoutStateMaintainer() noexcept
  {
    if (!_didPushLayoutState)
    {
      return;
    }
    _context.PopLayoutState();
    if (_paintOffsetCacheIsDisabled)
    {
      _context.EnablePaintOffsetCache();
    }
  }

  LayoutStateDisabler::LayoutStateDisabler(LocalFrameViewLayoutContext &context) noexcept : _context(context)
  {
    _context.DisablePaintOffsetCache();
  }

  LayoutStateDisabler::~LayoutStateDisabler() noexcept
  {
    _context.EnablePaintOffsetCache();
  }

  namespace
  {
    KRYS_NODISCARD bool ShouldDisablePaintOffsetCacheForSubtree(RenderElement &subtreeLayoutRoot) noexcept
    {
      for (auto *renderer = &subtreeLayoutRoot; renderer; renderer = renderer->Container())
      {
        // FIXME: Reflections too, once there are any.
        if (renderer->IsTransformed())
        {
          return true;
        }
      }
      return false;
    }
  }

  SubtreeLayoutStateMaintainer::SubtreeLayoutStateMaintainer(RenderElement *subtreeLayoutRoot) noexcept
  {
    if (subtreeLayoutRoot)
    {
      _context = &subtreeLayoutRoot->View().FrameView().LayoutContext();
      _context->PushLayoutState(*subtreeLayoutRoot);
      if (ShouldDisablePaintOffsetCacheForSubtree(*subtreeLayoutRoot))
      {
        _context->DisablePaintOffsetCache();
        _didDisablePaintOffsetCache = true;
      }
    }
  }

  SubtreeLayoutStateMaintainer::~SubtreeLayoutStateMaintainer() noexcept
  {
    if (_context)
    {
      _context->PopLayoutState();
      if (_didDisablePaintOffsetCache)
      {
        _context->EnablePaintOffsetCache();
      }
    }
  }

  FlexPercentResolveDisabler::FlexPercentResolveDisabler(LocalFrameViewLayoutContext &layoutContext,
                                                         const RenderBox &flexItem) noexcept
      : _layoutContext(layoutContext), _flexItem(flexItem)
  {
    _layoutContext.DisablePercentHeightResolveFor(flexItem);
  }

  FlexPercentResolveDisabler::~FlexPercentResolveDisabler() noexcept
  {
    _layoutContext.EnablePercentHeightResolveFor(_flexItem);
  }

  ContentVisibilityOverrideScope::ContentVisibilityOverrideScope(LocalFrameViewLayoutContext &layoutContext,
                                                                 EnumSet<OverrideType> overrideTypes) noexcept
      : _layoutContext(layoutContext)
  {
    if (overrideTypes.Contains(OverrideType::Hidden))
    {
      layoutContext.SetIsVisiblityHiddenIgnored(true);
    }
    if (overrideTypes.Contains(OverrideType::Auto))
    {
      layoutContext.SetIsVisiblityAutoIgnored(true);
    }
    if (overrideTypes.Contains(OverrideType::RevealedWhenFound))
    {
      layoutContext.SetIsRevealedWhenFoundIgnored(true);
    }
  }

  ContentVisibilityOverrideScope::~ContentVisibilityOverrideScope() noexcept
  {
    _layoutContext.SetIsVisiblityHiddenIgnored(false);
    _layoutContext.SetIsVisiblityAutoIgnored(false);
    _layoutContext.SetIsRevealedWhenFoundIgnored(false);
  }
}
