#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Internals/Event/EventListener.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include <ranges>

namespace krys::boo::dom
{
  namespace
  {
    void CallDefaultEventHandlersInBubblingOrder(Event &event, Span<const Ref<Node>> path) noexcept
    {
      if (path.empty())
      {
        return;
      }

      // Non-bubbling events call only one default event handler, the one for the target.
      path[0]->DefaultEventHandler(event);
      krys_debug_assert(!event.DefaultPrevented());
      if (event.DefaultHandled() || !event.Bubbles())
      {
        return;
      }

      for (size_t i = 1; i < path.size(); ++i)
      {
        path[i]->DefaultEventHandler(event);
        krys_debug_assert(!event.DefaultPrevented());
        if (event.DefaultHandled())
        {
          return;
        }
      }
    }
  }

  bool EventDispatcher::DispatchToTarget(Event &event, EventTarget *target, bool legacyTargetOverrideFlag,
                                         bool *legacyOutputDidListenersThrowFlag) noexcept
  {
    // `target` is a pointer, as step 6i.viii.A reassigns it.
    krys_debug_assert(target);
    auto *originalTarget = target;

    // 1. Set event's dispatch flag.
    event._dispatched = true;

    // 2. Let targetOverride be target, if legacy target override flag is not given, and target's associated
    // Document otherwise.
    auto *targetOverride = target;
    if (legacyTargetOverrideFlag)
    {
      // Only HTML gives the flag, and only for a Window.
      krys_debug_assert(target->IsWindow());
      targetOverride = Downcast<LocalDOMWindow>(*target).Document();
      krys_debug_assert(targetOverride);
    }

    // 3. Let activationTarget be null.
    EventTarget *activationTarget = nullptr;

    // 4. Let relatedTarget be the result of retargeting event's relatedTarget against target.
    EventTarget *relatedTarget = EventTargetAlgorithms::Retarget(event.RelatedTarget().get(), *target);

    // 5. Let clearTargets be false.
    bool clearTargets = false;

    // 6. If target is not relatedTarget or target is event's relatedTarget:
    if (target != relatedTarget || target == event.RelatedTarget())
    {
      // 6a. Let touchTargets be a new list.
      List<Ref<EventTarget>> touchTargets;

      // 6b. For each touchTarget of event's touch target list: append the result of retargeting touchTarget
      // against target to touchTargets.
      for (auto &touchTarget : event.TouchTargetList())
      {
        touchTargets.push_back(ShareRef(*EventTargetAlgorithms::Retarget(touchTarget.get(), *target)));
      }

      // 6c. Append to an event path with event, target, targetOverride, relatedTarget, touchTargets, and
      // false.
      AppendToEventPath(event, *target, targetOverride, relatedTarget, touchTargets, false);

      // 6d. Let isActivationEvent be true, if event is a MouseEvent object and event's type attribute is
      // "click"; otherwise false.
      bool isActivationEvent = event.IsMouseEvent() && event.Type() == EventNames::Click;

      // 6e. If isActivationEvent is true and target has activation behavior, then set activationTarget to
      // target.
      if (isActivationEvent && target->HasActivationBehavior())
      {
        activationTarget = target;
      }

      // 6f. Let slottable be target, if target is a slottable and is assigned, and null otherwise.
      EventTarget *slottable = nullptr;
      if (auto *nodeTarget = DynamicDowncast<Node>(target))
      {
        if (SlotAlgorithms::IsSlottable(*nodeTarget) && SlotAlgorithms::IsAssigned(*nodeTarget))
        {
          slottable = target;
        }
      }

      // 6g. Let slotInClosedTree be false.
      bool slotInClosedTree = false;

      // 6h. Let parent be the result of invoking target's get the parent with event.
      EventTarget *parent = target->GetParent(event);

      // 6i. While parent is non-null:
      while (parent != nullptr)
      {
        Node *nodeParent = DynamicDowncast<Node>(parent);

        // 6i.i. If slottable is non-null:
        if (slottable != nullptr)
        {
          // 6i.i.A. Assert: parent is a slot.
          krys_debug_assert(Is<html::HTMLSlotElement>(nodeParent));

          // 6i.i.B. Set slottable to null.
          slottable = nullptr;

          // 6i.i.C. If parent's root is a shadow root whose mode is "closed", then set slotInClosedTree to
          // true.
          if (auto shadowRootParent = DynamicDowncast<ShadowRoot>(TreeQueries::Root(*nodeParent)))
          {
            slotInClosedTree = shadowRootParent->Mode() == ShadowRootMode::Closed;
          }
        }

        // 6i.ii. If parent is a slottable and is assigned, then set slottable to parent.
        if (nodeParent != nullptr && SlotAlgorithms::IsSlottable(*nodeParent)
            && SlotAlgorithms::IsAssigned(*nodeParent))
        {
          slottable = parent;
        }

        // 6i.iii. Let relatedTarget be the result of retargeting event's relatedTarget against parent.
        relatedTarget = EventTargetAlgorithms::Retarget(event.RelatedTarget().get(), *parent);

        // 6i.iv. Let touchTargets be a new list.
        touchTargets.clear();

        // 6i.v. For each touchTarget of event's touch target list: append the result of retargeting
        // touchTarget against parent to touchTargets.
        for (auto &touchTarget : event.TouchTargetList())
        {
          touchTargets.push_back(ShareRef(*EventTargetAlgorithms::Retarget(touchTarget.get(), *parent)));
        }

        // 6i.vi. If parent is a Window object, or parent is a node and target's root is a shadow-including
        // inclusive ancestor of parent:
        if (parent->IsWindow()
            || (nodeParent != nullptr
                && TreeQueries::IsShadowIncludingInclusiveAncestor(TreeQueries::Root(Downcast<Node>(*target)),
                                                                   *nodeParent)))
        {
          // 6i.vi.A. If isActivationEvent is true, event's bubbles attribute is true, activationTarget is
          // null, and parent has activation behavior, then set activationTarget to parent.
          if (isActivationEvent && event.Bubbles() && activationTarget == nullptr
              && parent->HasActivationBehavior())
          {
            activationTarget = parent;
          }

          // 6i.vi.B. Append to an event path with event, parent, null, relatedTarget, touchTargets, and
          // slotInClosedTree.
          AppendToEventPath(event, *parent, nullptr, relatedTarget, touchTargets, slotInClosedTree);
        }
        // 6i.vii. Otherwise, if parent is relatedTarget, then set parent to null.
        else if (parent == relatedTarget)
        {
          parent = nullptr;
        }
        // 6i.viii. Otherwise:
        else
        {
          // 6i.viii.A. Set target to parent.
          target = parent;

          // 6i.viii.B. If isActivationEvent is true, activationTarget is null, and target has activation
          // behavior, then set activationTarget to target.
          if (isActivationEvent && activationTarget == nullptr && target->HasActivationBehavior())
          {
            activationTarget = target;
          }

          // 6i.viii.C. Append to an event path with event, parent, target, relatedTarget, touchTargets, and
          // slotInClosedTree.
          AppendToEventPath(event, *parent, target, relatedTarget, touchTargets, slotInClosedTree);
        }

        // 6i.ix. If parent is non-null, then set parent to the result of invoking parent's get the parent
        // with event.
        if (parent != nullptr)
        {
          parent = parent->GetParent(event);
        }

        // 6i.x. Set slotInClosedTree to false.
        slotInClosedTree = false;
      }

      // 6j. Let clearTargetsItem be the last event path item in event's path whose shadow-adjusted target is
      // non-null.
      auto clearTargetsItem =
        std::find_if(event._path.rbegin(), event._path.rend(),
                     [](const EventPathItem &item) { return item.ShadowAdjustedTarget() != nullptr; });

      // 6k. If clearTargetsItem's shadow-adjusted target, clearTargetsItem's relatedTarget, or an EventTarget
      // object in clearTargetsItem's touch target list is a node whose root is a shadow root: set
      // clearTargets to true.
      if (clearTargetsItem != event._path.rend())
      {
        auto IsNodeInShadowTree = [](const EventTarget *candidate)
        {
          auto *node = DynamicDowncast<Node>(candidate);
          return node != nullptr && Is<ShadowRoot>(TreeQueries::Root(*node));
        };

        clearTargets = IsNodeInShadowTree(clearTargetsItem->ShadowAdjustedTarget())
                       || IsNodeInShadowTree(clearTargetsItem->RelatedTarget())
                       || std::ranges::any_of(clearTargetsItem->TouchTargetList(), [&](auto &touchTarget)
                                              { return IsNodeInShadowTree(touchTarget.get()); });
      }

      // 6l. If activationTarget is non-null and activationTarget has legacy-pre-activation behavior, then run
      // activationTarget's legacy-pre-activation behavior.
      if (activationTarget != nullptr && activationTarget->HasLegacyPreActivationBehavior())
      {
        activationTarget->LegacyPreActivationBehavior(event);
      }

      // 6m. For each item of event's path, in reverse order:
      for (auto &item : event._path | std::views::reverse)
      {
        // 6m.i. If item's shadow-adjusted target is non-null, then set event's eventPhase attribute to
        // AT_TARGET.
        if (item.ShadowAdjustedTarget() != nullptr)
        {
          event._eventPhase = EventPhaseType::AT_TARGET;
        }
        // 6m.ii. Otherwise, set event's eventPhase attribute to CAPTURING_PHASE.
        else
        {
          event._eventPhase = EventPhaseType::CAPTURING_PHASE;
        }

        // 6m.iii. Invoke with item, event, "capturing", and legacyOutputDidListenersThrowFlag if given.
        Invoke(item, event, EventPhaseType::CAPTURING_PHASE, legacyOutputDidListenersThrowFlag);
      }

      // 6n. For each item of event's path:
      for (auto &item : event._path)
      {
        // 6n.i. If item's shadow-adjusted target is non-null, then set event's eventPhase attribute to
        // AT_TARGET.
        if (item.ShadowAdjustedTarget() != nullptr)
        {
          event._eventPhase = EventPhaseType::AT_TARGET;
        }
        // 6n.ii. Otherwise:
        else
        {
          // 6n.ii.A. If event's bubbles attribute is false, then continue.
          if (!event.Bubbles())
          {
            continue;
          }

          // 6n.ii.B. Set event's eventPhase attribute to BUBBLING_PHASE.
          event._eventPhase = EventPhaseType::BUBBLING_PHASE;
        }

        // 6n.iii. Invoke with item, event, "bubbling", and legacyOutputDidListenersThrowFlag if given.
        Invoke(item, event, EventPhaseType::BUBBLING_PHASE, legacyOutputDidListenersThrowFlag);
      }
    }

    // The nodes on the path, for their default event handlers.
    List<Ref<Node>> defaultEventHandlerPath;
    if (Is<Node>(originalTarget))
    {
      for (auto &item : event._path)
      {
        if (auto *node = DynamicDowncast<Node>(item.InvocationTarget()))
        {
          defaultEventHandlerPath.push_back(ShareRef(*node));
        }
      }
    }

    // 7. Set event's eventPhase attribute to NONE.
    event._eventPhase = EventPhaseType::NONE;

    // 8. Set event's currentTarget attribute to null.
    event._currentTarget = nullptr;

    // 9. Set event's path to the empty list.
    event._path.clear();

    // 10. Unset event's dispatch flag, stop propagation flag, and stop immediate propagation flag.
    event._dispatched = false;
    event._stopPropagation = false;
    event._stopImmediatePropagation = false;

    // 11. If clearTargets is true:
    if (clearTargets)
    {
      // 11a. Set event's target to null.
      event._target = nullptr;

      // 11b. Set event's relatedTarget to null.
      event.RelatedTarget(nullptr);

      // 11c. Set event's touch target list to the empty list.
      event.TouchTargetList({});
    }

    // 12. If activationTarget is non-null:
    if (activationTarget != nullptr)
    {
      // 12a. If event's canceled flag is unset, then run activationTarget's activation behavior with event.
      if (!event._canceled)
      {
        activationTarget->ActivationBehavior(event);
      }
      // 12b. Otherwise, if activationTarget has legacy-canceled-activation behavior, then run
      // activationTarget's legacy-canceled-activation behavior.
      else if (activationTarget->HasLegacyCanceledActivationBehavior())
      {
        activationTarget->LegacyCanceledActivationBehavior(event);
      }
    }

    // Default event handlers run for the nodes on the path, as upstream, unless the event was canceled or
    // handled.
    if (!event._canceled && !event._defaultHandled && !event._isDefaultEventHandlerIgnored)
    {
      auto finalTarget = event._target;
      event.SetTarget(ShareRefPtr(originalTarget));
      CallDefaultEventHandlersInBubblingOrder(event, defaultEventHandlerPath);
      event.SetTarget(krys::move(finalTarget));
    }

    // 13. Return false if event's canceled flag is set; otherwise true.
    return !event._canceled;
  }

