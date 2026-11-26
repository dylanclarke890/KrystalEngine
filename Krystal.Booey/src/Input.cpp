#include "Krystal.Booey/Input.hpp"
#include "Krystal.Core/Strings/CodePointIterator.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Input/UIEvents/LegacyKeyCodes.hpp"
#include <format>

namespace krys::boo::input
{
  namespace
  {
  }

  KeyDescription Describe(pal::Key key) noexcept
  {
    using pal::Key;
    // Digits and letters.
    if (key >= Key::Num0 && key <= Key::Num9)
    {
      constexpr static utf8_stringview codes[] {u8"Digit0", u8"Digit1", u8"Digit2", u8"Digit3", u8"Digit4",
                                                u8"Digit5", u8"Digit6", u8"Digit7", u8"Digit8", u8"Digit9"};
      auto index = static_cast<int32>(key) - static_cast<int32>(Key::Num0);
      return {.Code = codes[index], .LegacyKeyCode = '0' + index};
    }
    if (key >= Key::A && key <= Key::Z)
    {
      constexpr static utf8_stringview codes[] {
        u8"KeyA", u8"KeyB", u8"KeyC", u8"KeyD", u8"KeyE", u8"KeyF", u8"KeyG", u8"KeyH", u8"KeyI",
        u8"KeyJ", u8"KeyK", u8"KeyL", u8"KeyM", u8"KeyN", u8"KeyO", u8"KeyP", u8"KeyQ", u8"KeyR",
        u8"KeyS", u8"KeyT", u8"KeyU", u8"KeyV", u8"KeyW", u8"KeyX", u8"KeyY", u8"KeyZ"};
      auto index = static_cast<int32>(key) - static_cast<int32>(Key::A);
      return {.Code = codes[index], .LegacyKeyCode = 'A' + index};
    }
    if (key >= Key::Keypad0 && key <= Key::Keypad9)
    {
      constexpr static utf8_stringview codes[] {u8"Numpad0", u8"Numpad1", u8"Numpad2", u8"Numpad3",
                                                u8"Numpad4", u8"Numpad5", u8"Numpad6", u8"Numpad7",
                                                u8"Numpad8", u8"Numpad9"};
      auto index = static_cast<int32>(key) - static_cast<int32>(Key::Keypad0);
      return {.Code = codes[index], .LegacyKeyCode = web::vk::Numpad0 + index, .IsKeypad = true};
    }
    if (key >= Key::F1 && key <= Key::F20)
    {
      constexpr static utf8_stringview names[] {u8"F1",  u8"F2",  u8"F3",  u8"F4",  u8"F5",  u8"F6",  u8"F7",
                                                u8"F8",  u8"F9",  u8"F10", u8"F11", u8"F12", u8"F13", u8"F14",
                                                u8"F15", u8"F16", u8"F17", u8"F18", u8"F19", u8"F20"};
      auto index = static_cast<int32>(key) - static_cast<int32>(Key::F1);
      return {.Code = names[index], .NamedKey = names[index], .LegacyKeyCode = web::vk::F1 + index};
    }

    auto named = [](utf8_stringview code, utf8_stringview name, int32 keyCode,
                    bool isKeypad = false) -> KeyDescription
    {
      return {code, name, keyCode, isKeypad};
    };
    auto typing = [](utf8_stringview code, int32 keyCode, bool isKeypad = false) -> KeyDescription
    {
      return {.Code = code, .LegacyKeyCode = keyCode, .IsKeypad = isKeypad};
    };

    switch (key)
    {
      case Key::Equal:          return typing(u8"Equal", web::vk::OemPlus);
      case Key::Minus:          return typing(u8"Minus", web::vk::OemMinus);
      case Key::RightBracket:   return typing(u8"BracketRight", web::vk::Oem6);
      case Key::LeftBracket:    return typing(u8"BracketLeft", web::vk::Oem4);
      case Key::Quote:          return typing(u8"Quote", web::vk::Oem7);
      case Key::SemiColon:      return typing(u8"Semicolon", web::vk::Oem1);
      case Key::Backslash:      return typing(u8"Backslash", web::vk::Oem5);
      case Key::Comma:          return typing(u8"Comma", web::vk::OemComma);
      case Key::Slash:          return typing(u8"Slash", web::vk::Oem2);
      case Key::Period:         return typing(u8"Period", web::vk::OemPeriod);
      case Key::Grave:          return typing(u8"Backquote", web::vk::Oem3);
      case Key::Space:          return typing(u8"Space", web::vk::Space);
      case Key::KeypadDecimal:  return typing(u8"NumpadDecimal", web::vk::Decimal, true);
      case Key::KeypadMultiply: return typing(u8"NumpadMultiply", web::vk::Multiply, true);
      case Key::KeypadPlus:     return typing(u8"NumpadAdd", web::vk::Add, true);
      case Key::KeypadDivide:   return typing(u8"NumpadDivide", web::vk::Divide, true);
      case Key::KeypadMinus:    return typing(u8"NumpadSubtract", web::vk::Subtract, true);
      // No Windows key code: numeric keypads on Windows have no equals.
      case Key::KeypadEquals:   return typing(u8"NumpadEqual", 0, true);
      // Numpad 5 without Num Lock.
      case Key::KeypadClear:    return named(u8"Numpad5", u8"Clear", web::vk::Clear, true);
      case Key::KeypadEnter:    return named(u8"NumpadEnter", u8"Enter", web::vk::Return, true);
      case Key::Return:
      case Key::Enter:          return named(u8"Enter", u8"Enter", web::vk::Return);
      case Key::Tab:            return named(u8"Tab", u8"Tab", web::vk::Tab);
      // Backspace (named after the Mac key).
      case Key::Del:            return named(u8"Backspace", u8"Backspace", web::vk::Back);
      // Standard says that DEL becomes U+007F.
      case Key::ForwardDelete:  return named(u8"Delete", u8"Delete", web::vk::Delete);
      case Key::Escape:         return named(u8"Escape", u8"Escape", web::vk::Escape);
      case Key::Command:        return named(u8"MetaLeft", u8"Meta", web::vk::LWin);
      case Key::Shift:          return named(u8"ShiftLeft", u8"Shift", web::vk::LShift);
      case Key::RightShift:     return named(u8"ShiftRight", u8"Shift", web::vk::RShift);
      case Key::CapsLock:       return named(u8"CapsLock", u8"CapsLock", web::vk::Capital);
      case Key::Option:         return named(u8"AltLeft", u8"Alt", web::vk::LMenu);
      case Key::RightOption:    return named(u8"AltRight", u8"Alt", web::vk::RMenu);
      case Key::Control:        return named(u8"ControlLeft", u8"Control", web::vk::LControl);
      case Key::RightControl:   return named(u8"ControlRight", u8"Control", web::vk::RControl);
      case Key::Function:       return named(u8"Fn", u8"Fn", 0);
      case Key::VolumeUp:       return named(u8"AudioVolumeUp", u8"AudioVolumeUp", web::vk::VolumeUp);
      case Key::VolumeDown:     return named(u8"AudioVolumeDown", u8"AudioVolumeDown", web::vk::VolumeDown);
      case Key::Mute:           return named(u8"AudioVolumeMute", u8"AudioVolumeMute", web::vk::VolumeMute);
      case Key::Help:           return named(u8"Help", u8"Help", web::vk::Help);
      case Key::Home:           return named(u8"Home", u8"Home", web::vk::Home);
      case Key::End:            return named(u8"End", u8"End", web::vk::End);
      case Key::PageUp:         return named(u8"PageUp", u8"PageUp", web::vk::Prior);
      case Key::PageDown:       return named(u8"PageDown", u8"PageDown", web::vk::Next);
      case Key::LeftArrow:      return named(u8"ArrowLeft", u8"ArrowLeft", web::vk::Left);
      case Key::RightArrow:     return named(u8"ArrowRight", u8"ArrowRight", web::vk::Right);
      case Key::DownArrow:      return named(u8"ArrowDown", u8"ArrowDown", web::vk::Down);
      case Key::UpArrow:        return named(u8"ArrowUp", u8"ArrowUp", web::vk::Up);
      case Key::Insert:         return named(u8"Insert", u8"Insert", web::vk::Insert);
      case Key::PrintScreen:    return named(u8"PrintScreen", u8"PrintScreen", web::vk::Snapshot);
      case Key::Pause:          return named(u8"Pause", u8"Pause", web::vk::Pause);
      case Key::ScrollLock:     return named(u8"ScrollLock", u8"ScrollLock", web::vk::Scroll);
      case Key::NumLock:        return named(u8"NumLock", u8"NumLock", web::vk::NumLock);
      case Key::ContextMenu:    return named(u8"ContextMenu", u8"ContextMenu", web::vk::Apps);
      default:                  return named({}, u8"Unidentified", 0);
    }
  }

