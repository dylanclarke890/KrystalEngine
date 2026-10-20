#pragma once

#include "Krystal.Booey/DOM/Dicts/KeyboardEventInit.hpp"
#include "Krystal.Booey/DOM/UIEventWithKeyState.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::dom
{
  /// @brief An event from a keyboard: `keydown`, `keypress` and `keyup`.
  /// @see https://w3c.github.io/uievents/#interface-keyboardevent
  // FIXME: The view, and composing with an input method, once there are text fields; key commands.
  class KeyboardEvent final : public UIEventWithKeyState
  {
    friend class EventFactory;
    using dictionary_type = KeyboardEventInit;

  public:
    enum KeyLocationCode : uint32
    {
      DOM_KEY_LOCATION_STANDARD = 0x00,
      DOM_KEY_LOCATION_LEFT = 0x01,
      DOM_KEY_LOCATION_RIGHT = 0x02,
      DOM_KEY_LOCATION_NUMPAD = 0x03
    };

  private:
    UniquePtr<PlatformKeyboardEvent> _underlyingPlatformEvent;
    DOMString _key;
    DOMString _code;
    DOMString _keyIdentifier;
    uint32 _location {DOM_KEY_LOCATION_STANDARD};
    bool _repeat {false};
    bool _isComposing {false};
    Maybe<uint32> _charCode;
    Maybe<uint32> _keyCode;
    Maybe<uint32> _which;

  public:
    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-keyboardevent
    KeyboardEvent(DOMStringAtom type, const KeyboardEventInit &initializer = {}) noexcept;

    /// @brief A trusted event from @p key, which must be a raw key down, a char or a key up (a combined key
    /// down is disambiguated first).
    KRYS_NODISCARD static Ref<KeyboardEvent> Create(const PlatformKeyboardEvent &key) noexcept;

    /// @note Legacy.
    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-initkeyboardevent
    void InitKeyboardEvent(DOMStringAtom type, bool canBubble, bool cancelable, DOMStringView keyIdentifier,
                           uint32 location, bool ctrlKey, bool altKey, bool shiftKey, bool metaKey) noexcept;

    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-key
    KRYS_NODISCARD const DOMString &Key() const noexcept
    {
      return _key;
    }

    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-code
    KRYS_NODISCARD const DOMString &Code() const noexcept
    {
      return _code;
    }

    KRYS_NODISCARD const DOMString &KeyIdentifier() const noexcept
    {
      return _keyIdentifier;
    }

    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-location
    KRYS_NODISCARD uint32 Location() const noexcept
    {
      return _location;
    }

    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-repeat
    KRYS_NODISCARD bool Repeat() const noexcept
    {
      return _repeat;
    }

    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-iscomposing
    KRYS_NODISCARD bool IsComposing() const noexcept
    {
      return _isComposing;
    }

    KRYS_NODISCARD const PlatformKeyboardEvent *UnderlyingPlatformEvent() const noexcept
    {
      return _underlyingPlatformEvent.get();
    }

    /// @brief The key code for `keydown` and `keyup`, the character for `keypress`.
    /// @note Legacy.
    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-keycode
    KRYS_NODISCARD int32 KeyCode() const noexcept;
    /// @brief The key code for the `keydown` that matches a `keypress`.
    KRYS_NODISCARD int32 KeyCodeForKeyDown() const noexcept;
    /// @brief The character code for `keypress`, 0 for `keydown` and `keyup`.
    /// @note Legacy.
    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-charcode
    KRYS_NODISCARD int32 CharCode() const noexcept;
    /// @note Legacy: the same as `KeyCode()`.
    /// @see https://w3c.github.io/uievents/#dom-uievent-which
    KRYS_NODISCARD uint32 Which() const noexcept;

    KRYS_NODISCARD bool IsKeyboardEvent() const noexcept final
    {
      return true;
    }

  private:
    KeyboardEvent(DOMStringAtom type, const UIEventInit &eventInitDict,
                  const PlatformKeyboardEvent &key) noexcept;
  };
}
