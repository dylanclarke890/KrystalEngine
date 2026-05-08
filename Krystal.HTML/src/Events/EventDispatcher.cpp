#include "Krystal.HTML/Events/EventDispatcher.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ShadowRootAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Events/Event.hpp"
#include "Krystal.HTML/Events/EventNames.hpp"
#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <ranges>

namespace Krys::HTML
{
  bool EventDispatcher::DispatchToTarget(Event &event, RawPtr<EventTarget> target,
                                         bool legacyTargetOverrideFlag,
                                         bool legacyOutputDidListenersThrowFlag) noexcept
  {
    assert(target);
    event._dispatched = true;

    auto *targetOverride = target;
    if (legacyTargetOverrideFlag)
    {
      // legacy target override flag is only used by HTML and only when target is a Window object.
      assert(target->IsWindow());

      // SPEC-VIOLATION(DOM, HTML): We don't actually implement Window.
      // Let targetOverride be target, if legacy target override flag is not given, and target’s associated
      // Document otherwise.
    }

    RawPtr<EventTarget> activationTarget = nullptr;
    RawPtr<EventTarget> relatedTarget = ShadowRootAlgorithms::Retarget(event.RelatedTarget(), *target);
    bool clearTargets = false;

    if (target != relatedTarget || target == event.RelatedTarget())
    {
      List<Ref<EventTarget>> touchTargets;
      for (auto &touchTarget : event.TouchTargetList())
      {
        touchTargets.push_back(ShareRef(*ShadowRootAlgorithms::Retarget(touchTarget.get(), *target)));
      }

      assert(relatedTarget);
      AppendToEventPath(event, *target, targetOverride, *relatedTarget, touchTargets, false);

      bool isActivationEvent = event.IsMouseEvent() && event.Type() == EventNames::Click;
      if (isActivationEvent && target->HasActivationBehavior())
      {
        activationTarget = target;
      }

      bool slotInClosedTree = false;
      // TODO(impl): Let slottable be target, if target is a slottable and is assigned, and null otherwise.
      RawPtr<EventTarget> slottable = nullptr;
      RawPtr<EventTarget> parent = target->GetParent(event);

      while (parent != nullptr)
      {
        RawPtr<Node> parentNode = DynamicDowncast<Node>(parent);
        if (slottable != nullptr)
        {
          // TODO(impl): assert: parent is a slot.
          slottable = nullptr;

          auto parentRoot = parent->IsNode() ? &TreeQueries::Root(*parentNode) : nullptr;
          auto parentShadowRoot = DynamicDowncast<ShadowRoot>(parentRoot);
          if (parentShadowRoot && parentShadowRoot->Mode() == ShadowRootMode::Closed)
          {
            slotInClosedTree = true;
          }
        }

        // TODO(impl): If parent is a slottable and is assigned, then set slottable to parent.
        relatedTarget = ShadowRootAlgorithms::Retarget(event.RelatedTarget(), *parent);
        touchTargets.clear();
        for (auto &touchTarget : event.TouchTargetList())
        {
          touchTargets.push_back(ShareRef(*ShadowRootAlgorithms::Retarget(touchTarget.get(), *parent)));
        }

        RawPtr<Node> targetRoot = parentNode != nullptr ? Downcast<Node>(target) : nullptr;
        bool rootIsShadowIncludingInclusiveAncestor =
          parent->IsNode()
          && ShadowRootAlgorithms::IsShadowIncludingInclusiveAncestor(*targetRoot, *parentNode);

        if (parent->IsWindow() || rootIsShadowIncludingInclusiveAncestor)
        {
          if (isActivationEvent && event.Bubbles() && activationTarget == nullptr
              && parent->HasActivationBehavior())
          {
            activationTarget = parent;
          }

          assert(relatedTarget);
          AppendToEventPath(event, *parent, targetOverride, *relatedTarget, touchTargets, slotInClosedTree);
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
          AppendToEventPath(event, *parent, target, *relatedTarget, touchTargets, slotInClosedTree);
        }

        if (parent != nullptr)
        {
          parent = parent->GetParent(event);
        }

        slotInClosedTree = false;
      }

      auto clearTargetStruct =
        std::find_if(event._path->PathItems().rbegin(), event._path->PathItems().rend(),
                     [](const EventPathItem &i) { return i.ShadowAdjustedTarget() != nullptr; });

      if (clearTargetStruct != event._path->PathItems().rend())
      {
        auto IsNodeWithShadowRoot = [](RawPtr<EventTarget> target)
        {
          if (target == nullptr)
          {
            return false;
          }

          if (auto *node = DynamicDowncast<Node>(target))
          {
            if (Is<ShadowRoot>(ShadowRootAlgorithms::ShadowIncludingRoot(*node)))
            {
              return true;
            }
          }

          return false;
        };

        if (IsNodeWithShadowRoot(clearTargetStruct->ShadowAdjustedTarget())
            || IsNodeWithShadowRoot(clearTargetStruct->RelatedTarget()))
        {
          clearTargets = true;
        }
        else
        {
          auto touchTargetWithShadowRoot = std::find_if(
            clearTargetStruct->TouchTargetList().begin(), clearTargetStruct->TouchTargetList().end(),
            [&](auto &i) { return IsNodeWithShadowRoot(i.get()); });

          if (touchTargetWithShadowRoot != clearTargetStruct->TouchTargetList().end())
          {
            clearTargets = true;
          }
        }
      }

      if (activationTarget != nullptr && activationTarget->HasLegacyPreActivationBehavior())
      {
        activationTarget->LegacyPreActivationBehavior(event);
      }

      for (auto &structItem : event._path->PathItems() | std::views::reverse)
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

      for (auto &structItem : event._path->PathItems())
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
    event._path->PathItems().clear();
    event._dispatched = false;
    event._stopPropagation = false;
    event._stopImmediatePropagation = false;

    if (clearTargets)
    {
      event._target = nullptr;
      event.RelatedTarget(nullptr);
      event.TouchTargetList().clear();
    }

    if (activationTarget != nullptr)
    {
      if (!event._cancelled)
      {
        activationTarget->ActivationBehavior(event);
      }
      else if (activationTarget->HasLegacyCanceledActivationBehavior())
      {
        activationTarget->LegacyCanceledActivationBehavior(event);
      }
    }

    return !event._cancelled;
  }

  void EventDispatcher::AppendToEventPath(Event &event, EventTarget &invocationTarget,
                                          RawPtr<EventTarget> shadowAdjustedTarget,
                                          EventTarget &relatedTarget, List<Ref<EventTarget>> &touchTargets,
                                          bool slotInClosedTree) noexcept
  {
    bool invocationTargetInShadowTree = false;
    bool rootOfClosedTree = false;

    if (auto *invocationTargetNode = DynamicDowncast<Node>(invocationTarget))
    {
      if (Is<ShadowRoot>(TreeQueries::Root(*invocationTargetNode)))
      {
        invocationTargetInShadowTree = true;
      }

      if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(invocationTargetNode))
      {
        if (shadowRoot->Mode() == ShadowRootMode::Closed)
        {
          rootOfClosedTree = true;
        }
      }
    }

    event._path->PathItems().emplace_back(invocationTarget, invocationTargetInShadowTree,
                                          shadowAdjustedTarget, &relatedTarget, touchTargets,
                                          rootOfClosedTree, slotInClosedTree);
  }

