#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"

#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/PopoverData.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/PositionedLayoutConstraints.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerCompositor.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"
#include <algorithm>
#include <ranges>

namespace krys::boo
{
  AnchorScrollSnapshot::AnchorScrollSnapshot(const RenderBox &scroller, LayoutPoint snapshot) noexcept
      : Scroller(CreateWeakPtr(&scroller)), ScrollSnapshot(snapshot)
  {
  }

  AnchorScrollSnapshot::AnchorScrollSnapshot(LayoutPoint snapshot) noexcept : ScrollSnapshot(snapshot)
  {
  }

  LayoutSize AnchorScrollSnapshot::AdjustmentForCurrentScrollPosition() const noexcept
  {
    if (Scroller)
    {
      return ScrollSnapshot - LayoutPoint(Scroller->GetScrollPosition());
    }
    return {};
  }

  bool AnchorScrollSnapshot::operator==(const AnchorScrollSnapshot &other) const noexcept
  {
    return Scroller.get() == other.Scroller.get() && ScrollSnapshot == other.ScrollSnapshot;
  }

  bool AnchorScrollAdjuster::IsEmpty() const noexcept
  {
    return _scrollSnapshots.empty() && !(_hasChainedAnchor || _hasStickyAnchor);
  }

  AnchorScrollAdjuster::AnchorScrollAdjuster(RenderBox &anchored,
                                             const RenderBoxModelObject &defaultAnchor) noexcept
      : _anchored(ShareCheckedRef(anchored))
  {
    const auto &style = anchored.Style();

    auto compensatedAxes = style.AnchorFunctionScrollCompensatedAxes();
    _needsXAdjustment = compensatedAxes.Contains(BoxAxis::Horizontal);
    _needsYAdjustment = compensatedAxes.Contains(BoxAxis::Vertical);

    auto containingWritingMode = anchored.Container()->Style().GetWritingMode();
    if (auto positionAreaValue = style.GetPositionArea().TryValue())
    {
      if (positionAreaValue->CoordMatchedTrackForAxis(BoxAxis::Horizontal, containingWritingMode,
                                                      style.GetWritingMode())
          != style::PositionAreaTrack::SpanAll)
      {
        _needsXAdjustment = true;
      }
      else
      {
        if (containingWritingMode.IsHorizontal())
        {
          auto alignment = style.GetJustifySelf();
          _needsXAdjustment |= alignment.IsAuto() || alignment.IsNormal() || alignment.IsAnchorCenter();
        }
        else
        {
          auto alignment = style.GetAlignSelf();
          _needsXAdjustment |= alignment.IsAuto() || alignment.IsNormal() || alignment.IsAnchorCenter();
        }
      }
      if (positionAreaValue->CoordMatchedTrackForAxis(BoxAxis::Vertical, containingWritingMode,
                                                      style.GetWritingMode())
          != style::PositionAreaTrack::SpanAll)
      {
        _needsYAdjustment = true;
      }
      else
      {
        if (containingWritingMode.IsHorizontal())
        {
          auto alignment = style.GetAlignSelf();
          _needsYAdjustment |= alignment.IsAuto() || alignment.IsNormal() || alignment.IsAnchorCenter();
        }
        else
        {
          auto alignment = style.GetJustifySelf();
          _needsYAdjustment |= alignment.IsAuto() || alignment.IsNormal() || alignment.IsAnchorCenter();
        }
      }
    }

    if (!_needsXAdjustment)
    {
      _needsXAdjustment = containingWritingMode.IsHorizontal() ? style.GetJustifySelf().IsAnchorCenter()
                                                               : style.GetAlignSelf().IsAnchorCenter();
    }
    if (!_needsYAdjustment)
    {
      // FIXME: Both branches read align-self; the vertical writing mode one is likely meant to read
      // justify-self.
      _needsYAdjustment = containingWritingMode.IsHorizontal() ? style.GetAlignSelf().IsAnchorCenter()
                                                               : style.GetAlignSelf().IsAnchorCenter();
    }

    _isHidden = style.IsForceHidden();

    _hasStickyAnchor = defaultAnchor.IsStickilyPositioned();
    if (_hasStickyAnchor)
    {
      _stickySnapshot = defaultAnchor.StickyPositionOffset();
    }

    if (const auto *anchorBox = DynamicDowncast<RenderBox>(defaultAnchor))
    {
      if (style::AnchorPositionEvaluator::DefaultAnchorForBox(*anchorBox))
      {
        _hasChainedAnchor = true;
      }
    }
  }

  AnchorScrollAdjuster::AnchorScrollAdjuster(const AnchorScrollAdjuster &) noexcept = default;
  AnchorScrollAdjuster::AnchorScrollAdjuster(AnchorScrollAdjuster &&) noexcept = default;
  AnchorScrollAdjuster::~AnchorScrollAdjuster() noexcept = default;
  AnchorScrollAdjuster &AnchorScrollAdjuster::operator=(const AnchorScrollAdjuster &) noexcept = default;
  AnchorScrollAdjuster &AnchorScrollAdjuster::operator=(AnchorScrollAdjuster &&) noexcept = default;

  RenderBox *AnchorScrollAdjuster::Anchored() const noexcept
  {
    return _anchored.get();
  }

  bool AnchorScrollAdjuster::RecaptureDiffers(const AnchorScrollAdjuster &other) const noexcept
  {
    bool same = _anchored.get() == other._anchored.get() && _scrollSnapshots == other._scrollSnapshots
                && _stickySnapshot == other._stickySnapshot;
    return !same;
  }

  void AnchorScrollAdjuster::AddSnapshot(const RenderBox &scroller) noexcept
  {
    krys_debug_assert(scroller.HasPotentiallyScrollableOverflow() && !scroller.IsRenderView());
    _scrollSnapshots.emplace_back(scroller, LayoutPoint(scroller.ConstrainedScrollPosition()));
  }

  static ScrollPosition ConstrainedViewportScrollPosition(const LocalFrameView &view) noexcept
  {
    auto position = view.ScrollPositionRespectingCustomFixedPosition();
    return view.ConstrainedScrollPosition(ScrollPosition {position.X().ToInt(), position.Y().ToInt()});
  }

  void AnchorScrollAdjuster::AddViewportSnapshot(const RenderView &renderView) noexcept
  {
    auto position = ConstrainedViewportScrollPosition(renderView.FrameView());
    _scrollSnapshots.insert(_scrollSnapshots.begin(), AnchorScrollSnapshot {LayoutPoint(position)});
    _adjustForViewport = true;
  }

  LayoutSize AnchorScrollAdjuster::AdjustmentForViewport(const RenderView &renderView) const noexcept
  {
    if (_adjustForViewport)
    {
      // Viewport snapshot is stored in the first slot.
      krys_debug_assert(!_scrollSnapshots.empty() && !_scrollSnapshots.front().Scroller);
      return _scrollSnapshots.front().ScrollSnapshot
             - LayoutPoint(ConstrainedViewportScrollPosition(renderView.FrameView()));
    }
    return {};
  }

  LayoutSize AnchorScrollAdjuster::AccumulateAdjustments(const RenderView &renderView,
                                                         const RenderBox &anchored) const noexcept
  {
    krys_debug_assert(_anchored.get() == &anchored);
    LayoutSize scrollAdjustment;

    if (_hasChainedAnchor || _hasStickyAnchor)
    {
      if (auto *defaultAnchor = style::AnchorPositionEvaluator::DefaultAnchorForBox(anchored))
      {
        const auto *defaultAnchorBox = DynamicDowncast<RenderBox>(defaultAnchor);
        krys_debug_assert(defaultAnchorBox); // We shouldn't exist if there's no default anchor.
        if (defaultAnchorBox)
        {
          // The anchor may itself be scroll-compensated. Recurse if needed.
          if (const auto *chainedAdjuster =
                renderView.FrameView().LayoutContext().AnchorScrollAdjusterFor(*defaultAnchorBox))
          {
            scrollAdjustment += chainedAdjuster->AccumulateAdjustments(renderView, *defaultAnchorBox);
          }
          // Compensate for sticky adjustments.
          if (_hasStickyAnchor)
          {
            scrollAdjustment += defaultAnchorBox->StickyPositionOffset() - _stickySnapshot;
          }
        }
      }
    }

    for (const auto &snapshot : _scrollSnapshots)
    {
      scrollAdjustment += snapshot.AdjustmentForCurrentScrollPosition();
    }
    scrollAdjustment += AdjustmentForViewport(renderView);

    if (!_needsXAdjustment)
    {
      scrollAdjustment.SetWidth(0_lu);
    }
    if (!_needsYAdjustment)
    {
      scrollAdjustment.SetHeight(0_lu);
    }
    return scrollAdjustment;
  }

  void AnchorScrollAdjuster::SetFallbackLimits(const RenderBox &anchored) noexcept
  {
    auto xConstraints = PositionedLayoutConstraints {
      anchored, anchored.GetWritingMode().IsHorizontal() ? LogicalBoxAxis::Inline : LogicalBoxAxis::Block};
    auto yConstraints = PositionedLayoutConstraints {
      anchored, anchored.GetWritingMode().IsHorizontal() ? LogicalBoxAxis::Block : LogicalBoxAxis::Inline};
    auto xLimits = xConstraints.OriginalContainingRange();
    auto yLimits = yConstraints.OriginalContainingRange();

    auto marginRect = anchored.MarginBoxRect();
    marginRect.MoveBy(anchored.Location());
    _fallbackLimits.Min.SetWidth(xLimits.Min() - marginRect.X());
    _fallbackLimits.Min.SetHeight(yLimits.Min() - marginRect.Y());
    _fallbackLimits.Max.SetWidth(xLimits.Max() - marginRect.MaxX());
    _fallbackLimits.Max.SetHeight(yLimits.Max() - marginRect.MaxY());

    _hasFallback = true;
  }

