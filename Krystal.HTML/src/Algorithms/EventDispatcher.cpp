#include "Krystal.HTML/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ShadowRootAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.HTML/Constants/EventNames.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/Event.hpp"
#include "Krystal.HTML/DOM/Event/EventListener.hpp"
#include "Krystal.HTML/DOM/EventTarget.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include <cassert>
#include <ranges>

namespace Krys::HTML
{
  bool EventDispatcher::DispatchToTarget(Event &event, RawPtr<EventTarget> target,
                                         bool legacyTargetOverrideFlag,
                                         RawPtr<bool> legacyOutputDidListenersThrowFlag) noexcept
  {
    // NOTE: `target` is intentionally a raw pointer instead of a reference as we reassign the target later
    // on, would be a pain to do with a reference.
    assert(target);
    event._dispatched = true;

    auto *targetOverride = target;
    if (legacyTargetOverrideFlag) // only used by HTML and only when target is a Window object.
    {
      assert(target->IsWindow());

      // SPEC-VIOLATION(DOM, HTML): We don't actually implement Window. Might implement in the future.
      // If legacy target override flag is given, let targetOverride be target’s associated Document.
      assert(false && "Not implemented");
    }

    RawPtr<EventTarget> activationTarget = nullptr;
    RawPtr<EventTarget> relatedTarget = ShadowRootAlgorithms::Retarget(event.RelatedTarget().get(), *target);
    bool clearTargets = false;

    if (target != relatedTarget || target == event.RelatedTarget())
    {
      List<Ref<EventTarget>> touchTargets;
      for (auto &touchTarget : event.TouchTargetList())
      {
        touchTargets.push_back(ShareRef(*ShadowRootAlgorithms::Retarget(touchTarget.get(), *target)));
      }

      AppendToEventPath(event, *target, targetOverride, relatedTarget, touchTargets, false);

      bool isActivationEvent = event.IsMouseEvent() && event.Type() == EventNames::Click;
      if (isActivationEvent && target->HasActivationBehavior())
      {
        activationTarget = target;
      }

      RawPtr<EventTarget> slottable = nullptr;
      if (auto *nodeTarget = DynamicDowncast<Node>(target))
      {
        if (SlotAlgorithms::IsSlottable(*nodeTarget) && SlotAlgorithms::IsAssigned(*nodeTarget))
        {
          slottable = target;
        }
      }

      bool slotInClosedTree = false;
      RawPtr<EventTarget> parent = target->GetParent(event);

      while (parent != nullptr)
      {
        RawPtr<Node> nodeParent = DynamicDowncast<Node>(parent);

        if (slottable != nullptr)
        {
          assert(Is<HTMLSlotElement>(nodeParent));

          slottable = nullptr;

          if (auto shadowRootParent = DynamicDowncast<ShadowRoot>(TreeQueries::Root(*nodeParent)))
          {
            slotInClosedTree = shadowRootParent->Mode() == ShadowRootMode::Closed;
          }
        }

        if (nodeParent != nullptr && SlotAlgorithms::IsSlottable(*nodeParent)
            && SlotAlgorithms::IsAssigned(*nodeParent))
        {
          slottable = parent;
        }

        relatedTarget = ShadowRootAlgorithms::Retarget(event.RelatedTarget().get(), *parent);

        touchTargets.clear();
        for (auto &touchTarget : event.TouchTargetList())
        {
          touchTargets.push_back(ShareRef(*ShadowRootAlgorithms::Retarget(touchTarget.get(), *parent)));
        }

        RawPtr<Node> targetRoot =
          nodeParent != nullptr ? &TreeQueries::Root(Downcast<Node>(*target)) : nullptr;

        if (parent->IsWindow()
            || (Is<Node>(parent)
                && ShadowRootAlgorithms::IsShadowIncludingInclusiveAncestor(*targetRoot, *nodeParent)))
        {
          if (isActivationEvent && event.Bubbles() && activationTarget == nullptr
              && parent->HasActivationBehavior())
          {
            activationTarget = parent;
          }

          AppendToEventPath(event, *parent, nullptr, relatedTarget, touchTargets, slotInClosedTree);
        }
        else if (parent == relatedTarget)
        {
          parent = nullptr;
        }
        else
        {
          target = parent;
          if (isActivationEvent && activationTarget == nullptr && target->HasActivationBehavior())
          {
            activationTarget = target;
          }
          AppendToEventPath(event, *parent, target, relatedTarget, touchTargets, slotInClosedTree);
        }

        if (parent != nullptr)
        {
          parent = parent->GetParent(event);
        }

        slotInClosedTree = false;
      }

      auto clearTargetStruct =
        std::find_if(event._path.rbegin(), event._path.rend(),
                     [](const EventPathItem &i) { return i.ShadowAdjustedTarget() != nullptr; });

      if (clearTargetStruct != event._path.rend())
      {
        auto IsNodeWithShadowRoot = [](RawPtr<EventTarget> target)
        {
          if (auto *node = DynamicDowncast<Node>(target))
          {
            return Is<ShadowRoot>(ShadowRootAlgorithms::ShadowIncludingRoot(*node));
          }

          return false;
        };

        clearTargets = IsNodeWithShadowRoot(clearTargetStruct->ShadowAdjustedTarget())
                       || IsNodeWithShadowRoot(clearTargetStruct->RelatedTarget());

        if (!clearTargets)
        {
          if (std::ranges::any_of(clearTargetStruct->TouchTargetList(),
                                  [&](auto &touchTarget) { return IsNodeWithShadowRoot(touchTarget.get()); }))
          {
            clearTargets = true;
          }
        }
      }

      if (activationTarget != nullptr && activationTarget->HasLegacyPreActivationBehavior())
      {
        activationTarget->LegacyPreActivationBehavior(event);
      }

      for (auto &structItem : event._path | std::views::reverse)
      {
        if (structItem.ShadowAdjustedTarget() != nullptr)
        {
          event._eventPhase = EventPhaseType::AT_TARGET;
        }
        else
        {
          event._eventPhase = EventPhaseType::CAPTURING_PHASE;
        }

        Invoke(structItem, event, EventPhaseType::CAPTURING_PHASE, legacyOutputDidListenersThrowFlag);
      }

      for (auto &structItem : event._path)
      {
        if (structItem.ShadowAdjustedTarget() != nullptr)
        {
          event._eventPhase = EventPhaseType::AT_TARGET;
        }
        else
        {
          if (!event.Bubbles())
          {
            continue;
          }

          event._eventPhase = EventPhaseType::BUBBLING_PHASE;
        }

        Invoke(structItem, event, EventPhaseType::BUBBLING_PHASE, legacyOutputDidListenersThrowFlag);
      }
    }

    event._eventPhase = EventPhaseType::NONE;
    event._currentTarget = nullptr;
    event._path.clear();
    event._dispatched = false;
    event._stopPropagation = false;
    event._stopImmediatePropagation = false;

    if (clearTargets)
    {
      event._target = nullptr;
      event.RelatedTarget(nullptr);
      event.TouchTargetList({});
    }

    if (activationTarget != nullptr)
    {
      if (!event._canceled)
      {
        activationTarget->ActivationBehavior(event);
      }
      else if (activationTarget->HasLegacyCanceledActivationBehavior())
      {
        activationTarget->LegacyCanceledActivationBehavior(event);
      }
    }

    return !event._canceled;
  }

