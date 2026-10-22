#include "Krystal.Booey/Editing/VisibleSelection.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"

namespace krys::boo::tests
{
  namespace
  {
    struct SelectionFixture : PaintFixture
    {
      explicit SelectionFixture(utf8_stringview markup) noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}</style>"
                           + utf8_string(markup),
                         {400, 300})
      {
        (void)Document->UpdateLayout();
      }

      KRYS_NODISCARD dom::Text &TextIn(dom::DOMStringView id) noexcept
      {
        return Downcast<dom::Text>(*ById(id).FirstChild());
      }
    };

    KRYS_NODISCARD dom::Position At(dom::Node &node, uint32 offset) noexcept
    {
      return dom::MakeContainerOffsetPosition(ShareRefPtr(&node), offset);
    }

    KRYS_NODISCARD bool IsAt(const dom::Position &position, const dom::Node &node, int32 offset) noexcept
    {
      return position.DeprecatedNode() == &node && position.DeprecatedEditingOffset() == offset;
    }
  }

  TEST_CASE("VisibleSelection: types and order", "[Editing][VisibleSelection]")
  {
    SelectionFixture fixture(u8"<p id=p>The quick brown fox.</p><p id=q>Second   paragraph.</p>");
    auto &text = fixture.TextIn(u8"p");

    VisibleSelection none;
    CHECK(none.IsNone());
    CHECK(VisibleSelection::EmptySelection().IsNone());
    CHECK(!none.FirstRange());

    VisibleSelection caret(At(text, 4), Affinity::Downstream);
    CHECK(caret.IsCaret());
    CHECK(caret.IsCaretOrRange());
    CHECK(IsAt(caret.Start(), text, 4));
    CHECK(caret.Start() == caret.End());

    VisibleSelection forward(At(text, 4), At(text, 9));
    CHECK(forward.IsRange());
    CHECK(forward.IsBaseFirst());
    CHECK(IsAt(forward.Start(), text, 4));
    CHECK(IsAt(forward.End(), text, 9));
    CHECK(forward.VisibleStart().DeepEquivalent() == forward.Start());

    // Backwards: the anchor is after the focus; start and end are still in order.
    VisibleSelection backward(At(text, 9), At(text, 4));
    CHECK(backward.IsRange());
    CHECK(!backward.IsBaseFirst());
    CHECK(IsAt(backward.Start(), text, 4));
    CHECK(IsAt(backward.End(), text, 9));
    CHECK(IsAt(backward.Base(), text, 9));
    CHECK(IsAt(backward.Extent(), text, 4));
    CHECK(IsAt(backward.UncanonicalizedStart(), text, 4));
    CHECK(forward != backward);

    auto range = backward.FirstRange();
    REQUIRE(range);
    CHECK(range->Start.Container.get() == &text);
    CHECK(range->Start.Offset == 4);
    CHECK(range->End.Offset == 9);

    // Positions in collapsed whitespace are the same visible position, so a range between them is a caret.
    auto &second = fixture.TextIn(u8"q");
    VisibleSelection collapsed(At(second, 7), At(second, 8));
    CHECK(collapsed.IsCaret());

    // Extending moves the focus and keeps the anchor.
    VisibleSelection extended = caret;
    extended.SetExtent(At(second, 6));
    CHECK(extended.IsRange());
    CHECK(IsAt(extended.Start(), text, 4));
    CHECK(IsAt(extended.End(), second, 6));
    extended.SetBase(At(second, 6));
    CHECK(extended.IsCaret());

    CHECK(extended.GetDocument().get() == fixture.Document.get());
    CHECK(!forward.IsOrphan());
  }

  TEST_CASE("VisibleSelection: granularities", "[Editing][VisibleSelection]")
  {
    SelectionFixture fixture(u8"<p id=p>The quick, brown fox.</p><p id=q>Next one.</p>");
    auto &text = fixture.TextIn(u8"p");
    auto &next = fixture.TextIn(u8"q");

    // A word: the word the caret is inside of, or at the start of.
    VisibleSelection word(At(text, 6), Affinity::Downstream);
    CHECK(word.ExpandUsingGranularity(TextGranularity::WordGranularity));
    CHECK(word.IsRange());
    CHECK(IsAt(word.Start(), text, 4));
    CHECK(IsAt(word.End(), text, 9));

    // The space between words is a word of its own.
    VisibleSelection space(At(text, 10), Affinity::Downstream);
    space.ExpandUsingGranularity(TextGranularity::WordGranularity);
    CHECK(IsAt(space.Start(), text, 10));
    CHECK(IsAt(space.End(), text, 11));

    // The full stop after a word is a word of its own.
    VisibleSelection fox(At(text, 19), Affinity::Downstream);
    fox.ExpandUsingGranularity(TextGranularity::WordGranularity);
    CHECK(IsAt(fox.Start(), text, 17));
    CHECK(IsAt(fox.End(), text, 20));

    // A paragraph, with the break to the next one: the range's end is moved upstream, to the start of the
    // next paragraph's element.
    VisibleSelection paragraph(At(text, 6), Affinity::Downstream);
    paragraph.ExpandUsingGranularity(TextGranularity::ParagraphGranularity);
    CHECK(IsAt(paragraph.Start(), text, 0));
    CHECK(IsAt(paragraph.End(), fixture.ById(u8"q"), 0));
    CHECK(paragraph.VisibleEnd() == VisiblePosition(At(next, 0)));

    // The last paragraph has no break after it.
    VisibleSelection lastParagraph(At(next, 2), Affinity::Downstream);
    lastParagraph.ExpandUsingGranularity(TextGranularity::ParagraphGranularity);
    CHECK(IsAt(lastParagraph.Start(), next, 0));
    CHECK(IsAt(lastParagraph.End(), next, 9));

    VisibleSelection line(At(text, 6), Affinity::Downstream);
    line.ExpandUsingGranularity(TextGranularity::LineBoundary);
    CHECK(IsAt(line.Start(), text, 0));
    CHECK(IsAt(line.End(), text, 21));

    VisibleSelection document(At(text, 6), Affinity::Downstream);
    document.ExpandUsingGranularity(TextGranularity::DocumentBoundary);
    CHECK(IsAt(document.Start(), text, 0));
    CHECK(IsAt(document.End(), next, 9));
    CHECK(document.IsAll(EditingBoundaryCrossingRule::CanCrossEditingBoundary));
    CHECK(!word.IsAll(EditingBoundaryCrossingRule::CanCrossEditingBoundary));

    // Expanding none does nothing.
    VisibleSelection none;
    CHECK(!none.ExpandUsingGranularity(TextGranularity::WordGranularity));

    auto contents = VisibleSelection::SelectionFromContentsOfNode(&fixture.ById(u8"q"));
    CHECK(IsAt(contents.Start(), next, 0));
    CHECK(IsAt(contents.End(), next, 9));
  }

  TEST_CASE("VisibleSelection: trailing whitespace and normalised ranges", "[Editing][VisibleSelection]")
  {
    SelectionFixture fixture(u8"<p id=p>foo   bar <b id=b>baz</b></p>");
    auto &text = fixture.TextIn(u8"p");

    VisibleSelection word(At(text, 1), Affinity::Downstream);
    word.ExpandUsingGranularity(TextGranularity::WordGranularity);
    CHECK(IsAt(word.End(), text, 3));
    // Collapsed whitespace is one space to the text iterator, whose range is the first of them.
    word.AppendTrailingWhitespace();
    CHECK(IsAt(word.End(), text, 4));
    CHECK(IsAt(word.Focus(), text, 4));

    // The normalised range doesn't leak into the next text node.
    auto &bold = fixture.TextIn(u8"b");
    VisibleSelection selection(At(text, 10), At(bold, 3));
    auto normalised = selection.ToNormalizedRange();
    REQUIRE(normalised);
    CHECK(normalised->Start.Container.get() == &bold);
    CHECK(normalised->Start.Offset == 0);
    CHECK(normalised->End.Offset == 3);
  }

  TEST_CASE("VisibleSelection: editing boundaries", "[Editing][VisibleSelection]")
  {
    SelectionFixture fixture(u8"<p id=a>before</p><div contenteditable id=e>editable</div><p id=c>after</p>");
    auto &before = fixture.TextIn(u8"a");
    auto &editable = fixture.TextIn(u8"e");
    auto &after = fixture.TextIn(u8"c");

    // Based outside editable content: the end can't go into it, so it stops before.
    VisibleSelection intoEditable(At(before, 2), At(editable, 3));
    CHECK(intoEditable.IsRange());
    CHECK(IsAt(intoEditable.Start(), before, 2));
    CHECK(IsAt(intoEditable.End(), before, 6));
    CHECK(!intoEditable.IsContentEditable());

    // Across it, editable content is selected as a whole.
    VisibleSelection across(At(before, 2), At(after, 2));
    CHECK(IsAt(across.Start(), before, 2));
    CHECK(IsAt(across.End(), after, 2));

    // Based inside editable content: the selection stays inside its root.
    VisibleSelection outOfEditable(At(editable, 3), At(after, 2));
    CHECK(IsAt(outOfEditable.Start(), editable, 3));
    CHECK(IsAt(outOfEditable.End(), editable, 8));
    CHECK(outOfEditable.IsContentEditable());
    CHECK(outOfEditable.HasEditableStyle());
    CHECK(outOfEditable.RootEditableElement() == &fixture.ById(u8"e"));

    VisibleSelection backwardsOut(At(editable, 3), At(before, 2));
    CHECK(IsAt(backwardsOut.Start(), editable, 0));
    CHECK(IsAt(backwardsOut.End(), editable, 3));
    CHECK(!backwardsOut.IsBaseFirst());
  }
}