  bool AnchorScrollAdjuster::InvalidateForScroller(const RenderBox &scroller) noexcept
  {
    bool anchoredNeedsInvalidation = false;
    for (const auto &snapshot : _scrollSnapshots)
    {
      if (snapshot.Scroller.get() == &scroller)
      {
        anchoredNeedsInvalidation = true;
        break;
      }
    }
    if (anchoredNeedsInvalidation)
    {
      _anchored->SetNeedsLayout();
      krys_debug_assert(_anchored->GetElement());
      if (auto *element = _anchored->GetElement())
      {
        element->InvalidateForAnchorRectChange();
      }
    }
    return anchoredNeedsInvalidation;
  }
}

namespace krys::boo::style
{
  static const ScopedName &ImplicitAnchorElementName() noexcept
  {
    // User specified anchor names start with "--".
    static NeverDestroyed<ScopedName> name {ScopedName {css::CSSOMStringAtom(u8"implicit-anchor-element")}};
    return name;
  }

  static BoxAxis MapInsetPropertyToPhysicalAxis(css::PropertyId id, const WritingMode writingMode) noexcept
  {
    switch (id)
    {
      case css::PropertyId::Left:
      case css::PropertyId::Right:  return BoxAxis::Horizontal;
      case css::PropertyId::Top:
      case css::PropertyId::Bottom: return BoxAxis::Vertical;
      case css::PropertyId::InsetInlineStart:
      case css::PropertyId::InsetInlineEnd:
        return MapAxisLogicalToPhysical(writingMode, LogicalBoxAxis::Inline);
      case css::PropertyId::InsetBlockStart:
      case css::PropertyId::InsetBlockEnd:
        return MapAxisLogicalToPhysical(writingMode, LogicalBoxAxis::Block);
      default: krys_debug_assert(false); return BoxAxis::Horizontal;
    }
  }

  static LogicalBoxAxis MapInsetPropertyToLogicalAxis(css::PropertyId id,
                                                      const WritingMode writingMode) noexcept
  {
    switch (id)
    {
      case css::PropertyId::Left:
      case css::PropertyId::Right:
        return writingMode.IsHorizontal() ? LogicalBoxAxis::Inline : LogicalBoxAxis::Block;
      case css::PropertyId::Top:
      case css::PropertyId::Bottom:
        return writingMode.IsHorizontal() ? LogicalBoxAxis::Block : LogicalBoxAxis::Inline;
      case css::PropertyId::InsetInlineStart:
      case css::PropertyId::InsetInlineEnd:   return LogicalBoxAxis::Inline;
      case css::PropertyId::InsetBlockStart:
      case css::PropertyId::InsetBlockEnd:    return LogicalBoxAxis::Block;
      default:                                krys_debug_assert(false); return LogicalBoxAxis::Inline;
    }
  }

  // Physical sides (left/right/top/bottom) can only be used in certain inset properties. "For example, left
  // is usable in left, right, or the logical inset properties that refer to the horizontal axis." See:
  // https://drafts.csswg.org/css-anchor-position-1/#typedef-anchor-side
  static bool AnchorSideMatchesInsetProperty(css::ValueId anchorSideID, BoxAxis physicalAxis) noexcept
  {
    switch (anchorSideID)
    {
      case css::ValueId::Inside:
      case css::ValueId::Outside:
      case css::ValueId::Start:
      case css::ValueId::End:
      case css::ValueId::SelfStart:
      case css::ValueId::SelfEnd:
      case css::ValueId::Center:
      case css::ValueId::Invalid: // percentage
        return true;
      case css::ValueId::Top:
      case css::ValueId::Bottom: return BoxAxis::Vertical == physicalAxis;
      case css::ValueId::Left:
      case css::ValueId::Right:  return BoxAxis::Horizontal == physicalAxis;
      default:                   krys_debug_assert(false); return false;
    }
  }

  // The scroll position, truncated to whole pixels.
  static ScrollPosition TruncatedScrollPosition(LayoutPoint position) noexcept
  {
    return ScrollPosition {position.X().ToInt(), position.Y().ToInt()};
  }

  static ScrollPosition ConstrainedScrollPositionOfView(const LocalFrameView &view) noexcept
  {
    return view.ConstrainedScrollPosition(
      TruncatedScrollPosition(view.ScrollPositionRespectingCustomFixedPosition()));
  }

  static LayoutSize OffsetFromAncestorContainer(const RenderElement &descendantContainer,
                                                const RenderElement &ancestorContainer) noexcept
  {
    LayoutSize offset;
    LayoutPoint referencePoint;
    const RenderElement *currentContainer = &descendantContainer;
    const RenderElement *maxContainer = &ancestorContainer;
    if (const auto *ancestorInline = DynamicDowncast<RenderInline>(&ancestorContainer))
    {
      maxContainer = ancestorInline->ContainingBlock();
    }
    do
    {
      const RenderElement *nextContainer = currentContainer->Container();
      krys_debug_assert(nextContainer); // This means we reached the top without finding container.
      if (!nextContainer)
      {
        break;
      }
      LayoutSize currentOffset = currentContainer->OffsetFromContainer(*nextContainer, referencePoint);

      if (const auto *boxContainer = DynamicDowncast<RenderBox>(*nextContainer))
      {
        // Clamp overscroll so we don't layout into it.
        if (boxContainer->HasPotentiallyScrollableOverflow())
        {
          currentOffset += boxContainer->GetScrollPosition() - boxContainer->ConstrainedScrollPosition();
        }
      }

      offset += currentOffset;
      referencePoint.Move(currentOffset);
      currentContainer = nextContainer;
    } while (currentContainer != maxContainer);

    if (const auto *descendantInline = DynamicDowncast<RenderInline>(&descendantContainer))
    {
      // RenderInline objects do not automatically account for their offset above, so we incorporate this
      // offset here.
      offset += ToLayoutSize(LayoutPoint(descendantInline->LinesBoundingBox().Location()));
    }
    if (descendantContainer.ContainingBlock() == ancestorContainer.ContainingBlock())
    {
      // Account for 'position: relative' inline containing blocks by shifting back down into them.
      if (const auto *ancestorInline = DynamicDowncast<RenderInline>(&ancestorContainer))
      {
        offset -= ToLayoutSize(ancestorInline->FirstInlineBoxTopLeft()); // FIXME: Handle RTL.
      }
    }

    if (const auto *ancestorBox =
          DynamicDowncast<RenderBox>(ancestorContainer)) // Zero out containing block scroll position.
    {
      offset += ToLayoutSize(LayoutPoint(ancestorBox->ConstrainedScrollPosition()));
    }

    return offset;
  }

  // This computes the top left location, physical width, and physical height of the specified anchor element.
  // The location is computed relative to the specified containing block.
  // FIXME: The bounding rectangle of a fragmented anchor's fragments (boundingRectForFragmentedAnchor), once
  // there are fragmented flows.
  LayoutRect
    AnchorPositionEvaluator::ComputeAnchorRectRelativeToContainingBlock(const RenderBoxModelObject &anchorBox,
                                                                        const RenderElement &containingBlock,
                                                                        const RenderBox &anchoredBox) noexcept
  {
    auto anchorWidth = anchorBox.OffsetWidth();
    auto anchorHeight = anchorBox.OffsetHeight();
    auto anchorLocation = ToLayoutPoint(OffsetFromAncestorContainer(anchorBox, containingBlock));

    if (&containingBlock == &containingBlock.View() && anchoredBox.IsFixedPositioned())
    {
      // Handle fixed positioning x scrolling anchor.
      bool isFixedAnchor = false;
      for (const RenderElement *box = &anchorBox; box && box != &containingBlock; box = box->Container())
      {
        if (box->IsFixedPositioned())
        {
          isFixedAnchor = true;
          break;
        }
      }
      if (!isFixedAnchor)
      {
        auto &view = anchorBox.View().FrameView();
        anchorLocation.MoveBy(-LayoutPoint(ConstrainedScrollPositionOfView(view)));
      }
    }

    if (const auto *containingBox = DynamicDowncast<RenderBox>(containingBlock))
    {
      if (containingBox->Style().ShouldPlaceVerticalScrollbarOnLeft())
      {
        anchorLocation.Move(-containingBox->VerticalScrollbarWidth(), 0);
      }
    }

    return LayoutRect(anchorLocation, LayoutSize(anchorWidth, anchorHeight));
  }

  static bool IsInsetPropertyContainerStartSide(css::PropertyId insetPropertyID,
                                                PositionedLayoutConstraints &constraints) noexcept
  {
    switch (insetPropertyID)
    {
      case css::PropertyId::Left:   return constraints.ContainingWritingMode().IsAnyLeftToRight();
      case css::PropertyId::Right:  return !constraints.ContainingWritingMode().IsAnyLeftToRight();
      case css::PropertyId::Top:    return constraints.ContainingWritingMode().IsAnyTopToBottom();
      case css::PropertyId::Bottom: return !constraints.ContainingWritingMode().IsAnyTopToBottom();
      case css::PropertyId::InsetInlineStart:
        return !constraints.SelfWritingMode().IsInlineOpposing(constraints.ContainingWritingMode());
      case css::PropertyId::InsetInlineEnd:
        return constraints.SelfWritingMode().IsInlineOpposing(constraints.ContainingWritingMode());
      case css::PropertyId::InsetBlockStart:
        return !constraints.SelfWritingMode().IsBlockOpposing(constraints.ContainingWritingMode());
      case css::PropertyId::InsetBlockEnd:
        return constraints.SelfWritingMode().IsBlockOpposing(constraints.ContainingWritingMode());
      default: krys_debug_assert(false); return true;
    }
  }

