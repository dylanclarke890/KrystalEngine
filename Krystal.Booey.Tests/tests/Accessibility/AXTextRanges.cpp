#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"
#include "Krystal.Booey/Accessibility/AXObjectCache.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AXTextRanges: a text field", "[Accessibility][AXTextRanges]")
  {
    AccessibilityFixture page(u8"<input id=input value='Hello world'>");
    auto &input = page.Object(u8"input");
    CHECK(input.Text() == u8"Hello world");
    CHECK(input.TextLength() == 11);
    CHECK(input.DoAXStringForRange({6, 5}) == u8"world");
    CHECK(input.DoAXStringForRange({20, 1}).empty());

    auto range = input.VisiblePositionRangeForRange({6, 5});
    REQUIRE_FALSE(range.IsNull());
    CHECK(input.PlainTextRangeForVisiblePositionRange(range) == CharacterRange {6, 5});
    CHECK(input.VisiblePositionRangeForRange({6, 6}).IsNull());

    input.SetSelectedVisiblePositionRange(input.VisiblePositionRangeForRange({0, 5}));
    CHECK(input.SelectedTextRange() == CharacterRange {0, 5});
    CHECK(input.SelectedText() == u8"Hello");
  }

  TEST_CASE("AXTextRanges: a password's text isn't exposed", "[Accessibility][AXTextRanges]")
  {
    AccessibilityFixture page(u8"<input id=input type=password value=secret>");
    auto &input = page.Object(u8"input");
    CHECK(input.Text().empty());
    input.SetSelectedVisiblePositionRange(input.GetVisiblePositionRange());
    CHECK(input.SelectedText().empty());
  }

  TEST_CASE("AXTextRanges: a text area's lines", "[Accessibility][AXTextRanges]")
  {
    AccessibilityFixture page(u8"<textarea id=area>one\ntwo</textarea>");
    auto &area = page.Object(u8"area");
    CHECK(area.DoAXRangeForLine(0) == CharacterRange {0, 4});
    CHECK(area.DoAXRangeForLine(1) == CharacterRange {4, 3});
    CHECK(area.DoAXRangeForLine(2) == CharacterRange {});
    CHECK(area.LineForPosition(area.VisiblePositionForIndex(5)) == 1);
  }

  TEST_CASE("AXTextRanges: an editable region", "[Accessibility][AXTextRanges]")
  {
    AccessibilityFixture page(
      u8"<div id=editor contenteditable role=textbox>Some <b id=bold>bold</b> text</div>");
    auto &editor = page.Object(u8"editor");
    CHECK(editor.Text() == u8"Some bold text");
    CHECK(editor.StringValue() == u8"Some bold text");

    editor.SetSelectedVisiblePositionRange(editor.VisiblePositionRangeForRange({5, 4}));
    CHECK(editor.SelectedTextRange() == CharacterRange {5, 4});
    CHECK(editor.SelectedText() == u8"bold");

    auto &bold = page.Object(u8"bold");
    CHECK(bold.ObservableObject() == &editor);
  }

  TEST_CASE("AXTextRanges: what tells the platform about changes", "[Accessibility][AXTextRanges]")
  {
    AccessibilityFixture page(u8"<select id=list size=3><option id=option>One</option></select>"
                              u8"<p id=paragraph>Text</p>");
    CHECK(page.Object(u8"list").ObservableObject() == &page.Object(u8"list"));
    CHECK(page.Object(u8"paragraph").ObservableObject() == nullptr);
  }
}