  void EventDispatcher::Invoke(EventPathItem &pathStruct, Event &event, EventPhaseType phase,
                               bool legacyOutputDidListenersThrowFlag) noexcept
  {
    auto indexOfStruct =
      std::distance(event._path->PathItems().rbegin(),
                    std::find_if(event._path->PathItems().rbegin(), event._path->PathItems().rend(),
                                 [&](const EventPathItem &i) { return &i == &pathStruct; }));
    auto lastNonNullShadowAdjustedTarget =
      std::find_if(event._path->PathItems().rbegin(), event._path->PathItems().rbegin() + indexOfStruct,
                   [](const EventPathItem &i) { return i.ShadowAdjustedTarget() != nullptr; });
    assert(lastNonNullShadowAdjustedTarget != event._path->PathItems().rend());

    event._target = lastNonNullShadowAdjustedTarget != event._path->PathItems().rend()
                      ? ShareRefPtr(lastNonNullShadowAdjustedTarget->ShadowAdjustedTarget())
                      : nullptr;
    event.RelatedTarget(ShareRefPtr(pathStruct.RelatedTarget()));
    event.TouchTargetList(pathStruct.TouchTargetList());

    if (event._stopPropagation)
    {
      return;
    }

    event._currentTarget = ShareRefPtr(pathStruct.InvocationTarget());

    SmallList<Ref<RegisteredEventListener>> listeners;
    for (auto &listener : pathStruct.InvocationTarget()->_listeners)
    {
      listeners.push_back(ShareRef(*listener));
    }

    bool invocationTargetIsShadowRoot = pathStruct.InvocationTargetInShadowTree();
    bool found =
      InnerInvoke(event, listeners, phase, invocationTargetIsShadowRoot, legacyOutputDidListenersThrowFlag);

    if (found || !event._isTrusted)
    {
      return;
    }

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

  bool EventDispatcher::InnerInvoke(Event &event, SmallList<Ref<RegisteredEventListener>> &listeners,
                                    EventPhaseType phase, bool invocationTargetInShadowTree,
                                    bool &legacyOutputDidListenersThrowFlag) noexcept
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
        EventTarget::RemoveEventListener(*event._currentTarget, *listener);
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

      // SPEC-VIOLATION(HTML): global objects/Window currently not supported.
      // If global is a Window object, then record timing info for event listener given event and listener.

      // SPEC-VIOLATION(WEBIDL): this is javascript-specific and we don't have JS objects or exceptions, so
      // we'll just call the callback directly.
      // Call a user object’s operation with listener’s callback, "handleEvent", « event », and event’s
      // currentTarget attribute value. If this throws an exception exception:
      //   Report exception for listener’s callback’s corresponding JavaScript object’s associated realm’s
      //   global object.
      //   Set legacyOutputDidListenersThrowFlag if given.

      // TODO(impl): we should probably have some way to report exceptions that occur during event listener
      // invocation, even if we don't have JS objects or realms.
      listener->Callback()->HandleEvent(event);
      event._inPassiveListener = false;

      // SPEC-VIOLATION(HTML): global objects currently not supported.
      // NOTE: this uses the legacy extensions to the windows interface which exposes an event attribute.
      // If global is a Window object, then set global’s current event to currentEvent.

      if (event.IsImmediatePropagationStopped())
      {
        break;
      }
    }
    return found;
  }
}