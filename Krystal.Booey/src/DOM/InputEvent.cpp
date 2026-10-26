#include "Krystal.Booey/DOM/InputEvent.hpp"
#include "Krystal.Booey/DOM/DataTransfer.hpp"

namespace krys::boo::dom
{
  InputEvent::InputEvent(DOMStringAtom type, const InputEventInit &eventInitDict) noexcept
      : UIEvent(type, eventInitDict), _inputType(eventInitDict.InputType), _data(eventInitDict.Data),
        _dataTransfer(eventInitDict.DataTransfer), _isInputMethodComposing(eventInitDict.IsComposing)
  {
  }

  InputEvent::~InputEvent() noexcept = default;

  Ref<InputEvent> InputEvent::Create(DOMStringAtom type, DOMStringView inputType, bool cancelable,
                                     Maybe<DOMString> data, RefPtr<DataTransfer> &&dataTransfer,
                                     const List<RefPtr<StaticRange>> &targetRanges, int32 detail,
                                     IsInputMethodComposing isInputMethodComposing) noexcept
  {
    InputEventInit init;
    init.Bubbles = true;
    init.Cancelable = cancelable;
    init.Composed = true;
    init.Detail = detail;
    init.Data = krys::move(data);
    init.DataTransfer = krys::move(dataTransfer);
    init.IsComposing = isInputMethodComposing == IsInputMethodComposing::Yes;
    init.InputType = DOMString(inputType);
    auto event = CreateRef<InputEvent>(type, init);
    event->_targetRanges = targetRanges;
    event->_isTrusted = true;
    return event;
  }
}