  void EventDispatcher::AppendToEventPath(Event &event, EventTarget &invocationTarget,
                                          EventTarget *shadowAdjustedTarget, EventTarget *relatedTarget,
                                          List<Ref<EventTarget>> &touchTargets,
                                          bool slotInClosedTree) noexcept
  {
    // 1. Let invocationTargetInShadowTree be false.
    bool invocationTargetInShadowTree = false;

    // 2. If invocationTarget is a node and its root is a shadow root, then set invocationTargetInShadowTree
    // to true.
    auto *invocationTargetNode = DynamicDowncast<Node>(invocationTarget);
    if (invocationTargetNode != nullptr && Is<ShadowRoot>(TreeQueries::Root(*invocationTargetNode)))
    {
      invocationTargetInShadowTree = true;
    }

    // 3. Let rootOfClosedTree be false.
    bool rootOfClosedTree = false;

    // 4. If invocationTarget is a shadow root whose mode is "closed", then set rootOfClosedTree to true.
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(invocationTargetNode))
    {
      rootOfClosedTree = shadowRoot->Mode() == ShadowRootMode::Closed;
    }

    // 5. Append a new event path item to event's path whose invocation target is invocationTarget,
    // invocation-target-in-shadow-tree is invocationTargetInShadowTree, shadow-adjusted target is
    // shadowAdjustedTarget, relatedTarget is relatedTarget, touch target list is touchTargets,
    // root-of-closed-tree is rootOfClosedTree, and slot-in-closed-tree is slotInClosedTree.
    event._path.push_back(EventPathItem {invocationTarget, invocationTargetInShadowTree, shadowAdjustedTarget,
                                         relatedTarget, touchTargets, rootOfClosedTree, slotInClosedTree});
  }

  void EventDispatcher::Invoke(EventPathItem &pathItem, Event &event, EventPhaseType phase,
                               bool *legacyOutputDidListenersThrowFlag) noexcept
  {
    // 1. Let targetItem be pathItem.
    auto pathItemIt = std::find_if(event._path.begin(), event._path.end(),
                                   [&](const EventPathItem &item) { return &item == &pathItem; });
    krys_debug_assert(pathItemIt != event._path.end());

    // 2. While targetItem's shadow-adjusted target is null: set targetItem to the event path item preceding
    // targetItem in event's path.
    auto targetItem =
      std::find_if(std::make_reverse_iterator(pathItemIt + 1uz), event._path.rend(),
                   [](const EventPathItem &item) { return item.ShadowAdjustedTarget() != nullptr; });
    krys_debug_assert(targetItem != event._path.rend());

    // 3. Set event's target to targetItem's shadow-adjusted target.
    event.SetTarget(ShareRefPtr(targetItem->ShadowAdjustedTarget()));

    // 4. Set event's relatedTarget to pathItem's relatedTarget.
    event.RelatedTarget(pathItem.RelatedTarget());

    // 5. Set event's touch target list to pathItem's touch target list.
    event.TouchTargetList(pathItem.TouchTargetList());

    // 6. If event's stop propagation flag is set, then return.
    if (event._stopPropagation)
    {
      return;
    }

    // 7. Initialize event's currentTarget attribute to pathItem's invocation target.
    event._currentTarget = ShareRefPtr(pathItem.InvocationTarget());

    // 8. Let listeners be a clone of event's currentTarget attribute value's event listener list.
    SmallList<Ref<EventListener>> listeners = event._currentTarget->_eventListenerList;

    // 9. Let invocationTargetInShadowTree be pathItem's invocation-target-in-shadow-tree.
    bool invocationTargetInShadowTree = pathItem.InvocationTargetInShadowTree();

    // 10. Let found be the result of running inner invoke with event, listeners, phase,
    // invocationTargetInShadowTree, and legacyOutputDidListenersThrowFlag if given.
    bool found =
      InnerInvoke(event, listeners, phase, invocationTargetInShadowTree, legacyOutputDidListenersThrowFlag);

    // 11. If found is false and event's isTrusted attribute is true:
    if (!found && event._isTrusted)
    {
      // 11a. Let originalEventType be event's type attribute value.
      auto originalEventType = event.Type();

      // 11b. If event's type attribute value is a match for any of the strings in the first column in the
      // following table, set event's type attribute value to the string in the second column on the same row
      // as the matching string, and return otherwise.
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

      // 11c. Inner invoke with event, listeners, phase, invocationTargetInShadowTree, and
      // legacyOutputDidListenersThrowFlag if given.
      InnerInvoke(event, listeners, phase, invocationTargetInShadowTree, legacyOutputDidListenersThrowFlag);

      // 11d. Set event's type attribute value to originalEventType.
      event._type = originalEventType;
    }
  }

  LocalDOMWindow *EventDispatcher::RelevantWindow(EventTarget &target) noexcept
  {
    if (auto *window = DynamicDowncast<LocalDOMWindow>(target))
    {
      return window;
    }
    if (auto *node = DynamicDowncast<Node>(target))
    {
      return node->NodeDocument().Window();
    }
    // FIXME: The other targets' globals (signals, media query lists), once they record their document.
    return nullptr;
  }

  bool EventDispatcher::InnerInvoke(Event &event, SmallList<Ref<EventListener>> &listeners,
                                    EventPhaseType phase, bool invocationTargetInShadowTree,
                                    bool *legacyOutputDidListenersThrowFlag) noexcept
  {
    (void)legacyOutputDidListenersThrowFlag;

    // 1. Let found be false.
    bool found = false;

    // 2. For each listener of listeners, whose removed is false:
    for (auto &listener : listeners)
    {
      if (listener->Removed())
      {
        continue;
      }

      // 2a. If event's type attribute value is not listener's type, then continue.
      if (event.Type() != listener->Type())
      {
        continue;
      }

      // 2b. Set found to true.
      found = true;

      // 2c. If phase is "capturing" and listener's capture is false, then continue.
      if (phase == EventPhaseType::CAPTURING_PHASE && !listener->Capture())
      {
        continue;
      }

      // 2d. If phase is "bubbling" and listener's capture is true, then continue.
      if (phase == EventPhaseType::BUBBLING_PHASE && listener->Capture())
      {
        continue;
      }

      // 2e. If listener's once is true, then remove an event listener given event's currentTarget attribute
      // value and listener.
      if (listener->Once())
      {
        EventTargetAlgorithms::RemoveEventListener(*event._currentTarget, *listener);
      }

      // 2f. Let targetGlobal be event's currentTarget's node document's relevant global object if event's
      // currentTarget is a node; otherwise event's currentTarget's relevant global object.
      // 2g. Let listenerGlobal be listener callback's associated realm's global object.
      // Listeners are host code rather than script, so their global is targetGlobal: the current target's
      // window, or its node document's.
      RefPtr<LocalDOMWindow> global = ShareRefPtr(RelevantWindow(*event._currentTarget));

      // 2h. If targetGlobal or listenerGlobal is a Window object whose associated Document is not fully
      // active, then continue.
      if (global && global->Document() && !global->Document()->IsFullyActive())
      {
        continue;
      }

      // 2i. Let currentEvent be undefined.
      RefPtr<Event> currentEvent;

      // 2j. If listenerGlobal is a Window object:
      if (global)
      {
        // 2j.i. Set currentEvent to listenerGlobal's current event.
        currentEvent = global->_currentEvent;

        // 2j.ii. If invocationTargetInShadowTree is false, then set listenerGlobal's current event to event.
        if (!invocationTargetInShadowTree)
        {
          global->_currentEvent = ShareRefPtr(&event);
        }
      }

      // 2k. If listener's passive is true, then set event's in passive listener flag.
      if (listener->Passive() == true)
      {
        event._inPassiveListener = true;
      }

      // 2l. If listenerGlobal is a Window object, then record timing info for event listener given event and
      // listener.
      // FIXME: Record timing info, once there's Event Timing.

      // 2m. Call a user object's operation with listener's callback, "handleEvent", « event », and event's
      // currentTarget attribute value. If this throws an exception exception:
      //   2m.i. Report exception for listener's callback's corresponding JavaScript object's associated
      //   realm's global object.
      //   2m.ii. Set legacyOutputDidListenersThrowFlag if given.
      // (Skipped: listener callbacks are host code that can't throw.)
      {
        // The listener is host code, as script would be: the reactions of the methods it calls are its own.
        html::CustomElementReactionStackSuspension suspension;
        listener->Callback()->HandleEvent(event);
      }

      // 2n. Unset event's in passive listener flag.
      event._inPassiveListener = false;

      // 2o. If listenerGlobal is a Window object, then set listenerGlobal's current event to currentEvent.
      if (global)
      {
        global->_currentEvent = krys::move(currentEvent);
      }

      // 2p. If event's stop immediate propagation flag is set, then break.
      if (event._stopImmediatePropagation)
      {
        break;
      }
    }

    // 3. Return found.
    return found;
  }
}