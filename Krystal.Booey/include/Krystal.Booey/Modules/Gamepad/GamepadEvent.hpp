#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Modules/Gamepad/Gamepad.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class EventFactory;
}

namespace krys::boo
{
  /// @see https://w3c.github.io/gamepad/#dom-gamepadeventinit
  struct GamepadEventInit : dom::EventInit
  {
    // The specification says this member should be required and non-nullable. However, this does not match
    // the behavior of Chrome or Firefox.
    RefPtr<boo::Gamepad> Gamepad;
  };

  /// @brief Fired at a window when a gamepad connects (`gamepadconnected`) or disconnects
  /// (`gamepaddisconnected`).
  /// @see https://w3c.github.io/gamepad/#gamepadevent-interface
  class GamepadEvent final : public dom::Event
  {
    friend class dom::EventFactory;
    using dictionary_type = GamepadEventInit;

  private:
    RefPtr<boo::Gamepad> _gamepad;

  public:
    /// @see https://w3c.github.io/gamepad/#dom-gamepadevent-constructor
    GamepadEvent(dom::DOMStringAtom type, const GamepadEventInit &eventInitDict = {}) noexcept;

    /// @brief A trusted event of type @p type for @p gamepad, which doesn't bubble and can't be canceled.
    KRYS_NODISCARD static Ref<GamepadEvent> Create(dom::DOMStringAtom type, boo::Gamepad &gamepad) noexcept;

    /// @brief An event of type @p type; trusted if @p isTrusted.
    KRYS_NODISCARD static Ref<GamepadEvent> Create(dom::DOMStringAtom type, const GamepadEventInit &init,
                                                   bool isTrusted = false) noexcept;

    /// @see https://w3c.github.io/gamepad/#dom-gamepadevent-gamepad
    KRYS_NODISCARD boo::Gamepad *Gamepad() const noexcept
    {
      return _gamepad.get();
    }

    KRYS_NODISCARD bool IsGamepadEvent() const noexcept final
    {
      return true;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::GamepadEvent)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Event &event) noexcept
  {
    return event.IsGamepadEvent();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
