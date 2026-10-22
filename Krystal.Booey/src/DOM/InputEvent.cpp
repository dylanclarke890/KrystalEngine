#include "Krystal.Booey/DOM/InputEvent.hpp"

namespace krys::boo::dom
{
  InputEvent::InputEvent(DOMStringAtom type, const InputEventInit &eventInitDict) noexcept
      : UIEvent(type, eventInitDict), _inputType(eventInitDict.InputType), _data(eventInitDict.Data),
        _isInputMethodComposing(eventInitDict.IsComposing)
  {
  }

  Ref<InputEvent> InputEvent::Create(DOMStringAtom type, DOMStringView inputType, bool cancelable,
                                     Maybe<DOMString> data, const List<RefPtr<StaticRange>> &targetRanges,
                                     int32 detail, IsInputMethodComposing isInputMethodComposing) noexcept
  {
    InputEventInit init;
    init.Bubbles = true;
    init.Cancelable = cancelable;
    init.Composed = true;
    init.Detail = detail;
    init.Data = krys::move(data);
    init.IsComposing = isInputMethodComposing == IsInputMethodComposing::Yes;
    init.InputType = DOMString(inputType);
    auto event = CreateRef<InputEvent>(type, init);
    event->_targetRanges = targetRanges;
    event->_isTrusted = true;
    return event;
  }
}
