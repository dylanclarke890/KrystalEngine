#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include <algorithm>

namespace krys::boo::dom
{
  Event::Event(DOMStringAtom type, const EventInit &eventInitDict) noexcept
      : _type(type), _createTime(html::EventLoop::Current().Now()), _bubbles(eventInitDict.Bubbles),
        _cancellable(eventInitDict.Cancelable), _composed(eventInitDict.Composed)
  {
    // 1. Let event be the result of running the inner event creation steps with this interface, null, now,
    // and eventInitDict.
    // The inner event creation steps:
    //   1. Let event be the result of creating a new object using eventInterface. (This object.)
    //   2. Set event's initialized flag.
    //   3. Initialize event's timeStamp attribute to the relative high resolution coarse time given time and
    //   event's relevant global object. (_createTime is the time; TimeStampForBindings makes it relative.)
    //   4. For each member -> value of dictionary: if event has an attribute whose identifier is member,
    //   then initialize that attribute to value. (Here for EventInit's members, and in each derived
    //   interface's constructor for its own.)
    //   5. Run the event constructing steps with event and dictionary. (The derived constructors' bodies.)
    //   6. Return event.
    _initialized = true;

    // 2. Initialize event's type attribute to type. (_type above.)
    // 3. Return event.
  }

  DOMHighResTimeStamp Event::TimeStampForBindings(const Document &document) const noexcept
  {
    auto *window = document.Window();
    if (!window)
    {
      return 0;
    }
    return std::max(std::chrono::duration<double, std::milli>(_createTime - window->TimeOrigin()).count(),
                    0.);
  }

  List<Ref<EventTarget>> Event::ComposedPath() const noexcept
  {
    // 1. Let composedPath be an empty list.
    List<Ref<EventTarget>> composedPath {};

    // 2. Let path be this's path.
    auto &path = _path;

    // 3. If path is empty, then return composedPath.
    if (path.empty())
    {
      return composedPath;
    }

    // 4. Let currentTarget be this's currentTarget attribute value.
    EventTarget *currentTarget = _currentTarget.get();

    // 5. Assert: currentTarget is an EventTarget object.
    krys_debug_assert(currentTarget != nullptr);

    // 6. Append currentTarget to composedPath.
    composedPath.emplace_back(ShareRef(*currentTarget));

    // 7. Let currentTargetIndex be 0.
    int32 currentTargetIndex = 0;

    // 8. Let currentTargetHiddenSubtreeLevel be 0.
    int32 currentTargetHiddenSubtreeLevel = 0;

    // 9. Let index be path's size - 1.
    int32 index = static_cast<int32>(path.size()) - 1;

    // 10. While index is greater than or equal to 0:
    while (index >= 0)
    {
      auto &item = path[index];

      // 10a. If path[index]'s root-of-closed-tree is true, then increase currentTargetHiddenSubtreeLevel
      // by 1.
      if (item.RootOfClosedTree())
      {
        currentTargetHiddenSubtreeLevel++;
      }

      // 10b. If path[index]'s invocation target is currentTarget, then set currentTargetIndex to index and
      // break.
      if (item.InvocationTarget() == currentTarget)
      {
        currentTargetIndex = index;
        break;
      }

      // 10c. If path[index]'s slot-in-closed-tree is true, then decrease currentTargetHiddenSubtreeLevel
      // by 1.
      if (item.SlotInClosedTree())
      {
        currentTargetHiddenSubtreeLevel--;
      }

      // 10d. Decrease index by 1.
      --index;
    }

    // 11. Let currentHiddenLevel and maxHiddenLevel be currentTargetHiddenSubtreeLevel.
    int32 currentHiddenLevel = currentTargetHiddenSubtreeLevel;
    int32 maxHiddenLevel = currentTargetHiddenSubtreeLevel;

    // 12. Set index to currentTargetIndex - 1.
    index = currentTargetIndex - 1;

    // 13. While index is greater than or equal to 0:
    while (index >= 0)
    {
      auto &item = path[index];

      // 13a. If path[index]'s root-of-closed-tree is true, then increase currentHiddenLevel by 1.
      if (item.RootOfClosedTree())
      {
        currentHiddenLevel++;
      }

      // 13b. If currentHiddenLevel is less than or equal to maxHiddenLevel, then prepend path[index]'s
      // invocation target to composedPath.
      if (currentHiddenLevel <= maxHiddenLevel)
      {
        composedPath.insert(composedPath.begin(), ShareRef(*item.InvocationTarget()));
      }

      // 13c. If path[index]'s slot-in-closed-tree is true:
      if (item.SlotInClosedTree())
      {
        // 13c.i. Decrease currentHiddenLevel by 1.
        currentHiddenLevel--;

        // 13c.ii. If currentHiddenLevel is less than maxHiddenLevel, then set maxHiddenLevel to
        // currentHiddenLevel.
        if (currentHiddenLevel < maxHiddenLevel)
        {
          maxHiddenLevel = currentHiddenLevel;
        }
      }

      // 13d. Decrease index by 1.
      --index;
    }

    // 14. Set currentHiddenLevel and maxHiddenLevel to currentTargetHiddenSubtreeLevel.
    currentHiddenLevel = currentTargetHiddenSubtreeLevel;
    maxHiddenLevel = currentTargetHiddenSubtreeLevel;

    // 15. Set index to currentTargetIndex + 1.
    index = currentTargetIndex + 1;

    // 16. While index is less than path's size:
    while (index < static_cast<int32>(path.size()))
    {
      auto &item = path[index];

      // 16a. If path[index]'s slot-in-closed-tree is true, then increase currentHiddenLevel by 1.
      if (item.SlotInClosedTree())
      {
        currentHiddenLevel++;
      }

      // 16b. If currentHiddenLevel is less than or equal to maxHiddenLevel, then append path[index]'s
      // invocation target to composedPath.
      if (currentHiddenLevel <= maxHiddenLevel)
      {
        composedPath.emplace_back(ShareRef(*item.InvocationTarget()));
      }

      // 16c. If path[index]'s root-of-closed-tree is true:
      if (item.RootOfClosedTree())
      {
        // 16c.i. Decrease currentHiddenLevel by 1.
        currentHiddenLevel--;

        // 16c.ii. If currentHiddenLevel is less than maxHiddenLevel, then set maxHiddenLevel to
        // currentHiddenLevel.
        if (currentHiddenLevel < maxHiddenLevel)
        {
          maxHiddenLevel = currentHiddenLevel;
        }
      }

      // 16d. Increase index by 1.
      ++index;
    }

    // 17. Return composedPath.
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
    // 1. If this's dispatch flag is set, then return.
    if (_dispatched)
    {
      return;
    }

    // 2. Initialize this with type, bubbles, and cancelable.
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
    // 1. Set event's initialized flag.
    event._initialized = true;

    // 2. Unset event's stop propagation flag, stop immediate propagation flag, and canceled flag.
    event._stopPropagation = false;
    event._stopImmediatePropagation = false;
    event._canceled = false;

    // 3. Set event's isTrusted attribute to false.
    event._isTrusted = false;

    // 4. Set event's target to null.
    event._target = nullptr;

    // 5. Set event's type attribute to type.
    event._type = type;

    // 6. Set event's bubbles attribute to bubbles.
    event._bubbles = bubbles;

    // 7. Set event's cancelable attribute to cancelable.
    event._cancellable = cancelable;
  }
}