/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Animation/Timelines/AnimationTimeline.hpp"
#include "Krystal.Booey/Animation/Timelines/ScrollAxis.hpp"
#include "Krystal.Booey/Animation/Timelines/ScrollTimelineOptions.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class RenderElement;
  class ScrollableArea;

  namespace dom
  {
    class Document;
    class Element;
  }

  /// @brief A timeline whose progress is the scroll position of a scroll container, as a percentage.
  /// @see https://drafts.csswg.org/scroll-animations-1/#scrolltimeline-interface
  // FIXME: Accelerated representations, once there are threaded animations.
  class ScrollTimeline : public AnimationTimeline
  {
  public:
    struct ResolvedScrollDirection
    {
      bool IsVertical;
      bool IsReversed;
    };

  protected:
    struct Data
    {
      float ScrollOffset {0};
      float RangeStart {0};
      float RangeEnd {0};
    };

  private:
    struct CurrentTimeData
    {
      float ScrollOffset {0};
      float MaxScrollOffset {0};
    };

    style::WeakStyleable _source;
    ScrollAxis _axis {ScrollAxis::Block};
    css::CSSOMStringAtom _name {css::CSSOMStringAtom::Null()};
    Scroller _scroller {Scroller::Self};
    WeakPtr<dom::Element> _timelineScopeElement;
    CurrentTimeData _cachedCurrentTimeData {};
    bool _isInactiveStyleOriginatedTimeline {false};

  public:
    /// @see https://drafts.csswg.org/scroll-animations-1/#dom-scrolltimeline-scrolltimeline
    KRYS_NODISCARD static Ref<ScrollTimeline> Create(dom::Document &document,
                                                     ScrollTimelineOptions &&options = {}) noexcept;
    KRYS_NODISCARD static Ref<ScrollTimeline> Create(const css::CSSOMStringAtom &name,
                                                     ScrollAxis axis) noexcept;
    KRYS_NODISCARD static Ref<ScrollTimeline> Create(Scroller scroller, ScrollAxis axis) noexcept;
    KRYS_NODISCARD static Ref<ScrollTimeline>
      CreateInactiveStyleOriginatedTimeline(const css::CSSOMStringAtom &name) noexcept;

    KRYS_NODISCARD const style::WeakStyleable &SourceStyleable() const noexcept
    {
      return _source;
    }
    /// @see https://drafts.csswg.org/scroll-animations-1/#dom-scrolltimeline-source
    KRYS_NODISCARD virtual RefPtr<dom::Element> BindingsSource() const noexcept;
    KRYS_NODISCARD virtual RefPtr<dom::Element> Source() const noexcept;
    void SetSource(dom::Element *source) noexcept;
    void SetSource(const style::Styleable &styleable) noexcept;

    KRYS_NODISCARD ScrollAxis Axis() const noexcept
    {
      return _axis;
    }
    void SetAxis(ScrollAxis axis) noexcept
    {
      _axis = axis;
    }

    KRYS_NODISCARD const css::CSSOMStringAtom &GetName() const noexcept
    {
      return _name;
    }
    void SetName(const css::CSSOMStringAtom &name) noexcept
    {
      _name = name;
    }

    KRYS_NODISCARD bool IsInactiveStyleOriginatedTimeline() const noexcept
    {
      return _isInactiveStyleOriginatedTimeline;
    }

    KRYS_NODISCARD ShouldUpdateAnimationsAndSendEvents
      DocumentWillUpdateAnimationsAndSendEvents() noexcept override;
    void UpdateCurrentTimeIfStale() noexcept;

    KRYS_NODISCARD AnimationTimelinesController *Controller() const noexcept override;

    KRYS_NODISCARD Maybe<WebAnimationTime>
      CurrentTime(UseCachedCurrentTime useCachedCurrentTime = UseCachedCurrentTime::Yes) noexcept override;
    KRYS_NODISCARD style::SingleAnimationRange DefaultRange() const noexcept override;
    KRYS_NODISCARD const WeakPtr<dom::Element> &TimelineScopeDeclaredElement() const noexcept
    {
      return _timelineScopeElement;
    }
    void SetTimelineScopeElement(const dom::Element &element) noexcept;
    void ClearTimelineScopeDeclaredElement() noexcept
    {
      _timelineScopeElement = nullptr;
    }

    /// @brief The start and end of @p attachmentRange, as times (percentages) of this timeline.
    KRYS_NODISCARD virtual Pair<WebAnimationTime, WebAnimationTime>
      IntervalForAttachmentRange(const style::SingleAnimationRange &attachmentRange) const noexcept;

    void RemoveTimelineFromDocument(dom::Element *element) noexcept;

  protected:
    ScrollTimeline(const css::CSSOMStringAtom &name, ScrollAxis axis) noexcept;

    KRYS_NODISCARD virtual Data ComputeTimelineData(
      UseCachedCurrentTime useCachedCurrentTime = UseCachedCurrentTime::Yes) const noexcept;

    KRYS_NODISCARD static ScrollableArea *ScrollableAreaForSourceRenderer(const RenderElement *renderer,
                                                                          dom::Document &document) noexcept;
    KRYS_NODISCARD ResolvedScrollDirection GetResolvedScrollDirection() const noexcept;
    void SourceMetricsDidChange() noexcept;

  private:
    ScrollTimeline() noexcept;
    ScrollTimeline(Scroller scroller, ScrollAxis axis) noexcept;

    KRYS_NODISCARD bool IsScrollTimeline() const noexcept final
    {
      return true;
    }

    void AnimationTimingDidChange(WebAnimation &animation) noexcept override;

    KRYS_NODISCARD CurrentTimeData ComputeCurrentTimeData() const noexcept;
    void CacheCurrentTime() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_ANIMATION_TIMELINE(krys::boo::ScrollTimeline, IsScrollTimeline())
