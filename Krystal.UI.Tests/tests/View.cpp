#include "Krystal.UI/View.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/Markup.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/Platform/PasteboardStrategy.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Platform/WindowsKeyboardCodes.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Gfx.Tests/Golden.hpp"
#include "Krystal.UI/FileLoaderStrategy.hpp"
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

  TEST_CASE("View: frees a page's document when it loads another, or goes", "[UI]")
  {
    // A page with style, layout, composited layers, a running animation, a transition and a hovered element:
    // all of it goes with the document.
    const utf8_string page =
      u8"<!DOCTYPE html><style>body { margin: 0 } #box { width: 40px; height: 40px; background: teal;"
      u8" transition: background 1s } #box:hover { background: red }"
      u8"#blurred { width: 40px; height: 20px; filter: blur(2px); background: navy;"
      u8" animation: spin 1s infinite } @keyframes spin { to { transform: rotate(90deg) } }</style>"
      u8"<div id=box></div><div id=blurred></div><p>Some <b>text</b></p>";
    auto time = boo::html::EventLoop::Current().Now();
    auto run = [&](View &view)
    {
      for (int32 i = 0; i < 3; ++i)
      {
        time += 16ms;
        (void)view.Update(time);
      }
    };

    const auto animationsBefore = boo::WebAnimation::Instances().size();
    View view({200, 100});
    view.LoadHTML(utf8_string(page));
    run(view);
    view.MouseMoved({10, 10});
    run(view);
    auto first = CreateWeakPtr<boo::dom::Document>(&view.Document());
    auto box = CreateWeakPtr<boo::dom::Node>(view.Document().GetElementById(u8"box").get());
    REQUIRE(first);

    view.LoadHTML(u8"<!DOCTYPE html><p>Another page</p>");
    run(view);
    CHECK_FALSE(first);
    CHECK_FALSE(box);
    // Its animations and transitions went with it.
    CHECK(boo::WebAnimation::Instances().size() == animationsBefore);

    // A page loaded from files: its style sheets, fonts and images go too.
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    for (int32 i = 0; i < 10; ++i)
    {
      run(view);
    }
    auto editor = CreateWeakPtr<boo::dom::Document>(&view.Document());
    view.LoadHTML(u8"<!DOCTYPE html><p>Another page</p>");
    run(view);
    CHECK_FALSE(editor);

    // Loading it again reuses the cached resources the freed document's elements were clients of.
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    for (int32 i = 0; i < 20; ++i)
    {
      time += 50ms;
      (void)view.Update(time);
    }
    CHECK(view.Document().GetElementById(u8"nonexistent") == nullptr);

    WeakPtr<boo::dom::Document> second;
    {
      View other({200, 100});
      other.LoadHTML(utf8_string(page));
      run(other);
      second = CreateWeakPtr<boo::dom::Document>(&other.Document());
      REQUIRE(second);
    }
    CHECK_FALSE(second);
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
    // The view's base colour, the root's background over the canvas and the box's background, clipped to the
    // visible contents.
    REQUIRE(displayList->Items().size() == 8u);
    CHECK(displayList->AsText({}).starts_with(u8R"(
(save)
(clip
  (rect at (0,0) size 200x100)))"));
    CHECK(displayList->AsText({}).ends_with(u8R"((fill-composited-rect
  (rect at (0,0) size 40x20)
  (color #FF0000)
  (composite-operation source-over)
  (blend-mode normal))
(restore))"));
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

  TEST_CASE("View: a click that doesn't move doesn't select", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }</style><div></div>");
    view.UpdateRendering();

    view.MouseMoved({10, 10});
    CHECK(view.CurrentCursor() == pal::CursorShape::Arrow);

    // Windows reports a move with each press and release, at the same place.
    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.MouseMoved({10, 10});
    view.UpdateRendering();
    CHECK(view.CurrentCursor() == pal::CursorShape::Arrow);
    view.MouseReleased({10, 10}, pal::MouseButton::Left);
    view.MouseMoved({10, 10});
    CHECK(view.CurrentCursor() == pal::CursorShape::Arrow);
    CHECK_FALSE(view.Document().Selection().IsRange());
  }

  TEST_CASE("View: dragging a selection shows the I-beam only over text", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif } div { height: 50px }"
                  u8"</style><div></div><div>text</div><div></div>");
    view.UpdateRendering();

    view.MouseMoved({10, 10});
    view.MousePressed({10, 10}, pal::MouseButton::Left);
    view.MouseMoved({10, 120});
    CHECK(view.CurrentCursor() == pal::CursorShape::Arrow);
    view.MouseMoved({3, 60});
    CHECK(view.CurrentCursor() == pal::CursorShape::IBeam);
    view.MouseMoved({300, 60});
    CHECK(view.CurrentCursor() == pal::CursorShape::Arrow);
    view.MouseReleased({300, 60}, pal::MouseButton::Left);
    CHECK(view.Document().Selection().IsRange());
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

  TEST_CASE("View: the keyboard selects and copies", "[UI]")
  {
    /// @brief Records what's written to the clipboard, in place of the system's.
    struct RecordingPasteboard final : boo::PasteboardStrategy
    {
      List<utf8_string> Writes;

      void WriteToPasteboard(utf8_stringview, utf8_stringview text) noexcept override
      {
        Writes.emplace_back(text);
      }

      KRYS_NODISCARD utf8_string ReadStringFromPasteboard(utf8_stringview) noexcept override
      {
        return Writes.empty() ? utf8_string() : Writes.back();
      }
    };

    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif }</style>"
                  u8"<p id=a>The quick brown fox</p>");
    view.UpdateRendering();

    auto &strategies = boo::PlatformStrategies::Singleton();
    auto *systemPasteboard = strategies.GetPasteboardStrategy();
    RecordingPasteboard pasteboard;
    strategies.SetPasteboardStrategy(&pasteboard);

    auto &selection = view.Document().Selection();
    auto *text = view.Document().GetElementById(u8"a")->FirstChild();

    // Shift with the arrows extends a selection; without Shift, outside editable text, the arrows scroll.
    selection.MoveTo(boo::dom::MakeContainerOffsetPosition(ShareRefPtr(text), 4),
                     boo::dom::MakeContainerOffsetPosition(ShareRefPtr(text), 9), boo::Affinity::Downstream);
    CHECK(view.KeyPressed(pal::Key::RightArrow, {pal::KeyModifier::Shift}));
    CHECK(selection.Selection().End().DeprecatedEditingOffset() == 10);
    CHECK(view.KeyPressed(pal::Key::RightArrow, {pal::KeyModifier::Shift, pal::KeyModifier::Control}));
    CHECK(selection.Selection().End().DeprecatedEditingOffset() == 16);
    CHECK(view.KeyPressed(pal::Key::Home, {pal::KeyModifier::Shift}));
    CHECK(selection.Selection().Start().DeprecatedEditingOffset() == 0);

    // Ctrl+C copies the selected text.
    selection.MoveTo(boo::dom::MakeContainerOffsetPosition(ShareRefPtr(text), 4),
                     boo::dom::MakeContainerOffsetPosition(ShareRefPtr(text), 15), boo::Affinity::Downstream);
    CHECK(view.KeyPressed(pal::Key::C, {pal::KeyModifier::Control}, {u8"", u8"c"}));
    CHECK(pasteboard.Writes == List<utf8_string> {u8"quick brown"});

    // Ctrl+A selects everything.
    CHECK(view.KeyPressed(pal::Key::A, {pal::KeyModifier::Control}, {u8"", u8"a"}));
    CHECK(selection.IsRange());
    CHECK(view.Document().GetEditor().SelectedText() == u8"The quick brown fox");

    strategies.SetPasteboardStrategy(systemPasteboard);
  }

  TEST_CASE("View: the keyboard edits editable text", "[UI]")
  {
    /// @brief A clipboard in memory, in place of the system's.
    struct MemoryPasteboard final : boo::PasteboardStrategy
    {
      utf8_string Contents;

      void WriteToPasteboard(utf8_stringview, utf8_stringview text) noexcept override
      {
        Contents = utf8_string(text);
      }

      KRYS_NODISCARD utf8_string ReadStringFromPasteboard(utf8_stringview) noexcept override
      {
        return Contents;
      }
    };

    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif }</style>"
                  u8"<div id=e contenteditable>abc</div>");
    view.UpdateRendering();

    auto &strategies = boo::PlatformStrategies::Singleton();
    auto *systemPasteboard = strategies.GetPasteboardStrategy();
    MemoryPasteboard pasteboard;
    strategies.SetPasteboardStrategy(&pasteboard);

    auto &root = *view.Document().GetElementById(u8"e");
    auto markup = [&]
    {
      auto html = boo::SerializeFragment(root, boo::SerializedNodes::SubtreesOfChildren);
      return std::string(reinterpret_cast<const char *>(html.data()), html.size());
    };
    auto caretAt = [&](uint32 offset)
    {
      view.Document().Selection().MoveTo(
        boo::dom::MakeContainerOffsetPosition(ShareRefPtr(root.FirstChild()), offset),
        boo::Affinity::Downstream);
    };

    // Typed characters are inserted, Backspace deletes, and Enter starts a paragraph.
    caretAt(3);
    (void)view.KeyPressed(pal::Key::D, {}, {u8"d", u8"d"});
    CHECK(markup() == "abcd");
    CHECK(view.KeyPressed(pal::Key::Del));
    CHECK(markup() == "abc");
    (void)view.KeyPressed(pal::Key::Enter, {}, {u8"\r", u8"\r"});
    CHECK(markup() == "abc<div><br></div>");

    // Ctrl+Z undoes the paragraph, and Ctrl+Shift+Z redoes it.
    CHECK(view.KeyPressed(pal::Key::Z, {pal::KeyModifier::Control}, {u8"", u8"z"}));
    CHECK(markup() == "abc");
    CHECK(view.KeyPressed(pal::Key::Z, {pal::KeyModifier::Control, pal::KeyModifier::Shift}, {u8"", u8"Z"}));
    CHECK(markup() == "abc<div><br></div>");
    CHECK(view.KeyPressed(pal::Key::Z, {pal::KeyModifier::Control}, {u8"", u8"z"}));

    // Ctrl+X cuts, and Ctrl+V pastes.
    view.Document().Selection().MoveTo(
      boo::dom::MakeContainerOffsetPosition(ShareRefPtr(root.FirstChild()), 0),
      boo::dom::MakeContainerOffsetPosition(ShareRefPtr(root.FirstChild()), 1), boo::Affinity::Downstream);
    CHECK(view.KeyPressed(pal::Key::X, {pal::KeyModifier::Control}, {u8"", u8"x"}));
    CHECK(pasteboard.Contents == u8"a");
    CHECK(markup() == "bc");
    caretAt(2);
    CHECK(view.KeyPressed(pal::Key::V, {pal::KeyModifier::Control}, {u8"", u8"v"}));
    CHECK(markup() == "bca");

    strategies.SetPasteboardStrategy(systemPasteboard);
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

  TEST_CASE("View: Tab moves the focus", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(u8"<!DOCTYPE html><button id=a></button><button id=b></button>");
    view.UpdateRendering();
    auto a = view.Document().GetElementById(u8"a");
    auto b = view.Document().GetElementById(u8"b");

    CHECK(view.KeyPressed(pal::Key::Tab));
    CHECK(view.Document().FocusedElement() == a.get());
    CHECK(a->HasFocusVisible());
    CHECK(view.KeyPressed(pal::Key::Tab));
    CHECK(view.Document().FocusedElement() == b.get());
    CHECK(view.KeyPressed(pal::Key::Tab, {pal::KeyModifier::Shift}));
    CHECK(view.Document().FocusedElement() == a.get());
    CHECK_FALSE(view.KeyReleased(pal::Key::Tab));
  }

  TEST_CASE("View: scrolling", "[UI]")
  {
    View view({200, 100});
    view.LoadHTML(u8"<!DOCTYPE html><style>body { margin: 0 }</style><div style='height: 500px'></div>");
    view.UpdateRendering();
    struct ScrollCounter : boo::dom::EventListenerCallback
    {
      int32 Count {0};

      void HandleEvent(boo::dom::Event &) noexcept override
      {
        ++Count;
      }
    };
    Ref<ScrollCounter> counter = CreateRef<ScrollCounter>();
    view.Document().AddEventListener(boo::dom::DOMStringAtom(u8"scroll"),
                                     Ref<boo::dom::EventListenerCallback> {counter}, false);

    CHECK(view.ScrollPosition() == boo::IntPoint());
    view.ScrollTo({0, 150});
    CHECK(view.ScrollPosition() == boo::IntPoint(0, 150));
    // Clamped to the document, and `scroll` fires at the next rendering update.
    view.ScrollTo({0, 1'000});
    CHECK(view.ScrollPosition() == boo::IntPoint(0, 400));
    CHECK(counter->Count == 0);
    view.UpdateRendering();
    CHECK(counter->Count == 1);

    // The recording is scrolled.
    CHECK(view.Record()->AsText({}).contains(u8R"((translate
  (x 0.00)
  (y -400.00)))"));
  }

  TEST_CASE("View: the wheel scrolls", "[UI]")
  {
    View view({200, 100});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0 }</style><div style='width: 400px; height: 500px'></div>");
    view.UpdateRendering();
    pal::WheelScrollSettings threeLines {.Lines = 3u, .Chars = 3};

    // A notch towards the user is three lines down, at 100 pixels per three lines.
    CHECK(view.MouseWheel({50, 50}, 0, -1, {}, threeLines));
    CHECK(view.ScrollPosition() == boo::IntPoint(0, 100));
    // Fractions of a notch, from touchpads.
    CHECK(view.MouseWheel({50, 50}, 0, 0.5f, {}, threeLines));
    CHECK(view.ScrollPosition() == boo::IntPoint(0, 50));

    // The horizontal wheel, and the vertical one with Shift, scroll sideways.
    CHECK(view.MouseWheel({50, 50}, 0.5f, 0, {}, threeLines));
    CHECK(view.ScrollPosition() == boo::IntPoint(50, 50));
    CHECK(view.MouseWheel({50, 50}, 0, -0.5f, pal::KeyModifiers {pal::KeyModifier::Shift}, threeLines));
    CHECK(view.ScrollPosition() == boo::IntPoint(100, 50));

    // A page per notch: four fifths of the 79 pixels in view, less the horizontal scrollbar.
    CHECK(view.MouseWheel({50, 50}, 0, -1, {}, {.Lines = null, .Chars = 3}));
    CHECK(view.ScrollPosition() == boo::IntPoint(100, 113));

    // No turn, no event.
    CHECK_FALSE(view.MouseWheel({50, 50}, 0, 0, {}, threeLines));
  }

  TEST_CASE("View: Update runs the event loop with the host's time", "[UI]")
  {
    using namespace std::chrono_literals;
    View view({200, 100});
    view.LoadHTML(u8"<!DOCTYPE html><div id=box style='width: 10px; height: 10px'></div>");
    auto start = boo::html::EventLoop::Current().Now() + 1s;
    view.Update(start);

    // Style and layout run on their timers.
    auto box = view.Document().GetElementById(u8"box");
    REQUIRE_FALSE(box->SetAttribute(u8"style", u8"width: 20px; height: 10px").HasException());
    CHECK(view.Document().HasPendingStyleRecalc());
    view.Update(start);
    CHECK_FALSE(view.Document().HasPendingStyleRecalc());

    // Timers follow the time the host passes.
    int32 fired = 0;
    boo::Timer timer([&] { ++fired; });
    timer.StartOneShot(100ms);
    view.Update(start + 99ms);
    CHECK(fired == 0);
    view.Update(start + 100ms);
    CHECK(fired == 1);

    // Input is followed by a microtask checkpoint.
    struct MicrotaskQueuer : boo::dom::EventListenerCallback
    {
      int32 &Ran;

      explicit MicrotaskQueuer(int32 &ran) noexcept : Ran(ran)
      {
      }

      void HandleEvent(boo::dom::Event &) noexcept override
      {
        boo::html::EventLoop::Current().QueueMicrotask([this] { ++Ran; });
      }
    };
    int32 ran = 0;
    view.Document().AddEventListener(boo::dom::DOMStringAtom(u8"mousedown"),
                                     Ref<boo::dom::EventListenerCallback> {CreateRef<MicrotaskQueuer>(ran)},
                                     false);
    view.MousePressed({5, 5}, pal::MouseButton::Left);
    CHECK(ran == 1);
    view.MouseReleased({5, 5}, pal::MouseButton::Left);
  }

  TEST_CASE("View: Update runs the rendering update when the page changes", "[UI]")
  {
    using namespace std::chrono_literals;
    View view({200, 100});
    view.LoadHTML(
      u8"<!DOCTYPE html><div id=box autofocus tabindex=0 style='width: 10px; height: 10px'></div>");
    auto now = boo::html::EventLoop::Current().Now() + 1s;

    // The new document needs one, which focuses the autofocus candidate.
    CHECK(view.Update(now));
    auto box = view.Document().GetElementById(u8"box");
    CHECK(view.Document().FocusedElement() == box.get());

    // Nothing changed, so nothing to update.
    CHECK_FALSE(view.Update(now));

    REQUIRE_FALSE(box->SetAttribute(u8"style", u8"width: 20px; height: 10px").HasException());
    CHECK(view.Update(now));
    CHECK_FALSE(view.Update(now));
  }

  TEST_CASE("View: loads file images relative to the page's file, in CSS and <img>", "[UI]")
  {
    const auto directory = std::filesystem::temp_directory_path() / "krystal-ui-image-test";
    std::filesystem::create_directories(directory / "images");
    {
      // A 2x1 24-bit BMP: red, then green.
      const unsigned char bmp[] = {
        // File header: the signature, the file's size and the offset of the pixels.
        'B', 'M', 62, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0,
        // Info header: its size, 2x1 pixels, one plane, 24 bits a pixel, no compression, 8 bytes of pixels.
        40, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 1, 0, 24, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0,
        // The row, blue-green-red, padded to 4 bytes.
        0, 0, 255, 0, 255, 0, 0, 0};
      std::ofstream image(directory / "images" / "pair.bmp", std::ios::binary);
      image.write(reinterpret_cast<const char *>(bmp), sizeof(bmp));
      std::ofstream page(directory / "page.html", std::ios::binary);
      page << "<!DOCTYPE html><style>body { margin: 0 }</style>"
              "<div id=box style='width: 40px; height: 20px; background: url(images/pair.bmp)'></div>"
              "<img id=img src=images/pair.bmp>";
    }

    View view({200, 100});
    REQUIRE(view.LoadFile(directory / "page.html").has_value());
    CHECK(view.Document().GetURL().ProtocolIsFile());
    view.UpdateRendering();
    CHECK(view.Record()->AsText({}).find(u8"native-image") == utf8_string::npos);

    // The load completes as a networking task, which asks for a rendering update.
    auto now = boo::html::EventLoop::Current().Now();
    CHECK(view.Update(now));
    CHECK(view.Record()->AsText({}).find(u8"draw-pattern-native-image") != utf8_string::npos);
    // The <img> loads the same file, from the memory cache.
    CHECK(view.Record()->AsText({}).find(u8"(draw-native-image") != utf8_string::npos);
    auto &image = Downcast<boo::html::HTMLImageElement>(*view.Document().GetElementById(u8"img"));
    CHECK(image.Complete());
    CHECK(image.NaturalWidth() == 2u);

    std::filesystem::remove_all(directory);
  }

  TEST_CASE("View: loads a file page's linked and imported sheets, and reloads them", "[UI]")
  {
    const auto directory = std::filesystem::temp_directory_path() / "krystal-ui-sheet-test";
    std::filesystem::create_directories(directory / "css");
    auto write = [&](const std::filesystem::path &path, std::string_view text)
    {
      std::ofstream file(directory / path, std::ios::binary);
      file << text;
    };
    write("page.html", "<!DOCTYPE html><link rel=stylesheet href=css/page.css><div id=box></div>");
    write("css/page.css", "@import url(sizes.css); #box { height: 10px }");
    write("css/sizes.css", "#box { width: 30px }");

    View view({200, 100});
    REQUIRE(view.LoadFile(directory / "page.html").has_value());
    (void)view.Update(boo::html::EventLoop::Current().Now());
    CHECK(WidthOf(view, u8"box") == 30);

    // Loading the page again uses the cached sheets; revalidating loads the edited one.
    write("css/sizes.css", "#box { width: 50px }");
    REQUIRE(view.LoadFile(directory / "page.html").has_value());
    (void)view.Update(boo::html::EventLoop::Current().Now());
    CHECK(WidthOf(view, u8"box") == 30);
    REQUIRE(view.LoadFile(directory / "page.html", boo::CachePolicy::Revalidate).has_value());
    (void)view.Update(boo::html::EventLoop::Current().Now());
    CHECK(WidthOf(view, u8"box") == 50);

    std::filesystem::remove_all(directory);
  }

  TEST_CASE("FileLoaderStrategy: MIME types by extension", "[UI]")
  {
    CHECK(FileLoaderStrategy::MimeTypeForExtension(u8".PNG") == u8"image/png");
    CHECK(FileLoaderStrategy::MimeTypeForExtension(u8".jpeg") == u8"image/jpeg");
    CHECK(FileLoaderStrategy::MimeTypeForExtension(u8".webp") == u8"image/webp");
    CHECK(FileLoaderStrategy::MimeTypeForExtension(u8".woff2") == u8"font/woff2");
    CHECK(FileLoaderStrategy::MimeTypeForExtension(u8".TTF") == u8"font/ttf");
    CHECK(FileLoaderStrategy::MimeTypeForExtension(u8".unknown").empty());
  }

  TEST_CASE("View: animations run on the host's frames", "[UI]")
  {
    using namespace std::chrono_literals;
    // As a host's page: its style sheet loads from a file, as a networking task.
    const auto directory = std::filesystem::temp_directory_path() / "krystal-ui-animation-test";
    std::filesystem::create_directories(directory);
    {
      std::ofstream page(directory / "page.html", std::ios::binary);
      page << "<!DOCTYPE html><link rel=stylesheet href=page.css><div id=box></div>";
      std::ofstream sheet(directory / "page.css", std::ios::binary);
      sheet << "@keyframes fade { from { opacity: 0 } to { opacity: 1 } }"
               "#box { width: 10px; height: 10px; animation: fade 1s linear infinite }";
    }
    View view({200, 100});
    REQUIRE(view.LoadFile(directory / "page.html").has_value());
    // The animation runs on the box's layer, so its value is the renderer's animated style.
    auto opacity = [&]
    {
      auto box = view.Document().GetElementById(u8"box");
      REQUIRE(box);
      REQUIRE(box->Renderer());
      return box->Renderer()->AnimatedStyle()->GetOpacity().Value.Value;
    };

    // Frames at 60 Hz, longer than the timelines controller keeps its cached time: the animation still gets
    // its start time and runs.
    constexpr auto frame =
      std::chrono::duration_cast<MonotonicTime::duration>(std::chrono::microseconds {16'667});
    auto time = boo::html::EventLoop::Current().Now() + 1s;
    for (int32 i = 0; i < 4; ++i)
    {
      time += frame;
      (void)view.Update(time);
    }
    auto &animation = *view.Document().GetElementById(u8"box")->Animations(null)->first();
    CHECK_FALSE(animation.Pending());
    auto before = opacity();
    for (int32 i = 0; i < 15; ++i)
    {
      time += frame;
      // The page's style doesn't change while the animation runs on its layer.
      CHECK_FALSE(view.Update(time));
    }
    CHECK(opacity() == Catch::Approx(before + 0.25f).margin(0.02f));
    std::filesystem::remove_all(directory);
  }
}
