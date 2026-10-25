#include "Krystal.Booey/Modules/Gamepad/GamepadEvent.hpp"

namespace krys::boo
{
  GamepadEvent::GamepadEvent(dom::DOMStringAtom type, const GamepadEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _gamepad(eventInitDict.Gamepad)
  {
  }

  Ref<GamepadEvent> GamepadEvent::Create(dom::DOMStringAtom type, boo::Gamepad &gamepad) noexcept
  {
    GamepadEventInit init;
    init.Gamepad = ShareRefPtr(&gamepad);
    return Create(type, init, true);
  }

  Ref<GamepadEvent> GamepadEvent::Create(dom::DOMStringAtom type, const GamepadEventInit &init,
                                         bool isTrusted) noexcept
  {
    auto event = CreateRef<GamepadEvent>(type, init);
    event->_isTrusted = isTrusted;
    return event;
  }
}