  web::MouseButton ToWeb(pal::MouseButton button) noexcept
  {
    switch (button)
    {
      case pal::MouseButton::Left:   return web::MouseButton::Left;
      case pal::MouseButton::Right:  return web::MouseButton::Right;
      case pal::MouseButton::Middle: return web::MouseButton::Middle;
      case pal::MouseButton::Thumb1: return web::MouseButton::Back;
      case pal::MouseButton::Thumb2: return web::MouseButton::Forward;
      default:                       return web::MouseButton::None;
    }
  }

  EnumSet<web::PlatformEvent::Modifier> ToWeb(pal::KeyModifiers modifiers) noexcept
  {
    using Modifier = web::PlatformEvent::Modifier;
    EnumSet<Modifier> result;
    if (modifiers.contains(pal::KeyModifier::Shift))
    {
      result.Add(Modifier::ShiftKey);
    }
    if (modifiers.contains(pal::KeyModifier::Control))
    {
      result.Add(Modifier::ControlKey);
    }
    if (modifiers.contains(pal::KeyModifier::Alt))
    {
      result.Add(Modifier::AltKey);
    }
    if (modifiers.contains(pal::KeyModifier::Meta))
    {
      result.Add(Modifier::MetaKey);
    }
    if (modifiers.contains(pal::KeyModifier::CapsLock))
    {
      result.Add(Modifier::CapsLockKey);
    }
    return result;
  }