  static css::PropertyId GetOppositeInset(css::PropertyId propertyID) noexcept
  {
    switch (propertyID)
    {
      case css::PropertyId::Left:             return css::PropertyId::Right;
      case css::PropertyId::Right:            return css::PropertyId::Left;
      case css::PropertyId::Top:              return css::PropertyId::Bottom;
      case css::PropertyId::Bottom:           return css::PropertyId::Top;

      case css::PropertyId::InsetInlineStart: return css::PropertyId::InsetInlineEnd;
      case css::PropertyId::InsetInlineEnd:   return css::PropertyId::InsetInlineStart;
      case css::PropertyId::InsetBlockStart:  return css::PropertyId::InsetBlockEnd;
      case css::PropertyId::InsetBlockEnd:    return css::PropertyId::InsetBlockStart;

      default:                                krys_debug_assert(false); return css::PropertyId::InsetBlockStart;
    }
  }

  static Pair<css::PropertyId, bool>
    ApplyTryTacticsToInset(css::PropertyId propertyID, WritingMode writingMode,
                           const BuilderPositionTryFallback &fallback) noexcept
  {
    bool isFlipped = false;
    for (auto tactic : fallback.Tactics)
    {
      switch (tactic)
      {
        case PositionTryFallbackTactic::FlipInline:
          if (LogicalBoxAxis::Inline == MapInsetPropertyToLogicalAxis(propertyID, writingMode))
          {
            propertyID = GetOppositeInset(propertyID);
            isFlipped = true;
          }
          break;
        case PositionTryFallbackTactic::FlipBlock:
          if (LogicalBoxAxis::Block == MapInsetPropertyToLogicalAxis(propertyID, writingMode))
          {
            propertyID = GetOppositeInset(propertyID);
            isFlipped = true;
          }
          break;
        case PositionTryFallbackTactic::FlipX:
          if (BoxAxis::Horizontal == MapInsetPropertyToPhysicalAxis(propertyID, writingMode))
          {
            propertyID = GetOppositeInset(propertyID);
            isFlipped = true;
          }
          break;
        case PositionTryFallbackTactic::FlipY:
          if (BoxAxis::Vertical == MapInsetPropertyToPhysicalAxis(propertyID, writingMode))
          {
            propertyID = GetOppositeInset(propertyID);
            isFlipped = true;
          }
          break;
        case PositionTryFallbackTactic::FlipStart:
          propertyID = [&]
          {
            auto leftIsTop = writingMode.IsAnyLeftToRight() == writingMode.IsAnyTopToBottom();
            switch (propertyID)
            {
              case css::PropertyId::InsetInlineStart: return css::PropertyId::InsetBlockStart;
              case css::PropertyId::InsetInlineEnd:   return css::PropertyId::InsetBlockEnd;
              case css::PropertyId::InsetBlockStart:  return css::PropertyId::InsetInlineStart;
              case css::PropertyId::InsetBlockEnd:    return css::PropertyId::InsetInlineEnd;

              case css::PropertyId::Left:             return leftIsTop ? css::PropertyId::Top : css::PropertyId::Bottom;
              case css::PropertyId::Right:            return leftIsTop ? css::PropertyId::Bottom : css::PropertyId::Top;
              case css::PropertyId::Top:              return leftIsTop ? css::PropertyId::Left : css::PropertyId::Right;
              case css::PropertyId::Bottom:           return leftIsTop ? css::PropertyId::Right : css::PropertyId::Left;

              default:                                krys_debug_assert(false); return css::PropertyId::InsetBlockStart;
            }
          }();
          break;
      }
    }
    return {propertyID, isFlipped};
  }

  // "An anchor() function representing a valid anchor function resolves...to the <length> that would align
  // the edge of the positioned elements' inset-modified containing block corresponding to the property the
  // function appears in with the specified border edge of the target anchor element..." See:
  // https://drafts.csswg.org/css-anchor-position-1/#anchor-pos
  static LayoutUnit ComputeInsetValue(css::PropertyId insetPropertyID, const RenderBoxModelObject &anchorBox,
                                      const RenderBox &anchorPositionedRenderer,
                                      AnchorPositionEvaluator::Side anchorSide,
                                      const Maybe<BuilderPositionTryFallback> &positionTryFallback) noexcept
  {
    auto writingMode = anchorPositionedRenderer.GetWritingMode();
    bool isFlipped = false;
    if (positionTryFallback)
    {
      std::tie(insetPropertyID, isFlipped) =
        ApplyTryTacticsToInset(insetPropertyID, writingMode, *positionTryFallback);
    }

    auto selfLogicalAxis = MapInsetPropertyToLogicalAxis(insetPropertyID, writingMode);
    PositionedLayoutConstraints constraints(anchorPositionedRenderer, selfLogicalAxis);

    auto anchorPercentage = [&]() -> double
    {
      if (const auto *anchorSideID = std::get_if<css::ValueId>(&anchorSide))
      {
        switch (*anchorSideID)
        {
          case css::ValueId::Center:    return 0.5;
          case css::ValueId::Start:     return 0;
          case css::ValueId::End:       return 1;
          case css::ValueId::SelfStart: return constraints.IsOpposing() ? 1 : 0;
          case css::ValueId::SelfEnd:   return constraints.IsOpposing() ? 0 : 1;

          case css::ValueId::Top:       return constraints.ContainingWritingMode().IsAnyTopToBottom() ? 0 : 1;
          case css::ValueId::Bottom:    return constraints.ContainingWritingMode().IsAnyTopToBottom() ? 1 : 0;
          case css::ValueId::Left:      return constraints.ContainingWritingMode().IsAnyLeftToRight() ? 0 : 1;
          case css::ValueId::Right:     return constraints.ContainingWritingMode().IsAnyLeftToRight() ? 1 : 0;

          case css::ValueId::Inside:
            return IsInsetPropertyContainerStartSide(insetPropertyID, constraints) != isFlipped ? 0 : 1;
          case css::ValueId::Outside:
            return IsInsetPropertyContainerStartSide(insetPropertyID, constraints) != isFlipped ? 1 : 0;

          default: krys_debug_assert(false); return 0;
        }
      }
      return std::get<double>(anchorSide);
    }();
    if (constraints.StartIsBefore() == isFlipped)
    {
      anchorPercentage = 1 - anchorPercentage;
    }

    const auto *containingBlock = anchorPositionedRenderer.Container();
    krys_debug_assert(containingBlock);
    auto anchorRect = AnchorPositionEvaluator::ComputeAnchorRectRelativeToContainingBlock(
      anchorBox, *containingBlock, anchorPositionedRenderer);
    auto anchorRange = constraints.ExtractRange(anchorRect);

    auto anchorPosition = anchorRange.Min() + LayoutUnit(anchorRange.Size().ToDouble() * anchorPercentage);
    auto insetValue =
      IsInsetPropertyContainerStartSide(insetPropertyID, constraints) == constraints.StartIsBefore()
        ? anchorPosition - constraints.ContainingRange().Min()
        : constraints.ContainingRange().Max() - anchorPosition;
    return insetValue;
  }

  RenderBoxModelObject *AnchorPositionEvaluator::FindAnchorForAnchorFunctionAndAttemptResolution(
    BuilderState &builderState, Maybe<ScopedName> anchorNameArgument) noexcept
  {
    auto &style = builderState.Style();
    style.SetUsesAnchorFunctions();

    if (!builderState.AnchorPositionedStates())
    {
      return nullptr;
    }

    auto isValid = [&]
    {
      if (!builderState.GetElement())
      {
        return false;
      }

      // FIXME: Support remaining box generating pseudo-elements (like ::marker).
      auto pseudoElement = style.GetPseudoElementType();
      if (pseudoElement && *pseudoElement != PseudoElementType::Before
          && *pseudoElement != PseudoElementType::After)
      {
        return false;
      }

      return true;
    };

    if (!isValid())
    {
      return nullptr;
    }

    Ref<const dom::Element> elementOrHost = ShareRef(*builderState.GetElement());

    // PseudoElement nodes are created on-demand by render tree builder so dont' work as keys here.
    auto &anchorPositionedStates = *builderState.AnchorPositionedStates();
    auto &stateSlot = anchorPositionedStates[{elementOrHost, style.GetPseudoElementIdentifier()}];
    if (!stateSlot)
    {
      stateSlot = CreateUnique<AnchorPositionedState>();
    }
    auto &anchorPositionedState = *stateSlot;

    auto scopedAnchorName = [&]
    {
      if (anchorNameArgument)
      {
        return *anchorNameArgument;
      }
      return DefaultAnchorName(style);
    };

    auto resolvedAnchorName = ResolvedScopedName::CreateFromScopedName(*elementOrHost, scopedAnchorName());

    // Collect anchor names that this element refers to in anchor() or anchor-size()
    bool isNewAnchorName = anchorPositionedState.AnchorNames.insert(resolvedAnchorName).second;

    // If anchor resolution has progressed past FindAnchors, and we pick up a new anchor name, set the stage
    // back to FindAnchors. This restarts the resolution process to resolve newly added names.
    if (isNewAnchorName)
    {
      anchorPositionedState.Stage = AnchorPositionResolutionStage::FindAnchors;
    }

    // An anchor() instance will be ready to be resolved when all referenced anchor-names have been mapped to
    // an actual anchor element in the DOM tree. At that point, we should also have layout information for the
    // anchor-positioned element alongside the anchors referenced by the anchor-positioned element. Until
    // then, we cannot resolve this anchor() instance.
    if (anchorPositionedState.Stage <= AnchorPositionResolutionStage::FindAnchors)
    {
      return nullptr;
    }

    auto anchorPositionedElement = AnchorPositionedElementOrPseudoElement(builderState);

    auto *anchorPositionedRenderer = anchorPositionedElement ? anchorPositionedElement->Renderer() : nullptr;
    if (!anchorPositionedRenderer)
    {
      // If no render tree information is present, the procedure is finished.
      anchorPositionedState.Stage = AnchorPositionResolutionStage::Resolved;
      return nullptr;
    }

    // Anchor value may now be resolved using layout information

    RefPtr<dom::Element> anchorElement;
    if (auto it = anchorPositionedState.AnchorElements.find(resolvedAnchorName);
        it != anchorPositionedState.AnchorElements.end())
    {
      anchorElement = ShareRefPtr(it->second.get());
    }
    if (!anchorElement)
    {
      // See: https://drafts.csswg.org/css-anchor-position-1/#valid-anchor-function
      anchorPositionedState.Stage = AnchorPositionResolutionStage::Resolved;

      return nullptr;
    }

    if (auto it = anchorPositionedStates.find(KeyForElementOrPseudoElement(*anchorElement));
        it != anchorPositionedStates.end())
    {
      // Check if the anchor is itself anchor-positioned but hasn't been positioned yet.
      if (it->second->Stage < AnchorPositionResolutionStage::Positioned)
      {
        return nullptr;
      }
    }

    anchorPositionedState.Stage = AnchorPositionResolutionStage::Resolved;

    return DynamicDowncast<RenderBoxModelObject>(anchorElement->Renderer());
  }

