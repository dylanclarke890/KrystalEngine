#include "Krystal.Booey/Animation/ViewTimeline.hpp"
#include "Krystal.Booey/Animation/AnimationTimelinesController.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Timeline.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Page/Scrolling/ScrollingConstraints.hpp"
#include "Krystal.Booey/Platform/ScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGModelObject.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationRange.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperDeprecatedCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/ScrollSnap/ScrollPadding.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <utility>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool IsValidInset(const RefPtr<css::PrimitiveValue> &inset) noexcept
    {
      return !inset || inset->ValueId() == css::ValueId::Auto || inset->IsLength() || inset->IsPercentage();
    }
  }

  dom::ExceptionOr<Ref<ViewTimeline>> ViewTimeline::Create(dom::Document &document,
                                                           ViewTimelineOptions &&options) noexcept
  {
    auto viewTimeline = AdoptRef(*new ViewTimeline(options.Axis));

    auto specifiedInsetsOrException = viewTimeline->ValidateSpecifiedInsets(options.Inset, document);
    if (specifiedInsetsOrException.HasException())
    {
      return specifiedInsetsOrException.ReleaseException();
    }

    auto specifiedInsets = specifiedInsetsOrException.ReleaseValue();
    if (!IsValidInset(specifiedInsets.Start) || !IsValidInset(specifiedInsets.End))
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    viewTimeline->_specifiedInsets = krys::move(specifiedInsets);
    viewTimeline->SetSubject(options.Subject.get());
    if (RefPtr subject = options.Subject)
    {
      (void)subject->NodeDocument().UpdateLayoutIgnorePendingStylesheets();
    }
    viewTimeline->CacheCurrentTime();

    return viewTimeline;
  }

  Ref<ViewTimeline> ViewTimeline::Create(const css::CSSOMStringAtom &name, ScrollAxis axis,
                                         const style::ViewTimelineInsetItem &insets) noexcept
  {
    return AdoptRef(*new ViewTimeline(name, axis, insets));
  }

  ViewTimeline::ViewTimeline(ScrollAxis axis) noexcept
      : ScrollTimeline(css::CSSOMStringAtom::Null(), axis), _insets(css::keywords::Auto {})
  {
  }

  ViewTimeline::ViewTimeline(const css::CSSOMStringAtom &name, ScrollAxis axis,
                             const style::ViewTimelineInsetItem &insets) noexcept
      : ScrollTimeline(name, axis), _insets(insets)
  {
  }

  auto ViewTimeline::ValidateSpecifiedInsets(const ViewTimelineInsetValue &inset,
                                             const dom::Document &document) noexcept
    -> dom::ExceptionOr<SpecifiedViewTimelineInsets>
  {
    // https://drafts.csswg.org/scroll-animations-1/#dom-viewtimeline-viewtimeline

    // FIXME: note that we use CSSKeywordish instead of CSSKeywordValue to match Chrome, issue being tracked
    // at https://github.com/w3c/csswg-drafts/issues/11477.

    // If a DOMString value is provided as an inset, parse it as a <'view-timeline-inset'> value;
    if (const auto *insetString = std::get_if<dom::DOMString>(&inset))
    {
      if (insetString->empty())
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      auto consumedInset = css::PropertyParserHelpers::ParseSingleViewTimelineInsetItem(
        *insetString, document.CssParserContext());
      if (!consumedInset)
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }

      if (RefPtr insetPair = ShareRefPtr(DynamicDowncast<css::ValuePair>(consumedInset.get())))
      {
        return SpecifiedViewTimelineInsets {
          ShareRefPtr(DynamicDowncast<css::PrimitiveValue>(insetPair->First())),
          ShareRefPtr(DynamicDowncast<css::PrimitiveValue>(insetPair->Second())),
        };
      }
      return SpecifiedViewTimelineInsets {
        ShareRefPtr(DynamicDowncast<css::PrimitiveValue>(consumedInset.get())), nullptr};
    }

    auto cssPrimitiveValueForCSSNumericValue =
      [&](const RefPtr<css::CSSNumericValue> &numericValue) -> dom::ExceptionOr<RefPtr<css::PrimitiveValue>>
    {
      if (const auto *insetValue = DynamicDowncast<css::CSSUnitValue>(numericValue.get()))
      {
        auto cssValue = insetValue->ToCSSValue();
        return ShareRefPtr(DynamicDowncast<css::PrimitiveValue>(cssValue.get()));
      }
      return RefPtr<css::PrimitiveValue> {};
    };

    auto cssPrimitiveValueForCSSKeywordValue =
      [&](const Ref<css::CSSKeywordValue> &keywordValue) -> dom::ExceptionOr<RefPtr<css::PrimitiveValue>>
    {
      if (keywordValue->Value() != u8"auto")
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      return RefPtr<css::PrimitiveValue> {};
    };

    auto cssPrimitiveValueForIndividualInset =
      [&](const ViewTimelineIndividualInset &individualInset) -> dom::ExceptionOr<RefPtr<css::PrimitiveValue>>
    {
      if (const auto *numericInset = std::get_if<RefPtr<css::CSSNumericValue>>(&individualInset))
      {
        return cssPrimitiveValueForCSSNumericValue(*numericInset);
      }
      if (const auto *stringInset = std::get_if<dom::DOMString>(&individualInset))
      {
        return cssPrimitiveValueForCSSKeywordValue(css::CSSKeywordValue::RectifyKeywordish(*stringInset));
      }
      krys_debug_assert(std::holds_alternative<RefPtr<css::CSSKeywordValue>>(individualInset));
      return cssPrimitiveValueForCSSKeywordValue(
        css::CSSKeywordValue::RectifyKeywordish(std::get<RefPtr<css::CSSKeywordValue>>(individualInset)));
    };

    // if a sequence is provided, the first value represents the start inset and the second value represents
    // the end inset. If the sequence has only one value, it is duplicated. If it has zero values or more than
    // two values, or if it contains a CSSKeywordValue whose value is not "auto", throw a TypeError.
    const auto &insetList = std::get<List<ViewTimelineIndividualInset>>(inset);
    auto numberOfInsets = insetList.size();

    if (!numberOfInsets || numberOfInsets > 2)
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    auto startInsetOrException = cssPrimitiveValueForIndividualInset(insetList.at(0));
    if (startInsetOrException.HasException())
    {
      return startInsetOrException.ReleaseException();
    }
    auto startInset = startInsetOrException.ReleaseValue();

    if (numberOfInsets == 1)
    {
      return SpecifiedViewTimelineInsets {startInset, startInset};
    }

    auto endInsetOrException = cssPrimitiveValueForIndividualInset(insetList.at(1));
    if (endInsetOrException.HasException())
    {
      return endInsetOrException.ReleaseException();
    }
    auto endInset = endInsetOrException.ReleaseValue();
    return SpecifiedViewTimelineInsets {startInset, endInset};
  }

  const dom::Element *ViewTimeline::Subject() const noexcept
  {
    if (auto subject = _subject.GetStyleable())
    {
      return &subject->Element;
    }
    return nullptr;
  }

  void ViewTimeline::SetSubject(dom::Element *subject) noexcept
  {
    if (subject)
    {
      SetSubject(style::Styleable::FromElement(*subject));
    }
    else
    {
      RemoveTimelineFromDocument(_subject.GetElement().get());
      _subject = style::WeakStyleable();
    }
  }

  void ViewTimeline::SetSubject(const style::Styleable &styleable) noexcept
  {
    if (_subject == style::WeakStyleable(styleable))
    {
      return;
    }

    RefPtr previousSubject = ShareRefPtr(_subject.GetElement().get());
    _subject = styleable;

    if (previousSubject && &previousSubject->NodeDocument() == &styleable.Element.NodeDocument())
    {
      return;
    }

    RemoveTimelineFromDocument(previousSubject.get());

    styleable.Element.NodeDocument().EnsureTimelinesController().AddTimeline(*this);
  }

  AnimationTimelinesController *ViewTimeline::Controller() const noexcept
  {
    if (auto subject = _subject.GetStyleable())
    {
      return &subject->Element.NodeDocument().EnsureTimelinesController();
    }
    return nullptr;
  }

  StickinessAdjustmentData StickinessAdjustmentData::ComputeStickinessAdjustmentData(
    const StickyPositionViewportConstraints &constraints,
    ScrollTimeline::ResolvedScrollDirection scrollDirection, float scrollContainerSize, float subjectSize,
    float subjectOffset) noexcept
  {
    // For a sticky container, determine the amount of adjustment that is possible, which is the distance from
    // the edge of the sticky container to the edge of its containing block. We also need to determine where
    // the subject element is relative to the scroller when the stickiness occurs, so that we can properly
    // adjust the start and end of the range, as well as for a specific animation-range.

    StickinessAdjustmentData data;

    auto computeSubjectStickinessLocation = [](float stickyBoxStuckPosition, float stickyBoxStaticPosition,
                                               float scrollContainerSize, float subjectSize,
                                               float subjectOffset)
    {
      float subjectPositionInScroller = stickyBoxStuckPosition + subjectOffset - stickyBoxStaticPosition;
      if (subjectPositionInScroller > scrollContainerSize)
      {
        return StickinessLocation::BeforeEntry;
      }
      if (subjectPositionInScroller + subjectSize > scrollContainerSize)
      {
        return StickinessLocation::DuringEntry;
      }
      if (subjectPositionInScroller + subjectSize < 0)
      {
        return StickinessLocation::AfterExit;
      }
      if (subjectPositionInScroller < 0)
      {
        return StickinessLocation::DuringExit;
      }
      return StickinessLocation::WhileContained;
    };

    if (scrollDirection.IsVertical)
    {
      if (constraints.HasAnchorEdge(AnchorEdge::Top))
      {
        data.StickyTopOrLeftAdjustment =
          constraints.ContainingBlockRect().MaxY() - constraints.StickyBoxRect().MaxY();
        data.TopOrLeftAdjustmentLocation =
          computeSubjectStickinessLocation(constraints.TopOffset(), constraints.StickyBoxRect().Y(),
                                           scrollContainerSize, subjectSize, subjectOffset);
      }
      if (constraints.HasAnchorEdge(AnchorEdge::Bottom))
      {
        data.StickyBottomOrRightAdjustment =
          constraints.ContainingBlockRect().Y() - constraints.StickyBoxRect().Y();
        data.BottomOrRightAdjustmentLocation = computeSubjectStickinessLocation(
          scrollContainerSize - constraints.BottomOffset() - constraints.StickyBoxRect().Height(),
          constraints.StickyBoxRect().Y(), scrollContainerSize, subjectSize, subjectOffset);
      }
    }
    else
    {
      if (constraints.HasAnchorEdge(AnchorEdge::Left))
      {
        data.StickyTopOrLeftAdjustment =
          constraints.ContainingBlockRect().MaxX() - constraints.StickyBoxRect().MaxX();
        data.TopOrLeftAdjustmentLocation =
          computeSubjectStickinessLocation(constraints.LeftOffset(), constraints.StickyBoxRect().X(),
                                           scrollContainerSize, subjectSize, subjectOffset);
      }
      if (constraints.HasAnchorEdge(AnchorEdge::Right))
      {
        data.StickyBottomOrRightAdjustment =
          constraints.ContainingBlockRect().X() - constraints.StickyBoxRect().X();
        data.BottomOrRightAdjustmentLocation = computeSubjectStickinessLocation(
          scrollContainerSize - constraints.RightOffset() - constraints.StickyBoxRect().Width(),
          constraints.StickyBoxRect().X(), scrollContainerSize, subjectSize, subjectOffset);
      }
    }
    return data;
  }

  float StickinessAdjustmentData::EntryDistanceAdjustment() const noexcept
  {
    float entryDistanceAdjustment = 0;
    if (TopOrLeftAdjustmentLocation == StickinessLocation::DuringEntry)
    {
      entryDistanceAdjustment += StickyTopOrLeftAdjustment;
    }
    if (BottomOrRightAdjustmentLocation == StickinessLocation::DuringEntry)
    {
      entryDistanceAdjustment -= StickyBottomOrRightAdjustment;
    }
    return entryDistanceAdjustment;
  }

  float StickinessAdjustmentData::ExitDistanceAdjustment() const noexcept
  {
    float exitDistanceAdjustment = 0;
    if (TopOrLeftAdjustmentLocation == StickinessLocation::DuringExit)
    {
      exitDistanceAdjustment += StickyTopOrLeftAdjustment;
    }
    if (BottomOrRightAdjustmentLocation == StickinessLocation::DuringExit)
    {
      exitDistanceAdjustment -= StickyBottomOrRightAdjustment;
    }
    return exitDistanceAdjustment;
  }

  float StickinessAdjustmentData::RangeStartAdjustment() const noexcept
  {
    float rangeStartAdjustment = 0;
    if (TopOrLeftAdjustmentLocation == StickinessLocation::BeforeEntry)
    {
      rangeStartAdjustment += StickyTopOrLeftAdjustment;
    }
    if (BottomOrRightAdjustmentLocation != StickinessLocation::BeforeEntry)
    {
      rangeStartAdjustment += StickyBottomOrRightAdjustment;
    }
    return rangeStartAdjustment;
  }

  float StickinessAdjustmentData::RangeEndAdjustment() const noexcept
  {
    float rangeEndAdjustment = 0;
    if (TopOrLeftAdjustmentLocation != StickinessLocation::AfterExit)
    {
      rangeEndAdjustment += StickyTopOrLeftAdjustment;
    }
    if (BottomOrRightAdjustmentLocation == StickinessLocation::AfterExit)
    {
      rangeEndAdjustment += StickyBottomOrRightAdjustment;
    }
    return rangeEndAdjustment;
  }

  void ViewTimeline::CacheCurrentTime() noexcept
  {
    auto previousCurrentTimeData = _cachedCurrentTimeData;

    auto pointForLocalToContainer = [](const ScrollableArea &area) -> FloatPoint
    {
      // For subscrollers we need to ajust the point fed into LocalToContainerPoint as the returned value can
      // be outside of the scroller.
      if (Is<RenderLayerScrollableArea>(area))
      {
        return FloatPoint(area.GetScrollOffset());
      }
      return {};
    };

    _cachedCurrentTimeData = [&]() -> CurrentTimeData
    {
      auto subject = _subject.GetStyleable();
      if (!subject)
      {
        return {};
      }

      const auto *subjectRenderer = subject->Renderer();
      if (!subjectRenderer)
      {
        return {};
      }

      const auto *sourceRenderer = SourceScrollerRenderer();
      auto *sourceScrollableArea =
        ScrollableAreaForSourceRenderer(sourceRenderer, subject->Element.NodeDocument());
      if (!sourceScrollableArea)
      {
        return {};
      }

      auto scrollDirection = GetResolvedScrollDirection();
      auto scrollOffset =
        static_cast<float>(scrollDirection.IsVertical ? sourceScrollableArea->GetScrollOffset().Y()
                                                      : sourceScrollableArea->GetScrollOffset().X());
      auto scrollContainerSize =
        static_cast<float>(scrollDirection.IsVertical ? sourceScrollableArea->VisibleHeight()
                                                      : sourceScrollableArea->VisibleWidth());

      // https://drafts.csswg.org/scroll-animations-1/#view-timelines-ranges
      // Transforms and sticky position offsets are ignored, but relative and absolute positioning are
      // accounted for.
      EnumSet<MapCoordinatesMode> options {MapCoordinatesMode::IgnoreStickyOffsets};
      auto subjectOffsetFromSource = subjectRenderer->LocalToContainerPoint(
        pointForLocalToContainer(*sourceScrollableArea), sourceRenderer, options);
      float subjectOffset =
        scrollDirection.IsVertical ? subjectOffsetFromSource.Y() : subjectOffsetFromSource.X();

      // Ensure borders are subtracted.
      auto scrollerPaddingBoxOrigin = sourceRenderer->PaddingBoxRect().Location();
      subjectOffset -=
        scrollDirection.IsVertical ? scrollerPaddingBoxOrigin.Y() : scrollerPaddingBoxOrigin.X();

      auto subjectBounds = [&]() -> FloatSize
      {
        if (const auto *subjectRenderBoxModelObject = DynamicDowncast<RenderBoxModelObject>(subjectRenderer))
        {
          return FloatSize(subjectRenderBoxModelObject->BorderBoundingBox().Size());
        }
        if (Is<LegacyRenderSVGModelObject>(subjectRenderer))
        {
          return subjectRenderer->ObjectBoundingBox().Size();
        }
        return {};
      }();

      auto subjectSize = scrollDirection.IsVertical ? subjectBounds.Height() : subjectBounds.Width();

      if (_specifiedInsets)
      {
        RefPtr subjectElement = ShareRefPtr(&subject->Element);

        auto computedInset = [&](const css::PrimitiveValue &specifiedInset)
        {
          return style::DeprecatedConvertCSSValueToStyle<style::ViewTimelineInsetItem::Length>(subjectElement,
                                                                                               specifiedInset)
            .value_or(style::ViewTimelineInsetItem::Length {css::keywords::Auto {}});
        };

        if (_specifiedInsets->Start && _specifiedInsets->End)
        {
          _insets = {computedInset(*_specifiedInsets->Start), computedInset(*_specifiedInsets->End)};
        }
        else if (_specifiedInsets->Start)
        {
          _insets = {computedInset(*_specifiedInsets->Start)};
        }
        else if (_specifiedInsets->End)
        {
          _insets = {style::ViewTimelineInsetItem::Length {css::keywords::Auto {}},
                     computedInset(*_specifiedInsets->End)};
        }
        else
        {
          _insets = {style::ViewTimelineInsetItem::Length {css::keywords::Auto {}},
                     style::ViewTimelineInsetItem::Length {css::keywords::Auto {}}};
        }
      }

      enum class PaddingEdge : bool
      {
        Start,
        End
      };
      auto scrollPadding = [&](PaddingEdge edge) -> const style::ScrollPaddingEdge &
      {
        const auto &scrollPaddingBox = sourceRenderer->Style().GetScrollPaddingBox();
        if (edge == PaddingEdge::Start)
        {
          return scrollDirection.IsVertical ? scrollPaddingBox.Top() : scrollPaddingBox.Left();
        }
        return scrollDirection.IsVertical ? scrollPaddingBox.Bottom() : scrollPaddingBox.Right();
      };

      float insetStart = 0;
      float insetEnd = 0;

      if (_insets.Start().IsAuto())
      {
        insetStart = style::Evaluate<float>(scrollPadding(PaddingEdge::Start), scrollContainerSize,
                                            style::ZoomNeeded {});
      }
      else
      {
        insetStart = style::Evaluate<float>(_insets.Start(), scrollContainerSize, style::ZoomNeeded {});
      }

      if (_insets.End().IsAuto())
      {
        insetEnd =
          style::Evaluate<float>(scrollPadding(PaddingEdge::End), scrollContainerSize, style::ZoomNeeded {});
      }
      else
      {
        insetEnd = style::Evaluate<float>(_insets.End(), scrollContainerSize, style::ZoomNeeded {});
      }

      StickinessAdjustmentData stickyData;
      if (const auto *stickyContainer = DynamicDowncast<RenderBoxModelObject>(StickyContainer()))
      {
        FloatRect constrainingRect = stickyContainer->ConstrainingRectForStickyPosition();
        StickyPositionViewportConstraints constraints;
        stickyContainer->ComputeStickyPositionConstraints(constraints, constrainingRect);
        stickyData = StickinessAdjustmentData::ComputeStickinessAdjustmentData(
          constraints, scrollDirection, scrollContainerSize, subjectSize, subjectOffset);
      }

      return {scrollOffset, scrollContainerSize, subjectOffset, subjectSize, insetStart,
              insetEnd,     stickyData};
    }();

    auto metricsChanged =
      previousCurrentTimeData.ScrollContainerSize != _cachedCurrentTimeData.ScrollContainerSize
      || previousCurrentTimeData.SubjectOffset != _cachedCurrentTimeData.SubjectOffset
      || previousCurrentTimeData.SubjectSize != _cachedCurrentTimeData.SubjectSize
      || previousCurrentTimeData.InsetStart != _cachedCurrentTimeData.InsetStart
      || previousCurrentTimeData.InsetEnd != _cachedCurrentTimeData.InsetEnd
      || previousCurrentTimeData.StickinessData != _cachedCurrentTimeData.StickinessData;

    if (metricsChanged)
    {
      SourceMetricsDidChange();
    }
  }

  AnimationTimeline::ShouldUpdateAnimationsAndSendEvents
    ViewTimeline::DocumentWillUpdateAnimationsAndSendEvents() noexcept
  {
    CacheCurrentTime();
    if (_subject.GetElement() && _subject.GetElement()->IsConnected())
    {
      return ShouldUpdateAnimationsAndSendEvents::Yes;
    }
    return ShouldUpdateAnimationsAndSendEvents::No;
  }

  style::SingleAnimationRange ViewTimeline::DefaultRange() const noexcept
  {
    return style::SingleAnimationRange::DefaultForViewTimeline();
  }

  RefPtr<dom::Element> ViewTimeline::BindingsSource() const noexcept
  {
    if (auto subject = _subject.GetStyleable())
    {
      (void)subject->Element.NodeDocument().UpdateStyleIfNeeded();
    }
    return ScrollTimeline::BindingsSource();
  }

  RefPtr<dom::Element> ViewTimeline::Source() const noexcept
  {
    if (const auto *sourceRender = SourceScrollerRenderer())
    {
      return ShareRefPtr(sourceRender->GetElement());
    }
    return nullptr;
  }

  const RenderBox *ViewTimeline::SourceScrollerRenderer() const noexcept
  {
    auto subject = _subject.GetStyleable();
    if (!subject)
    {
      return nullptr;
    }

    const auto *subjectRenderer = subject->Renderer();
    if (!subjectRenderer)
    {
      return {};
    }

    // https://drafts.csswg.org/scroll-animations-1/#dom-scrolltimeline-source
    // Determine source renderer by looking for the nearest ancestor that establishes a scroll container
    return subjectRenderer->EnclosingScrollableContainer();
  }

  const RenderElement *ViewTimeline::StickyContainer() const noexcept
  {
    auto subject = _subject.GetStyleable();
    if (!subject)
    {
      return nullptr;
    }

    const RenderElement *renderer = subject->Renderer();

    const auto *scrollerRenderer = SourceScrollerRenderer();
    while (renderer && renderer != scrollerRenderer)
    {
      if (renderer->IsStickilyPositioned())
      {
        return renderer;
      }
      renderer = renderer->ContainingBlock();
    }
    return nullptr;
  }

  ScrollTimeline::Data ViewTimeline::ComputeTimelineData(UseCachedCurrentTime) const noexcept
  {
    // FIXME: account for UseCachedCurrentTime parameter.
    if (!_cachedCurrentTimeData.ScrollOffset && !_cachedCurrentTimeData.ScrollContainerSize)
    {
      return {};
    }

    auto rangeStart = _cachedCurrentTimeData.SubjectOffset - _cachedCurrentTimeData.ScrollContainerSize;
    auto range = _cachedCurrentTimeData.SubjectSize + _cachedCurrentTimeData.ScrollContainerSize;
    auto rangeEnd = rangeStart + range;

    return {_cachedCurrentTimeData.ScrollOffset,
            rangeStart + _cachedCurrentTimeData.InsetEnd
              + _cachedCurrentTimeData.StickinessData.RangeStartAdjustment(),
            rangeEnd - _cachedCurrentTimeData.InsetStart
              + _cachedCurrentTimeData.StickinessData.RangeEndAdjustment()};
  }

  Pair<double, double>
    ViewTimeline::IntervalForTimelineRangeName(const ScrollTimeline::Data &data,
                                               style::SingleAnimationRangeName name) const noexcept
  {
    auto subjectRangeStart = [&]() -> double
    {
      switch (name)
      {
        case style::SingleAnimationRangeName::Normal:
        case style::SingleAnimationRangeName::Omitted:
        case style::SingleAnimationRangeName::Cover:
        case style::SingleAnimationRangeName::EntryCrossing: return data.RangeStart;
        case style::SingleAnimationRangeName::Entry:
          // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-entry
          // 0% is equivalent to 0% of the cover range.
          return IntervalForTimelineRangeName(data, style::SingleAnimationRangeName::Cover).first;
        case style::SingleAnimationRangeName::Contain:
          return data.RangeStart + _cachedCurrentTimeData.SubjectSize
                 + _cachedCurrentTimeData.StickinessData.EntryDistanceAdjustment();
        case style::SingleAnimationRangeName::Exit:
          // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-exit
          // 0% is equivalent to 100% of the contain range.
          return IntervalForTimelineRangeName(data, style::SingleAnimationRangeName::Contain).second;
        case style::SingleAnimationRangeName::ExitCrossing:
          return data.RangeEnd - _cachedCurrentTimeData.SubjectSize
                 - _cachedCurrentTimeData.StickinessData.ExitDistanceAdjustment();
      }
      krys_unreachable();
    }();

    auto subjectRangeEnd = [&]() -> double
    {
      switch (name)
      {
        case style::SingleAnimationRangeName::Normal:
        case style::SingleAnimationRangeName::Omitted:
        case style::SingleAnimationRangeName::Cover:
        case style::SingleAnimationRangeName::ExitCrossing: return data.RangeEnd;
        case style::SingleAnimationRangeName::Exit:
          // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-exit
          // 100% is equivalent to 100% of the cover range.
          return IntervalForTimelineRangeName(data, style::SingleAnimationRangeName::Cover).second;
        case style::SingleAnimationRangeName::Contain:
          return data.RangeEnd - _cachedCurrentTimeData.SubjectSize
                 - _cachedCurrentTimeData.StickinessData.ExitDistanceAdjustment();
        case style::SingleAnimationRangeName::Entry:
          // https://drafts.csswg.org/scroll-animations-1/#valdef-animation-timeline-range-entry
          // 100% is equivalent to 0% of the contain range.
          return IntervalForTimelineRangeName(data, style::SingleAnimationRangeName::Contain).first;
        case style::SingleAnimationRangeName::EntryCrossing:
          return data.RangeStart + _cachedCurrentTimeData.SubjectSize
                 + _cachedCurrentTimeData.StickinessData.EntryDistanceAdjustment();
      }
      krys_unreachable();
    }();

    if (subjectRangeEnd < subjectRangeStart)
    {
      std::swap(subjectRangeStart, subjectRangeEnd);
    }

    return {subjectRangeStart, subjectRangeEnd};
  }

  template <typename F>
  double ViewTimeline::MapOffsetToTimelineRange(const ScrollTimeline::Data &data,
                                                style::SingleAnimationRangeName name,
                                                F &&valueWithinSubjectRange) const noexcept
  {
    auto timelineRange = data.RangeEnd - data.RangeStart;
    krys_debug_assert(timelineRange);
    auto [subjectRangeStart, subjectRangeEnd] = IntervalForTimelineRangeName(data, name);
    auto subjectRange = subjectRangeEnd - subjectRangeStart;
    auto positionWithinContainer =
      subjectRangeStart + valueWithinSubjectRange(static_cast<float>(subjectRange));
    auto positionWithinTimelineRange = positionWithinContainer - data.RangeStart;
    return positionWithinTimelineRange / timelineRange;
  }

  Pair<double, double>
    ViewTimeline::OffsetIntervalForTimelineRangeName(style::SingleAnimationRangeName name) const noexcept
  {
    auto data = ComputeTimelineData();
    auto computeOffset = [&](double offset)
    {
      return MapOffsetToTimelineRange(data, name,
                                      [&](float subjectRange) -> double { return offset * subjectRange; });
    };
    return {computeOffset(0), computeOffset(1)};
  }

  Pair<double, double> ViewTimeline::OffsetIntervalForAttachmentRange(
    const style::SingleAnimationRange &attachmentRange) const noexcept
  {
    auto data = ComputeTimelineData();
    auto timelineRange = data.RangeEnd - data.RangeStart;
    krys_debug_assert(timelineRange);

    auto offsetForSingleTimelineRange = [&](const auto &rangeToConvert) -> double
    {
      auto [conversionRangeStart, conversionRangeEnd] =
        IntervalForTimelineRangeName(data, rangeToConvert.GetName());
      auto conversionRange = conversionRangeEnd - conversionRangeStart;
      auto convertedValue = style::Evaluate<float>(rangeToConvert.GetOffset(),
                                                   static_cast<float>(conversionRange), style::ZoomNeeded {});
      auto position = conversionRangeStart + convertedValue;
      return (position - data.RangeStart) / timelineRange;
    };

    return {offsetForSingleTimelineRange(attachmentRange.Start),
            offsetForSingleTimelineRange(attachmentRange.End)};
  }

  Pair<WebAnimationTime, WebAnimationTime> ViewTimeline::IntervalForAttachmentRange(
    const style::SingleAnimationRange &attachmentRange) const noexcept
  {
    // https://drafts.csswg.org/scroll-animations-1/#view-timelines-ranges
    auto data = ComputeTimelineData();
    auto timelineRange = data.RangeEnd - data.RangeStart;
    if (!timelineRange)
    {
      return {WebAnimationTime::FromPercentage(0), WebAnimationTime::FromPercentage(100)};
    }

    auto computeTime = [&](const auto &rangeToConvert)
    {
      auto mappedOffset = MapOffsetToTimelineRange(
        data, rangeToConvert.GetName(), [&](float subjectRange) -> double
        { return style::Evaluate<float>(rangeToConvert.GetOffset(), subjectRange, style::ZoomNeeded {}); });
      return WebAnimationTime::FromPercentage(mappedOffset * 100);
    };

    auto attachmentRangeOrDefault = attachmentRange.IsDefault() ? DefaultRange() : attachmentRange;
    return {computeTime(attachmentRangeOrDefault.Start), computeTime(attachmentRangeOrDefault.End)};
  }

  Ref<css::CSSNumericValue> ViewTimeline::StartOffset() const noexcept
  {
    return css::CSSNumericFactory::Px(ComputeTimelineData().RangeStart);
  }

  Ref<css::CSSNumericValue> ViewTimeline::EndOffset() const noexcept
  {
    return css::CSSNumericFactory::Px(ComputeTimelineData().RangeEnd);
  }
}
