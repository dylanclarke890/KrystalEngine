#pragma once

#include "Krystal.Booey/Animation/ScrollAxis.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/NameScope.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::boo
{
  class CSSAnimation;
  class ScrollTimeline;
  class WebAnimation;

  namespace style
  {
    struct ViewTimelineInsetItem;
  }

  /// @brief A document's style-originated timelines: the named scroll and view timelines elements declare,
  /// and the CSS animations that name them.
  /// @note A style-originated timeline is a timeline that is assigned to a CSS Animation via the
  /// `animation-timeline` property. These timelines may be created directly as a result of that property
  /// being set to a `scroll()` or `view()` value, but the `scroll-timeline-name` and `view-timeline-name`
  /// properties may be used on another element to define those elements as the scroll timeline's source or
  /// view timeline's subject. Additionally, the `timeline-scope` property can also be used on a shared
  /// ancestor to allow a `{scroll|view}-timeline-name` value to be used as an `animation-timeline` property
  /// in more complex hierarchies.
  /// @see https://drafts.csswg.org/css-animations-2/#animation-timeline
  /// @see https://drafts.csswg.org/scroll-animations-1/#scroll-timeline-name
  /// @see https://drafts.csswg.org/scroll-animations-1/#view-timeline-name
  /// @see https://drafts.csswg.org/scroll-animations-1/#timeline-scope
  class StyleOriginatedTimelinesController final
  {
    KRYS_NON_COPY_MOVABLE(StyleOriginatedTimelinesController);

  private:
    enum class AllowsDeferral : bool
    {
      No,
      Yes
    };

    List<Ref<CSSAnimation>> _cssAnimationsPendingAttachment;
    List<Pair<style::NameScope, style::WeakStyleable>> _timelineScopeEntries;
    HashMap<css::CSSOMStringAtom, List<Ref<ScrollTimeline>>> _nameToTimelineMap;
    ListHashSet<Ref<ScrollTimeline>> _removedTimelines;

  public:
    StyleOriginatedTimelinesController() noexcept;
    ~StyleOriginatedTimelinesController() noexcept;

    void RegisterNamedScrollTimeline(const css::CSSOMStringAtom &name, const style::Styleable &source,
                                     ScrollAxis axis) noexcept;
    void RegisterNamedViewTimeline(const css::CSSOMStringAtom &name, const style::Styleable &subject,
                                   ScrollAxis axis, const style::ViewTimelineInsetItem &insets) noexcept;
    void UnregisterNamedTimeline(const css::CSSOMStringAtom &name,
                                 const style::Styleable &styleable) noexcept;
    void AttachAnimation(CSSAnimation &animation) noexcept;
    void UpdateNamedTimelineMapForTimelineScope(const style::NameScope &scope,
                                                const style::Styleable &styleable) noexcept;
    void UpdateTimelineForTimelineScope(const Ref<ScrollTimeline> &timeline,
                                        const css::CSSOMStringAtom &name) noexcept;
    void UnregisterNamedTimelinesAssociatedWithElement(const style::Styleable &styleable) noexcept;
    void RemovePendingOperationsForCSSAnimation(const CSSAnimation &animation) noexcept;
    KRYS_NODISCARD bool IsPendingTimelineAttachment(const WebAnimation &animation) const noexcept;
    void DocumentDidResolveStyle() noexcept;
    void StyleableWasRemoved(const style::Styleable &styleable) noexcept;

  private:
    KRYS_NODISCARD List<Ref<ScrollTimeline>> &TimelinesForName(const css::CSSOMStringAtom &name) noexcept;
    KRYS_NODISCARD List<style::WeakStyleable>
      RelatedTimelineScopeElements(const css::CustomIdentifier &name) noexcept;
    void UpdateCSSAnimationsAssociatedWithNamedTimeline(const css::CSSOMStringAtom &name) noexcept;

    void AttachAnimation(CSSAnimation &animation, AllowsDeferral allowsDeferral) noexcept;
    KRYS_NODISCARD ScrollTimeline *
      DetermineTimelineForElement(const List<Ref<ScrollTimeline>> &timelines,
                                  const style::Styleable &styleable,
                                  const List<style::WeakStyleable> &timelineScopeElements) noexcept;
    KRYS_NODISCARD ScrollTimeline *
      DetermineTreeOrder(const List<Ref<ScrollTimeline>> &ancestorTimelines,
                         const style::Styleable &styleable,
                         const List<style::WeakStyleable> &timelineScopeElements) noexcept;
    KRYS_NODISCARD ScrollTimeline &InactiveNamedTimeline(const css::CSSOMStringAtom &name) noexcept;
  };
}
