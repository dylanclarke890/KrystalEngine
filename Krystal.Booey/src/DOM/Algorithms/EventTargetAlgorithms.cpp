#include "Krystal.Booey/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/AbortAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/Internals/Event/EventListener.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include <ranges>

namespace krys::boo::dom
{
  EventListenerOptions
    EventTargetAlgorithms::FlattenOptions(const EventListenerOptionsOrBool &optionsOrBool) noexcept
  {
    EventListenerOptions options;

    // 1. If options is a boolean, then return options.
    if (std::holds_alternative<bool>(optionsOrBool))
    {
      options.Capture = std::get<bool>(optionsOrBool);
    }
    // 2. Return options["capture"].
    else
    {
      options = std::get<EventListenerOptions>(optionsOrBool);
    }

    return options;
  }

  AddEventListenerOptions
    EventTargetAlgorithms::FlattenMoreOptions(const AddEventListenerOptionsOrBool &optionsOrBool) noexcept
  {
    // 2. Let once be false.
    // 3. Let passive and signal be null.
    // (The defaults of AddEventListenerOptions.)
    AddEventListenerOptions options;

    // 1. Let capture be the result of flattening options.
    if (std::holds_alternative<bool>(optionsOrBool))
    {
      options.Capture = std::get<bool>(optionsOrBool);
    }
    // 4. If options is a dictionary:
    //   4a. Set once to options["once"].
    //   4b. If options["passive"] exists, then set passive to options["passive"].
    //   4c. If options["signal"] exists, then set signal to options["signal"].
    else
    {
      options = std::get<AddEventListenerOptions>(optionsOrBool);
    }

    // 5. Return capture, passive, once, and signal.
    return options;
  }

  bool EventTargetAlgorithms::DefaultPassiveValue(const DOMStringAtom &type,
                                                  const EventTarget &eventTarget) noexcept
  {
    // 1. Return true if all of the following are true:
    //   - type is one of "touchstart", "touchmove", "wheel", or "mousewheel".
    //   - eventTarget is a Window object, or is a node whose node document is eventTarget, or is a node whose
    //   node document's document element is eventTarget, or is a node whose node document's body element is
    //   eventTarget.
    // 2. Return false.
    if (type != EventNames::TouchStart && type != EventNames::TouchMove && type != EventNames::Wheel
        && type != EventNames::MouseWheel)
    {
      return false;
    }

    if (eventTarget.IsWindow())
    {
      return true;
    }

    if (const Node *node = DynamicDowncast<Node>(eventTarget))
    {
      if (&node->NodeDocument() == &eventTarget)
      {
        return true;
      }

      if (node->NodeDocument().DocumentElement().get() == &eventTarget)
      {
        return true;
      }

      if (node->NodeDocument().BodyOrFrameset() == &eventTarget)
      {
        return true;
      }
    }

    return false;
  }

  bool EventTargetAlgorithms::AddEventListener(EventTarget &eventTarget, EventListener &listener) noexcept
  {
    // 1. If eventTarget is a ServiceWorkerGlobalScope object, its service worker's script resource's has ever
    // been evaluated flag is set, and listener's type matches the type attribute value of any of the service
    // worker events, then report a warning to the console that this might not give the expected results.
    // (Skipped: no service workers.)

    // 2. If listener's signal is non-null and is aborted, then return.
    if (listener.Signal() != nullptr && listener.Signal()->Aborted())
    {
      return false;
    }

    // 3. If listener's callback is null, then return.
    if (listener.Callback() == nullptr)
    {
      return false;
    }

    // 4. If listener's passive is null, then set it to the default passive value given listener's type and
    // eventTarget.
    if (!listener.Passive().has_value())
    {
      listener._passive = DefaultPassiveValue(listener.Type(), eventTarget);
    }

    // 5. If eventTarget's event listener list does not contain an event listener whose type is listener's
    // type, callback is listener's callback, and capture is listener's capture, then append listener to
    // eventTarget's event listener list.

    auto AlreadyExists = [&](const Ref<EventListener> &l)
    {
      return (l->Type() == listener.Type() && l->Callback() == listener.Callback()
              && l->Capture() == listener.Capture());
    };

    if (const auto &existing = std::ranges::find_if(eventTarget._eventListenerList, std::move(AlreadyExists));
        existing != eventTarget._eventListenerList.end())
    {
      return false;
    }
    eventTarget._eventListenerList.push_back(ShareRef(listener));
    eventTarget.DidAddEventListener(listener);

    // 6. If listener's signal is non-null, then add the following abort steps to it:
    if (listener.Signal() != nullptr)
    {
      AbortAlgorithms::Add(
        [weakListener = CreateWeakPtr(&listener), weakTarget = CreateWeakPtr(&eventTarget)](const Any &)
        {
          auto strongListener = weakListener.lock();
          auto strongTarget = weakTarget.lock();

          // 6a. Remove an event listener with eventTarget and listener.
          if (strongListener && strongTarget)
          {
            RemoveEventListener(*strongTarget, *strongListener);
          }
        },
        *listener.Signal());
    }

    return true;
  }

  void EventTargetAlgorithms::RemoveEventListener(EventTarget &eventTarget, EventListener &listener) noexcept
  {
    // 1. If eventTarget is a ServiceWorkerGlobalScope object and its service worker's set of event types to
    // handle contains listener's type, then report a warning to the console that this might not give the
    // expected results. (Skipped: no service workers.)

    // 2. Set listener's removed to true and remove listener from eventTarget's event listener list.
    listener._removed = true;
    const auto &existing = std::ranges::find_if(
      eventTarget._eventListenerList, [&](const Ref<EventListener> &l) { return l.get() == &listener; });

    if (existing != eventTarget._eventListenerList.end())
    {
      // Erasing it may drop the listener's last reference.
      Ref<EventListener> protectedListener = ShareRef(listener);
      eventTarget._eventListenerList.erase(existing);
      eventTarget.DidRemoveEventListener(listener);
    }
  }

  void EventTargetAlgorithms::RemoveAllEventListeners(EventTarget &eventTarget) noexcept
  {
    while (!eventTarget._eventListenerList.empty())
    {
      RemoveEventListener(eventTarget, *eventTarget._eventListenerList.front().get());
    }
  }

  EventTarget *EventTargetAlgorithms::Retarget(EventTarget *a, EventTarget &b) noexcept
  {
    auto *current = a;
    auto *bNode = DynamicDowncast<Node>(b);
    while (true)
    {
      if (current == nullptr)
      {
        return nullptr;
      }

      if (!Is<Node>(current))
      {
        return current;
      }

      // Return A if its root is not a shadow root, or is a shadow-including inclusive ancestor of B.
      auto *currentNode = Downcast<Node>(current);
      auto &currentRoot = TreeQueries::Root(*currentNode);
      if (!Is<ShadowRoot>(currentRoot))
      {
        return current;
      }

      if (bNode != nullptr && TreeQueries::IsShadowIncludingInclusiveAncestor(currentRoot, *bNode))
      {
        return current;
      }

      auto *host = Downcast<ShadowRoot>(&currentRoot)->Host();
      current = host;
    }

    krys_unreachable();
  }
}