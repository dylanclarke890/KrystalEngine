#include "Krystal.HTML/Events/Event.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.Lib/Time/MonotonicTime.hpp"
#include <cassert>

namespace Krys::HTML
{
  Event::Event(DOMStringAtom type, const EventInit &eventInitDict) noexcept
      : _type(type), _timeStamp(MonotonicTime::Now()), _bubbles(eventInitDict.Bubbles),
        _cancellable(eventInitDict.Cancellable), _composed(eventInitDict.Composed)
  {
  }

  /// @see https://dom.spec.whatwg.org/#dom-event-composedpath
  List<Ref<EventTarget>> Event::ComposedPath() const noexcept
  {
    List<Ref<EventTarget>> composedPath {};

    if (_path.empty())
    {
      return composedPath;
    }

    RawPtr<EventTarget> currentTarget = CurrentTarget();
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

      if (item.RootOfClosedTree())
      {
        currentHiddenLevel++;
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
        currentHiddenLevel--;
      }

      if (currentHiddenLevel <= maxHiddenLevel)
      {
        composedPath.emplace_back(ShareRef(*item.InvocationTarget()));
      }

      if (item.SlotInClosedTree())
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

  void Event::InitEvent(DOMStringAtom type, bool bubbles, bool cancelable) noexcept
  {
    if (_dispatched)
    {
      return;
    }

    _type = type;
    _bubbles = bubbles;
    _cancellable = cancelable;
  }
}