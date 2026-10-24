#include "Krystal.Booey/HTML/MessageEvent.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"

namespace krys::boo::html
{
  MessageEvent::MessageEvent(dom::DOMStringAtom type, const MessageEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _data(eventInitDict.Data), _origin(eventInitDict.Origin),
        _lastEventId(eventInitDict.LastEventId), _source(eventInitDict.Source)
  {
  }

  MessageEvent::~MessageEvent() noexcept = default;

  Ref<MessageEvent> MessageEvent::Create(RefPtr<SerializedScriptValue> &&data, dom::USVString &&origin,
                                         RefPtr<LocalDOMWindow> &&source) noexcept
  {
    MessageEventInit init;
    init.Data = krys::move(data);
    init.Origin = krys::move(origin);
    init.Source = krys::move(source);
    auto event = CreateRef<MessageEvent>(dom::DOMStringAtom(EventNames::Message), init);
    event->_isTrusted = true;
    return event;
  }

  LocalDOMWindow *MessageEvent::Source() const noexcept
  {
    return _source.get();
  }
}
