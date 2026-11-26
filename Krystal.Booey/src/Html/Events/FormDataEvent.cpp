#include "Krystal.Booey/Html/Events/FormDataEvent.hpp"
#include "Krystal.Booey/Html/Forms/DOMFormData.hpp"

namespace krys::boo
{
  FormDataEvent::FormDataEvent(DOMStringAtom type, const FormDataEventInit &eventInitDict) noexcept
      : Event(krys::move(type), eventInitDict),
        _formData(eventInitDict.FormData ? Ref<html::DOMFormData>(eventInitDict.FormData)
                                         : html::DOMFormData::Create())
  {
  }

  Ref<FormDataEvent> FormDataEvent::Create(const DOMStringAtom &type, bool canBubble, bool isCancelable,
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
