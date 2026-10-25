#pragma once

#include "Krystal.Booey/Animation/ScrollTimeline.hpp"
#include "Krystal.Booey/Animation/ViewTimelineOptions.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericValue.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Style/Values/ScrollAnimations/ViewTimelineInsets.hpp"

namespace krys::boo
{
  class RenderBox;
  class StickyPositionViewportConstraints;

  namespace css
  {
    class PrimitiveValue;
  }

  namespace style
  {
    enum class SingleAnimationRangeName : uint8;
  }

  /// @brief How far a sticky container moves a view timeline's subject, and where the subject is in its
  /// scroll container when it does.
  struct StickinessAdjustmentData
  {
    enum class StickinessLocation : uint8
    {
      BeforeEntry,
      DuringEntry,
      WhileContained,
      DuringExit,
      AfterExit
    };

    float StickyTopOrLeftAdjustment {0};
    StickinessLocation TopOrLeftAdjustmentLocation {StickinessLocation::WhileContained};
    float StickyBottomOrRightAdjustment {0};
    StickinessLocation BottomOrRightAdjustmentLocation {StickinessLocation::WhileContained};

    KRYS_NODISCARD bool operator==(const StickinessAdjustmentData &other) const noexcept = default;

    KRYS_NODISCARD float EntryDistanceAdjustment() const noexcept;
    KRYS_NODISCARD float ExitDistanceAdjustment() const noexcept;
    KRYS_NODISCARD float RangeStartAdjustment() const noexcept;
    KRYS_NODISCARD float RangeEndAdjustment() const noexcept;

    KRYS_NODISCARD static StickinessAdjustmentData
      ComputeStickinessAdjustmentData(const StickyPositionViewportConstraints &constraints,
                                      ScrollTimeline::ResolvedScrollDirection scrollDirection,
                                      float scrollContainerSize, float subjectSize,
                                      float subjectOffset) noexcept;
  };

  /// @brief A timeline whose progress is how far its subject has moved through its scroll container's
  /// scrollport, as a percentage.
  /// @see https://drafts.csswg.org/scroll-animations-1/#viewtimeline-interface
  class ViewTimeline final : public ScrollTimeline
  {
  private:
    struct CurrentTimeData
    {
      float ScrollOffset {0};
      float ScrollContainerSize {0};
      float SubjectOffset {0};
      float SubjectSize {0};
      float InsetStart {0};
      float InsetEnd {0};
      StickinessAdjustmentData StickinessData {};
    };

    struct SpecifiedViewTimelineInsets
    {
      RefPtr<css::PrimitiveValue> Start;
      RefPtr<css::PrimitiveValue> End;
    };

    style::WeakStyleable _subject;
    Maybe<SpecifiedViewTimelineInsets> _specifiedInsets;
    style::ViewTimelineInsetItem _insets;
    CurrentTimeData _cachedCurrentTimeData {};

  public:
    /// @see https://drafts.csswg.org/scroll-animations-1/#dom-viewtimeline-viewtimeline
    KRYS_NODISCARD static dom::ExceptionOr<Ref<ViewTimeline>>
      Create(dom::Document &document, ViewTimelineOptions &&options = {}) noexcept;
    KRYS_NODISCARD static Ref<ViewTimeline> Create(const css::CSSOMStringAtom &name, ScrollAxis axis,
                                                   const style::ViewTimelineInsetItem &insets) noexcept;

    /// @see https://drafts.csswg.org/scroll-animations-1/#dom-viewtimeline-subject
    KRYS_NODISCARD const dom::Element *Subject() const noexcept;
    KRYS_NODISCARD const style::WeakStyleable &SubjectStyleable() const noexcept
    {
      return _subject;
    }
    void SetSubject(dom::Element *subject) noexcept;
    void SetSubject(const style::Styleable &styleable) noexcept;

    KRYS_NODISCARD const style::ViewTimelineInsetItem &Insets() const noexcept
    {
      return _insets;
    }
    void SetInsets(const style::ViewTimelineInsetItem &insets) noexcept
    {
      _insets = insets;
    }

    /// @see https://drafts.csswg.org/scroll-animations-1/#dom-viewtimeline-startoffset
    KRYS_NODISCARD Ref<css::CSSNumericValue> StartOffset() const noexcept;
    /// @see https://drafts.csswg.org/scroll-animations-1/#dom-viewtimeline-endoffset
    KRYS_NODISCARD Ref<css::CSSNumericValue> EndOffset() const noexcept;

    KRYS_NODISCARD ShouldUpdateAnimationsAndSendEvents
      DocumentWillUpdateAnimationsAndSendEvents() noexcept override;
    KRYS_NODISCARD AnimationTimelinesController *Controller() const noexcept override;

    KRYS_NODISCARD const RenderBox *SourceScrollerRenderer() const noexcept;
    KRYS_NODISCARD const RenderElement *StickyContainer() const noexcept;
    KRYS_NODISCARD RefPtr<dom::Element> BindingsSource() const noexcept override;
    KRYS_NODISCARD RefPtr<dom::Element> Source() const noexcept override;
    KRYS_NODISCARD style::SingleAnimationRange DefaultRange() const noexcept final;

    KRYS_NODISCARD Pair<WebAnimationTime, WebAnimationTime>
      IntervalForAttachmentRange(const style::SingleAnimationRange &attachmentRange) const noexcept final;
    /// @brief The start and end of @p attachmentRange, as offsets (0 to 1) of this timeline.
    KRYS_NODISCARD Pair<double, double>
      OffsetIntervalForAttachmentRange(const style::SingleAnimationRange &attachmentRange) const noexcept;
    /// @brief The start and end of the named timeline range @p name, as offsets (0 to 1) of this timeline.
    KRYS_NODISCARD Pair<double, double>
      OffsetIntervalForTimelineRangeName(style::SingleAnimationRangeName name) const noexcept;

  private:
    explicit ViewTimeline(ScrollAxis axis) noexcept;
    ViewTimeline(const css::CSSOMStringAtom &name, ScrollAxis axis,
                 const style::ViewTimelineInsetItem &insets) noexcept;

    KRYS_NODISCARD ScrollTimeline::Data ComputeTimelineData(
      UseCachedCurrentTime useCachedCurrentTime = UseCachedCurrentTime::Yes) const noexcept final;
    KRYS_NODISCARD Pair<double, double>
      IntervalForTimelineRangeName(const ScrollTimeline::Data &data,
                                   style::SingleAnimationRangeName name) const noexcept;
    template <typename F>
    KRYS_NODISCARD double MapOffsetToTimelineRange(const ScrollTimeline::Data &data,
                                                   style::SingleAnimationRangeName name,
                                                   F &&valueWithinSubjectRange) const noexcept;

    KRYS_NODISCARD bool IsViewTimeline() const noexcept final
    {
      return true;
    }

    void CacheCurrentTime() noexcept;

    KRYS_NODISCARD dom::ExceptionOr<SpecifiedViewTimelineInsets>
      ValidateSpecifiedInsets(const ViewTimelineInsetValue &inset, const dom::Document &document) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_ANIMATION_TIMELINE(krys::boo::ViewTimeline, IsViewTimeline())
