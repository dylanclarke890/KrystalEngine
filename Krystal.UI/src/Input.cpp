#include "Krystal.UI/Input.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include <format>

namespace krys::ui::input
{
  namespace
  {
    /// @brief The legacy key identifier of a key that types @p text: `U+` and its first character in hex,
    /// with ASCII letters in upper case.
    KRYS_NODISCARD utf8_string KeyIdentifierForText(utf8_stringview text) noexcept
    {
      if (text.empty())
      {
        return u8"Unidentified";
      }
      auto codePoint =
        static_cast<uint32>(text::DecodeUTF8(Span<const char8>(text.data(), text.size())).CodePoint);
      if (codePoint >= 'a' && codePoint <= 'z')
      {
        codePoint -= 'a' - 'A';
      }
      auto identifier = std::format("U+{:04X}", codePoint);
      return utf8_string(identifier.begin(), identifier.end());
    }
  }

  KeyDescription Describe(pal::Key key) noexcept
  {
    using pal::Key;
    // Digits and letters.
    if (key >= Key::Num0 && key <= Key::Num9)
    {
      static constexpr utf8_stringview codes[] {u8"Digit0", u8"Digit1", u8"Digit2", u8"Digit3", u8"Digit4",
                                                u8"Digit5", u8"Digit6", u8"Digit7", u8"Digit8", u8"Digit9"};
      auto index = static_cast<int32>(key) - static_cast<int32>(Key::Num0);
      return {.Code = codes[index], .WindowsVirtualKeyCode = '0' + index};
    }
    if (key >= Key::A && key <= Key::Z)
    {
      static constexpr utf8_stringview codes[] {
        u8"KeyA", u8"KeyB", u8"KeyC", u8"KeyD", u8"KeyE", u8"KeyF", u8"KeyG", u8"KeyH", u8"KeyI",
        u8"KeyJ", u8"KeyK", u8"KeyL", u8"KeyM", u8"KeyN", u8"KeyO", u8"KeyP", u8"KeyQ", u8"KeyR",
        u8"KeyS", u8"KeyT", u8"KeyU", u8"KeyV", u8"KeyW", u8"KeyX", u8"KeyY", u8"KeyZ"};
      auto index = static_cast<int32>(key) - static_cast<int32>(Key::A);
      return {.Code = codes[index], .WindowsVirtualKeyCode = 'A' + index};
    }
    if (key >= Key::Keypad0 && key <= Key::Keypad9)
    {
      static constexpr utf8_stringview codes[] {u8"Numpad0", u8"Numpad1", u8"Numpad2", u8"Numpad3",
                                                u8"Numpad4", u8"Numpad5", u8"Numpad6", u8"Numpad7",
                                                u8"Numpad8", u8"Numpad9"};
      auto index = static_cast<int32>(key) - static_cast<int32>(Key::Keypad0);
      return {.Code = codes[index], .WindowsVirtualKeyCode = boo::vk::Numpad0 + index, .IsKeypad = true};
    }
    if (key >= Key::F1 && key <= Key::F20)
    {
      static constexpr utf8_stringview names[] {u8"F1",  u8"F2",  u8"F3",  u8"F4",  u8"F5",  u8"F6",  u8"F7",
                                                u8"F8",  u8"F9",  u8"F10", u8"F11", u8"F12", u8"F13", u8"F14",
                                                u8"F15", u8"F16", u8"F17", u8"F18", u8"F19", u8"F20"};
      auto index = static_cast<int32>(key) - static_cast<int32>(Key::F1);
      return {.Code = names[index],
              .NamedKey = names[index],
              .KeyIdentifier = names[index],
              .WindowsVirtualKeyCode = boo::vk::F1 + index};
    }

    auto named = [](utf8_stringview code, utf8_stringview name, utf8_stringview identifier, int32 keyCode,
                    bool isKeypad = false) -> KeyDescription
    {
      return {code, name, identifier, keyCode, isKeypad};
    };
    auto typing = [](utf8_stringview code, int32 keyCode, bool isKeypad = false) -> KeyDescription
    {
      return {.Code = code, .WindowsVirtualKeyCode = keyCode, .IsKeypad = isKeypad};
    };

    switch (key)
    {
      case Key::Equal:          return typing(u8"Equal", boo::vk::OemPlus);
      case Key::Minus:          return typing(u8"Minus", boo::vk::OemMinus);
      case Key::RightBracket:   return typing(u8"BracketRight", boo::vk::Oem6);
      case Key::LeftBracket:    return typing(u8"BracketLeft", boo::vk::Oem4);
      case Key::Quote:          return typing(u8"Quote", boo::vk::Oem7);
      case Key::SemiColon:      return typing(u8"Semicolon", boo::vk::Oem1);
      case Key::Backslash:      return typing(u8"Backslash", boo::vk::Oem5);
      case Key::Comma:          return typing(u8"Comma", boo::vk::OemComma);
      case Key::Slash:          return typing(u8"Slash", boo::vk::Oem2);
      case Key::Period:         return typing(u8"Period", boo::vk::OemPeriod);
      case Key::Grave:          return typing(u8"Backquote", boo::vk::Oem3);
      case Key::Space:          return typing(u8"Space", boo::vk::Space);
      case Key::KeypadDecimal:  return typing(u8"NumpadDecimal", boo::vk::Decimal, true);
      case Key::KeypadMultiply: return typing(u8"NumpadMultiply", boo::vk::Multiply, true);
      case Key::KeypadPlus:     return typing(u8"NumpadAdd", boo::vk::Add, true);
      case Key::KeypadDivide:   return typing(u8"NumpadDivide", boo::vk::Divide, true);
      case Key::KeypadMinus:    return typing(u8"NumpadSubtract", boo::vk::Subtract, true);
      // No Windows key code: numeric keypads on Windows have no equals.
      case Key::KeypadEquals:   return typing(u8"NumpadEqual", 0, true);
      // Numpad 5 without Num Lock.
      case Key::KeypadClear:    return named(u8"Numpad5", u8"Clear", u8"Clear", boo::vk::Clear, true);
      case Key::KeypadEnter:    return named(u8"NumpadEnter", u8"Enter", u8"Enter", boo::vk::Return, true);
      case Key::Return:
      case Key::Enter:          return named(u8"Enter", u8"Enter", u8"Enter", boo::vk::Return);
      case Key::Tab:            return named(u8"Tab", u8"Tab", u8"U+0009", boo::vk::Tab);
      // Backspace (named after the Mac key).
      case Key::Del:            return named(u8"Backspace", u8"Backspace", u8"U+0008", boo::vk::Back);
      // Standard says that DEL becomes U+007F.
      case Key::ForwardDelete:  return named(u8"Delete", u8"Delete", u8"U+007F", boo::vk::Delete);
      case Key::Escape:         return named(u8"Escape", u8"Escape", u8"U+001B", boo::vk::Escape);
      case Key::Command:        return named(u8"MetaLeft", u8"Meta", u8"Meta", boo::vk::LWin);
      case Key::Shift:          return named(u8"ShiftLeft", u8"Shift", u8"Shift", boo::vk::LShift);
      case Key::RightShift:     return named(u8"ShiftRight", u8"Shift", u8"Shift", boo::vk::RShift);
      case Key::CapsLock:       return named(u8"CapsLock", u8"CapsLock", u8"CapsLock", boo::vk::Capital);
      case Key::Option:         return named(u8"AltLeft", u8"Alt", u8"Alt", boo::vk::LMenu);
      case Key::RightOption:    return named(u8"AltRight", u8"Alt", u8"Alt", boo::vk::RMenu);
      case Key::Control:        return named(u8"ControlLeft", u8"Control", u8"Control", boo::vk::LControl);
      case Key::RightControl:   return named(u8"ControlRight", u8"Control", u8"Control", boo::vk::RControl);
      case Key::Function:       return named(u8"Fn", u8"Fn", u8"Fn", 0);
      case Key::VolumeUp:       return named(u8"AudioVolumeUp", u8"AudioVolumeUp", u8"VolumeUp", boo::vk::VolumeUp);
      case Key::VolumeDown:
        return named(u8"AudioVolumeDown", u8"AudioVolumeDown", u8"VolumeDown", boo::vk::VolumeDown);
      case Key::Mute:
        return named(u8"AudioVolumeMute", u8"AudioVolumeMute", u8"VolumeMute", boo::vk::VolumeMute);
      case Key::Help:       return named(u8"Help", u8"Help", u8"Help", boo::vk::Help);
      case Key::Home:       return named(u8"Home", u8"Home", u8"Home", boo::vk::Home);
      case Key::End:        return named(u8"End", u8"End", u8"End", boo::vk::End);
      case Key::PageUp:     return named(u8"PageUp", u8"PageUp", u8"PageUp", boo::vk::Prior);
      case Key::PageDown:   return named(u8"PageDown", u8"PageDown", u8"PageDown", boo::vk::Next);
      case Key::LeftArrow:  return named(u8"ArrowLeft", u8"ArrowLeft", u8"Left", boo::vk::Left);
      case Key::RightArrow: return named(u8"ArrowRight", u8"ArrowRight", u8"Right", boo::vk::Right);
      case Key::DownArrow:  return named(u8"ArrowDown", u8"ArrowDown", u8"Down", boo::vk::Down);
      case Key::UpArrow:    return named(u8"ArrowUp", u8"ArrowUp", u8"Up", boo::vk::Up);
      default:              return named({}, u8"Unidentified", u8"Unidentified", 0);
    }
  }

