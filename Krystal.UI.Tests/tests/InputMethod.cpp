#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/CompositionEvent.hpp"
#include "Krystal.Booey/DOM/InputEvent.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/Loader/Cache/MemoryCache.hpp"
#include "Krystal.Editor/PageTextInput.hpp"
#include "Krystal.Gfx.Tests/Golden.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/View.hpp"
#include <string>

namespace krys::ui::tests
{
  namespace
  {
    KRYS_NODISCARD std::string Narrow(utf8_stringview text) noexcept
    {
      return std::string(reinterpret_cast<const char *>(text.data()), text.size());
    }

    /// @brief Records the key, composition and input events as "type data", in order.
    struct EventLog : boo::dom::EventListenerCallback
    {
      List<std::string> Events;

      void HandleEvent(boo::dom::Event &event) noexcept override
      {
        auto type = Narrow(event.Type().View());
        if (event.IsCompositionEvent())
        {
          Events.push_back(type + " " + Narrow(Downcast<boo::dom::CompositionEvent>(event).Data()));
        }
        else if (event.IsInputEvent())
        {
          auto &input = Downcast<boo::dom::InputEvent>(event);
          Events.push_back(type + " " + Narrow(input.InputType())
                           + (input.IsComposing() ? " composing" : ""));
        }
        else if (event.IsKeyboardEvent())
        {
          auto &key = Downcast<boo::dom::KeyboardEvent>(event);
          Events.push_back(type + " " + std::to_string(key.KeyCode()));
        }
      }
    };
  }

  TEST_CASE("View: the input method", "[UI]")
  {
    View view({400, 300});
    view.LoadHTML(
      u8"<!DOCTYPE html><style>body { margin: 0; font: 16px sans-serif } input { position: absolute;"
      u8" left: 10px; width: 200px; font: inherit }</style><input id=field style='top: 20px'>"
      u8"<input id=password type=password style='top: 60px'><div id=plain tabindex=0>x</div>");
    WaitForLoad(view);
    view.UpdateRendering();

    auto log = CreateRef<EventLog>();
    for (auto type : {boo::EventNames::CompositionStart, boo::EventNames::CompositionUpdate,
                      boo::EventNames::CompositionEnd, boo::EventNames::Input, boo::EventNames::KeyDown,
                      boo::EventNames::KeyPress})
    {
      view.Document().AddEventListener(boo::dom::DOMStringAtom(type),
                                       Ref<boo::dom::EventListenerCallback> {log}, true);
    }
    int32 discarded = 0;
    view.SetCompositionDiscardedHandler([&discarded] { ++discarded; });

    auto &field = Downcast<boo::html::HTMLInputElement>(*view.Document().GetElementById(u8"field"));
    auto &editor = view.Document().GetEditor();
    auto value = [&field]
    {
      return Narrow(field.Value());
    };

    SECTION("The focus decides whether it's on")
    {
      CHECK_FALSE(view.AcceptsTextInput());
      field.Focus();
      CHECK(view.AcceptsTextInput());
      Downcast<boo::html::HTMLElement>(*view.Document().GetElementById(u8"password")).Focus();
      CHECK_FALSE(view.AcceptsTextInput());
      Downcast<boo::html::HTMLElement>(*view.Document().GetElementById(u8"plain")).Focus();
      CHECK_FALSE(view.AcceptsTextInput());
    }

    field.Focus();
    view.UpdateRendering();

    SECTION("Composing and committing")
    {
      // The caret's rect places the input method's windows: in the field.
      auto caret = view.TextInputRect();
      REQUIRE(caret);
      CHECK(caret->X() >= 10);
      CHECK(caret->Y() >= 20);
      CHECK(caret->MaxY() <= 60);

      // The keys the input method takes don't reach the page.
      CHECK(view.KeyPressed(pal::Key::K, {}, {.HandledByInputMethod = true}));
      CHECK(log->Events.empty());

      // A composition comes as a key press with key code 229.
      pal::CompositionClause target {.Start = 0, .End = 3, .Target = true};
      view.SetComposition(u8"か", Span<const pal::CompositionClause>(&target, 1), 3);
      CHECK(value() == "か");
      CHECK(editor.HasComposition());
      CHECK(log->Events
            == List<std::string> {"keydown 229", "compositionstart ", "compositionupdate か",
                                  "input insertCompositionText composing"});
      auto composition = view.TextInputRect();
      REQUIRE(composition);
      CHECK(composition->X() == caret->X());

      log->Events.clear();
      view.SetComposition(u8"かな", {}, 6);
      CHECK(value() == "かな");
      CHECK(log->Events.back() == "input insertCompositionText composing");

      // The result takes its place.
      log->Events.clear();
      view.ConfirmComposition(u8"仮名");
      CHECK(value() == "仮名");
      CHECK_FALSE(editor.HasComposition());
      CHECK(log->Events.front() == "keydown 229");
      CHECK(std::ranges::find(log->Events, "compositionend 仮名") != log->Events.end());

      // Emptied, a composition goes.
      view.SetComposition(u8"x", {}, 1);
      CHECK(value() == "仮名x");
      view.SetComposition(u8"", {}, 0);
      CHECK(value() == "仮名");
      CHECK_FALSE(editor.HasComposition());
      CHECK(discarded == 0);
    }

    SECTION("A result without a composition")
    {
      // As the emoji panel's may come.
      view.ConfirmComposition(u8"😀");
      CHECK(value() == "😀");
      CHECK(log->Events.back() == "compositionend 😀");
    }

    SECTION("Text without a key press")
    {
      CHECK(view.InsertText(u8"😀"));
      CHECK(value() == "😀");
      CHECK(log->Events.front() == "keypress 128512");
    }

    SECTION("The page ends the composition")
    {
      view.SetComposition(u8"ab", {}, 2);

      // Losing the window's focus confirms it.
      view.SetFocused(false);
      CHECK_FALSE(editor.HasComposition());
      CHECK(value() == "ab");
      CHECK(std::ranges::find(log->Events, "compositionend ab") != log->Events.end());
      CHECK(discarded == 1);
      view.SetFocused(true);

      // The selection leaving it cancels it, leaving its text.
      view.SetComposition(u8"cd", {}, 2);
      field.SetSelectionRange(0, 0);
      CHECK_FALSE(editor.HasComposition());
      CHECK(value() == "abcd");
      CHECK(discarded == 2);

      // A press confirms it.
      field.SetSelectionRange(4, 4);
      view.SetComposition(u8"ef", {}, 2);
      view.MousePressed({300, 250}, pal::MouseButton::Left);
      view.MouseReleased({300, 250}, pal::MouseButton::Left);
      CHECK_FALSE(editor.HasComposition());
      CHECK(value() == "abcdef");
      CHECK(discarded == 3);
      CHECK_FALSE(view.AcceptsTextInput());
    }
  }

