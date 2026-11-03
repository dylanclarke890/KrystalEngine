#include "Krystal.Booey/DOM/Events/ToggleEventTask.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/ToggleEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo::dom
{
  ToggleEventTask::ToggleEventTask(Element &element) noexcept : _element(CreateWeakPtr(&element))
  {
  }

  void ToggleEventTask::Queue(ToggleState oldState, ToggleState newState, Element *source) noexcept
  {
    if (_data)
    {
      oldState = _data->OldState;
    }

    RefPtr<Element> element = _element.lock();
    if (!element)
    {
      return;
    }

    _data = ToggleEventData {oldState, newState, ShareRefPtr(source)};
    html::EventLoop::Current().QueueTask(
      html::TaskSource::DOMManipulation, element->NodeDocument(),
      [task = ShareRef(*this), element, newState]
      {
        if (!task->_data || task->_data->NewState != newState)
        {
          return;
        }

        auto stringForState = [](ToggleState state)
        {
          return DOMString(state == ToggleState::Closed ? u8"closed" : u8"open");
        };

        auto data = *std::exchange(task->_data, null);
        ToggleEventInit init;
        init.OldState = stringForState(data.OldState);
        init.NewState = stringForState(data.NewState);
        init.Source = krys::move(data.Source);
        auto event = ToggleEvent::Create(DOMStringAtom(EventNames::Toggle), init, false);
        (void)EventDispatcher::DispatchToTarget(*event, element.get());
      });
  }
}