  boo::MouseButton ToBooey(pal::MouseButton button) noexcept
  {
    switch (button)
    {
      case pal::MouseButton::Left:   return boo::MouseButton::Left;
      case pal::MouseButton::Right:  return boo::MouseButton::Right;
      case pal::MouseButton::Middle: return boo::MouseButton::Middle;
      case pal::MouseButton::Thumb1: return boo::MouseButton::Back;
      case pal::MouseButton::Thumb2: return boo::MouseButton::Forward;
      default:                       return boo::MouseButton::None;
    }
  }

  EnumSet<boo::PlatformEvent::Modifier> ToBooey(pal::KeyModifiers modifiers) noexcept
  {
    using Modifier = boo::PlatformEvent::Modifier;
    EnumSet<Modifier> result;
    if (modifiers.Contains(pal::KeyModifier::Shift))
    {
      result.Add(Modifier::ShiftKey);
    }
    if (modifiers.Contains(pal::KeyModifier::Control))
    {
      result.Add(Modifier::ControlKey);
    }
    if (modifiers.Contains(pal::KeyModifier::Alt))
    {
      result.Add(Modifier::AltKey);
    }
    if (modifiers.Contains(pal::KeyModifier::Meta))
    {
      result.Add(Modifier::MetaKey);
    }
    if (modifiers.Contains(pal::KeyModifier::CapsLock))
    {
      result.Add(Modifier::CapsLockKey);
    }
    return result;
  }

