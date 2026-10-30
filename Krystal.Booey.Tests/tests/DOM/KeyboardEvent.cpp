#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD PlatformKeyboardEvent Key(PlatformEvent::Type type, utf8_string text, utf8_string key,
                                             utf8_string code, int32 keyCode, bool isKeypad = false,
                                             EnumSet<PlatformEvent::Modifier> modifiers = {}) noexcept
    {
      return PlatformKeyboardEvent(type, text, text, krys::move(key), krys::move(code), u8"", keyCode, true,
                                   isKeypad, false, modifiers, MonotonicTime::Now());
    }
  }

  TEST_CASE("KeyboardEvent: from a dictionary", "[DOM][KeyboardEvent]")
  {
    Ref<KeyboardEvent> defaults = CreateRef<KeyboardEvent>(DOMStringAtom(u8"keydown"));
    CHECK(defaults->IsKeyboardEvent());
    CHECK_FALSE(defaults->IsMouseEvent());
    CHECK(defaults->Key().empty());
    CHECK(defaults->Code().empty());
    CHECK(defaults->Location() == KeyboardEvent::DOM_KEY_LOCATION_STANDARD);
    CHECK_FALSE(defaults->Repeat());
    CHECK_FALSE(defaults->IsComposing());
    CHECK(defaults->KeyCode() == 0);
    CHECK(defaults->CharCode() == 0);
    CHECK(defaults->Which() == 0);
    CHECK_FALSE(defaults->IsTrusted());
    CHECK_FALSE(defaults->Bubbles());

    KeyboardEventInit init;
    init.Bubbles = true;
    init.Key = u8"A";
    init.Code = u8"KeyA";
    init.Location = KeyboardEvent::DOM_KEY_LOCATION_RIGHT;
    init.Repeat = true;
    init.ShiftKey = true;
    init.CharCode = 65;
    init.KeyCode = 66;
    init.Which = 67;
    Ref<KeyboardEvent> event = CreateRef<KeyboardEvent>(DOMStringAtom(u8"keypress"), init);
    CHECK(event->Bubbles());
    CHECK(event->Key() == u8"A");
    CHECK(event->Code() == u8"KeyA");
    CHECK(event->Location() == KeyboardEvent::DOM_KEY_LOCATION_RIGHT);
    CHECK(event->Repeat());
    CHECK(event->ShiftKey());
    CHECK(event->GetModifierState(u8"Shift"));
    // The dictionary's legacy codes win over the platform's.
    CHECK(event->CharCode() == 65);
    CHECK(event->KeyCode() == 66);
    CHECK(event->Which() == 67);

    SECTION("initKeyboardEvent")
    {
      event->InitKeyboardEvent(DOMStringAtom(u8"keyup"), false, true, u8"U+0041",
                               KeyboardEvent::DOM_KEY_LOCATION_LEFT, true, false, false, true);
      CHECK(event->Type() == u8"keyup");
      CHECK_FALSE(event->Bubbles());
      CHECK(event->Cancelable());
      CHECK(event->KeyIdentifier() == u8"U+0041");
      CHECK(event->Location() == KeyboardEvent::DOM_KEY_LOCATION_LEFT);
      CHECK(event->CtrlKey());
      CHECK(event->MetaKey());
      CHECK_FALSE(event->ShiftKey());
      CHECK(event->Key().empty());
      CHECK_FALSE(event->Repeat());
      CHECK(event->KeyCode() == 0);
      CHECK(event->CharCode() == 0);
      CHECK(event->Which() == 0);
    }
  }

  TEST_CASE("KeyboardEvent: from the platform", "[DOM][KeyboardEvent]")
  {
    SECTION("keydown")
    {
      auto down = KeyboardEvent::Create(Key(PlatformEvent::Type::RawKeyDown, u8"", u8"Shift", u8"ShiftLeft",
                                            vk::LShift, false, {PlatformEvent::Modifier::ShiftKey}));
      CHECK(down->Type() == u8"keydown");
      CHECK(down->IsTrusted());
      CHECK(down->Bubbles());
      CHECK(down->Cancelable());
      CHECK(down->Composed());
      CHECK(down->Key() == u8"Shift");
      CHECK(down->Code() == u8"ShiftLeft");
      CHECK(down->Repeat());
      CHECK(down->ShiftKey());
      // The key code is the key's without its side, which is the location.
      CHECK(down->Location() == KeyboardEvent::DOM_KEY_LOCATION_LEFT);
      CHECK(down->KeyCode() == vk::Shift);
      CHECK(down->Which() == vk::Shift);
      CHECK(down->CharCode() == 0);
      REQUIRE(down->UnderlyingPlatformEvent());
      CHECK(down->UnderlyingPlatformEvent()->WindowsVirtualKeyCode() == vk::LShift);
    }

    SECTION("keypress")
    {
      auto press = KeyboardEvent::Create(Key(PlatformEvent::Type::Char, u8"é", u8"é", u8"Digit2", '2'));
      CHECK(press->Type() == u8"keypress");
      // A keypress's codes are its character.
      CHECK(press->CharCode() == 0xe9);
      CHECK(press->KeyCode() == 0xe9);
      CHECK(press->Which() == 0xe9);
      CHECK(press->KeyCodeForKeyDown() == '2');
    }

    SECTION("keyup on the keypad")
    {
      auto up =
        KeyboardEvent::Create(Key(PlatformEvent::Type::KeyUp, u8"", u8"7", u8"Numpad7", vk::Numpad7, true));
      CHECK(up->Type() == u8"keyup");
      CHECK(up->Location() == KeyboardEvent::DOM_KEY_LOCATION_NUMPAD);
      CHECK(up->KeyCode() == vk::Numpad7);
      CHECK(up->CharCode() == 0);
    }

    SECTION("a combined key down splits into keydown and keypress")
    {
      PlatformKeyboardEvent combined(PlatformEvent::Type::KeyDown, u8"a", u8"a", u8"a", u8"KeyA", u8"U+0041",
                                     'A', false, false, false, {}, MonotonicTime::Now());
      auto rawKeyDown = combined;
      rawKeyDown.DisambiguateKeyDownEvent(PlatformEvent::Type::RawKeyDown);
      CHECK(rawKeyDown.GetType() == PlatformEvent::Type::RawKeyDown);
      CHECK(rawKeyDown.Text().empty());
      CHECK(rawKeyDown.UnmodifiedText().empty());
      CHECK(rawKeyDown.KeyIdentifier() == u8"U+0041");
      CHECK(rawKeyDown.WindowsVirtualKeyCode() == 'A');

      auto keyPress = combined;
      keyPress.DisambiguateKeyDownEvent(PlatformEvent::Type::Char);
      CHECK(keyPress.Text() == u8"a");
      CHECK(keyPress.KeyIdentifier().empty());
      // A char has no key code, though the keypress's keydown does.
      CHECK(keyPress.WindowsVirtualKeyCode() == 0);
      CHECK(keyPress.WindowsVirtualKeyCodeWithoutKeyPressOverride() == 'A');
    }
  }
}