  void EventDispatcher::AppendToEventPath(Event &event, EventTarget &invocationTarget,
                                          RawPtr<EventTarget> shadowAdjustedTarget,
                                          RawPtr<EventTarget> relatedTarget,
                                          List<Ref<EventTarget>> &touchTargets,
                                          bool slotInClosedTree) noexcept
  {
    bool invocationTargetInShadowTree = false;
    bool rootOfClosedTree = false;

    if (auto *invocationTargetNode = DynamicDowncast<Node>(invocationTarget))
    {
      invocationTargetInShadowTree = TreeQueries::IsInShadowTree(*invocationTargetNode);

      if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(invocationTargetNode))
      {
        rootOfClosedTree = shadowRoot->Mode() == ShadowRootMode::Closed;
      }
    }

    event._path.push_back(EventPathItem {invocationTarget, invocationTargetInShadowTree, shadowAdjustedTarget,
                                         relatedTarget, touchTargets, rootOfClosedTree, slotInClosedTree});
  }

  void EventDispatcher::Invoke(EventPathItem &pathStruct, Event &event, EventPhaseType phase,
                               RawPtr<bool> legacyOutputDidListenersThrowFlag) noexcept
  {
    auto structIt = std::find_if(event._path.begin(), event._path.end(),
                                 [&](const EventPathItem &i) { return &i == &pathStruct; });

    assert(structIt != event._path.end());

    auto lastNonNullShadowAdjustedTarget =
      std::find_if(std::make_reverse_iterator(structIt + 1uz), event._path.rend(),
                   [](const EventPathItem &i) { return i.ShadowAdjustedTarget() != nullptr; });

    assert(lastNonNullShadowAdjustedTarget != event._path.rend());

    event._target = ShareRefPtr(lastNonNullShadowAdjustedTarget->ShadowAdjustedTarget());
    event.RelatedTarget(pathStruct.RelatedTarget());
    event.TouchTargetList(pathStruct.TouchTargetList());

    if (event._stopPropagation)
    {
      return;
    }

    event._currentTarget = ShareRefPtr(pathStruct.InvocationTarget());

    SmallList<Ref<EventListener>> listeners = event._currentTarget->_eventListenerList;

    bool invocationTargetIsShadowRoot = pathStruct.InvocationTargetInShadowTree();
    bool found =
      InnerInvoke(event, listeners, phase, invocationTargetIsShadowRoot, legacyOutputDidListenersThrowFlag);

    if (!found && event._isTrusted)
    {
      auto originalEventType = event.Type();

      if (event.Type() == EventNames::AnimationEnd)
      {
        event._type = EventNames::Legacy::AnimationEnd;
      }
      else if (event.Type() == EventNames::AnimationIteration)
      {
        event._type = EventNames::Legacy::AnimationIteration;
      }
      else if (event.Type() == EventNames::AnimationStart)
      {
        event._type = EventNames::Legacy::AnimationStart;
      }
      else if (event.Type() == EventNames::TransitionEnd)
      {
        event._type = EventNames::Legacy::TransitionEnd;
      }
      else
      {
        return;
      }

      InnerInvoke(event, listeners, phase, invocationTargetIsShadowRoot, legacyOutputDidListenersThrowFlag);
      event._type = originalEventType;
    }
  }

  bool EventDispatcher::InnerInvoke(Event &event, SmallList<Ref<EventListener>> &listeners,
                                    EventPhaseType phase, bool invocationTargetInShadowTree,
                                    RawPtr<bool> legacyOutputDidListenersThrowFlag) noexcept
  {
    bool found = false;

    for (auto &listener : listeners)
    {
      if (listener->Removed())
      {
        continue;
      }

      if (event.Type() != listener->Type())
      {
        continue;
      }

      found = true;

      if (phase == EventPhaseType::CAPTURING_PHASE && !listener->Capture())
      {
        continue;
      }

      if (phase == EventPhaseType::BUBBLING_PHASE && listener->Capture())
      {
        continue;
      }

      if (listener->Once())
      {
        EventTargetAlgorithms::RemoveEventListener(*event._currentTarget, *listener);
      }

      // SPEC-VIOLATION(HTML): realms/global objects currently not supported.
      // Let global be listener callback’s associated realm’s global object. 
      // Let currentEvent be undefined.
      // If global is a Window object :
      //   Set currentEvent to global’s current event.
      //   If invocationTargetInShadowTree is false, then set global’s current event to event.

      if (listener->Passive() == true)
      {
        event._inPassiveListener = true;
      }

      // SPEC-VIOLATION(HTML): window/global objects currently not supported.
      // If global is a Window object, then record timing info for event listener given event and listener.

      // SPEC-VIOLATION(WEBIDL): this is javascript-specific and we don't have JS objects or exceptions, so
      // we'll just call the callback directly.
      // Call a user object’s operation with listener’s callback, "handleEvent", « event », and event’s
      // currentTarget attribute value. If this throws an exception exception:
      //   Report exception for listener’s callback’s corresponding JavaScript object’s associated realm’s
      //   global object.
      //   Set legacyOutputDidListenersThrowFlag if given.

      // TODO(impl): LOGGING - we should probably have some way to report exceptions that occur during event
      // listener invocations, even if we don't have JS objects or realms.

      listener->Callback()->HandleEvent(event);
      event._inPassiveListener = false;

      // SPEC-VIOLATION(HTML): window/global objects currently not supported.
      // If global is a Window object, then set global’s current event to currentEvent.

      if (event._stopImmediatePropagation)
      {
        break;
      }
    }

    return found;
  }
}