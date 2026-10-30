#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorCheckerTestFunctions.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Rendering/RenderTextControlSingleLine.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Runs the tasks the event loop has queued (the `select` and `selectionchange` events).
    void RunQueuedTasks() noexcept
    {
      html::EventLoop::Current().RunUntilIdle();
    }

    /// @brief Presses the key @p key (with the key identifier @p identifier) in the frame, as the host does.
    bool PressKey(EditorFixture &fixture, utf8_string key, utf8_string identifier, int32 keyCode) noexcept
    {
      return fixture.Frame->GetEventHandler().KeyEvent(
        PlatformKeyboardEvent(PlatformEvent::Type::RawKeyDown, u8"", u8"", key, key, krys::move(identifier),
                              keyCode, false, false, false, {}, {}));
    }

    KRYS_NODISCARD html::HTMLInputElement &InputById(EditorFixture &fixture, utf8_stringview id) noexcept
    {
      return Downcast<html::HTMLInputElement>(fixture.ById(id));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLTextFormControlElement: shadow trees",
                   "[HTML][HTMLTextFormControlElement]")
  {
    EditorFixture fixture(u8"<input id=field value=ab placeholder=Name><input id=search type=search>"
                          u8"<input id=box type=checkbox><textarea id=area placeholder=Notes>xy</textarea>");
    auto &field = InputById(fixture, u8"field");

    SECTION("A text field's user agent shadow tree holds its value in an editable inner text element")
    {
      auto *root = field.UserAgentShadowRoot();
      REQUIRE(root);
      CHECK(root->Mode() == dom::ShadowRootMode::UserAgent);
      // Pages can't see it.
      CHECK_FALSE(field.ShadowRoot());
      auto innerText = field.InnerTextElement();
      REQUIRE(innerText);
      CHECK(innerText->ParentNode() == root);
      CHECK(Narrow(innerText->TextContent().value_or(u8"")) == "ab");
      CHECK(innerText->GetAttribute(u8"contenteditable") == u8"plaintext-only");
      CHECK(Narrow(field.InnerTextValue()) == "ab");
    }

    SECTION("The placeholder is a ::placeholder part before the inner text, shown while the field is empty")
    {
      auto *placeholder = field.PlaceholderElement();
      REQUIRE(placeholder);
      CHECK(placeholder->NextSibling() == field.InnerTextElement().get());
      CHECK(placeholder->UserAgentPart() == css::UserAgentParts::Placeholder());
      CHECK_FALSE(field.IsPlaceholderVisible());
      CHECK_FALSE(placeholder->Renderer());

      (void)field.SetValue({});
      (void)fixture.Document->UpdateLayout();
      CHECK(field.IsPlaceholderVisible());
      CHECK(css::IsPlaceholderShown(field));
      CHECK(placeholder->Renderer());
      CHECK(Narrow(placeholder->TextContent().value_or(u8"")) == "Name");
    }

    SECTION("Changing the value, the placeholder and the type update the shadow tree")
    {
      (void)field.SetValue(u8"xyz");
      CHECK(Narrow(field.InnerTextElement()->TextContent().value_or(u8"")) == "xyz");

      (void)field.SetAttribute(u8"placeholder", dom::DOMString(u8"Full name"));
      CHECK(Narrow(field.PlaceholderElement()->TextContent().value_or(u8"")) == "Full name");
      field.RemoveAttribute(u8"placeholder");
      CHECK_FALSE(field.PlaceholderElement());

      field.Type(dom::DOMString(u8"checkbox"));
      CHECK_FALSE(field.InnerTextElement());
      CHECK_FALSE(field.UserAgentShadowRoot()->HasChildNodes());
      field.Type(dom::DOMString(u8"text"));
      CHECK(field.InnerTextElement());
    }

    SECTION("Search fields have a decoration and a cancel button in a flex container")
    {
      auto &search = InputById(fixture, u8"search");
      auto *container = search.ContainerElement();
      REQUIRE(container);
      CHECK(container->UserAgentPart() == css::UserAgentParts::WebkitTextfieldDecorationContainer());
      REQUIRE(search.ResultsButtonElement());
      CHECK(search.ResultsButtonElement()->UserAgentPart() == css::UserAgentParts::WebkitSearchDecoration());
      REQUIRE(search.CancelButtonElement());
      CHECK(search.CancelButtonElement()->UserAgentPart() == css::UserAgentParts::WebkitSearchCancelButton());
      CHECK(search.InnerBlockElement()->FirstChild() == search.InnerTextElement().get());
      CHECK((search.Renderer() && search.Renderer()->IsRenderTextControlSingleLine()));
      CHECK((container->Renderer() && container->Renderer()->IsRenderTextControlInnerContainer()));
    }

    SECTION("Checkboxes have no shadow tree")
    {
      CHECK_FALSE(InputById(fixture, u8"box").UserAgentShadowRoot());
    }

    SECTION("A text area's shadow tree holds its inner text, then its placeholder")
    {
      auto &area = Downcast<html::HTMLTextAreaElement>(fixture.ById(u8"area"));
      auto innerText = area.InnerTextElement();
      REQUIRE(innerText);
      CHECK(Narrow(area.InnerTextValue()) == "xy");
      CHECK(innerText->NextSibling() == area.PlaceholderElement());
      CHECK((area.Renderer() && area.Renderer()->IsRenderTextControlMultiLine()));
      CHECK((innerText->Renderer() && innerText->Renderer()->IsRenderTextControlInnerBlock()));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLTextFormControlElement: styling the parts",
                   "[HTML][HTMLTextFormControlElement]")
  {
    EditorFixture fixture(u8"<style>input::placeholder { color: rgb(0, 0, 255) }"
                          u8"input::-webkit-search-cancel-button { color: rgb(255, 0, 0) }</style>"
                          u8"<input id=field placeholder=Name><input id=search type=search value=x>");
    auto &field = InputById(fixture, u8"field");
    auto &search = InputById(fixture, u8"search");
    (void)fixture.Document->UpdateLayout();

    // A page styles the standard parts.
    CHECK(field.PlaceholderElement()->GetRenderStyle()->GetColor() == Color(Color::blue));

    // The internal parts are named only in the user agent sheet: a page's rule naming one isn't valid.
    CHECK(search.CancelButtonElement()->GetRenderStyle()->GetColor() != Color(Color::red));
    CHECK_FALSE(
      css::SelectorParser::ParseSelectorList(u8"input::-webkit-search-cancel-button", {}).has_value());
    css::SelectorParserContext userAgentContext;
    userAgentContext.Mode = css::ParserMode::UASheet;
    CHECK(css::SelectorParser::ParseSelectorList(u8"input::-webkit-search-cancel-button", userAgentContext)
            .has_value());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLTextFormControlElement: typing",
                   "[HTML][HTMLTextFormControlElement]")
  {
    EditorFixture fixture(
      u8"<form id=form action=https://example.com/>"
      u8"<input id=field value=ab><input id=short maxlength=3><input id=number type=number>"
      u8"<textarea id=area>xy</textarea><button id=go>Go</button></form>");
    auto &editor = fixture.GetEditor();
    auto &field = InputById(fixture, u8"field");
    auto inputs = fixture.Listen(EventNames::Input);
    auto changes = fixture.Listen(EventNames::Change);

    SECTION("Focusing a field selects its value; typing replaces it and fires input")
    {
      field.Focus();
      CHECK(field.SelectionStart() == 0);
      CHECK(field.SelectionEnd() == 2);
      CHECK(editor.InsertText(u8"cd", nullptr));
      CHECK(Narrow(field.Value()) == "cd");
      CHECK(field.SelectionStart() == 2);
      CHECK(inputs->Count == 1);
      CHECK(field.LastChangeWasUserEdit());
    }

    SECTION("Typed text goes in at the caret; the value is dirty and change fires on blur")
    {
      field.Focus();
      field.SetSelectionRange(2, 2);
      CHECK(editor.InsertText(u8"c", nullptr));
      CHECK(editor.InsertText(u8"d", nullptr));
      CHECK(Narrow(field.Value()) == "abcd");
      CHECK(field.HasDirtyValue());
      CHECK(changes->Count == 0);

      field.Blur();
      CHECK(changes->Count == 1);
    }

    SECTION("Deleting backward removes the character before the caret")
    {
      field.Focus();
      field.SetSelectionRange(2, 2);
      fixture.Client->TextControlNotifications.clear();
      CHECK(fixture.Run(u8"DeleteBackward"));
      CHECK(Narrow(field.Value()) == "a");

      // The host hears of the deletion before it happens, and of the change after.
      CHECK(fixture.Client->TextControlNotifications == List<std::string> {"will delete", "field changed"});
    }

    SECTION("Undo and redo restore the value")
    {
      field.Focus();
      field.SetSelectionRange(2, 2);
      (void)editor.InsertText(u8"cd", nullptr);
      CHECK(Narrow(field.Value()) == "abcd");
      editor.Undo();
      CHECK(Narrow(field.Value()) == "ab");
      editor.Redo();
      CHECK(Narrow(field.Value()) == "abcd");
    }

    SECTION("A field's line breaks become spaces, and a trailing one is dropped")
    {
      field.Focus();
      field.SetSelectionRange(2, 2);
      (void)editor.InsertText(u8"c\nd\n", nullptr);
      CHECK(Narrow(field.Value()) == "abc d");
    }

    SECTION("Typing stops at the maximum length")
    {
      auto &shortField = InputById(fixture, u8"short");
      shortField.Focus();
      (void)editor.InsertText(u8"abcdef", nullptr);
      CHECK(Narrow(shortField.Value()) == "abc");
    }

    SECTION("A number field only takes what a number can have, and its arrow keys step it")
    {
      auto &number = InputById(fixture, u8"number");
      number.Focus();
      (void)editor.InsertText(u8"1x2", nullptr);
      CHECK(Narrow(number.Value()) == "12");
      (void)editor.InsertText(u8"-", nullptr);
      CHECK(Narrow(number.Value()) == "12");

      CHECK(PressKey(fixture, u8"ArrowUp", u8"Up", vk::Up));
      CHECK(Narrow(number.Value()) == "13");
    }

    SECTION("A text area keeps line breaks")
    {
      auto &area = Downcast<html::HTMLTextAreaElement>(fixture.ById(u8"area"));
      area.Focus();
      CHECK(area.SelectionStart() == 0);
      area.SetSelectionRange(2, 2);
      CHECK(fixture.Run(u8"InsertNewline"));
      (void)editor.InsertText(u8"z", nullptr);
      CHECK(Narrow(area.Value()) == "xy\nz");
      CHECK(inputs->Count == 2);
    }

    SECTION("Enter in a field submits its form implicitly, with its default button")
    {
      field.Focus();
      auto submits = fixture.Listen(u8"submit");
      auto clicks = fixture.Listen(EventNames::Click);
      CHECK(fixture.Run(u8"InsertNewline"));
      CHECK(submits->Count == 1);
      CHECK(clicks->Count == 1);
      CHECK(Narrow(field.Value()) == "ab");
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLTextFormControlElement: the selection APIs",
                   "[HTML][HTMLTextFormControlElement]")
  {
    EditorFixture fixture(u8"<input id=field value=hello><input id=other><input id=box type=checkbox>");
    auto &field = InputById(fixture, u8"field");
    auto selects = fixture.Listen(u8"select");
    auto selectionChanges = fixture.Listen(EventNames::SelectionChange);

    SECTION("An unfocused field caches its selection")
    {
      field.SetSelectionRange(1, 3, u8"backward");
      CHECK(field.SelectionStart() == 1);
      CHECK(field.SelectionEnd() == 3);
      CHECK(Narrow(field.SelectionDirection()) == "backward");
      CHECK(Narrow(field.SelectedText()) == "el");
      // The frame's selection is elsewhere.
      CHECK(fixture.Selection().Selection().IsNone());

      // Focusing restores it.
      field.Focus();
      CHECK(field.SelectionStart() == 1);
      CHECK(field.SelectionEnd() == 3);
      CHECK(html::EnclosingTextFormControl(fixture.Selection().Selection().Start()) == &field);
    }

    SECTION("A focused field's selection is the frame's")
    {
      field.Focus();
      field.SetSelectionRange(0, 2);
      auto range = fixture.Selection().Selection().ToNormalizedRange();
      REQUIRE(range);
      CHECK(range->StartOffset() == 0);
      CHECK(range->EndOffset() == 2);
      CHECK(&range->StartContainer() == field.InnerTextElement()->FirstChild());
    }

    SECTION("Offsets are clamped to the value")
    {
      field.SetSelectionRange(3, 99);
      CHECK(field.SelectionStart() == 3);
      CHECK(field.SelectionEnd() == 5);
      field.SetSelectionStart(9);
      CHECK(field.SelectionStart() == 5);
    }

    SECTION("select() selects everything, and fires select and selectionchange at the field")
    {
      field.Focus();
      field.SetSelectionRange(1, 1);
      RunQueuedTasks();
      selects->Count = 0;
      selectionChanges->Count = 0;

      field.Select();
      CHECK(fixture.Document->FocusedElement() == &field);
      CHECK(field.SelectionStart() == 0);
      CHECK(field.SelectionEnd() == 5);
      RunQueuedTasks();
      CHECK(selects->Count == 1);
      CHECK(selectionChanges->Count >= 1);
    }

    SECTION("setRangeText() replaces a range and selects as asked")
    {
      field.SetSelectionRange(1, 1);
      CHECK_FALSE(field.SetRangeText(u8"XY", 0, 2, u8"select").HasException());
      CHECK(Narrow(field.Value()) == "XYllo");
      CHECK(field.SelectionStart() == 0);
      CHECK(field.SelectionEnd() == 2);

      CHECK_FALSE(field.SetRangeText(u8"!", 5, 5, u8"end").HasException());
      CHECK(Narrow(field.Value()) == "XYllo!");
      CHECK(field.SelectionStart() == 6);

      CHECK(field.SetRangeText(u8"?", 3, 1, u8"preserve").HasException());
    }

    SECTION("Types without a selection have none")
    {
      auto &box = InputById(fixture, u8"box");
      CHECK_FALSE(box.SelectionStartForBindings());
      CHECK(box.SetSelectionRangeForBindings(0, 0).HasException());
      CHECK(box.SetRangeText(u8"x", 0, 0, u8"").HasException());
      // A field's caret starts at the start of a value it was parsed with.
      CHECK(field.SelectionStartForBindings() == 0u);
    }

    SECTION("Changing the value puts the caret at its end")
    {
      (void)field.SetValue(u8"hi");
      CHECK(field.SelectionStart() == 2);
      CHECK(field.SelectionEnd() == 2);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLTextFormControlElement: password fields",
                   "[HTML][HTMLTextFormControlElement]")
  {
    EditorFixture fixture(u8"<input id=field type=password value=secret>");
    auto &field = InputById(fixture, u8"field");
    auto &text = *field.InnerTextElement()->FirstChild()->Renderer();

    // The text is drawn masked, one disc per character.
    CHECK(text.Style().GetTextSecurity() == TextSecurity::Disc);
    field.Focus();
    CHECK(fixture.Selection().Selection().IsInPasswordField());

    // It can't be copied or cut.
    CHECK_FALSE(fixture.GetEditor().CanCopy());
    (void)fixture.Run(u8"Copy");
    (void)fixture.Run(u8"Cut");
    CHECK(fixture.Pasteboard.Writes == 0);
    CHECK(Narrow(field.Value()) == "secret");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "HTMLTextFormControlElement: search fields",
                   "[HTML][HTMLTextFormControlElement]")
  {
    EditorFixture fixture(u8"<input id=search type=search value=cats>");
    auto &search = InputById(fixture, u8"search");
    auto &cancel = *search.CancelButtonElement();
    (void)fixture.Document->UpdateLayout();
    CHECK(cancel.Renderer()->Style().GetVisibility() == Visibility::Visible);

    SECTION("Clicking the cancel button clears the field")
    {
      cancel.Click();
      CHECK(search.Value().empty());
      (void)fixture.Document->UpdateLayout();
      CHECK(cancel.Renderer()->Style().GetVisibility() == Visibility::Hidden);
    }

    SECTION("Escape clears the field")
    {
      search.Focus();
      CHECK(PressKey(fixture, u8"Escape", u8"U+001B", vk::Escape));
      CHECK(search.Value().empty());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "HTMLTextFormControlElement: a field scrolls to keep the caret in view",
                   "[HTML][HTMLTextFormControlElement]")
  {
    EditorFixture fixture(u8"<input id=field style=width:50px>");
    auto &field = InputById(fixture, u8"field");
    field.Focus();
    (void)fixture.GetEditor().InsertText(u8"a long line of text that doesn't fit", nullptr);
    (void)fixture.Document->UpdateLayout();
    auto *renderer = Downcast<RenderTextControlSingleLine>(field.Renderer());
    REQUIRE(renderer);
    CHECK(renderer->ScrollLeft() > 0);
    CHECK(renderer->ScrollWidth() > 50);

    // Blurring scrolls it back to the start.
    field.Blur();
    CHECK(renderer->ScrollLeft() == 0);
  }
}
