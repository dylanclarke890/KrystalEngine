#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include <cassert>

namespace krys::boo::dom
{
  Event::Event(DOMStringAtom type, const EventInit &eventInitDict) noexcept
      : _type(type), _timeStamp(MonotonicTime::Now().count()), _bubbles(eventInitDict.Bubbles),
        _cancellable(eventInitDict.Cancelable), _composed(eventInitDict.Composed)
  {
    // The inner event creation steps set the initialized flag.
    _initialized = true;
  }

  List<Ref<EventTarget>> Event::ComposedPath() const noexcept
  {
    List<Ref<EventTarget>> composedPath {};

    if (_path.empty())
    {
      return composedPath;
    }

    EventTarget *currentTarget = _currentTarget.get();
    assert(currentTarget != nullptr);
    composedPath.emplace_back(ShareRef(*currentTarget));

    int32 currentTargetIndex = 0;
    int32 currentTargetHiddenSubtreeLevel = 0;
    int32 index = static_cast<int32>(_path.size()) - 1;

    while (index >= 0)
    {
      auto &item = _path[index];

      if (item.RootOfClosedTree())
      {
        currentTargetHiddenSubtreeLevel++;
      }

      if (item.InvocationTarget() == currentTarget)
      {
        currentTargetIndex = index;
        break;
      }

      if (item.SlotInClosedTree())
      {
        currentTargetHiddenSubtreeLevel--;
      }

      --index;
    }

    int32 currentHiddenLevel = currentTargetHiddenSubtreeLevel;
    int32 maxHiddenLevel = currentTargetHiddenSubtreeLevel;

    index = currentTargetIndex - 1;
    while (index >= 0)
    {
      auto &item = _path[index];

      if (item.RootOfClosedTree())
      {
        currentHiddenLevel++;
      }

      if (currentHiddenLevel <= maxHiddenLevel)
      {
        composedPath.insert(composedPath.begin(), ShareRef(*item.InvocationTarget()));
      }

      if (item.SlotInClosedTree())
      {
        currentHiddenLevel--;
        if (currentHiddenLevel < maxHiddenLevel)
        {
          maxHiddenLevel = currentHiddenLevel;
        }
      }

      --index;
    }

    currentHiddenLevel = currentTargetHiddenSubtreeLevel;
    maxHiddenLevel = currentTargetHiddenSubtreeLevel;

    index = currentTargetIndex + 1;

    while (index < static_cast<int32>(_path.size()))
    {
      auto &item = _path[index];

      if (item.SlotInClosedTree())
      {
        currentHiddenLevel++;
      }

      if (currentHiddenLevel <= maxHiddenLevel)
      {
        composedPath.emplace_back(ShareRef(*item.InvocationTarget()));
      }

      if (item.RootOfClosedTree())
      {
        currentHiddenLevel--;
        if (currentHiddenLevel < maxHiddenLevel)
        {
          maxHiddenLevel = currentHiddenLevel;
        }
      }
      ++index;
    }

    return composedPath;
  }

  Ref<Event> Event::Create(DOMStringAtom type, bool canBubble, bool isCancelable, bool isComposed) noexcept
  {
    auto event = CreateRef<Event>(
      type, EventInit {.Bubbles = canBubble, .Cancelable = isCancelable, .Composed = isComposed});
    event->_isTrusted = true;
    return event;
  }

  void Event::SetUnderlyingEvent(Event *underlyingEvent) noexcept
  {
    // Prohibit creation of a cycle by doing nothing if a cycle would be created.
    for (auto *event = underlyingEvent; event; event = event->UnderlyingEvent())
    {
      if (event == this)
      {
        return;
      }
    }
    _underlyingEvent = ShareRefPtr(underlyingEvent);
  }

  void Event::InitEvent(DOMStringAtom type, bool bubbles, bool cancelable) noexcept
  {
    if (_dispatched)
    {
      return;
    }

    Initialize(*this, type, bubbles, cancelable);
  }

  void Event::SetTarget(RefPtr<EventTarget> target) noexcept
  {
    if (_target == target)
    {
      return;
    }

    _target = krys::move(target);
    if (_target)
    {
      ReceivedTarget();
    }
  }

  void Event::Initialize(Event &event, DOMStringAtom type, bool bubbles, bool cancelable) noexcept
  {
    event._initialized = true;
    event._stopPropagation = false;
    event._stopImmediatePropagation = false;
    event._canceled = false;
    event._isTrusted = false;
    event._target = nullptr;
    event._type = type;
    event._bubbles = bubbles;
    event._cancellable = cancelable;
  }
}