  EnumSet<web::DragOperation> ToWeb(pal::DropOperations operations) noexcept
  {
    EnumSet<web::DragOperation> result;
    if (operations.contains(pal::DropOperation::Copy))
    {
      result.Add(web::DragOperation::Copy);
    }
    if (operations.contains(pal::DropOperation::Move))
    {
      result.Add(web::DragOperation::Move);
    }
    if (operations.contains(pal::DropOperation::Link))
    {
      result.Add(web::DragOperation::Link);
    }
    return result;
  }

  Maybe<pal::DropOperation> ToDropOperation(web::DragOperation operation) noexcept
  {
    switch (operation)
    {
      case web::DragOperation::Copy: return pal::DropOperation::Copy;
      case web::DragOperation::Move: return pal::DropOperation::Move;
      case web::DragOperation::Link: return pal::DropOperation::Link;
      default:                       return null;
    }
  }

  web::PlatformKeyboardEvent KeyboardEvent(web::PlatformEvent::Type type, pal::Key key,
                                           pal::KeyModifiers modifiers, const pal::KeyText &text,
                                           bool isAutoRepeat, utf8_stringview releasedKeyValue) noexcept
  {
    auto description = Describe(key);
    utf8_string keyValue;
    if (type == web::PlatformEvent::Type::KeyUp && !releasedKeyValue.empty())
    {
      keyValue = utf8_string(releasedKeyValue);
    }
    else if (!description.NamedKey.empty())
    {
      keyValue = utf8_string(description.NamedKey);
    }
    else if (text.DeadKey)
    {
      // A dead key types nothing itself; its accent joins the next key's text.
      // https://w3c.github.io/uievents-key/#key-Dead
      keyValue = u8"Dead";
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
      keyValue = u8"Unidentified";
    }

    // Alt combinations are system keys, as on Windows.
    bool isSystemKey = modifiers.contains(pal::KeyModifier::Alt);
    return web::PlatformKeyboardEvent(type, text.Text, text.UnmodifiedText, krys::move(keyValue),
                                      utf8_string(description.Code), description.LegacyKeyCode, isAutoRepeat,
                                      description.IsKeypad, isSystemKey, ToWeb(modifiers),
                                      web::html::EventLoop::Current().Now());
  }

