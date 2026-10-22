#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Editing/EditorTestUtils.hpp"
#include "Krystal.Booey/Editing/EditingStyle.hpp"

namespace krys::boo::tests
{
  TEST_CASE_METHOD(css::tests::ValueFixture, "ApplyStyleCommand: styling the selection",
                   "[Editing][ApplyStyleCommand]")
  {
    EditorFixture fixture(u8"<div id=e contenteditable>abc</div>");
    auto &root = fixture.ById(u8"e");
    fixture.Select(fixture.TextIn(u8"e"), 1, 2);

    auto apply = [&](css::PropertyId property, css::ValueId value, EditAction action)
    {
      auto style = EditingStyle::Create(property, value);
      ApplyStyleCommand::Create(*fixture.Document, style.get(), action)->Apply();
    };

    SECTION("Bold is a b element")
    {
      apply(css::PropertyId::FontWeight, css::ValueId::Bold, EditAction::Bold);
      CHECK(Narrow(fixture.InnerHTML(root)) == "a<b>b</b>c");
      CHECK(Narrow(fixture.GetEditor().SelectedText()) == "b");
    }

    SECTION("Italic is an i element")
    {
      apply(css::PropertyId::FontStyle, css::ValueId::Italic, EditAction::Italics);
      CHECK(Narrow(fixture.InnerHTML(root)) == "a<i>b</i>c");
    }

    SECTION("Removing bold unwraps the b element")
    {
      apply(css::PropertyId::FontWeight, css::ValueId::Bold, EditAction::Bold);
      apply(css::PropertyId::FontWeight, css::ValueId::Normal, EditAction::Bold);
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc");
    }

    SECTION("Styling is undone as one step")
    {
      apply(css::PropertyId::FontWeight, css::ValueId::Bold, EditAction::Bold);
      CHECK(fixture.Client->UndoStack.size() == 1);
      CHECK(fixture.Run(u8"Undo"));
      CHECK(Narrow(fixture.InnerHTML(root)) == "abc");
    }
  }
}
