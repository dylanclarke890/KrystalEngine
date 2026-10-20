#include "Krystal.UI/View.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"
#include "Krystal.UI/Input.hpp"
#include <filesystem>
#include <fstream>
#include <string>

namespace krys::ui::tests
{
  namespace
  {
    KRYS_NODISCARD int32 WidthOf(View &view, boo::dom::DOMStringView id)
    {
      auto element = view.Document().GetElementById(id);
      REQUIRE(element);
      return element->OffsetWidth();
    }

    /// @brief Records the keyboard events it handles as `type:key/code/keyCode/charCode/location`, with
    /// `+shift` and `+ctrl` for the modifiers.
    struct KeyRecorder : public boo::dom::EventListenerCallback
    {
      List<std::string> &Records;

      explicit KeyRecorder(List<std::string> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(boo::dom::Event &event) noexcept override
      {
        auto &key = static_cast<boo::dom::KeyboardEvent &>(event);
        auto type = key.Type().View();
        auto record =
          std::string(type.begin(), type.end()) + ":" + std::string(key.Key().begin(), key.Key().end()) + "/"
          + std::string(key.Code().begin(), key.Code().end()) + "/" + std::to_string(key.KeyCode()) + "/"
          + std::to_string(key.CharCode()) + "/" + std::to_string(key.Location());
        if (key.ShiftKey())
        {
          record += "+shift";
        }
        if (key.CtrlKey())
        {
          record += "+ctrl";
        }
        Records.push_back(record);
      }
    };

    /// @brief Records the presses, releases and clicks it handles as `type:id/detail`.
    struct ClickRecorder : public boo::dom::EventListenerCallback
    {
      List<std::string> &Records;

      explicit ClickRecorder(List<std::string> &records) noexcept : Records(records)
      {
      }

      void HandleEvent(boo::dom::Event &event) noexcept override
      {
        auto &mouse = static_cast<boo::dom::MouseEvent &>(event);
        auto type = mouse.Type().View();
        auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
        auto *element = node ? DynamicDowncast<boo::dom::Element>(*node) : nullptr;
        auto id =
          element ? element->GetAttribute(u8"id").value_or(boo::dom::DOMString()) : boo::dom::DOMString();
        Records.push_back(std::string(type.begin(), type.end()) + ":" + std::string(id.begin(), id.end())
                          + "/" + std::to_string(mouse.Detail()) + "/" + std::to_string(mouse.Buttons()));
      }
    };
  }

  TEST_CASE("View: lays out at the viewport's size", "[UI]")
  {
    View view({400, 300});
    CHECK(view.ViewportSize() == boo::IntSize(400, 300));
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } #half { width: 50% }</style>"
                  u8"<div id=full>a</div><div id=half>b</div>");
    view.UpdateRendering();
    CHECK(WidthOf(view, u8"full") == 400);
    CHECK(WidthOf(view, u8"half") == 200);

    SECTION("A resize lays it out again")
    {
      view.Resize({600, 300});
      view.UpdateRendering();
      CHECK(view.ViewportSize() == boo::IntSize(600, 300));
      CHECK(WidthOf(view, u8"full") == 600);
      CHECK(WidthOf(view, u8"half") == 300);
    }

