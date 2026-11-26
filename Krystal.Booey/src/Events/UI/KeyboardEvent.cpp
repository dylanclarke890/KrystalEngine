#include "Krystal.Booey/Events/UI/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/HTML/Window.hpp"
#include "Krystal.Booey/Input/LegacyKeyCodes.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Core/Strings/CodePointIterator.hpp"

namespace krys::boo
{
  namespace
  {
    /// @brief Whether @p view's editor is in the middle of an input method composition.
    KRYS_NODISCARD bool ViewIsCompositing(const Window *view) noexcept
    {
      if (!view)
      {
        return false;
      }
      auto *localFrame = view->Frame();
      return localFrame && localFrame->Document() && localFrame->Document()->GetEditor().HasComposition();
    }

    /// @brief The DOM event type of a platform key event. A combined key down must have been split into its
    /// raw key down and char first.
    KRYS_NODISCARD DOMStringAtom EventTypeForKeyboardEventType(PlatformEvent::Type type) noexcept
    {
      switch (type)
      {
        case PlatformEvent::Type::KeyUp:      return event_names::KeyUp;
        case PlatformEvent::Type::RawKeyDown: return event_names::KeyDown;
        case PlatformEvent::Type::Char:       return event_names::KeyPress;
        case PlatformEvent::Type::KeyDown:
        default:                              break;
      }
      krys_debug_assert(false);
      return event_names::KeyDown;
    }

    /// @brief @p keyCode with the side of a modifier key dropped: `keyCode` is 16 for either Shift key.
    KRYS_NODISCARD int32 LegacyKeyCodeWithoutLocation(int32 keyCode) noexcept
    {
      switch (keyCode)
      {
        case vk::LControl:
        case vk::RControl: return vk::Control;
        case vk::LShift:
        case vk::RShift:   return vk::Shift;
        case vk::LMenu:
        case vk::RMenu:    return vk::Menu;
        default:           return keyCode;
      }
    }

    /// @see https://w3c.github.io/uievents/#dom-keyboardevent-location
    KRYS_NODISCARD KeyboardEvent::KeyLocationCode
      LocationCodeForKey(const PlatformKeyboardEvent &key) noexcept
    {
      if (key.IsKeypad())
      {
        return KeyboardEvent::DOM_KEY_LOCATION_NUMPAD;
      }

      switch (key.LegacyKeyCode())
      {
        case vk::LControl:
        case vk::LShift:
        case vk::LMenu: // Alt or Option
        case vk::LWin:  // Windows or Command
          return KeyboardEvent::DOM_KEY_LOCATION_LEFT;
        case vk::RControl:
        case vk::RShift:
        case vk::RMenu:
        case vk::RWin:     return KeyboardEvent::DOM_KEY_LOCATION_RIGHT;
        default:           return KeyboardEvent::DOM_KEY_LOCATION_STANDARD;
      }
    }
  }

  KeyboardEvent::KeyboardEvent(DOMStringAtom type, const KeyboardEventInit &initializer) noexcept
      : UIEventWithKeyState(krys::move(type), initializer), _key(initializer.Key), _code(initializer.Code),
        _location(initializer.Location), _repeat(initializer.Repeat), _isComposing(initializer.IsComposing),
        _charCode(initializer.CharCode), _keyCode(initializer.KeyCode), _which(initializer.Which)
  {
  }

  KeyboardEvent::KeyboardEvent(DOMStringAtom type, const UIEventInit &eventInitDict,
                               const PlatformKeyboardEvent &key) noexcept
      : UIEventWithKeyState(krys::move(type), eventInitDict, key.Modifiers()),
        _underlyingPlatformEvent(CreateUnique<PlatformKeyboardEvent>(key)), _key(key.Key()),
        _code(key.Code()), _location(LocationCodeForKey(key)), _repeat(key.IsAutoRepeat())
  {
    _isTrusted = true;
    _createTime = key.Timestamp();
  }

  Ref<KeyboardEvent> KeyboardEvent::Create(const PlatformKeyboardEvent &key, Window *view) noexcept
  {
    UIEventInit init;
    init.View = view;
    init.Bubbles = true;
    init.Cancelable = true;
    init.Composed = true;
    auto event = AdoptRef(*new KeyboardEvent(EventTypeForKeyboardEventType(key.GetType()), init, key));
    event->_isComposing = ViewIsCompositing(view);
    return event;
  }

  void KeyboardEvent::InitKeyboardEvent(DOMStringAtom type, bool canBubble, bool cancelable, Window *view,
                                        DOMStringView key, uint32 location, bool ctrlKey, bool altKey,
                                        bool shiftKey, bool metaKey) noexcept
  {
    // As initUIEvent: nothing changes while the event is being dispatched.
    if (_dispatched)
    {
      return;
    }

    InitUIEvent(krys::move(type), canBubble, cancelable, view, 0);

    _key = DOMString(key);
    _location = location;

    SetModifierKeys(ctrlKey, altKey, shiftKey, metaKey);

    // What the arguments don't give goes back to its default, and the event no longer comes from the host.
    _charCode = null;
    _isComposing = false;
    _keyCode = null;
    _repeat = false;
    _underlyingPlatformEvent = nullptr;
    _which = null;
    _code.clear();
  }

  int32 KeyboardEvent::KeyCode() const noexcept
  {
    if (_keyCode)
    {
      return static_cast<int32>(*_keyCode);
    }

    // keydown and keyup give the key's virtual key code. keypress gives its character, as in UI Events'
    // conflated model: https://w3c.github.io/uievents/#legacy-key-models
    if (!_underlyingPlatformEvent)
    {
      return 0;
    }
    if (Type() == event_names::KeyDown || Type() == event_names::KeyUp)
    {
      return LegacyKeyCodeWithoutLocation(_underlyingPlatformEvent->LegacyKeyCode());
    }

    return CharCode();
  }

  int32 KeyboardEvent::KeyCodeForKeyDown() const noexcept
  {
    krys_debug_assert(Type() == event_names::KeyPress);
    if (_keyCode)
    {
      return static_cast<int32>(*_keyCode);
    }

    if (!_underlyingPlatformEvent)
    {
      return 0;
    }

    return LegacyKeyCodeWithoutLocation(_underlyingPlatformEvent->LegacyKeyCodeWithoutKeyPressOverride());
  }

  int32 KeyboardEvent::CharCode() const noexcept
  {
    if (_charCode)
    {
      return static_cast<int32>(*_charCode);
    }

    // Only keypress has a character code: its text's first code point.
    if (!_underlyingPlatformEvent || Type() != event_names::KeyPress
        || _underlyingPlatformEvent->Text().empty())
    {
      return 0;
    }
    const auto &text = _underlyingPlatformEvent->Text();
    return static_cast<int32>(text::DecodeUTF8(Span<const char8>(text.data(), text.size())).CodePoint);
  }

  uint32 KeyboardEvent::Which() const noexcept
  {
    // For a keyboard event, which is keyCode unless the dictionary gave it.
    if (_which)
    {
      return *_which;
    }
    return static_cast<uint32>(KeyCode());
  }
}
