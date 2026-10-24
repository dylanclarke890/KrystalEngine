#include "Krystal.Booey/HTML/PopStateEvent.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/Page/History.hpp"

namespace krys::boo::html
{
  PopStateEvent::PopStateEvent(dom::DOMStringAtom type, const PopStateEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _state(eventInitDict.State),
        _hasUAVisualTransition(eventInitDict.HasUAVisualTransition)
  {
  }

  PopStateEvent::~PopStateEvent() noexcept = default;

  Ref<PopStateEvent> PopStateEvent::Create(RefPtr<SerializedScriptValue> &&state,
                                           boo::History *history) noexcept
  {
    PopStateEventInit init;
    init.State = krys::move(state);
    auto event = CreateRef<PopStateEvent>(dom::DOMStringAtom(EventNames::PopState), init);
    event->_history = ShareRefPtr(history);
    event->_isTrusted = true;
    return event;
  }

  boo::History *PopStateEvent::GetHistory() const noexcept
  {
    return _history.get();
  }
}
