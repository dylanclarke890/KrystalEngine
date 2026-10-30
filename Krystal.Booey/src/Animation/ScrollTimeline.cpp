#include "Krystal.Booey/Animation/ScrollTimeline.hpp"
#include "Krystal.Booey/Animation/AnimationTimelinesController.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationRange.hpp"
#include <algorithm>

namespace krys::boo
{
  Ref<ScrollTimeline> ScrollTimeline::Create(dom::Document &document,
                                             ScrollTimelineOptions &&options) noexcept
  {
    // https://drafts.csswg.org/scroll-animations-1/#dom-scrolltimeline-scrolltimeline

    // 1. Let timeline be the new ScrollTimeline object.
    auto timeline = AdoptRef(*new ScrollTimeline);

    // 2. Set the source of timeline to:
    if (auto optionalSource = options.Source)
    {
      // If the source member of options is present,
      // The source member of options.
      timeline->SetSource(optionalSource->get());
    }
    else if (RefPtr scrollingElement = document.ScrollingElementForAPI())
    {
      // Otherwise,
      // The scrollingElement of the Document associated with the Window that is the current global object.
      timeline->SetSource(scrollingElement.get());
    }

    // 3. Set the axis property of timeline to the corresponding value from options.
    timeline->SetAxis(options.Axis);

    if (RefPtr source = ShareRefPtr(timeline->_source.GetElement().get()))
    {
      (void)source->NodeDocument().UpdateLayoutIgnorePendingStylesheets();
      timeline->CacheCurrentTime();
    }

    return timeline;
  }

  Ref<ScrollTimeline> ScrollTimeline::Create(const css::CSSOMStringAtom &name, ScrollAxis axis) noexcept
  {
    return AdoptRef(*new ScrollTimeline(name, axis));
  }

  Ref<ScrollTimeline> ScrollTimeline::Create(Scroller scroller, ScrollAxis axis) noexcept
  {
    return AdoptRef(*new ScrollTimeline(scroller, axis));
  }

  Ref<ScrollTimeline>
    ScrollTimeline::CreateInactiveStyleOriginatedTimeline(const css::CSSOMStringAtom &name) noexcept
  {
    auto timeline = AdoptRef(*new ScrollTimeline(name, ScrollAxis::Block));
    timeline->_isInactiveStyleOriginatedTimeline = true;
    return timeline;
  }

  // https://drafts.csswg.org/web-animations-2/#timelines
  // For a monotonic timeline, there is no upper bound on current time, and timeline duration is unresolved.
  // For a non-monotonic (e.g. scroll) timeline, the duration has a fixed upper bound. In this case, the
  // timeline is a progress-based timeline, and its timeline duration is 100%.
  ScrollTimeline::ScrollTimeline() noexcept : AnimationTimeline(WebAnimationTime::FromPercentage(100))
  {
  }

  ScrollTimeline::ScrollTimeline(const css::CSSOMStringAtom &name, ScrollAxis axis) noexcept
      : ScrollTimeline()
  {
    _axis = axis;
    _name = name;
  }

  ScrollTimeline::ScrollTimeline(Scroller scroller, ScrollAxis axis) noexcept : ScrollTimeline()
  {
    _axis = axis;
    _scroller = scroller;
  }

  RefPtr<dom::Element> ScrollTimeline::BindingsSource() const noexcept
  {
    return Source();
  }