  boo::PlatformKeyboardEvent KeyboardEvent(boo::PlatformEvent::Type type, pal::Key key,
                                           pal::KeyModifiers modifiers, const pal::KeyText &text,
                                           bool isAutoRepeat, utf8_stringview releasedKeyValue) noexcept
  {
    auto description = Describe(key);
    utf8_string keyValue;
    if (type == boo::PlatformEvent::Type::KeyUp && !releasedKeyValue.empty())
    {
      keyValue = utf8_string(releasedKeyValue);
    }
    else if (!description.NamedKey.empty())
    {
      keyValue = utf8_string(description.NamedKey);
    }
    else if (!text.Text.empty())
    {
      keyValue = text.Text;
    }
    else if (!text.UnmodifiedText.empty())
    {
      keyValue = text.UnmodifiedText;
    }
    else
    {
      // FIXME: Dead keys ("Dead"), once the platform layer reports them.
      keyValue = u8"Unidentified";
    }

    utf8_string keyIdentifier = !description.KeyIdentifier.empty()
                                  ? utf8_string(description.KeyIdentifier)
                                  : KeyIdentifierForText(!text.UnmodifiedText.empty() ? text.UnmodifiedText
                                                         : !text.Text.empty()         ? text.Text
                                                                                      : keyValue);

    // Alt combinations are system keys, as on Windows.
    bool isSystemKey = modifiers.Contains(pal::KeyModifier::Alt);
    return boo::PlatformKeyboardEvent(type, text.Text, text.UnmodifiedText, krys::move(keyValue),
                                      utf8_string(description.Code), krys::move(keyIdentifier),
                                      description.WindowsVirtualKeyCode, isAutoRepeat, description.IsKeypad,
                                      isSystemKey, ToBooey(modifiers), MonotonicTime::Now());
  }

  namespace
  {
    /// @brief A key press the input method handled, with what it made of it.
    class InputMethodPlatformKeyboardEvent : public boo::PlatformKeyboardEvent
    {
    public:
      InputMethodPlatformKeyboardEvent(utf8_string text, Maybe<List<boo::CompositionUnderline>> underlines,
                                       uint32 selectionStart, uint32 selectionLength) noexcept
          : PlatformKeyboardEvent(Type::KeyDown, text, text, u8"Unidentified", u8"Unidentified", u8"U+0000",
                                  boo::vk::Process, false, false, false, {}, MonotonicTime::Now())
      {
        _handledByInputMethod = true;
        if (underlines)
        {
          _preeditSelectionRangeStart = selectionStart;
          _preeditSelectionRangeLength = selectionLength;
        }
        _preeditUnderlines = krys::move(underlines);
      }
    };
  }

  boo::PlatformKeyboardEvent InputMethodKeyboardEvent(utf8_string text,
                                                      Maybe<List<boo::CompositionUnderline>> underlines,
                                                      uint32 selectionStart, uint32 selectionLength) noexcept
  {
    return InputMethodPlatformKeyboardEvent(krys::move(text), krys::move(underlines), selectionStart,
                                            selectionLength);
  }