  Maybe<double> AnchorPositionEvaluator::Evaluate(BuilderState &builderState, Maybe<ScopedName> elementName,
                                                  Side side) noexcept
  {
    auto &style = builderState.Style();

    auto propertyID = builderState.CssPropertyId();

    // https://drafts.csswg.org/css-anchor-position-1/#anchor-valid
    auto isValidAnchor = [&]
    {
      // It’s being used in an inset property...
      if (!PropertyAllowsAnchorFunction(propertyID))
      {
        return false;
      }

      // ...on an absolutely-positioned element.
      if (!style.HasOutOfFlowPosition())
      {
        return false;
      }

      // If its <anchor-side> specifies a physical keyword, it’s being used in an inset property in that axis.
      // (For example, left can only be used in left, right, or a logical inset property in the horizontal
      // axis.)
      if (const auto *sideID = std::get_if<css::ValueId>(&side);
          sideID
          && !AnchorSideMatchesInsetProperty(
            *sideID, MapInsetPropertyToPhysicalAxis(propertyID, style.GetWritingMode())))
      {
        return false;
      }

      return true;
    };

    if (!isValidAnchor())
    {
      return null;
    }

    auto *anchorRenderer = FindAnchorForAnchorFunctionAndAttemptResolution(builderState, elementName);
    if (!anchorRenderer)
    {
      return null;
    }

    auto anchorPositionedElement = AnchorPositionedElementOrPseudoElement(builderState);
    if (!anchorPositionedElement)
    {
      return null;
    }

    const auto *anchorPositionedRenderer = DynamicDowncast<RenderBox>(anchorPositionedElement->Renderer());
    if (!anchorPositionedRenderer)
    {
      return null;
    }

    AddAnchorFunctionScrollCompensatedAxis(
      style, *anchorPositionedRenderer, *anchorRenderer,
      MapInsetPropertyToPhysicalAxis(propertyID, style.GetWritingMode()));

    // Proceed with computing the inset value for the specified inset property.
    double insetValue = ComputeInsetValue(propertyID, *anchorRenderer, *anchorPositionedRenderer, side,
                                          builderState.GetPositionTryFallback())
                          .ToDouble();

    // Adjust for CSS `zoom` property and page zoom.
    return insetValue / style.UsedZoom();
  }

  // Returns the default anchor size dimension to use when it is not specified in anchor-size(). This matches
  // the axis of the property that anchor-size() is used in.
  static AnchorSizeDimension DefaultDimensionForPropertyID(css::PropertyId propertyID) noexcept
  {
    switch (propertyID)
    {
      case css::PropertyId::Width:
      case css::PropertyId::MinWidth:
      case css::PropertyId::MaxWidth:
      case css::PropertyId::Left:
      case css::PropertyId::Right:
      case css::PropertyId::MarginLeft:
      case css::PropertyId::MarginRight:       return AnchorSizeDimension::Width;

      case css::PropertyId::Height:
      case css::PropertyId::MinHeight:
      case css::PropertyId::MaxHeight:
      case css::PropertyId::Top:
      case css::PropertyId::Bottom:
      case css::PropertyId::MarginTop:
      case css::PropertyId::MarginBottom:      return AnchorSizeDimension::Height;

      case css::PropertyId::BlockSize:
      case css::PropertyId::MinBlockSize:
      case css::PropertyId::MaxBlockSize:
      case css::PropertyId::InsetBlockStart:
      case css::PropertyId::InsetBlockEnd:
      case css::PropertyId::MarginBlockStart:
      case css::PropertyId::MarginBlockEnd:    return AnchorSizeDimension::Block;

      case css::PropertyId::InlineSize:
      case css::PropertyId::MinInlineSize:
      case css::PropertyId::MaxInlineSize:
      case css::PropertyId::InsetInlineStart:
      case css::PropertyId::InsetInlineEnd:
      case css::PropertyId::MarginInlineStart:
      case css::PropertyId::MarginInlineEnd:   return AnchorSizeDimension::Inline;

      default:
        krys_debug_assert_msg(false,
                              "anchor-size() being used in disallowed CSS property, which should not happen");
        return AnchorSizeDimension::Width;
    }
  }

  // Convert anchor size dimension to the physical dimension (width or height).
  static BoxAxis AnchorSizeDimensionToPhysicalDimension(AnchorSizeDimension dimension,
                                                        const RenderStyle &style,
                                                        const RenderStyle &containerStyle) noexcept
  {
    switch (dimension)
    {
      case AnchorSizeDimension::Width:  return BoxAxis::Horizontal;
      case AnchorSizeDimension::Height: return BoxAxis::Vertical;
      case AnchorSizeDimension::Block:
        return MapAxisLogicalToPhysical(containerStyle.GetWritingMode(), LogicalBoxAxis::Block);
      case AnchorSizeDimension::Inline:
        return MapAxisLogicalToPhysical(containerStyle.GetWritingMode(), LogicalBoxAxis::Inline);
      case AnchorSizeDimension::SelfBlock:
        return MapAxisLogicalToPhysical(style.GetWritingMode(), LogicalBoxAxis::Block);
      case AnchorSizeDimension::SelfInline:
        return MapAxisLogicalToPhysical(style.GetWritingMode(), LogicalBoxAxis::Inline);
    }

    krys_debug_assert(false);
    return BoxAxis::Horizontal;
  }

  Maybe<double> AnchorPositionEvaluator::EvaluateSize(BuilderState &builderState,
                                                      Maybe<ScopedName> elementName,
                                                      Maybe<AnchorSizeDimension> dimension) noexcept
  {
    auto propertyID = builderState.CssPropertyId();
    const auto &style = builderState.Style();

    auto isValidAnchorSize = [&]
    {
      // It’s being used in a sizing property, an inset property, or a margin property...
      if (!PropertyAllowsAnchorSizeFunction(propertyID))
      {
        return false;
      }

      // ...on an absolutely-positioned element.
      if (!style.HasOutOfFlowPosition())
      {
        return false;
      }

      return true;
    };

    if (!isValidAnchorSize())
    {
      return null;
    }

    auto *anchorRenderer = FindAnchorForAnchorFunctionAndAttemptResolution(builderState, elementName);
    if (!anchorRenderer)
    {
      return null;
    }

    // Resolve the dimension (width or height) to return from the anchor positioned element.
    auto anchorPositionedElement = AnchorPositionedElementOrPseudoElement(builderState);
    if (!anchorPositionedElement)
    {
      return null;
    }

    const auto *anchorPositionedRenderer = anchorPositionedElement->Renderer();
    krys_debug_assert(anchorPositionedRenderer);

    const auto *anchorPositionedContainerRenderer = anchorPositionedRenderer->Container();
    krys_debug_assert(anchorPositionedContainerRenderer);

    auto resolvedDimension = dimension.value_or(DefaultDimensionForPropertyID(propertyID));
    auto physicalDimension = AnchorSizeDimensionToPhysicalDimension(
      resolvedDimension, anchorPositionedRenderer->Style(), anchorPositionedContainerRenderer->Style());

    if (const auto &fallback = builderState.GetPositionTryFallback())
    {
      // "For sizing properties, change the specified axis in anchor-size() functions to maintain the same
      // relative relationship to the new direction that they had to the old."
      if (css::Property::IsSizingProperty(propertyID))
      {
        auto swapDimensions = std::ranges::contains(fallback->Tactics, PositionTryFallbackTactic::FlipStart);
        if (swapDimensions)
        {
          physicalDimension = OppositeAxis(physicalDimension);
        }
      }
    }

    // FIXME: The bounding rectangle of a fragmented anchor's fragments (boundingRectForFragmentedAnchor),
    // once there are fragmented flows.
    auto anchorBorderBoundingBox = anchorRenderer->BorderBoundingBox();

    // Adjust for CSS `zoom` property and page zoom.

    switch (physicalDimension)
    {
      case BoxAxis::Horizontal: return anchorBorderBoundingBox.Width().ToDouble() / style.UsedZoom();
      case BoxAxis::Vertical:   return anchorBorderBoundingBox.Height().ToDouble() / style.UsedZoom();
    }

    krys_debug_assert(false);
    return null;
  }

