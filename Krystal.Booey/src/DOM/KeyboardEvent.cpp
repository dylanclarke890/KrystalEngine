#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD bool ViewIsCompositing(const LocalDOMWindow *view) noexcept
    {
      if (!view)
      {
        return false;
      }
      auto *localFrame = view->Frame();
      return localFrame && localFrame->Document() && localFrame->Document()->GetEditor().HasComposition();
    }

    KRYS_NODISCARD DOMStringAtom EventTypeForKeyboardEventType(PlatformEvent::Type type) noexcept
    {
      switch (type)
      {
        case PlatformEvent::Type::KeyUp:      return DOMStringAtom(EventNames::KeyUp);
        case PlatformEvent::Type::RawKeyDown: return DOMStringAtom(EventNames::KeyDown);
        case PlatformEvent::Type::Char:       return DOMStringAtom(EventNames::KeyPress);
        case PlatformEvent::Type::KeyDown:
          // The caller should disambiguate the combined event into RawKeyDown or Char events.
          break;
        default: break;
      }
      krys_debug_assert(false);
      return DOMStringAtom(EventNames::KeyDown);
    }

    KRYS_NODISCARD int32 WindowsVirtualKeyCodeWithoutLocation(int32 keycode) noexcept
    {
      switch (keycode)
      {
        case vk::LControl:
        case vk::RControl: return vk::Control;
        case vk::LShift:
        case vk::RShift:   return vk::Shift;
        case vk::LMenu:
        case vk::RMenu:    return vk::Menu;
        default:           return keycode;
      }
    }

    KRYS_NODISCARD KeyboardEvent::KeyLocationCode
      LocationCodeForKey(const PlatformKeyboardEvent &key) noexcept
    {
      if (key.IsKeypad())
      {
        return KeyboardEvent::DOM_KEY_LOCATION_NUMPAD;
      }

      switch (key.WindowsVirtualKeyCode())
      {
        case vk::LControl:
        case vk::LShift:
        case vk::LMenu: // Left Option/Alt
        case vk::LWin:  // Left Command/Windows key (Natural keyboard)
          return KeyboardEvent::DOM_KEY_LOCATION_LEFT;
        case vk::RControl:
        case vk::RShift:
        case vk::RMenu: // Right Option/Alt
        case vk::RWin:  // Right Windows key (Natural keyboard)
          return KeyboardEvent::DOM_KEY_LOCATION_RIGHT;
        default: return KeyboardEvent::DOM_KEY_LOCATION_STANDARD;
      }
    }
  }

  KeyboardEvent::KeyboardEvent(DOMStringAtom type, const KeyboardEventInit &initializer) noexcept
      : UIEventWithKeyState(type, initializer), _key(initializer.Key), _code(initializer.Code),
        _keyIdentifier(initializer.KeyIdentifier), _location(initializer.Location),
        _repeat(initializer.Repeat), _isComposing(initializer.IsComposing), _charCode(initializer.CharCode),
        _keyCode(initializer.KeyCode), _which(initializer.Which)
  {
  }

  KeyboardEvent::KeyboardEvent(DOMStringAtom type, const UIEventInit &eventInitDict,
                               const PlatformKeyboardEvent &key) noexcept
      : UIEventWithKeyState(type, eventInitDict, key.Modifiers()),
        _underlyingPlatformEvent(CreateUnique<PlatformKeyboardEvent>(key)), _key(key.Key()),
        _code(key.Code()), _keyIdentifier(key.KeyIdentifier()), _location(LocationCodeForKey(key)),
        _repeat(key.IsAutoRepeat())
  {
    _isTrusted = true;
    _timeStamp = static_cast<DOMHighResTimeStamp>(key.Timestamp().count());
  }

  Ref<KeyboardEvent> KeyboardEvent::Create(const PlatformKeyboardEvent &key,
                                           const LocalDOMWindow *view) noexcept
  {
    UIEventInit init;
    init.Bubbles = true;
    init.Cancelable = true;
    init.Composed = true;
    auto event = AdoptRef(*new KeyboardEvent(EventTypeForKeyboardEventType(key.GetType()), init, key));
    event->_isComposing = ViewIsCompositing(view);
    return event;
  }

  void KeyboardEvent::InitKeyboardEvent(DOMStringAtom type, bool canBubble, bool cancelable,
                                        DOMStringView keyIdentifier, uint32 location, bool ctrlKey,
                                        bool altKey, bool shiftKey, bool metaKey) noexcept
  {
    if (_dispatched)
    {
      return;
    }

    InitUIEvent(type, canBubble, cancelable, 0);

    _keyIdentifier = DOMString(keyIdentifier);
    _location = location;

    SetModifierKeys(ctrlKey, altKey, shiftKey, metaKey);

    _charCode = null;
    _isComposing = false;
    _keyCode = null;
    _repeat = false;
    _underlyingPlatformEvent = nullptr;
    _which = null;
    _code.clear();
    _key.clear();
  }

  int32 KeyboardEvent::KeyCode() const noexcept
  {
    if (_keyCode)
    {
      return static_cast<int32>(*_keyCode);
    }

    // IE: virtual key code for keyup/keydown, character code for keypress
    // Firefox: virtual key code for keyup/keydown, zero for keypress
    // We match IE.
    if (!_underlyingPlatformEvent)
    {
      return 0;
    }
    if (Type() == EventNames::KeyDown || Type() == EventNames::KeyUp)
    {
      return WindowsVirtualKeyCodeWithoutLocation(_underlyingPlatformEvent->WindowsVirtualKeyCode());
    }

    return CharCode();
  }

  int32 KeyboardEvent::KeyCodeForKeyDown() const noexcept
  {
    krys_debug_assert(Type() == EventNames::KeyPress);
    if (_keyCode)
    {
      return static_cast<int32>(*_keyCode);
    }

    if (!_underlyingPlatformEvent)
    {
      return 0;
    }

    return WindowsVirtualKeyCodeWithoutLocation(
      _underlyingPlatformEvent->WindowsVirtualKeyCodeWithoutKeyPressOverride());
  }

  int32 KeyboardEvent::CharCode() const noexcept
  {
    if (_charCode)
    {
      return static_cast<int32>(*_charCode);
    }

    // IE: not supported
    // Firefox: 0 for keydown/keyup events, character code for keypress
    // We match Firefox.
    // FIXME: The backward compatibility mode (needsKeyboardEventDisambiguationQuirks), where the character
    // code is always returned, if it's ever needed.
    if (!_underlyingPlatformEvent || Type() != EventNames::KeyPress
        || _underlyingPlatformEvent->Text().empty())
    {
      return 0;
    }
    const auto &text = _underlyingPlatformEvent->Text();
    return static_cast<int32>(text::DecodeUTF8(Span<const char8>(text.data(), text.size())).CodePoint);
  }

  uint32 KeyboardEvent::Which() const noexcept
  {
    // Netscape's "which" returns a virtual key code for keydown and keyup, and a character code for keypress.
    // That's exactly what IE's "keyCode" returns. So they are the same for keyboard events.
    if (_which)
    {
      return *_which;
    }
    return static_cast<uint32>(KeyCode());
  }
}
