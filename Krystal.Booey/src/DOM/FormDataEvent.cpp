#include "Krystal.Booey/DOM/FormDataEvent.hpp"

namespace krys::boo::dom
{
  FormDataEvent::FormDataEvent(DOMStringAtom type, const FormDataEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict),
        _formData(eventInitDict.FormData ? Ref<html::DOMFormData>(eventInitDict.FormData)
                                         : html::DOMFormData::Create())
  {
  }

  Ref<FormDataEvent> FormDataEvent::Create(DOMStringAtom type, bool canBubble, bool isCancelable,
                                           bool isComposed, Ref<html::DOMFormData> &&formData) noexcept
  {
    FormDataEventInit init;
    init.Bubbles = canBubble;
    init.Cancelable = isCancelable;
    init.Composed = isComposed;
    init.FormData = krys::move(formData);
    auto event = CreateRef<FormDataEvent>(type, init);
    event->_isTrusted = true;
    return event;
  }
}