    SECTION("Loading another document replaces it")
    {
      view.LoadHTML(
        u8"<!DOCTYPE html><style>body { margin: 0 }</style><p id=other style='width: 120px'>c</p>");
      view.UpdateRendering();
      CHECK_FALSE(view.Document().GetElementById(u8"full"));
      CHECK(WidthOf(view, u8"other") == 120);
    }
  }

  TEST_CASE("View: loads files", "[UI]")
  {
    const auto path = std::filesystem::temp_directory_path() / "krystal-ui-page-test.html";
    {
      std::ofstream file(path, std::ios::binary);
      // A byte order mark is decoded away rather than becoming text above the box.
      file << "\xEF\xBB\xBF<!DOCTYPE html><style>body { margin: 0 }</style><div id=box style='width: "
              "75px'>x</div>";
    }

    View view({200, 100});
    REQUIRE(view.LoadFile(path).has_value());
    view.UpdateRendering();
    CHECK(WidthOf(view, u8"box") == 75);
    auto box = view.Document().GetElementById(u8"box");
    REQUIRE(box);
    CHECK(box->OffsetTop() == 0);
    std::filesystem::remove(path);

    CHECK_FALSE(view.LoadFile(path).has_value());
  }

  TEST_CASE("View: records its painting", "[UI]")
  {
    View view({200, 100});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 }</style>"
                  u8"<div style='width: 40px; height: 20px; background: red'></div>");
    const auto displayList = view.Record();
    // The view's base colour, the root's background over the canvas and the box's background.
    REQUIRE(displayList->Items().size() == 5u);
    CHECK(displayList->AsText({}).ends_with(u8R"((fill-composited-rect
  (rect at (0,0) size 40x20)
  (color #FF0000)
  (composite-operation source-over)
  (blend-mode normal)))"));
  }

  TEST_CASE("View: hit tests, hovers and activates", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }"
      u8"#a:hover { width: 100px } #a:active { width: 50px }</style><div id=a></div><div id=b></div>");
    auto a = view.Document().GetElementById(u8"a");
    auto b = view.Document().GetElementById(u8"b");
    CHECK(view.HitTest({10, 10}) == a.get());
    CHECK(view.HitTest({10, 60}) == b.get());

    view.MouseMoved({10, 10});
    view.UpdateRendering();
    CHECK(a->Hovered());
    CHECK(WidthOf(view, u8"a") == 100);

    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.UpdateRendering();
    CHECK(a->Active());
    CHECK(WidthOf(view, u8"a") == 50);

    // While the button is down, only the active chain stays hovered.
    view.MouseMoved({10, 60});
    CHECK_FALSE(b->Hovered());
    view.MouseReleased({10, 60}, pal::MouseButton::Left);
    CHECK_FALSE(a->Active());
    view.MouseMoved({10, 10});
    CHECK(a->Hovered());
    view.MouseMoved({10, 60});
    CHECK(b->Hovered());
    CHECK_FALSE(a->Hovered());

    view.MouseLeft();
    CHECK_FALSE(b->Hovered());
  }

  TEST_CASE("View: dispatches mouse events and chooses the cursor", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif } div { height: 50px }"
                  u8"#b { cursor: pointer }</style><div id=a>text</div><div id=b></div>");
    view.UpdateRendering();
    List<std::string> records;
    Ref<boo::dom::EventListenerCallback> recorder = CreateRef<ClickRecorder>(records);
    for (auto type : {u8"mousedown", u8"mouseup", u8"click", u8"auxclick", u8"dblclick"})
    {
      view.Document().AddEventListener(boo::dom::DOMStringAtom(type), Ref {recorder}, false);
    }

    // The cursor follows the mouse: the arrow over a box, the I-beam over text and the style's cursor.
    view.MouseMoved({300, 10});
    CHECK(view.CurrentCursor() == pal::CursorShape::Arrow);
    view.MouseMoved({3, 10});
    CHECK(view.CurrentCursor() == pal::CursorShape::IBeam);
    view.MouseMoved({10, 60});
    CHECK(view.CurrentCursor() == pal::CursorShape::Hand);

    // Two quick presses in the same place are a double click; the buttons held are reported.
    view.MousePressed({10, 60}, pal::MouseButton::Left);
    view.MouseReleased({10, 60}, pal::MouseButton::Left);
    view.MousePressed({11, 61}, pal::MouseButton::Left);
    view.MouseReleased({11, 61}, pal::MouseButton::Left);
    CHECK(records
          == List<std::string> {"mousedown:b/1/1", "mouseup:b/1/0", "click:b/1/0", "mousedown:b/2/1",
                                "mouseup:b/2/0", "click:b/2/0", "dblclick:b/2/0"});
    records.clear();

    // A press far from the last starts counting again, as does another button.
    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.MouseReleased({10, 10}, pal::MouseButton::Left);
    view.MousePressed({10, 10}, pal::MouseButton::Right);
    view.MouseReleased({10, 10}, pal::MouseButton::Right);
    CHECK(records
          == List<std::string> {"mousedown:a/1/1", "mouseup:a/1/0", "click:a/1/0", "mousedown:a/1/2",
                                "mouseup:a/1/0", "auxclick:a/1/0"});
  }

  TEST_CASE("View: a click focuses and the window's focus", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }</style>"
                  u8"<div id=a tabindex=0></div><div id=b></div>");
    view.UpdateRendering();
    auto a = view.Document().GetElementById(u8"a");
    REQUIRE(a);

    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.MouseReleased({10, 10}, pal::MouseButton::Left);
    CHECK(view.Document().FocusedElement() == a.get());
    CHECK(view.Document().HasFocus());

    // The window losing focus keeps the focused element, but the page doesn't have focus.
    view.SetFocused(false);
    CHECK(view.Document().FocusedElement() == a.get());
    CHECK_FALSE(view.Document().HasFocus());
    // A page loaded meanwhile doesn't have focus either.
    view.LoadHTML(u8"<!DOCTYPE html><p>text</p>");
    CHECK_FALSE(view.Document().HasFocus());
    view.SetFocused(true);
    CHECK(view.Document().HasFocus());

    // A press on nothing focusable focuses the viewport.
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }</style>"
                  u8"<div id=a tabindex=0></div><div id=b></div>");
    view.UpdateRendering();
    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.MouseReleased({10, 10}, pal::MouseButton::Left);
    REQUIRE(view.Document().FocusedElement());
    view.MousePressed({10, 60}, pal::MouseButton::Left);
    view.MouseReleased({10, 60}, pal::MouseButton::Left);
    CHECK_FALSE(view.Document().FocusedElement());
  }

  TEST_CASE("View: keys", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><p>text</p>");
    view.UpdateRendering();
    List<std::string> records;
    Ref<boo::dom::EventListenerCallback> recorder = CreateRef<KeyRecorder>(records);
    for (auto type : {u8"keydown", u8"keypress", u8"keyup"})
    {
      view.Document().AddEventListener(boo::dom::DOMStringAtom(type), Ref {recorder}, false);
    }

    // A key that types text: its value is the text, and its release reports the same value.
    CHECK_FALSE(view.KeyPressed(pal::Key::A, {pal::KeyModifier::Shift}, {u8"A", u8"A"}));
    CHECK_FALSE(view.KeyReleased(pal::Key::A));
    CHECK(records
          == List<std::string> {"keydown:A/KeyA/65/0/0+shift", "keypress:A/KeyA/65/65/0+shift",
                                "keyup:A/KeyA/65/0/0"});
    records.clear();

    // A shortcut types nothing, but its value is the key's text without Ctrl.
    (void)view.KeyPressed(pal::Key::A, {pal::KeyModifier::Control}, {u8"", u8"a"});
    // Named keys have their names, even when they type text; modifiers and the keypad have locations.
    (void)view.KeyPressed(pal::Key::Enter, {}, {u8"\r", u8"\r"});
    (void)view.KeyPressed(pal::Key::RightShift, {pal::KeyModifier::Shift});
    (void)view.KeyPressed(pal::Key::Keypad7, {}, {u8"7", u8"7"});
    CHECK(records
          == List<std::string> {"keydown:a/KeyA/65/0/0+ctrl", "keydown:Enter/Enter/13/0/0",
                                "keypress:Enter/Enter/13/13/0", "keydown:Shift/ShiftRight/16/0/2+shift",
                                "keydown:7/Numpad7/103/0/3", "keypress:7/Numpad7/55/55/3"});
    records.clear();

    // The page can cancel a key.
    struct Canceller : boo::dom::EventListenerCallback
    {
      void HandleEvent(boo::dom::Event &event) noexcept override
      {
        event.PreventDefault();
      }
    };
    view.Document().AddEventListener(boo::dom::DOMStringAtom(u8"keydown"), CreateRef<Canceller>(), false);
    CHECK(view.KeyPressed(pal::Key::B, {}, {u8"b", u8"b"}));
    CHECK(records == List<std::string> {"keydown:b/KeyB/66/0/0"});
  }

  TEST_CASE("View: input conversions", "[UI]")
  {
    auto tab = input::Describe(pal::Key::Tab);
    CHECK(tab.Code == u8"Tab");
    CHECK(tab.NamedKey == u8"Tab");
    CHECK(tab.KeyIdentifier == u8"U+0009");
    CHECK(tab.WindowsVirtualKeyCode == boo::vk::Tab);
    CHECK(input::Describe(pal::Key::Num4).Code == u8"Digit4");
    CHECK(input::Describe(pal::Key::Num4).WindowsVirtualKeyCode == '4');
    CHECK(input::Describe(pal::Key::Z).Code == u8"KeyZ");
    CHECK(input::Describe(pal::Key::F12).NamedKey == u8"F12");
    CHECK(input::Describe(pal::Key::F12).WindowsVirtualKeyCode == boo::vk::F1 + 11);
    CHECK(input::Describe(pal::Key::Del).NamedKey == u8"Backspace");
    CHECK(input::Describe(pal::Key::Option).WindowsVirtualKeyCode == boo::vk::LMenu);
    CHECK(input::Describe(pal::Key::KeypadEnter).IsKeypad);

    // A key that types text has a key identifier from it.
    auto a = input::KeyboardEvent(boo::PlatformEvent::Type::KeyDown, pal::Key::A, {}, {u8"a", u8"a"}, false);
    CHECK(a.KeyIdentifier() == u8"U+0041");
    CHECK(a.Key() == u8"a");

    CHECK(input::ToBooey(pal::MouseButton::Thumb1) == boo::MouseButton::Back);
    auto modifiers = input::ToBooey(pal::KeyModifiers {pal::KeyModifier::Alt, pal::KeyModifier::Meta});
    CHECK(modifiers.Contains(boo::PlatformEvent::Modifier::AltKey));
    CHECK(modifiers.Contains(boo::PlatformEvent::Modifier::MetaKey));
    CHECK_FALSE(modifiers.Contains(boo::PlatformEvent::Modifier::ShiftKey));
    CHECK(input::ToCursorShape(boo::Cursor(boo::Cursor::Type::Grab)) == pal::CursorShape::Hand);
  }
}
