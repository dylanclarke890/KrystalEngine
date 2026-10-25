#include "Krystal.Booey/Animation/StyleOriginatedTimelinesController.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/ScrollTimeline.hpp"
#include "Krystal.Booey/Animation/ViewTimeline.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include <algorithm>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD style::WeakStyleable OriginatingElement(const Ref<ScrollTimeline> &timeline) noexcept
    {
      if (const auto *viewTimeline = DynamicDowncast<ViewTimeline>(timeline.get()))
      {
        return viewTimeline->SubjectStyleable();
      }
      return timeline->SourceStyleable();
    }

    KRYS_NODISCARD style::WeakStyleable
      OriginatingStyleableIncludingTimelineScope(const Ref<ScrollTimeline> &timeline) noexcept
    {
      if (const auto &element = timeline->TimelineScopeDeclaredElement())
      {
        return style::Styleable::FromElement(*element);
      }
      return OriginatingElement(timeline);
    }

    KRYS_NODISCARD style::WeakStyleable
      OriginatingElementExcludingTimelineScope(const Ref<ScrollTimeline> &timeline) noexcept
    {
      return timeline->TimelineScopeDeclaredElement() ? style::WeakStyleable() : OriginatingElement(timeline);
    }

    KRYS_NODISCARD bool ContainsElement(const List<style::WeakStyleable> &timelineScopeElements,
                                        const dom::Element *matchElement) noexcept
    {
      return std::ranges::any_of(timelineScopeElements, [matchElement](const style::WeakStyleable &entry)
                                 { return entry.GetElement().get() == matchElement; });
    }

    void UpdateTimelinesForTimelineScope(const List<Ref<ScrollTimeline>> &entries,
                                         const style::Styleable &styleable) noexcept
    {
      for (const auto &entry : entries)
      {
        if (auto entryElement = OriginatingElementExcludingTimelineScope(entry).GetStyleable())
        {
          if (entryElement->Element.IsComposedTreeDescendantOf(styleable.Element))
          {
            entry->SetTimelineScopeElement(styleable.Element);
          }
        }
      }
    }
  }

  StyleOriginatedTimelinesController::StyleOriginatedTimelinesController() noexcept = default;

  StyleOriginatedTimelinesController::~StyleOriginatedTimelinesController() noexcept = default;

  List<style::WeakStyleable> StyleOriginatedTimelinesController::RelatedTimelineScopeElements(
    const css::CustomIdentifier &name) noexcept
  {
    List<style::WeakStyleable> timelineScopeElements;
    for (const auto &scope : _timelineScopeEntries)
    {
      if (scope.second
          && (scope.first.Type == style::NameScope::Type::All
              || (scope.first.Type == style::NameScope::Type::Ident
                  && scope.first.Names.Value.contains(name))))
      {
        timelineScopeElements.push_back(scope.second);
      }
    }
    return timelineScopeElements;
  }

  ScrollTimeline &
    StyleOriginatedTimelinesController::InactiveNamedTimeline(const css::CSSOMStringAtom &name) noexcept
  {
    auto &timelines = TimelinesForName(name);
    timelines.push_back(ScrollTimeline::CreateInactiveStyleOriginatedTimeline(name));
    return *timelines.back();
  }

  ScrollTimeline *StyleOriginatedTimelinesController::DetermineTreeOrder(
    const List<Ref<ScrollTimeline>> &ancestorTimelines, const style::Styleable &styleable,
    const List<style::WeakStyleable> &timelineScopeElements) noexcept
  {
    RefPtr element = ShareRefPtr(&styleable.Element);
    while (element)
    {
      List<Ref<ScrollTimeline>> matchedTimelines;
      for (const auto &timeline : ancestorTimelines)
      {
        if (element.get() == OriginatingStyleableIncludingTimelineScope(timeline).GetElement().get())
        {
          matchedTimelines.push_back(timeline);
        }
      }
      if (!matchedTimelines.empty())
      {
        if (ContainsElement(timelineScopeElements, element.get()))
        {
          if (matchedTimelines.size() == 1)
          {
            return matchedTimelines.front().get();
          }
          // Naming conflict due to timeline-scope, see if the element declares a non-deferred timeline.
          for (const auto &matchedTimeline : matchedTimelines)
          {
            if (element.get() == OriginatingElement(matchedTimeline).GetElement().get())
            {
              return matchedTimeline.get();
            }
          }
          // If we only have deferred timelines, then the timeline is the inactive timeline.
          return &InactiveNamedTimeline(matchedTimelines.front()->GetName());
        }
        krys_debug_assert(matchedTimelines.size() <= 2);
        // Favor scroll timelines in case of conflict
        if (!Is<ViewTimeline>(*matchedTimelines.front()))
        {
          return matchedTimelines.front().get();
        }
        return matchedTimelines.back().get();
      }
      // Has blocking timeline scope element
      if (ContainsElement(timelineScopeElements, element.get()))
      {
        return nullptr;
      }
      element = ShareRefPtr(element->ParentElementInComposedTree());
    }

    krys_debug_assert(false);
    return nullptr;
  }

  ScrollTimeline *StyleOriginatedTimelinesController::DetermineTimelineForElement(
    const List<Ref<ScrollTimeline>> &timelines, const style::Styleable &styleable,
    const List<style::WeakStyleable> &timelineScopeElements) noexcept
  {
    // https://drafts.csswg.org/scroll-animations-1/#timeline-scoping
    // A named scroll progress timeline or view progress timeline is referenceable by:
    // 1. the name-declaring element itself
    // 2. that element’s descendants
    // If multiple elements have declared the same timeline name, the matching timeline is the one declared on
    // the nearest element in tree order. In case of a name conflict on the same element, names declared later
    // in the naming property (scroll-timeline-name, view-timeline-name) take precedence, and scroll progress
    // timelines take precedence over view progress timelines.
    List<Ref<ScrollTimeline>> matchedTimelines;
    for (const auto &timeline : timelines)
    {
      auto styleableForTimeline = OriginatingStyleableIncludingTimelineScope(timeline).GetStyleable();
      if (!styleableForTimeline)
      {
        continue;
      }
      if (&styleableForTimeline->Element == &styleable.Element
          || styleable.Element.IsComposedTreeDescendantOf(styleableForTimeline->Element))
      {
        matchedTimelines.push_back(timeline);
      }
    }
    if (matchedTimelines.empty())
    {
      return nullptr;
    }
    return DetermineTreeOrder(matchedTimelines, styleable, timelineScopeElements);
  }

  List<Ref<ScrollTimeline>> &
    StyleOriginatedTimelinesController::TimelinesForName(const css::CSSOMStringAtom &name) noexcept
  {
    return _nameToTimelineMap[name];
  }

  void StyleOriginatedTimelinesController::UpdateTimelineForTimelineScope(
    const Ref<ScrollTimeline> &timeline, const css::CSSOMStringAtom &name) noexcept
  {
    List<style::Styleable> matchedTimelineScopeElements;
    auto timelineElement = OriginatingElementExcludingTimelineScope(timeline).GetStyleable();
    if (!timelineElement)
    {
      return;
    }

    for (const auto &entry : _timelineScopeEntries)
    {
      if (auto entryElement = entry.second.GetStyleable())
      {
        if (timelineElement->Element.IsComposedTreeDescendantOf(entryElement->Element)
            && (entry.first.Type == style::NameScope::Type::All
                || entry.first.Names.Value.contains(css::CustomIdentifier {name})))
        {
          auto alreadyMatched =
            std::ranges::any_of(matchedTimelineScopeElements,
                                [&](const style::Styleable &matched) { return matched == *entryElement; });
          if (!alreadyMatched)
          {
            matchedTimelineScopeElements.push_back(*entryElement);
          }
        }
      }
    }
    RefPtr element = ShareRefPtr(&timelineElement->Element);
    while (element)
    {
      auto it = std::ranges::find_if(matchedTimelineScopeElements, [&](const style::Styleable &entry)
                                     { return &entry.Element == element.get(); });
      if (it != matchedTimelineScopeElements.end())
      {
        timeline->SetTimelineScopeElement(it->Element);
        return;
      }
      element = ShareRefPtr(element->ParentElementInComposedTree());
    }
  }

  void StyleOriginatedTimelinesController::RegisterNamedScrollTimeline(const css::CSSOMStringAtom &name,
                                                                       const style::Styleable &source,
                                                                       ScrollAxis axis) noexcept
  {
    auto &timelines = TimelinesForName(name);

    auto existingTimeline = std::ranges::find_if(
      timelines,
      [&](const Ref<ScrollTimeline> &timeline)
      {
        return !Is<ViewTimeline>(*timeline) && timeline->SourceStyleable() == style::WeakStyleable(source);
      });

    if (existingTimeline != timelines.end())
    {
      Ref existingScrollTimeline = *existingTimeline;
      existingScrollTimeline->SetAxis(axis);
    }
    else
    {
      auto newScrollTimeline = ScrollTimeline::Create(name, axis);
      newScrollTimeline->SetSource(source);
      UpdateTimelineForTimelineScope(newScrollTimeline, name);
      timelines.push_back(krys::move(newScrollTimeline));
      UpdateCSSAnimationsAssociatedWithNamedTimeline(name);
    }
  }

  void StyleOriginatedTimelinesController::UpdateCSSAnimationsAssociatedWithNamedTimeline(
    const css::CSSOMStringAtom &name) noexcept
  {
    // First, we need to gather all CSS Animations attached to existing timelines with the specified name. We
    // do this prior to updating animation-to-timeline relationship because this could mutate the timeline's
    // animations list.
    ListHashSet<Ref<CSSAnimation>> cssAnimationsWithMatchingTimelineName;
    for (const auto &timeline : TimelinesForName(name))
    {
      for (const auto &animation : timeline->RelevantAnimations())
      {
        if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(animation.get()))
        {
          if (!cssAnimation->OwningElement())
          {
            continue;
          }
          if (auto timelineName = cssAnimation->BackingStyleAnimation().GetTimeline().TryCustomIdentifier())
          {
            if (timelineName->Value == name)
            {
              cssAnimationsWithMatchingTimelineName.add(ShareRef(*cssAnimation));
            }
          }
        }
      }
    }

    for (const auto &cssAnimation : cssAnimationsWithMatchingTimelineName)
    {
      cssAnimation->SyncStyleOriginatedTimeline();
    }
  }

  void StyleOriginatedTimelinesController::RemovePendingOperationsForCSSAnimation(
    const CSSAnimation &animation) noexcept
  {
    std::erase_if(_cssAnimationsPendingAttachment, [&](const Ref<CSSAnimation> &pendingAnimation)
                  { return pendingAnimation.get() == &animation; });
  }

  void StyleOriginatedTimelinesController::DocumentDidResolveStyle() noexcept
  {
    auto cssAnimationsPendingAttachment = std::exchange(_cssAnimationsPendingAttachment, {});
    for (auto &cssAnimationPendingAttachment : cssAnimationsPendingAttachment)
    {
      if (cssAnimationPendingAttachment->OwningElement())
      {
        AttachAnimation(*cssAnimationPendingAttachment, AllowsDeferral::No);
      }
    }

    // Purge any inactive named timeline no longer attached to an animation.
    for (auto it = _nameToTimelineMap.begin(); it != _nameToTimelineMap.end();)
    {
      auto &timelines = it->second;
      std::erase_if(timelines,
                    [](const Ref<ScrollTimeline> &timeline)
                    {
                      return timeline->IsInactiveStyleOriginatedTimeline()
                             && timeline->RelevantAnimations().isEmpty();
                    });
      if (timelines.empty())
      {
        it = _nameToTimelineMap.erase(it);
      }
      else
      {
        ++it;
      }
    }

    _removedTimelines.clear();
  }

  void StyleOriginatedTimelinesController::RegisterNamedViewTimeline(
    const css::CSSOMStringAtom &name, const style::Styleable &subject, ScrollAxis axis,
    const style::ViewTimelineInsetItem &insets) noexcept
  {
    auto &timelines = TimelinesForName(name);

    auto existingTimeline =
      std::ranges::find_if(timelines,
                           [&](const Ref<ScrollTimeline> &timeline)
                           {
                             if (const auto *viewTimeline = DynamicDowncast<ViewTimeline>(timeline.get()))
                             {
                               return viewTimeline->SubjectStyleable() == style::WeakStyleable(subject);
                             }
                             return false;
                           });

    auto hasExistingTimeline = existingTimeline != timelines.end();

    if (hasExistingTimeline)
    {
      Ref existingViewTimeline = ShareRef(Downcast<ViewTimeline>(**existingTimeline));
      existingViewTimeline->SetAxis(axis);
      existingViewTimeline->SetInsets(insets);
    }
    else
    {
      Ref<ScrollTimeline> newViewTimeline = ViewTimeline::Create(name, axis, insets);
      Downcast<ViewTimeline>(*newViewTimeline).SetSubject(subject);
      UpdateTimelineForTimelineScope(newViewTimeline, name);
      timelines.push_back(krys::move(newViewTimeline));
    }

    if (!hasExistingTimeline)
    {
      UpdateCSSAnimationsAssociatedWithNamedTimeline(name);
    }
  }

  void StyleOriginatedTimelinesController::UnregisterNamedTimeline(const css::CSSOMStringAtom &name,
                                                                   const style::Styleable &styleable) noexcept
  {
    auto it = _nameToTimelineMap.find(name);
    if (it == _nameToTimelineMap.end())
    {
      return;
    }

    auto &timelines = it->second;

    auto i = std::ranges::find_if(timelines, [&](const Ref<ScrollTimeline> &entry)
                                  { return OriginatingElement(entry) == style::WeakStyleable(styleable); });

    if (i == timelines.end())
    {
      return;
    }

    Ref timeline = *i;

    // Make sure to remove the named timeline from our name-to-timelines map first, such that re-syncing any
    // CSS Animation previously registered with it resolves their `animation-timeline` properly.
    timelines.erase(i);

    List<Ref<WebAnimation>> animations(timeline->RelevantAnimations().begin(),
                                       timeline->RelevantAnimations().end());
    for (auto &animation : animations)
    {
      if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(animation.get()))
      {
        if (cssAnimation->OwningElement())
        {
          cssAnimation->SyncStyleOriginatedTimeline();
        }
      }
    }

    if (timelines.empty())
    {
      _nameToTimelineMap.erase(it);
    }
    else
    {
      UpdateCSSAnimationsAssociatedWithNamedTimeline(name);
    }
  }

  void StyleOriginatedTimelinesController::AttachAnimation(CSSAnimation &animation) noexcept
  {
    AttachAnimation(animation, AllowsDeferral::Yes);
  }

  void StyleOriginatedTimelinesController::AttachAnimation(CSSAnimation &animation,
                                                           AllowsDeferral allowsDeferral) noexcept
  {
    Ref protectedAnimation = ShareRef(animation);

    auto target = protectedAnimation->OwningElement();
    if (!target)
    {
      return;
    }

    auto timelineName = protectedAnimation->BackingStyleAnimation().GetTimeline().TryCustomIdentifier();
    if (!timelineName)
    {
      return;
    }

    auto it = _nameToTimelineMap.find(timelineName->Value);
    auto hasNamedTimeline =
      it != _nameToTimelineMap.end()
      && std::ranges::any_of(it->second, [](const Ref<ScrollTimeline> &timeline)
                             { return !timeline->IsInactiveStyleOriginatedTimeline(); });

    // If we don't have an active named timeline yet and deferral is allowed, just register a pending timeline
    // attachment operation so we can try again when style has resolved.
    if (!hasNamedTimeline && allowsDeferral == AllowsDeferral::Yes)
    {
      _cssAnimationsPendingAttachment.push_back(ShareRef(animation));
      return;
    }

    auto timelineScopeElements = RelatedTimelineScopeElements(*timelineName);

    if (!hasNamedTimeline)
    {
      // First, determine whether the name is within scope, ie. whether a parent element has a
      // `timeline-scope` property that contains this timeline name.
      auto nameIsWithinScope = [&]
      {
        for (const auto &timelineScopeElement : timelineScopeElements)
        {
          krys_debug_assert(timelineScopeElement.GetElement());
          if (style::WeakStyleable(*target) == timelineScopeElement
              || target->Element.IsComposedTreeDescendantOf(*timelineScopeElement.GetElement()))
          {
            return true;
          }
        }
        return false;
      }();

      krys_debug_assert(allowsDeferral == AllowsDeferral::No);
      // We don't have an active named timeline and yet we must set a timeline since we've already dealt with
      // the defferal case before. There are two cases:
      //     1. the name is within scope and we should create a placeholder inactive scroll timeline, or,
      //     2. the name is not within scope and the timeline is null.
      if (nameIsWithinScope)
      {
        protectedAnimation->SetTimeline(
          ShareRefPtr<AnimationTimeline>(&InactiveNamedTimeline(timelineName->Value)));
      }
      else
      {
        protectedAnimation->SetTimeline(nullptr);
      }
    }
    else
    {
      auto &timelines = it->second;
      RefPtr timeline = ShareRefPtr(DetermineTimelineForElement(timelines, *target, timelineScopeElements));
      // A deferred inactive timeline means there was a conflict with multiple timelines existing within a
      // parent element with a "timeline-scope" property. In that case, we must reconsider timeline attachment
      // once style resolution completes as further updates may occur that would yield a different timeline
      // and possibly also mark that animation's target as dirty to update the animated style.
      if (allowsDeferral == AllowsDeferral::Yes && timeline && timeline->IsInactiveStyleOriginatedTimeline())
      {
        _cssAnimationsPendingAttachment.push_back(ShareRef(animation));
      }
      protectedAnimation->SetTimeline(RefPtr<AnimationTimeline> {krys::move(timeline)});
    }

    // Since we have no timeline defined for this name yet, we need to mark this animation as pending
    // attachment in case this name resolves in the future.
    if (!protectedAnimation->Timeline())
    {
      _cssAnimationsPendingAttachment.push_back(ShareRef(animation));
    }
  }

  void StyleOriginatedTimelinesController::UpdateNamedTimelineMapForTimelineScope(
    const style::NameScope &scope, const style::Styleable &styleable) noexcept
  {
    // https://drafts.csswg.org/scroll-animations-1/#timeline-scope
    // This property declares the scope of the specified timeline names to extend across this element’s
    // subtree. This allows a named timeline (such as a named scroll progress timeline or named view progress
    // timeline) to be referenced by elements outside the timeline-defining element’s subtree—​for example,
    // by siblings, cousins, or ancestors.
    switch (scope.Type)
    {
      case style::NameScope::Type::None:
      {
        ListHashSet<Ref<ScrollTimeline>> namedTimelinesToUnregister;
        for (auto &entry : _nameToTimelineMap)
        {
          for (auto &timeline : entry.second)
          {
            if (timeline->TimelineScopeDeclaredElement().get() == &styleable.Element)
            {
              timeline->ClearTimelineScopeDeclaredElement();
              // Make sure to track this time to be unregistered in a separate step as it would modify the map
              // we're currently iterating over.
              namedTimelinesToUnregister.add(timeline);
            }
          }
        }
        std::erase_if(_timelineScopeEntries, [&](const Pair<style::NameScope, style::WeakStyleable> &entry)
                      { return entry.second == style::WeakStyleable(styleable); });
        // We need to unregister all timelines that are no longer within this scope.
        for (const auto &timeline : namedTimelinesToUnregister)
        {
          if (auto associatedElement = OriginatingElement(timeline).GetStyleable())
          {
            UnregisterNamedTimeline(timeline->GetName(), *associatedElement);
          }
        }
        break;
      }
      case style::NameScope::Type::All:
        for (auto &entry : _nameToTimelineMap)
        {
          UpdateTimelinesForTimelineScope(entry.second, styleable);
        }
        _timelineScopeEntries.emplace_back(scope, styleable);
        break;
      case style::NameScope::Type::Ident:
        for (const auto &name : scope.Names)
        {
          auto it = _nameToTimelineMap.find(name.Value);
          if (it != _nameToTimelineMap.end())
          {
            UpdateTimelinesForTimelineScope(it->second, styleable);
          }
        }
        _timelineScopeEntries.emplace_back(scope, styleable);
        break;
    }
  }

  bool StyleOriginatedTimelinesController::IsPendingTimelineAttachment(
    const WebAnimation &animation) const noexcept
  {
    if (const auto *cssAnimation = DynamicDowncast<CSSAnimation>(animation))
    {
      return std::ranges::any_of(_cssAnimationsPendingAttachment,
                                 [&](const Ref<CSSAnimation> &pendingAnimation)
                                 { return pendingAnimation.get() == cssAnimation; });
    }
    return false;
  }

  void StyleOriginatedTimelinesController::UnregisterNamedTimelinesAssociatedWithElement(
    const style::Styleable &styleable) noexcept
  {
    for (auto it = _nameToTimelineMap.begin(); it != _nameToTimelineMap.end();)
    {
      auto &timelines = it->second;
      for (size_t i = 0; i < timelines.size(); ++i)
      {
        auto &timeline = timelines[i];
        if (OriginatingElement(timeline) == style::WeakStyleable(styleable))
        {
          _removedTimelines.add(timeline);
          timelines.erase(timelines.begin() + static_cast<std::ptrdiff_t>(i--));
        }
      }
      if (timelines.empty())
      {
        it = _nameToTimelineMap.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  void StyleOriginatedTimelinesController::StyleableWasRemoved(const style::Styleable &styleable) noexcept
  {
    List<Ref<ScrollTimeline>> removedTimelines(_removedTimelines.begin(), _removedTimelines.end());
    for (const auto &timeline : removedTimelines)
    {
      if (OriginatingElement(timeline) != style::WeakStyleable(styleable))
      {
        continue;
      }
      List<Ref<WebAnimation>> animations(timeline->RelevantAnimations().begin(),
                                         timeline->RelevantAnimations().end());
      for (const auto &animation : animations)
      {
        if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(animation.get()))
        {
          if (auto owningElement = cssAnimation->OwningElement())
          {
            AttachAnimation(*cssAnimation, AllowsDeferral::Yes);
            owningElement->Element.InvalidateStyleForAnimation();
          }
        }
      }
    }
  }
}