  RefPtr<dom::Element> ScrollTimeline::Source() const noexcept
  {
    auto source = _source.GetStyleable();
    if (!source)
    {
      return nullptr;
    }

    switch (_scroller)
    {
      case Scroller::Nearest:
      {
        if (auto *subjectRenderer = source->Renderer())
        {
          if (auto *nearestScrollableContainer = subjectRenderer->EnclosingScrollableContainer())
          {
            if (RefPtr nearestSource = ShareRefPtr(nearestScrollableContainer->GetElement()))
            {
              Ref document = ShareRef(nearestSource->NodeDocument());
              RefPtr documentElement = document->DocumentElement();
              if (nearestSource.get() != documentElement.get())
              {
                return nearestSource;
              }
              // RenderObject::EnclosingScrollableContainer() will return the document element even in quirks
              // mode, but the scrolling element in that case is the <body> element, so we must make sure to
              // return Document::ScrollingElement() in case the document element is returned by
              // EnclosingScrollableContainer() but it was not explicitly set as the source.
              return &source->Element == documentElement.get() ? nearestSource : document->ScrollingElement();
            }
          }
        }
        return nullptr;
      }
      case Scroller::Root: return source->Element.NodeDocument().ScrollingElement();
      case Scroller::Self: return ShareRefPtr(&source->Element);
    }

    krys_unreachable();
  }

  void ScrollTimeline::SetSource(dom::Element *source) noexcept
  {
    if (source)
    {
      SetSource(style::Styleable::FromElement(*source));
    }
    else
    {
      RemoveTimelineFromDocument(_source.GetElement().get());
      _source = style::WeakStyleable();
    }
  }

  void ScrollTimeline::SetSource(const style::Styleable &styleable) noexcept
  {
    if (_source == style::WeakStyleable(styleable))
    {
      return;
    }

    RefPtr previousSource = ShareRefPtr(_source.GetElement().get());
    _source = styleable;

    if (previousSource && &previousSource->NodeDocument() == &styleable.Element.NodeDocument())
    {
      return;
    }

    RemoveTimelineFromDocument(previousSource.get());

    styleable.Element.NodeDocument().EnsureTimelinesController().AddTimeline(*this);
  }

  void ScrollTimeline::RemoveTimelineFromDocument(dom::Element *element) noexcept
  {
    if (element)
    {
      if (auto *timelinesController = element->NodeDocument().TimelinesController())
      {
        timelinesController->RemoveTimeline(*this);
      }
    }
  }

  AnimationTimelinesController *ScrollTimeline::Controller() const noexcept
  {
    if (auto styleable = _source.GetStyleable())
    {
      return &styleable->Element.NodeDocument().EnsureTimelinesController();
    }
    return nullptr;
  }

  ScrollTimeline::ResolvedScrollDirection ScrollTimeline::GetResolvedScrollDirection() const noexcept
  {
    auto writingMode = [&]() -> WritingMode
    {
      if (RefPtr source = Source())
      {
        if (auto *renderer = source->Renderer())
        {
          return renderer->Style().GetWritingMode();
        }
      }

      return {style::ComputedStyle::InitialWritingMode(), style::ComputedStyle::InitialDirection(),
              style::ComputedStyle::InitialTextOrientation()};
    }();

    auto isVertical = [&]
    {
      switch (_axis)
      {
        case ScrollAxis::Block:
          // https://drafts.csswg.org/scroll-animations-1/#valdef-scroll-block
          // Specifies to use the measure of progress along the block axis of the scroll container.
          // https://drafts.csswg.org/css-writing-modes-4/#block-axis
          // The axis in the block dimension, i.e. the vertical axis in horizontal writing modes and the
          // horizontal axis in vertical writing modes.
          return writingMode.IsHorizontal();
        case ScrollAxis::Inline:
          // https://drafts.csswg.org/scroll-animations-1/#valdef-scroll-inline
          // Specifies to use the measure of progress along the inline axis of the scroll container.
          // https://drafts.csswg.org/css-writing-modes-4/#inline-axis
          // The axis in the inline dimension, i.e. the horizontal axis in horizontal writing modes and the
          // vertical axis in vertical writing modes.
          return writingMode.IsVertical();
        case ScrollAxis::X:
          // https://drafts.csswg.org/scroll-animations-1/#valdef-scroll-x
          // Specifies to use the measure of progress along the horizontal axis of the scroll container.
          return false;
        case ScrollAxis::Y:
          // https://drafts.csswg.org/scroll-animations-1/#valdef-scroll-y
          // Specifies to use the measure of progress along the vertical axis of the scroll container.
          return true;
      }
      krys_unreachable();
    }();

    auto isReversed =
      (isVertical && !writingMode.IsAnyTopToBottom()) || (!isVertical && !writingMode.IsAnyLeftToRight());

    return {isVertical, isReversed};
  }