  static const RenderElement *PenultimateContainingBlockChainElement(const RenderElement &descendant,
                                                                     const RenderElement *ancestor) noexcept
  {
    const auto *currentElement = &descendant;
    for (const auto *nextElement = currentElement->Container(); nextElement;
         nextElement = nextElement->Container())
    {
      if (nextElement == ancestor)
      {
        return currentElement;
      }
      currentElement = nextElement;
    }
    return nullptr;
  }

  static bool FirstChildPrecedesSecondChild(const RenderObject *firstChild, const RenderObject *secondChild,
                                            const RenderObject *containingBlock) noexcept
  {
    HashSet<const RenderObject *> firstAncestorChain;

    for (const auto *first = firstChild; first; first = first->Parent())
    {
      firstAncestorChain.insert(first);
      if (first == containingBlock)
      {
        break;
      }
    }

    const auto *second = secondChild;
    for (; second != containingBlock; second = second->Parent())
    {
      if (firstAncestorChain.contains(second->Parent()))
      {
        for (const auto *sibling = second->PreviousSibling(); sibling; sibling = sibling->PreviousSibling())
        {
          if (firstAncestorChain.contains(sibling))
          {
            return true;
          }
        }
        return false;
      }
    }
    return false;
  }

  // Given an element and its anchor name, locate the closest ancestor (*) element that establishes an anchor
  // scope affecting this anchor name, and return the pointer to such element. If no ancestor establishes an
  // anchor scope affecting this name, returns nullptr.
  // (*): an anchor element can also establish an anchor scope containing itself. In this case, the return
  // value is itself.
  static const dom::Element *AnchorScopeForAnchorName(const RenderBoxModelObject &renderer,
                                                      const ResolvedScopedName &anchorName) noexcept
  {
    // Traverse up the composed tree through itself and each ancestor.
    const dom::Element *anchorElement = renderer.GetElement();
    krys_debug_assert(anchorElement);
    for (const auto *currentAncestor = anchorElement; currentAncestor;
         currentAncestor = currentAncestor->ParentElementInComposedTree())
    {
      const auto *currentAncestorStyle = currentAncestor->GetRenderStyle();
      if (!currentAncestorStyle)
      {
        continue;
      }
      const auto &currentAncestorAnchorScope = currentAncestorStyle->AnchorScope();

      if (NameScope::Type::None == currentAncestorAnchorScope.Type)
      {
        continue;
      }

      const auto *styleScope = Scope::ForOrdinal(*currentAncestor, currentAncestorAnchorScope.ScopeOrdinal);
      krys_debug_assert(styleScope);
      if (!styleScope || anchorName.GetScopeIdentifier() != styleScope->Identifier())
      {
        continue;
      }

      if (NameScope::Type::All == currentAncestorAnchorScope.Type
          || currentAncestorAnchorScope.Names.contains(css::CustomIdentifier {anchorName.Name()}))
      {
        return currentAncestor;
      }
    }

    return nullptr;
  }

  enum class TopLayerStatus : uint8
  {
    Same,
    Lower,
    Higher
  };

  static TopLayerStatus ComputeTopLayerStatus(const RenderBox &anchored,
                                              const RenderBoxModelObject &anchor) noexcept
  {
    // Two elements are in the same top layer if they have the same top layer root (including if both are
    // none). An element A is in a higher top layer than an element B if A has a top layer root, and either B
    // has a top layer root earlier in the top layer than A’s, or B doesn’t have a top layer root at all.
    // https://drafts.csswg.org/css-position-4/#top-layer

    if (!anchored.GetDocument().HasTopLayerElement())
    {
      return TopLayerStatus::Same;
    }

    auto topLayerRoot = [&](const RenderObject &renderer) -> const RenderLayerModelObject *
    {
      for (auto *layer = renderer.EnclosingLayer(); layer; layer = layer->Parent())
      {
        if (layer->EstablishesTopLayer())
        {
          return &layer->Renderer();
        }
      }
      return nullptr;
    };

    const auto *anchoredRoot = topLayerRoot(anchored);
    const auto *anchorRoot = topLayerRoot(anchor);
    if (anchoredRoot == anchorRoot)
    {
      return TopLayerStatus::Same;
    }
    if (!anchoredRoot)
    {
      return TopLayerStatus::Lower;
    }
    if (!anchorRoot)
    {
      return TopLayerStatus::Higher;
    }

    for (const auto &topLayerElement : anchored.GetDocument().TopLayerElements())
    {
      if (topLayerElement.get() == anchoredRoot->GetElement())
      {
        return TopLayerStatus::Lower;
      }
      if (topLayerElement.get() == anchorRoot->GetElement())
      {
        return TopLayerStatus::Higher;
      }
    }
    return TopLayerStatus::Lower;
  }

  // See: https://drafts.csswg.org/css-anchor-position-1/#acceptable-anchor-element
  static bool IsAcceptableAnchorElement(const RenderBoxModelObject &anchorRenderer,
                                        const dom::Element &anchorPositionedElement,
                                        const Maybe<ResolvedScopedName> &anchorName = null) noexcept
  {
    // "Possible anchor is either an element or a fully styleable tree-abiding pseudo-element."
    // This always have an associated Element (for ::before/::after it is PseudoElement).
    if (!anchorRenderer.GetElement())
    {
      return false;
    }

    const auto *anchorPositionedRenderer = DynamicDowncast<RenderBox>(anchorPositionedElement.Renderer());
    krys_debug_assert(anchorPositionedRenderer);
    if (!anchorPositionedRenderer)
    {
      return false;
    }

    if (anchorName)
    {
      // Check that anchorRenderer has the specified name.
      krys_debug_assert(std::ranges::any_of(anchorRenderer.Style().GetAnchorNames(),
                                            [&](const auto &scopedName)
                                            { return scopedName.Name == anchorName->Name(); }));

      // The anchor and anchor-positioned element must be in the same scope.
      const auto *anchorScopeElement = AnchorScopeForAnchorName(anchorRenderer, *anchorName);
      const auto *anchorPositionedScopeElement =
        AnchorScopeForAnchorName(*anchorPositionedRenderer, *anchorName);
      if (anchorScopeElement != anchorPositionedScopeElement)
      {
        return false;
      }
    }

    const auto *containingBlock = anchorPositionedRenderer->Container();
    krys_debug_assert(containingBlock);

    // "possible anchor is laid out strictly before positioned el, aka one of the following is true:"
    auto topLayerStatus = ComputeTopLayerStatus(*anchorPositionedRenderer, anchorRenderer);
    switch (topLayerStatus)
    {
      case TopLayerStatus::Higher:
        // "- positioned el is in a higher top layer than possible anchor"
        return true;
      case TopLayerStatus::Same:
      {
        // "- Both elements are in the same top layer..."
        const auto *penultimateElement =
          PenultimateContainingBlockChainElement(anchorRenderer, containingBlock);
        if (!penultimateElement)
        {
          return false;
        }

        if (!penultimateElement->IsOutOfFlowPositioned())
        {
          return true;
        }

        return FirstChildPrecedesSecondChild(penultimateElement, anchorPositionedRenderer, containingBlock);
      }
      case TopLayerStatus::Lower: return false;
    }
    krys_debug_assert(false);
    return false;
  }

  static RefPtr<dom::Element> FindImplicitAnchor(const dom::Element &anchorPositionedElement) noexcept
  {
    auto find = [&]() -> RefPtr<dom::Element>
    {
      // "The implicit anchor element of a pseudo-element is its originating element, unless otherwise
      // specified." https://drafts.csswg.org/css-anchor-position-1/#implicit
      if (const auto *pseudoElement = DynamicDowncast<dom::PseudoElement>(anchorPositionedElement))
      {
        return ShareRefPtr(pseudoElement->HostElement());
      }

      // https://html.spec.whatwg.org/multipage/popover.html#the-popover-attribute
      // 24. Set element's implicit anchor element to invoker.
      if (auto *popoverData = anchorPositionedElement.GetPopoverData())
      {
        return ShareRefPtr<dom::Element>(popoverData->Invoker());
      }

      return nullptr;
    };

    if (auto implicitAnchorElement = find())
    {
      // "If [a spec] defines is an implicit anchor element for query el which is an acceptable anchor element
      // for query el, return that element." https://drafts.csswg.org/css-anchor-position-1/#target
      const auto *anchor = DynamicDowncast<RenderBoxModelObject>(implicitAnchorElement->Renderer());
      if (anchor && IsAcceptableAnchorElement(*anchor, anchorPositionedElement))
      {
        return implicitAnchorElement;
      }
    }

    return nullptr;
  }

  static RefPtr<dom::Element>
    FindLastAcceptableAnchorWithName(const ResolvedScopedName &anchorName,
                                     const dom::Element &anchorPositionedElement,
                                     const AnchorsForAnchorName &anchorsForAnchorName) noexcept
  {
    if (anchorName.Name() == ImplicitAnchorElementName().Name)
    {
      return FindImplicitAnchor(anchorPositionedElement);
    }

    auto it = anchorsForAnchorName.find(anchorName);
    if (it == anchorsForAnchorName.end())
    {
      return nullptr;
    }

    for (const auto *anchor : it->second | std::views::reverse)
    {
      if (IsAcceptableAnchorElement(*anchor, anchorPositionedElement, anchorName))
      {
        return ShareRefPtr(anchor->GetElement());
      }
    }

    return nullptr;
  }

