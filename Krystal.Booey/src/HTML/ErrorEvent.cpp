#include "Krystal.Booey/HTML/ErrorEvent.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"

namespace krys::boo::html
{
  ErrorEvent::ErrorEvent(dom::DOMStringAtom type, const ErrorEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _message(eventInitDict.Message), _filename(eventInitDict.Filename),
        _lineno(eventInitDict.Lineno), _colno(eventInitDict.Colno), _error(eventInitDict.Error)
  {
  }

  Ref<ErrorEvent> ErrorEvent::Create(dom::DOMStringAtom type, dom::DOMString &&message,
                                     dom::USVString &&filename, uint32 lineno, uint32 colno,
                                     RefPtr<SerializedScriptValue> &&error) noexcept
  {
    ErrorEventInit init;
    init.Message = krys::move(message);
    init.Filename = krys::move(filename);
    init.Lineno = lineno;
    init.Colno = colno;
    init.Error = krys::move(error);
    auto event = CreateRef<ErrorEvent>(type, init);
    event->_isTrusted = true;
    return event;
  }

  Ref<ErrorEvent> ErrorEvent::Create(dom::DOMString &&message, dom::USVString &&filename, uint32 lineno,
                                     uint32 colno, RefPtr<SerializedScriptValue> &&error) noexcept
  {
    ErrorEventInit init;
    init.Cancelable = true;
    init.Message = krys::move(message);
    init.Filename = krys::move(filename);
    init.Lineno = lineno;
    init.Colno = colno;
    init.Error = krys::move(error);
    auto event = CreateRef<ErrorEvent>(dom::DOMStringAtom(EventNames::Error), init);
    event->_isTrusted = true;
    return event;
  }
}