  TEST_CASE("View: the editor page's input methods", "[UI]")
  {
    boo::MemoryCache::Singleton().EvictResources();
    View view({1'200, 600});
    view.SetNavigationHandler([](const NavigationRequest &) { return NavigationPolicy::Allow; });
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    auto &document = view.Document();
    // As the editor makes it once the page has loaded.
    editor::PageTextInput textInput(document);
    auto log = [&document]
    {
      auto element = document.GetElementById(u8"ime-log");
      REQUIRE(element);
      return Narrow(element->TextContent().value_or(u8""));
    };
    CHECK(log().starts_with("Type into a field"));

    auto focus = [&document, &view](utf8_stringview id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      Downcast<boo::html::HTMLElement>(*element).Focus();
      CHECK(view.AcceptsTextInput());
    };

    focus(u8"ime-input");
    view.SetComposition(u8"か", {}, 3);
    view.ConfirmComposition(u8"仮");
    CHECK(log()
          == "input: compositionstart “”\n"
             "input: compositionupdate “か”\n"
             "input: beforeinput insertCompositionText “か” (composing)\n"
             "input: input insertCompositionText “か” (composing)\n"
             "input: beforeinput deleteCompositionText (composing)\n"
             "input: input deleteCompositionText (composing)\n"
             "input: beforeinput insertFromComposition “仮” (composing)\n"
             "input: input insertFromComposition “仮” (composing)\n"
             "input: compositionend “仮”");

    // The other fields log too, and the log keeps the latest events.
    focus(u8"ime-textarea");
    (void)view.InsertText(u8"😀");
    CHECK(log().ends_with("textarea: input insertText “😀”"));
    focus(u8"ime-editable");
    for (int i = 0; i < 4; ++i)
    {
      view.SetComposition(u8"x", {}, 1);
      view.SetComposition(u8"", {}, 0);
    }
    auto lines = log();
    CHECK(std::ranges::count(lines, '\n') + 1 == editor::PageTextInput::MaxLines);
    CHECK(lines.ends_with("div: compositionend “”"));

    auto clear = document.GetElementById(u8"ime-clear");
    REQUIRE(clear);
    Downcast<boo::html::HTMLElement>(*clear).Click();
    CHECK(log().starts_with("Type into a field"));
  }
}