  boo::PlatformKeyboardEvent TextInputEvent(utf8_string text) noexcept
  {
    auto keyIdentifier = KeyIdentifierForText(text);
    return boo::PlatformKeyboardEvent(boo::PlatformEvent::Type::Char, text, text, text, {},
                                      krys::move(keyIdentifier), 0, false, false, false, {},
                                      MonotonicTime::Now());
  }

  List<boo::CompositionUnderline> CompositionUnderlines(utf8_stringview text,
                                                        Span<const pal::CompositionClause> clauses) noexcept
  {
    List<boo::CompositionUnderline> underlines;
    for (const auto &clause : clauses)
    {
      underlines.push_back({.StartOffset = clause.Start,
                            .EndOffset = clause.End,
                            .UnderlineColor = boo::CompositionUnderlineColor::TextColor,
                            .Thick = clause.Target});
    }
    if (underlines.empty() && !text.empty())
    {
      underlines.push_back({.StartOffset = 0,
                            .EndOffset = static_cast<uint32>(text.size()),
                            .UnderlineColor = boo::CompositionUnderlineColor::TextColor});
    }
    return underlines;
  }

  pal::CursorShape ToCursorShape(boo::Cursor cursor) noexcept
  {
    using Type = boo::Cursor::Type;
    switch (cursor.GetType())
    {
      case Type::Hand:
      case Type::Grab:
      case Type::Grabbing:                 return pal::CursorShape::Hand;
      case Type::IBeam:
      case Type::VerticalText:             return pal::CursorShape::IBeam;
      case Type::Cross:
      case Type::Cell:                     return pal::CursorShape::Crosshair;
      case Type::Wait:                     return pal::CursorShape::Wait;
      case Type::Progress:                 return pal::CursorShape::Progress;
      case Type::Help:                     return pal::CursorShape::Help;
      case Type::NoDrop:
      case Type::NotAllowed:               return pal::CursorShape::NotAllowed;
      case Type::Move:                     return pal::CursorShape::Move;
      case Type::NorthResize:
      case Type::SouthResize:
      case Type::NorthSouthResize:
      case Type::RowResize:                return pal::CursorShape::ResizeNS;
      case Type::EastResize:
      case Type::WestResize:
      case Type::EastWestResize:
      case Type::ColumnResize:             return pal::CursorShape::ResizeEW;
      case Type::NorthEastResize:
      case Type::SouthWestResize:
      case Type::NorthEastSouthWestResize: return pal::CursorShape::ResizeNESW;
      case Type::NorthWestResize:
      case Type::SouthEastResize:
      case Type::NorthWestSouthEastResize: return pal::CursorShape::ResizeNWSE;
      case Type::None:                     return pal::CursorShape::Hidden;
      default:                             return pal::CursorShape::Arrow;
    }
  }

  boo::PlatformWheelEvent WheelEvent(const boo::IntPoint &point, float notchesX, float notchesY,
                                     pal::KeyModifiers palModifiers,
                                     const pal::WheelScrollSettings &settings) noexcept
  {
    // As a Windows wheel message: one wheel at a time, the horizontal one if it turned.
    auto modifiers = ToBooey(palModifiers);
    bool isHorizontalWheel = notchesX != 0;
    float delta = isHorizontalWheel ? notchesX : notchesY;

    float wheelTicksX = 0;
    float wheelTicksY = 0;
    if (isHorizontalWheel)
    {
      // Windows is <-- -/+ -->, Booey wants <-- +/- -->, so we negate |delta| after saving the original value
      // on the wheel tick member.
      wheelTicksX = delta;
      delta = -delta;
    }
    else
    {
      // Even though we use shift + vertical wheel to scroll horizontally, we still note it as a vertical
      // scroll on the wheel tick member, so that the DOM event we later construct will match the real
      // hardware event better.
      wheelTicksY = delta;
    }

    float deltaX = 0;
    float deltaY = 0;
    auto granularity = boo::PlatformWheelEventGranularity::ScrollByPixelWheelEvent;
    if (isHorizontalWheel || modifiers.Contains(boo::PlatformEvent::Modifier::ShiftKey))
    {
      deltaX = delta * static_cast<float>(settings.Chars) * boo::ScrollbarPixelsPerLine;
    }
    else
    {
      deltaY = delta;
      if (settings.Lines)
      {
        deltaY *= static_cast<float>(*settings.Lines) * boo::ScrollbarPixelsPerLine;
      }
      else
      {
        granularity = boo::PlatformWheelEventGranularity::ScrollByPageWheelEvent;
      }
    }

    // FIXME: The screen position, once the host reports where the page is on the screen.
    return boo::PlatformWheelEvent(point, point, deltaX, deltaY, wheelTicksX, wheelTicksY, granularity,
                                   modifiers, MonotonicTime::Now());
  }
}