  namespace
  {
    /// @brief A key press the input method handled, with what it made of it.
    class InputMethodPlatformKeyboardEvent : public web::PlatformKeyboardEvent
    {
    public:
      InputMethodPlatformKeyboardEvent(const utf8_string &text,
                                       Maybe<List<web::CompositionUnderline>> underlines,
                                       uint32 selectionStart, uint32 selectionLength) noexcept
          : PlatformKeyboardEvent(Type::KeyDown, text, text, u8"Unidentified", u8"Unidentified",
                                  web::vk::Process, false, false, false, {},
                                  web::html::EventLoop::Current().Now())
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

  web::PlatformKeyboardEvent InputMethodKeyboardEvent(utf8_string text,
                                                      Maybe<List<web::CompositionUnderline>> underlines,
                                                      uint32 selectionStart, uint32 selectionLength) noexcept
  {
    return InputMethodPlatformKeyboardEvent(krys::move(text), krys::move(underlines), selectionStart,
                                            selectionLength);
  }

  web::PlatformKeyboardEvent TextInputEvent(const utf8_string &text) noexcept
  {
    return web::PlatformKeyboardEvent(web::PlatformEvent::Type::Char, text, text, text, {}, 0, false, false,
                                      false, {}, web::html::EventLoop::Current().Now());
  }

  List<web::CompositionUnderline> CompositionUnderlines(utf8_stringview text,
                                                        Span<const pal::CompositionClause> clauses) noexcept
  {
    List<web::CompositionUnderline> underlines;
    for (const auto &clause : clauses)
    {
      underlines.push_back({.StartOffset = clause.Start,
                            .EndOffset = clause.End,
                            .UnderlineColor = web::CompositionUnderlineColor::TextColor,
                            .Thick = clause.Target});
    }
    if (underlines.empty() && !text.empty())
    {
      underlines.push_back({.StartOffset = 0,
                            .EndOffset = static_cast<uint32>(text.size()),
                            .UnderlineColor = web::CompositionUnderlineColor::TextColor});
    }
    return underlines;
  }

  pal::CursorShape ToCursorShape(const web::Cursor &cursor) noexcept
  {
    using Type = web::Cursor::Type;
    switch (cursor.GetType())
    {
      case Type::Hand:
      case Type::Grab:
      case Type::Grabbing:                 return pal::CursorShape::Hand;
      // An image's cursor is `View::CurrentCursorImage`; this is the system cursor in its place.
      case Type::Custom:                   return pal::CursorShape::Arrow;
      case Type::IBeam:
      case Type::VerticalText:             return pal::CursorShape::IBeam;
      case Type::Cross:
      case Type::Cell:                     return pal::CursorShape::Crosshair;
      case Type::Wait:                     return pal::CursorShape::Wait;
      case Type::Progress:                 return pal::CursorShape::Progress;
      case Type::Help:                     return pal::CursorShape::Help;
      case Type::NoDrop:
      case Type::NotAllowed:               return pal::CursorShape::NotAllowed;
      case Type::Move:
      case Type::MiddlePanning:            return pal::CursorShape::Move;
      case Type::NorthResize:
      case Type::SouthResize:
      case Type::NorthPanning:
      case Type::SouthPanning:
      case Type::NorthSouthResize:
      case Type::RowResize:                return pal::CursorShape::ResizeNS;
      case Type::EastResize:
      case Type::WestResize:
      case Type::EastPanning:
      case Type::WestPanning:
      case Type::EastWestResize:
      case Type::ColumnResize:             return pal::CursorShape::ResizeEW;
      case Type::NorthEastResize:
      case Type::SouthWestResize:
      case Type::NorthEastPanning:
      case Type::SouthWestPanning:
      case Type::NorthEastSouthWestResize: return pal::CursorShape::ResizeNESW;
      case Type::NorthWestResize:
      case Type::SouthEastResize:
      case Type::NorthWestPanning:
      case Type::SouthEastPanning:
      case Type::NorthWestSouthEastResize: return pal::CursorShape::ResizeNWSE;
      case Type::None:                     return pal::CursorShape::Hidden;
      default:                             return pal::CursorShape::Arrow;
    }
  }