  auto ScrollTimeline::ComputeCurrentTimeData() const noexcept -> CurrentTimeData
  {
    RefPtr source = Source();
    if (!source)
    {
      return {};
    }
    auto *sourceScrollableArea = ScrollableAreaForSourceRenderer(source->Renderer(), source->NodeDocument());
    if (!sourceScrollableArea)
    {
      return {};
    }
    auto scrollDirection = GetResolvedScrollDirection();
    auto scrollOffset =
      static_cast<float>(scrollDirection.IsVertical ? sourceScrollableArea->GetScrollOffset().Y()
                                                    : sourceScrollableArea->GetScrollOffset().X());
    auto maxScrollOffset =
      static_cast<float>(scrollDirection.IsVertical ? sourceScrollableArea->MaximumScrollOffset().Y()
                                                    : sourceScrollableArea->MaximumScrollOffset().X());
    // Chrome appears to clip the current time of a scroll timeline in the [0-100] range. We match this
    // behavior for compatibility reasons, see https://github.com/w3c/csswg-drafts/issues/11033.
    if (maxScrollOffset > 0)
    {
      scrollOffset = std::clamp(scrollOffset, 0.f, maxScrollOffset);
    }
    return {scrollOffset, maxScrollOffset};
  }

  void ScrollTimeline::CacheCurrentTime() noexcept
  {
    auto previousMaxScrollOffset = _cachedCurrentTimeData.MaxScrollOffset;
    _cachedCurrentTimeData = ComputeCurrentTimeData();
    if (previousMaxScrollOffset != _cachedCurrentTimeData.MaxScrollOffset)
    {
      SourceMetricsDidChange();
    }
  }

  void ScrollTimeline::SourceMetricsDidChange() noexcept
  {
    for (const auto &animation : _animations)
    {
      animation->ProgressBasedTimelineSourceDidChangeMetrics();
    }
  }

  AnimationTimeline::ShouldUpdateAnimationsAndSendEvents
    ScrollTimeline::DocumentWillUpdateAnimationsAndSendEvents() noexcept
  {
    CacheCurrentTime();
    auto source = _source.GetStyleable();
    if (source && source->Element.IsConnected())
    {
      return ShouldUpdateAnimationsAndSendEvents::Yes;
    }
    return ShouldUpdateAnimationsAndSendEvents::No;
  }

  void ScrollTimeline::UpdateCurrentTimeIfStale() noexcept
  {
    // https://drafts.csswg.org/scroll-animations-1/#event-loop
    // We must update timelines that became stale in the process of updating the page rendering. This function
    // will be called during Page::UpdateRendering() after animations have been updated, requestAnimationFrame
    // callbacks have been serviced, styles have been updated and resize observers have been run.
    // See https://github.com/w3c/csswg-drafts/issues/12120 about clarifying this.
    auto source = _source.GetStyleable();
    if (!source || _animations.isEmpty())
    {
      return;
    }

    auto previousMaxScrollOffset = _cachedCurrentTimeData.MaxScrollOffset;
    CacheCurrentTime();
    if (previousMaxScrollOffset == _cachedCurrentTimeData.MaxScrollOffset)
    {
      return;
    }

    bool needsStyleUpdate = false;
    for (const auto &animation : _animations)
    {
      if (RefPtr effect = ShareRefPtr(animation->GetKeyframeEffect()))
      {
        effect->Invalidate();
        needsStyleUpdate = true;
      }
    }

    if (needsStyleUpdate)
    {
      (void)source->Element.NodeDocument().UpdateStyleIfNeeded();
    }
  }

  void ScrollTimeline::SetTimelineScopeElement(const dom::Element &element) noexcept
  {
    _timelineScopeElement = CreateWeakPtr(const_cast<dom::Element *>(&element));
  }