  static AnchorsForAnchorName CollectAnchorsForAnchorName(const dom::Document &document) noexcept
  {
    const auto *renderView = document.GetRenderView();
    if (!renderView)
    {
      return {};
    }

    AnchorsForAnchorName anchorsForAnchorName;

    for (const auto *anchorRenderer : renderView->Anchors())
    {
      const auto *anchorElement = anchorRenderer->GetElement();
      krys_debug_assert(anchorElement);
      if (!anchorElement)
      {
        continue;
      }

      for (const auto &scopedName : anchorRenderer->Style().GetAnchorNames())
      {
        auto resolvedScopedName = ResolvedScopedName::CreateFromScopedName(*anchorElement, scopedName);
        anchorsForAnchorName[resolvedScopedName].push_back(anchorRenderer);
      }
    }

    // Sort them in tree order.
    for (auto &[name, anchors] : anchorsForAnchorName)
    {
      std::ranges::sort(anchors,
                        [](const RenderBoxModelObject *a, const RenderBoxModelObject *b)
                        {
                          // FIXME: Figure out anonymous pseudo-elements.
                          if (!a->GetElement() || !b->GetElement())
                          {
                            return !!b->GetElement();
                          }
                          return std::is_lt(
                            dom::TreeOrder<dom::TreeType::ComposedTree>(*a->GetElement(), *b->GetElement()));
                        });
    }

    return anchorsForAnchorName;
  }

  AnchorElements AnchorPositionEvaluator::FindAnchorsForAnchorPositionedElement(
    const dom::Element &anchorPositionedElement, const HashSet<ResolvedScopedName> &anchorNames,
    const AnchorsForAnchorName &anchorsForAnchorName) noexcept
  {
    AnchorElements anchorElements;

    for (const auto &anchorName : anchorNames)
    {
      auto anchor =
        FindLastAcceptableAnchorWithName(anchorName, anchorPositionedElement, anchorsForAnchorName);
      anchorElements.try_emplace(anchorName, anchor ? CreateWeakPtr(anchor.get()) : WeakPtr<dom::Element> {});
    }

    return anchorElements;
  }

  static inline void ClearAnchorScrollSnapshots(RenderBox &anchored) noexcept
  {
    if (!anchored.Layer()->AnchorScrollAdjustment())
    {
      return;
    }
    anchored.View().FrameView().LayoutContext().UnregisterAnchorScrollAdjusterFor(anchored);
  }

  void AnchorPositionEvaluator::CaptureScrollSnapshots(RenderBox &anchored,
                                                       bool invalidateStyleForScrollPositionChanges) noexcept
  {
    if (!anchored.Layer())
    {
      return;
    }

    auto *defaultAnchor = AnchorPositionEvaluator::DefaultAnchorForBox(anchored);
    if (!defaultAnchor)
    {
      return ClearAnchorScrollSnapshots(anchored);
    }

    AnchorScrollAdjuster adjuster(anchored, *defaultAnchor);
    // Note: We sometimes hit this path during interleaved layout because style bits aren't set yet.
    if (!adjuster.MayNeedAdjustment())
    {
      return ClearAnchorScrollSnapshots(anchored);
    }

    auto *containingBlock = anchored.ContainingBlock();
    krys_debug_assert(defaultAnchor->IsDescendantOf(containingBlock));

    bool isFixedAnchor = defaultAnchor->Layer() && defaultAnchor->Layer()->BehavesAsFixed();
    for (auto *ancestor = defaultAnchor->Container(); ancestor && ancestor != containingBlock;
         ancestor = ancestor->Container())
    {
      if (auto *box = DynamicDowncast<RenderBox>(ancestor))
      {
        if (box->HasPotentiallyScrollableOverflow())
        {
          adjuster.AddSnapshot(*box);
        }
        if (box->Layer() && box->Layer()->BehavesAsFixed())
        {
          isFixedAnchor = true;
        }
      }
    }

    if (anchored.Layer() && anchored.Layer()->BehavesAsFixed() && !isFixedAnchor)
    {
      adjuster.AddViewportSnapshot(anchored.View());
    }

    if (adjuster.IsEmpty())
    {
      return ClearAnchorScrollSnapshots(anchored);
    }

    if (!anchored.Style().GetPositionTryFallbacks().IsNone()
        || anchored.Style().GetPositionVisibility().Contains(PositionVisibilityValue::NoOverflow))
    {
      adjuster.SetFallbackLimits(anchored);
    }

    auto captureDiff =
      anchored.View().FrameView().LayoutContext().RegisterAnchorScrollAdjuster(krys::move(adjuster));
    if (invalidateStyleForScrollPositionChanges && AnchorScrollAdjuster::Diff::SnapshotsDiffer == captureDiff
        && anchored.Style().UsesAnchorFunctions())
    {
      // Scroll positions changed since the last capture, which means anchor() resolution needs updating.
      if (auto *element = anchored.GetElement())
      {
        element->InvalidateForAnchorRectChange();
      }
    }
    (void)anchored.Layer()->SetAnchorScrollAdjustment({});
  }

  void AnchorPositionEvaluator::UpdateScrollAdjustments(RenderView &renderView) noexcept
  {
    // https://drafts.csswg.org/css-anchor-position-1/#scroll
    auto &layoutContext = renderView.FrameView().LayoutContext();
    bool needsRecompositing = false;

    for (const auto &adjuster : layoutContext.AnchorScrollAdjusters())
    {
      auto *anchored = adjuster.Anchored();
      if (!anchored || !anchored->Layer())
      {
        krys_debug_assert(false); // RenderBox failed to clean up.
        continue;
      }

      LayoutSize scrollOffset = adjuster.AccumulateAdjustments(renderView, *anchored);
      if (!anchored->Layer()->SetAnchorScrollAdjustment(scrollOffset))
      {
        continue;
      }

      bool shouldBeHidden = false;
      bool needsInvalidation = false;
      if (adjuster.HasFallbackLimits())
      {
        if (adjuster.ExceedsFallbackLimits(scrollOffset))
        {
          if (!anchored->Style().GetPositionTryFallbacks().IsNone())
          {
            anchored->SetNeedsLayout();
            needsInvalidation = true;
          }
          else
          {
            shouldBeHidden =
              anchored->Style().GetPositionVisibility().Contains(PositionVisibilityValue::NoOverflow);
          }
        }
      }
      if (!shouldBeHidden
          && anchored->Style().GetPositionVisibility().Contains(PositionVisibilityValue::AnchorsVisible))
      {
        shouldBeHidden =
          AnchorPositionEvaluator::IsDefaultAnchorInvisibleOrClippedByInterveningBoxes(*anchored);
      }

      if (needsInvalidation || shouldBeHidden != adjuster.IsHidden())
      {
        krys_debug_assert(anchored->GetElement());
        if (auto *element = anchored->GetElement())
        {
          element->InvalidateForAnchorRectChange(); // FIXME: Optimize this.
        }
      }
      else
      {
        needsRecompositing = true;
      }
    }

    if (needsRecompositing && !layoutContext.IsInLayout())
    {
      renderView.Compositor().ScheduleCompositingLayerUpdate();
    }
  }

  void AnchorPositionEvaluator::UpdateAnchorPositioningStatesAfterInterleavedLayout(
    dom::Document &document, AnchorPositionedStates &anchorPositionedStates) noexcept
  {
    if (anchorPositionedStates.empty())
    {
      return;
    }

    // FIXME: Make the code below oeprate on renderers (boxes) rather than elements.
    auto anchorsForAnchorName = CollectAnchorsForAnchorName(document);

    for (auto &[key, statePtr] : anchorPositionedStates)
    {
      auto &state = *statePtr;

      switch (state.Stage)
      {
        case AnchorPositionResolutionStage::FindAnchors:
        {
          RefPtr<const dom::Element> element = key.first;
          if (key.second)
          {
            element = element->PseudoElementIfExists(*key.second);
          }

          auto *renderer = element ? element->Renderer() : nullptr;
          if (renderer)
          {
            // FIXME: Remove the redundant anchorElements member. The mappings are available in
            // anchorPositionedToAnchorMap.
            state.AnchorElements =
              FindAnchorsForAnchorPositionedElement(*element, state.AnchorNames, anchorsForAnchorName);
            if (IsLayoutTimeAnchorPositioned(renderer->Style()))
            {
              renderer->SetNeedsLayout();
            }

            List<ResolvedAnchor> anchors;
            for (const auto &[anchorName, anchorElement] : state.AnchorElements)
            {
              auto *anchorRenderer =
                anchorElement ? DynamicDowncast<RenderBoxModelObject>(anchorElement->Renderer()) : nullptr;
              anchors.push_back(
                ResolvedAnchor {.Renderer = CreateWeakPtr(anchorRenderer), .Name = anchorName});
            }
            document.StyleScope().AnchorPositionedToAnchorMap().insert_or_assign(
              element.get(), AnchorPositionedToAnchorEntry {
                               .Element = CreateWeakPtr(const_cast<dom::Element *>(element.get())),
                               .PseudoElementIdentifier = key.second,
                               .Anchors = krys::move(anchors)});
          }
          state.Stage = renderer && renderer->Style().UsesAnchorFunctions()
                          ? AnchorPositionResolutionStage::ResolveAnchorFunctions
                          : AnchorPositionResolutionStage::Resolved;
          break;
        }

        case AnchorPositionResolutionStage::ResolveAnchorFunctions:
        case AnchorPositionResolutionStage::Resolved:
          if (auto *anchored = key.first->Renderer())
          {
            if (auto *anchoredBox = DynamicDowncast<RenderBox>(anchored))
            {
              AnchorPositionEvaluator::CaptureScrollSnapshots(*anchoredBox, false);
            }
          }
          state.Stage = AnchorPositionResolutionStage::Positioned;
          break;

        case AnchorPositionResolutionStage::Positioned: break;
      }
    }
  }

