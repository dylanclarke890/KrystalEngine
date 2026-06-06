#include "Krystal.HTML/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "Krystal.HTML/Constants/EventNames.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/AbortAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Document.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/EventTarget.hpp"
#include "Krystal.HTML/DOM/Internals/Event/EventListener.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLBodyElement.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include <ranges>

namespace Krys::HTML
{
  EventListenerOptions
    EventTargetAlgorithms::FlattenOptions(const EventListenerOptionsOrBool &optionsOrBool) noexcept
  {
    EventListenerOptions options;
    if (std::holds_alternative<bool>(optionsOrBool))
    {
      options.Capture = std::get<bool>(optionsOrBool);
    }
    else
    {
      options = std::get<EventListenerOptions>(optionsOrBool);
    }

    return options;
  }

  AddEventListenerOptions
    EventTargetAlgorithms::FlattenMoreOptions(const AddEventListenerOptionsOrBool &optionsOrBool) noexcept
  {
    AddEventListenerOptions options;
    if (std::holds_alternative<bool>(optionsOrBool))
    {
      options.Capture = std::get<bool>(optionsOrBool);
    }
    else
    {
      options = std::get<AddEventListenerOptions>(optionsOrBool);
    }

    return options;
  }

  bool EventTargetAlgorithms::DefaultPassiveValue(const DOMStringAtom &type,
                                                  const EventTarget &eventTarget) noexcept
  {
    if (type != EventNames::TouchStart || type != EventNames::TouchMove || type != EventNames::Wheel
        || type != EventNames::MouseWheel)
    {
      return false;
    }

    if (eventTarget.IsWindow())
    {
      return true;
    }

    if (RawPtr<const Node> node = DynamicDowncast<Node>(eventTarget))
    {
      if (&node->NodeDocument() == &eventTarget)
      {
        return true;
      }

      if (node->NodeDocument().DocumentElement().get() == &eventTarget)
      {
        return true;
      }

      if (node->NodeDocument().Body().get() == &eventTarget)
      {
        return true;
      }
    }

    return false;
  }

  void EventTargetAlgorithms::AddEventListener(EventTarget &eventTarget, EventListener &listener) noexcept
  {
    // SPEC-VIOLATION(DOM, SERVICE-WORKERS): service workers currently not supported.
    // "If eventTarget is a ServiceWorkerGlobalScope object, its service worker’s script resource’s has ever
    // been evaluated flag is set, and listener’s type matches the type attribute value of any of the service
    // worker events, then report a warning to the console that this might not give the expected results."

    if (listener.Signal() != nullptr && listener.Signal()->Aborted())
    {
      return;
    }

    if (listener.Callback() == nullptr)
    {
      return;
    }

    if (!listener.Passive().has_value())
    {
      listener._passive = DefaultPassiveValue(listener.Type(), eventTarget);
    }

    auto AlreadyExists = [&](const Ref<EventListener> &l)
    {
      return (l->Type() == listener.Type() && l->Callback() == listener.Callback()
              && l->Capture() == listener.Capture());
    };

    if (const auto &existing = std::ranges::find_if(eventTarget._eventListenerList, std::move(AlreadyExists));
        existing == eventTarget._eventListenerList.end())
    {
      eventTarget._eventListenerList.push_back(ShareRef(listener));
    }

    if (listener.Signal() != nullptr)
    {
      AbortAlgorithms::Add(
        [weakListener = CreateWeakPtr(&listener), weakTarget = CreateWeakPtr(&eventTarget)](const Any &)
        {
          auto strongListener = weakListener.lock();
          auto strongTarget = weakTarget.lock();

          if (strongListener && strongTarget)
          {
            RemoveEventListener(*strongTarget, *strongListener);
          }
        },
        *listener.Signal());
    }
  }

  void EventTargetAlgorithms::RemoveEventListener(EventTarget &eventTarget, EventListener &listener) noexcept
  {
    // SPEC-VIOLATION(DOM, SERVICE-WORKERS): service workers currently not supported.
    // "If eventTarget is a ServiceWorkerGlobalScope object and its service worker’s set of event types to
    // handle contains listener’s type, then report a warning to the console that this might not give the
    // expected results."

    listener._removed = true;
    const auto &existing = std::ranges::find_if(
      eventTarget._eventListenerList, [&](const Ref<EventListener> &l) { return l.get() == &listener; });

    if (existing != eventTarget._eventListenerList.end())
    {
      eventTarget._eventListenerList.erase(existing);
    }
  }

  void EventTargetAlgorithms::RemoveAllEventListeners(EventTarget &eventTarget) noexcept
  {
    while (!eventTarget._eventListenerList.empty())
    {
      RemoveEventListener(eventTarget, *eventTarget._eventListenerList.front().get());
    }
  }

  RawPtr<EventTarget> EventTargetAlgorithms::Retarget(RawPtr<EventTarget> a, EventTarget &b) noexcept
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

      auto *currentNode = Downcast<Node>(current);
      auto &currentRoot = TreeQueries::Root(*currentNode);
      if (!Is<ShadowRoot>(currentNode))
      {
        return current;
      }

      if (bNode != nullptr && !TreeQueries::IsShadowIncludingInclusiveAncestor(currentRoot, *bNode))
      {
        return current;
      }

      auto *host = Downcast<ShadowRoot>(&currentRoot)->Host();
      current = host;
    }

    // The spec handles the necessary cases to ensure we never get here, but we need this to satisfy the
    // compiler that current is always valid.
    std::unreachable();
    return current;
  }
}