  ScrollableArea *ScrollTimeline::ScrollableAreaForSourceRenderer(const RenderElement *renderer,
                                                                  dom::Document &document) noexcept
  {
    const auto *renderBox = DynamicDowncast<RenderBox>(renderer);
    if (!renderBox)
    {
      return nullptr;
    }

    if (renderer->GetElement() == document.ScrollingElement().get())
    {
      return &renderer->View().FrameView();
    }

    return renderBox->HasLayer() ? renderBox->Layer()->GetScrollableArea() : nullptr;
  }

  style::SingleAnimationRange ScrollTimeline::DefaultRange() const noexcept
  {
    return style::SingleAnimationRange::DefaultForScrollTimeline();
  }

  ScrollTimeline::Data
    ScrollTimeline::ComputeTimelineData(UseCachedCurrentTime useCachedCurrentTime) const noexcept
  {
    auto currentTimeData =
      useCachedCurrentTime == UseCachedCurrentTime::Yes ? _cachedCurrentTimeData : ComputeCurrentTimeData();
    if (!currentTimeData.ScrollOffset && !currentTimeData.MaxScrollOffset)
    {
      return {};
    }
    return {currentTimeData.ScrollOffset, 0.f, currentTimeData.MaxScrollOffset};
  }

  Pair<WebAnimationTime, WebAnimationTime> ScrollTimeline::IntervalForAttachmentRange(
    const style::SingleAnimationRange &attachmentRange) const noexcept
  {
    auto maxScrollOffset = _cachedCurrentTimeData.MaxScrollOffset;
    if (!maxScrollOffset)
    {
      return {WebAnimationTime::FromPercentage(0), WebAnimationTime::FromPercentage(100)};
    }

    auto attachmentRangeOrDefault = attachmentRange.IsDefault() ? DefaultRange() : attachmentRange;

    auto computedPercentageIfNecessary = [&](const style::SingleAnimationRangeLength &rangeOffset) -> double
    {
      if (auto percentage = rangeOffset.TryPercentage())
      {
        return percentage->Value;
      }
      return style::Evaluate<float>(rangeOffset, maxScrollOffset, style::ZoomNeeded {}) / maxScrollOffset
             * 100;
    };

    return {WebAnimationTime::FromPercentage(
              computedPercentageIfNecessary(attachmentRangeOrDefault.Start.GetOffset())),
            WebAnimationTime::FromPercentage(
              computedPercentageIfNecessary(attachmentRangeOrDefault.End.GetOffset()))};
  }

  Maybe<WebAnimationTime> ScrollTimeline::CurrentTime(UseCachedCurrentTime useCachedCurrentTime) noexcept
  {
    // https://drafts.csswg.org/scroll-animations-1/#scroll-timeline-progress
    // Progress (the current time) for a scroll progress timeline is calculated as:
    // scroll offset ÷ (scrollable overflow size − scroll container size)
    auto data = ComputeTimelineData(useCachedCurrentTime);
    auto range = data.RangeEnd - data.RangeStart;
    if (!range)
    {
      return null;
    }

    auto scrollDirection = GetResolvedScrollDirection();
    auto distance =
      scrollDirection.IsReversed ? data.RangeEnd - data.ScrollOffset : data.ScrollOffset - data.RangeStart;
    auto progress = distance / range;
    return WebAnimationTime::FromPercentage(progress * 100);
  }

  void ScrollTimeline::AnimationTimingDidChange(WebAnimation &animation) noexcept
  {
    AnimationTimeline::AnimationTimingDidChange(animation);

    auto source = _source.GetStyleable();
    if (!source || !animation.Pending() || animation.IsEffectInvalidationSuspended())
    {
      return;
    }

    if (auto *page = source->Element.NodeDocument().GetPage())
    {
      page->ScheduleRenderingUpdate(RenderingUpdateStep::Animations);
    }
  }
}