  void AnchorPositionEvaluator::UpdateAnchorPositionedStateForDefaultAnchorAndPositionVisibility(
    dom::Element &element, const RenderStyle &style, AnchorPositionedStates &states) noexcept
  {
    auto shouldResolveDefaultAnchor = IsAnchorPositioned(style);

    // `position-visibility: no-overflow` should also work for non-anchor positioned out-of-flow boxes. Create
    // an empty anchor positioning state for it so we perform the required layout interleaving.
    auto hasPositionVisibilityNoOverflow =
      GeneratesBox(style) && style.HasOutOfFlowPosition()
      && style.GetPositionVisibility().Contains(PositionVisibilityValue::NoOverflow);

    if (!shouldResolveDefaultAnchor && !hasPositionVisibilityNoOverflow)
    {
      return;
    }

    auto &state = states[{ShareRefPtr<const dom::Element>(&element), style.GetPseudoElementIdentifier()}];
    if (!state)
    {
      state = CreateUnique<AnchorPositionedState>();
    }

    if (shouldResolveDefaultAnchor)
    {
      // Always resolve the default anchor. Even if nothing is anchored to it we need it to compute the scroll
      // compensation.
      auto resolvedDefaultAnchor =
        ResolvedScopedName::CreateFromScopedName(element, DefaultAnchorName(style));
      if (state->AnchorNames.insert(resolvedDefaultAnchor).second)
      {
        // If anchor resolution has progressed past FindAnchors, and we pick up a new anchor name, set the
        // stage back to FindAnchors. This restarts the resolution process to resolve newly added names.
        state->Stage = AnchorPositionResolutionStage::FindAnchors;
      }
    }
  }

  AnchorToAnchorPositionedMap AnchorPositionEvaluator::MakeAnchorPositionedForAnchorMap(
    AnchorPositionedToAnchorMap &toAnchorMap) noexcept
  {
    AnchorToAnchorPositionedMap map;

    for (auto &[address, entry] : toAnchorMap)
    {
      auto *anchorPositionedElement = entry.Element.get();
      if (!anchorPositionedElement)
      {
        continue;
      }
      for (auto &anchor : entry.Anchors)
      {
        if (!anchor.Renderer)
        {
          continue;
        }
        map[anchor.Renderer.get()].push_back(ShareRef(*anchorPositionedElement));
      }
    }
    return map;
  }

  bool AnchorPositionEvaluator::IsAnchorPositioned(const RenderStyle &style) noexcept
  {
    return IsStyleTimeAnchorPositioned(style) || IsLayoutTimeAnchorPositioned(style);
  }

  bool AnchorPositionEvaluator::IsStyleTimeAnchorPositioned(const RenderStyle &style) noexcept
  {
    if (!GeneratesBox(style) || !style.HasOutOfFlowPosition())
    {
      return false;
    }

    return style.UsesAnchorFunctions();
  }

  bool AnchorPositionEvaluator::IsLayoutTimeAnchorPositioned(const RenderStyle &style) noexcept
  {
    if (!GeneratesBox(style) || !style.HasOutOfFlowPosition())
    {
      return false;
    }

    if (!style.GetPositionArea().IsNone())
    {
      return true;
    }

    return style.GetJustifySelf().IsAnchorCenter() || style.GetAlignSelf().IsAnchorCenter();
  }

  static css::PropertyId FlipHorizontal(css::PropertyId propertyID) noexcept
  {
    switch (propertyID)
    {
      case css::PropertyId::Left:        return css::PropertyId::Right;
      case css::PropertyId::Right:       return css::PropertyId::Left;
      case css::PropertyId::MarginLeft:  return css::PropertyId::MarginRight;
      case css::PropertyId::MarginRight: return css::PropertyId::MarginLeft;
      default:                           return propertyID;
    }
  }

  static css::PropertyId FlipVertical(css::PropertyId propertyID) noexcept
  {
    switch (propertyID)
    {
      case css::PropertyId::Top:          return css::PropertyId::Bottom;
      case css::PropertyId::Bottom:       return css::PropertyId::Top;
      case css::PropertyId::MarginTop:    return css::PropertyId::MarginBottom;
      case css::PropertyId::MarginBottom: return css::PropertyId::MarginTop;
      default:                            return propertyID;
    }
  }

  static css::PropertyId FlipStart(css::PropertyId propertyID, WritingMode writingMode) noexcept
  {
    auto logicalProperty = css::Property::UnresolvePhysicalProperty(propertyID, writingMode);

    auto flippedLogical = [&]
    {
      switch (logicalProperty)
      {
        case css::PropertyId::InsetBlockStart:   return css::PropertyId::InsetInlineStart;
        case css::PropertyId::InsetBlockEnd:     return css::PropertyId::InsetInlineEnd;
        case css::PropertyId::BlockSize:         return css::PropertyId::InlineSize;
        case css::PropertyId::MinBlockSize:      return css::PropertyId::MinInlineSize;
        case css::PropertyId::MaxBlockSize:      return css::PropertyId::MaxInlineSize;
        case css::PropertyId::InsetInlineStart:  return css::PropertyId::InsetBlockStart;
        case css::PropertyId::InsetInlineEnd:    return css::PropertyId::InsetBlockEnd;
        case css::PropertyId::InlineSize:        return css::PropertyId::BlockSize;
        case css::PropertyId::MinInlineSize:     return css::PropertyId::MinBlockSize;
        case css::PropertyId::MaxInlineSize:     return css::PropertyId::MaxBlockSize;
        case css::PropertyId::MarginBlockStart:  return css::PropertyId::MarginInlineStart;
        case css::PropertyId::MarginBlockEnd:    return css::PropertyId::MarginInlineEnd;
        case css::PropertyId::MarginInlineStart: return css::PropertyId::MarginBlockStart;
        case css::PropertyId::MarginInlineEnd:   return css::PropertyId::MarginBlockEnd;
        case css::PropertyId::AlignSelf:         return css::PropertyId::JustifySelf;
        case css::PropertyId::JustifySelf:       return css::PropertyId::AlignSelf;
        default:                                 return propertyID;
      }
    };
    return css::Property::ResolveDirectionAwareProperty(flippedLogical(), writingMode);
  }

  css::PropertyId AnchorPositionEvaluator::ResolvePositionTryFallbackProperty(
    css::PropertyId propertyID, WritingMode writingMode, const BuilderPositionTryFallback &fallback) noexcept
  {
    krys_debug_assert(!css::Property::IsDirectionAwareProperty(propertyID));

    for (auto tactic : fallback.Tactics)
    {
      switch (tactic)
      {
        case PositionTryFallbackTactic::FlipInline:
          propertyID = writingMode.IsHorizontal() ? FlipHorizontal(propertyID) : FlipVertical(propertyID);
          break;
        case PositionTryFallbackTactic::FlipBlock:
          propertyID = writingMode.IsHorizontal() ? FlipVertical(propertyID) : FlipHorizontal(propertyID);
          break;
        case PositionTryFallbackTactic::FlipX:     propertyID = FlipHorizontal(propertyID); break;
        case PositionTryFallbackTactic::FlipY:     propertyID = FlipVertical(propertyID); break;
        case PositionTryFallbackTactic::FlipStart: propertyID = FlipStart(propertyID, writingMode); break;
      }
    }
    return propertyID;
  }

  css::ValueId AnchorPositionEvaluator::ResolvePositionTryFallbackValueForSelfPosition(
    css::PropertyId propertyID, css::ValueId position, WritingMode writingMode,
    const BuilderPositionTryFallback &fallback) noexcept
  {
    // Implements the bullet starting "For the self-alignment properties" from step 4 of
    // https://drafts.csswg.org/css-anchor-position-1/#swap-due-to-a-try-tactic.

    krys_debug_assert(propertyID == css::PropertyId::AlignSelf || propertyID == css::PropertyId::JustifySelf);

    auto flipSidedPosition = [](css::ValueId position) -> css::ValueId
    {
      // Swap to the "opposite" position if the current position is "sided".
      // If there is no opposite value, nothing is changed.
      switch (position)
      {
        case css::ValueId::Start:     return css::ValueId::End;
        case css::ValueId::End:       return css::ValueId::Start;
        case css::ValueId::SelfStart: return css::ValueId::SelfEnd;
        case css::ValueId::SelfEnd:   return css::ValueId::SelfStart;
        case css::ValueId::FlexStart: return css::ValueId::FlexEnd;
        case css::ValueId::FlexEnd:   return css::ValueId::FlexStart;
        case css::ValueId::Left:      return css::ValueId::Right;
        case css::ValueId::Right:     return css::ValueId::Left;
        default:                      return position;
      }
    };

    auto flipStart = [](WritingMode writingMode, css::ValueId position) -> css::ValueId
    {
      // `justify-self` additionally takes `left`/`right`, `align-self` doesn't. When applying `flip-start`,
      // `justify-self` gets swapped with `align-self`. So if we're resolving `justify-self` (which later gets
      // swapped with `align-self`), and the position is `left`/`right`, resolve it to
      // `self-start`/`self-end`.
      switch (position)
      {
        case css::ValueId::Left:
          return writingMode.IsBidiLTR() ? css::ValueId::SelfStart : css::ValueId::SelfEnd;
        case css::ValueId::Right:
          return writingMode.IsBidiLTR() ? css::ValueId::SelfEnd : css::ValueId::SelfStart;
        default: return position;
      }
    };

    for (auto tactic : fallback.Tactics)
    {
      switch (tactic)
      {
        case PositionTryFallbackTactic::FlipBlock:
          if (propertyID == css::PropertyId::AlignSelf)
          {
            position = flipSidedPosition(position);
          }
          break;
        case PositionTryFallbackTactic::FlipInline:
          if (propertyID == css::PropertyId::JustifySelf)
          {
            position = flipSidedPosition(position);
          }
          break;
        case PositionTryFallbackTactic::FlipX:
          if (propertyID
              == (writingMode.IsHorizontal() ? css::PropertyId::JustifySelf : css::PropertyId::AlignSelf))
          {
            position = flipSidedPosition(position);
          }
          break;
        case PositionTryFallbackTactic::FlipY:
          if (propertyID
              == (writingMode.IsHorizontal() ? css::PropertyId::AlignSelf : css::PropertyId::JustifySelf))
          {
            position = flipSidedPosition(position);
          }
          break;
        case PositionTryFallbackTactic::FlipStart:
          if (propertyID == css::PropertyId::JustifySelf)
          {
            position = flipStart(writingMode, position);
          }
          break;
      }
    }

    return position;
  }