  web::PlatformWheelEvent WheelEvent(const PointI &point, const PointI &screenPoint, float notchesX,
                                     float notchesY, pal::KeyModifiers palModifiers,
                                     const pal::WheelScrollSettings &settings) noexcept
  {
    // As a Windows wheel message: one wheel at a time, the horizontal one if it turned.
    auto modifiers = ToWeb(palModifiers);
    bool isHorizontalWheel = notchesX != 0;
    float delta = isHorizontalWheel ? notchesX : notchesY;

    float wheelTicksX = 0;
    float wheelTicksY = 0;
    if (isHorizontalWheel)
    {
      // Windows is <-- -/+ -->, Krystal.Web wants <-- +/- -->, so we negate |delta| after saving the original value
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
    auto granularity = web::PlatformWheelEventGranularity::ScrollByPixelWheelEvent;
    if (isHorizontalWheel || modifiers.contains(web::PlatformEvent::Modifier::ShiftKey))
    {
      deltaX = delta * static_cast<float>(settings.Chars) * web::ScrollbarPixelsPerLine;
    }
    else
    {
      deltaY = delta;
      if (settings.Lines)
      {
        deltaY *= static_cast<float>(*settings.Lines) * web::ScrollbarPixelsPerLine;
      }
      else
      {
        granularity = web::PlatformWheelEventGranularity::ScrollByPageWheelEvent;
      }
    }

    return web::PlatformWheelEvent(point, screenPoint, deltaX, deltaY, wheelTicksX, wheelTicksY, granularity,
                                   modifiers, web::html::EventLoop::Current().Now());
  }

  web::PlatformWheelEvent TouchpadPanEvent(const PointI &point, const PointI &screenPoint,
                                           const pal::TouchpadPanInput &input) noexcept
  {
    using Phase = web::PlatformWheelEventPhase;
    auto phase = Phase::None;
    auto momentumPhase = Phase::None;
    switch (input.Phase)
    {
      case pal::TouchpadPanPhase::Began:           phase = Phase::Began; break;
      case pal::TouchpadPanPhase::Changed:         phase = Phase::Changed; break;
      case pal::TouchpadPanPhase::Ended:           phase = Phase::Ended; break;
      case pal::TouchpadPanPhase::MomentumBegan:   momentumPhase = Phase::Began; break;
      case pal::TouchpadPanPhase::MomentumChanged: momentumPhase = Phase::Changed; break;
      case pal::TouchpadPanPhase::MomentumEnded:   momentumPhase = Phase::Ended; break;
    }

    // The content moves with the fingers, as Krystal.Web's deltas go. The ticks are the notches a wheel would
    // have turned, at 100 pixels per three lines a notch (right and away from the user positive).
    constexpr float pixelsPerNotch = 3 * web::ScrollbarPixelsPerLine;
    return web::PlatformWheelEvent(point, screenPoint, input.DeltaX, input.DeltaY,
                                   -input.DeltaX / pixelsPerNotch, input.DeltaY / pixelsPerNotch,
                                   web::PlatformWheelEventGranularity::ScrollByPixelWheelEvent,
                                   ToWeb(input.Modifiers), web::html::EventLoop::Current().Now())
      .CopyAsTouchpadPan(phase, momentumPhase);
  }
}
