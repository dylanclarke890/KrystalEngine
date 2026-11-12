#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"

namespace krys::boo::ax::tests
{
  namespace
  {
    using TextChanges = List<NotificationRecorder::TextChange>;

    KRYS_NODISCARD bool HasTextChange(const TextChanges &changes, const AccessibilityObject &object,
                                      AXTextChange change, uint64 offset, utf8_stringview text) noexcept
    {
      for (const auto &posted : changes)
      {
        if (posted.Object.get() == &object && posted.Change == change && posted.Offset == offset
            && posted.Text == text)
        {
          return true;
        }
      }
      return false;
    }

    /// @brief Puts the selection over @p range of @p object's text, then forgets what that posted.
    void Select(AccessibilityFixture &page, AccessibilityObject &object, CharacterRange range) noexcept
    {
      object.SetSelectedVisiblePositionRange(object.VisiblePositionRangeForRange(range));
      (void)page.TakeNotifications();
      (void)page.TakeTextChanges();
    }
  }

  TEST_CASE("AXTextStateChanges: typing", "[Accessibility][AXTextStateChanges]")
  {
    AccessibilityFixture page(u8"<div id=editor contenteditable>Hello</div>");
    auto &editor = page.Object(u8"editor");

    SECTION("Inserting text")
    {
      Select(page, editor, {5, 0});
      REQUIRE(page.Document->ExecCommand(u8"insertText", false, u8"!").Value());
      CHECK(HasTextChange(page.TakeTextChanges(), editor, AXTextChange::Inserted, 5, u8"!"));
    }

    SECTION("Typing over a selection replaces it")
    {
      Select(page, editor, {0, 5});
      REQUIRE(page.Document->ExecCommand(u8"insertText", false, u8"Bye").Value());
      auto changes = page.TakeTextChanges();
      CHECK(HasTextChange(changes, editor, AXTextChange::Deleted, 0, u8"Hello"));
      CHECK(HasTextChange(changes, editor, AXTextChange::Inserted, 0, u8"Bye"));
    }

    SECTION("Deleting")
    {
      Select(page, editor, {5, 0});
      REQUIRE(page.Document->ExecCommand(u8"delete").Value());
      CHECK(HasTextChange(page.TakeTextChanges(), editor, AXTextChange::Deleted, 4, u8"o"));

      SECTION("Undoing it puts the text back")
      {
        REQUIRE(page.Document->GetEditor().CanUndo());
        page.Document->GetEditor().Undo();
        CHECK(HasTextChange(page.TakeTextChanges(), editor, AXTextChange::Inserted, 4, u8"o"));

        page.Document->GetEditor().Redo();
        CHECK(HasTextChange(page.TakeTextChanges(), editor, AXTextChange::Deleted, 4, u8"o"));
      }
    }
  }

  TEST_CASE("AXTextStateChanges: the selection moving", "[Accessibility][AXTextStateChanges]")
  {
    AccessibilityFixture page(u8"<div id=editor contenteditable>Hello</div><p id=text>Text</p>");
    auto &editor = page.Object(u8"editor");
    Select(page, editor, {0, 0});

    editor.SetSelectedVisiblePositionRange(editor.VisiblePositionRangeForRange({1, 2}));
    // The host's next rendering update tells accessibility.
    page.TestPage->UpdateRendering();
    CHECK(
      AccessibilityFixture::WasPosted(page.TakeNotifications(), editor, AXNotification::SelectedTextChanged));
  }
}