  bool AnchorPositionEvaluator::OverflowsInsetModifiedContainingBlock(const RenderBox &anchoredBox) noexcept
  {
    if (!anchoredBox.IsOutOfFlowPositioned())
    {
      return false;
    }

    auto inlineConstraints = PositionedLayoutConstraints {anchoredBox, LogicalBoxAxis::Inline};
    auto blockConstraints = PositionedLayoutConstraints {anchoredBox, LogicalBoxAxis::Block};
    inlineConstraints.ComputeInsets();
    blockConstraints.ComputeInsets();

    auto anchorInlineSize = anchoredBox.LogicalWidth() + anchoredBox.MarginStart() + anchoredBox.MarginEnd();
    auto anchorBlockSize =
      anchoredBox.LogicalHeight() + anchoredBox.MarginBefore() + anchoredBox.MarginAfter();

    return inlineConstraints.InsetModifiedContainingSize() < anchorInlineSize
           || blockConstraints.InsetModifiedContainingSize() < anchorBlockSize;
  }

  bool AnchorPositionEvaluator::IsDefaultAnchorInvisibleOrClippedByInterveningBoxes(
    const RenderBox &anchoredBox) noexcept
  {
    const auto *defaultAnchor = DefaultAnchorForBox(anchoredBox);
    if (!defaultAnchor)
    {
      return false;
    }

    if (defaultAnchor->Style().UsedVisibility() == Visibility::Hidden)
    {
      return true;
    }

    const auto *anchorBox = DynamicDowncast<RenderBox>(*defaultAnchor);

    // https://drafts.csswg.org/css-anchor-position-1/#position-visibility
    // "An anchor box anchor is clipped by intervening boxes relative to a positioned box abspos relying on it
    // if anchor’s ink overflow rectangle is fully clipped by a box which is an ancestor of anchor but a
    // descendant of abspos’s containing block."

    auto localAnchorRect = [&]
    {
      if (anchorBox)
      {
        return anchorBox->VisualOverflowRect();
      }
      return Downcast<RenderInline>(*defaultAnchor).LinesVisualOverflowBoundingBox();
    }();
    const auto *anchoredContainingBlock = anchoredBox.Container();

    auto anchorRect =
      defaultAnchor->LocalToAbsoluteQuad(FloatQuad {FloatRect {localAnchorRect}}).BoundingBox();

    for (const auto *anchorAncestor = defaultAnchor->Container();
         anchorAncestor && anchorAncestor != anchoredContainingBlock;
         anchorAncestor = anchorAncestor->Container())
    {
      if (!anchorAncestor->HasNonVisibleOverflow())
      {
        continue;
      }
      const auto *clipAncestor = DynamicDowncast<RenderBox>(*anchorAncestor);
      if (!clipAncestor)
      {
        continue;
      }
      auto localClipRect = clipAncestor->OverflowClipRect({});
      auto clipRect = clipAncestor->LocalToAbsoluteQuad(FloatQuad {FloatRect {localClipRect}}).BoundingBox();
      if (!clipRect.Intersects(anchorRect))
      {
        return true;
      }
    }

    if (anchorBox)
    {
      // Test for chained anchors.
      if (IsDefaultAnchorInvisibleOrClippedByInterveningBoxes(*anchorBox))
      {
        return true;
      }
    }

    return false;
  }

  // FIXME: The code should operate fully on host/pseudoElementIdentifier pairs and not use PseudoElements to
  // support pseudo-elements other than ::before/::after.
  RefPtr<const dom::Element>
    AnchorPositionEvaluator::AnchorPositionedElementOrPseudoElement(BuilderState &builderState) noexcept
  {
    const auto *element = builderState.GetElement();
    if (auto identifier = builderState.Style().GetPseudoElementIdentifier())
    {
      RefPtr<const dom::Element> pseudoElement = element->PseudoElementIfExists(*identifier);
      return pseudoElement;
    }
    return ShareRefPtr(element);
  }

  AnchorPositionedKey
    AnchorPositionEvaluator::KeyForElementOrPseudoElement(const dom::Element &element) noexcept
  {
    if (const auto *pseudoElement = DynamicDowncast<dom::PseudoElement>(element))
    {
      return {ShareRefPtr<const dom::Element>(pseudoElement->HostElement()),
              PseudoElementIdentifier {pseudoElement->GetPseudoElementType()}};
    }
    return {ShareRefPtr(&element), null};
  }

  bool AnchorPositionEvaluator::IsAnchor(const RenderStyle &style) noexcept
  {
    if (!style.GetAnchorNames().IsNone())
    {
      return true;
    }

    return IsImplicitAnchor(style);
  }

  bool AnchorPositionEvaluator::IsImplicitAnchor(const RenderStyle &style) noexcept
  {
    // The invoker is an implicit anchor for the popover.
    // https://drafts.csswg.org/css-anchor-position-1/#implicit
    if (style.IsPopoverInvoker())
    {
      return true;
    }

    // "The implicit anchor element of a pseudo-element is its originating element, unless otherwise
    // specified." https://drafts.csswg.org/css-anchor-position-1/#implicit
    auto isImplicitAnchorForPseudoElement = [&](PseudoElementType pseudoElementType)
    {
      const auto *pseudoElementStyle = style.GetCachedPseudoStyle({pseudoElementType});
      if (!pseudoElementStyle)
      {
        return false;
      }
      // If we have an explicit anchor name then there is no need for an implicit anchor.
      if (!pseudoElementStyle->GetPositionAnchor().IsAuto())
      {
        return false;
      }

      return pseudoElementStyle->UsesAnchorFunctions() || IsLayoutTimeAnchorPositioned(*pseudoElementStyle);
    };
    return isImplicitAnchorForPseudoElement(PseudoElementType::Before)
           || isImplicitAnchorForPseudoElement(PseudoElementType::After);
  }

  ScopedName AnchorPositionEvaluator::DefaultAnchorName(const RenderStyle &style) noexcept
  {
    if (auto name = style.GetPositionAnchor().TryName())
    {
      return *name;
    }
    return ImplicitAnchorElementName();
  }

  RenderBoxModelObject *AnchorPositionEvaluator::DefaultAnchorForBox(const RenderBox &box) noexcept
  {
    auto *element = box.GetElement();
    if (!element)
    {
      return nullptr;
    }

    auto &anchorPositionedMap = box.GetDocument().StyleScope().AnchorPositionedToAnchorMap();
    auto it = anchorPositionedMap.find(element);
    if (it == anchorPositionedMap.end() || it->second.Element.get() != element)
    {
      return nullptr;
    }

    auto anchorName = ResolvedScopedName::CreateFromScopedName(*element, DefaultAnchorName(box.Style()));

    for (auto &anchor : it->second.Anchors)
    {
      if (anchorName == anchor.Name)
      {
        return anchor.Renderer.get();
      }
    }
    return nullptr;
  }

  HashMap<AnchorPositionedKey, size_t, AnchorPositionedKeyHash>
    AnchorPositionEvaluator::RecordLastSuccessfulPositionOptions(
      const HashSet<const RenderBox *> &positionTryBoxes) noexcept
  {
    HashMap<AnchorPositionedKey, size_t, AnchorPositionedKeyHash> lastSuccessfulPositionOptionMap;

    for (const auto *positionTryBox : positionTryBoxes)
    {
      auto styleable = Styleable::FromRenderer(*positionTryBox);
      krys_debug_assert(styleable);
      if (!styleable)
      {
        continue;
      }

      if (auto usedPositionOptionIndex = positionTryBox->Style().UsedPositionOptionIndex())
      {
        lastSuccessfulPositionOptionMap.emplace(
          AnchorPositionedKey {ShareRefPtr<const dom::Element>(&styleable->Element),
                               styleable->PseudoElementIdentifier},
          *usedPositionOptionIndex);
      }
    }

    return lastSuccessfulPositionOptionMap;
  }

  void AnchorPositionEvaluator::AddAnchorFunctionScrollCompensatedAxis(RenderStyle &style,
                                                                       const RenderBox &anchored,
                                                                       const RenderBoxModelObject &anchor,
                                                                       BoxAxis axis) noexcept
  {
    // https://drafts.csswg.org/css-anchor-position-1/#scroll
    // An absolutely positioned box abspos compensates for scroll in the horizontal or vertical axis if both
    // of the following conditions are true:
    // - abspos has a default anchor box.
    auto *defaultAnchor = DefaultAnchorForBox(anchored);
    if (!defaultAnchor)
    {
      return;
    }

    // - at least one anchor() function on abspos’s used inset properties in the axis refers to a target
    // anchor
    //   element with the same nearest scroll container ancestor as abspos’s default anchor box.
    if (defaultAnchor != &anchor
        && defaultAnchor->EnclosingScrollableContainer() != anchor.EnclosingScrollableContainer())
    {
      return;
    }

    auto axes = style.AnchorFunctionScrollCompensatedAxes();
    axes.Add(axis);
    style.SetAnchorFunctionScrollCompensatedAxes(axes);
  }
}
