#include "Krystal.Booey/Editing/VisibleUnits.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"

namespace krys::boo::tests
{
  namespace
  {
    struct UnitsFixture : PaintFixture
    {
      explicit UnitsFixture(utf8_stringview markup) noexcept
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

    KRYS_NODISCARD VisiblePosition At(dom::Node &node, uint32 offset) noexcept
    {
      return VisiblePosition(dom::MakeContainerOffsetPosition(ShareRefPtr(&node), offset));
    }

    KRYS_NODISCARD int32 OffsetOf(const VisiblePosition &position) noexcept
    {
      return position.DeepEquivalent().DeprecatedEditingOffset();
    }
  }

  TEST_CASE("VisibleUnits: words", "[Editing][VisibleUnits]")
  {
    UnitsFixture fixture(u8"<p id=p>The quick, brown fox.</p>");
    auto &text = fixture.TextIn(u8"p");

    CHECK(OffsetOf(StartOfWord(At(text, 6))) == 4);
    CHECK(OffsetOf(EndOfWord(At(text, 6))) == 9);
    // On a boundary, the right word by default, the left word if asked.
    CHECK(OffsetOf(StartOfWord(At(text, 4))) == 4);
    CHECK(OffsetOf(EndOfWord(At(text, 9), WordSide::LeftWordIfOnBoundary)) == 9);
    CHECK(OffsetOf(StartOfWord(At(text, 9), WordSide::LeftWordIfOnBoundary)) == 4);
    CHECK(IsStartOfWord(At(text, 11)));
    CHECK(!IsStartOfWord(At(text, 12)));

    CHECK(OffsetOf(NextWordPosition(At(text, 0))) == 3);
    CHECK(OffsetOf(NextWordPosition(At(text, 3))) == 9);
    CHECK(OffsetOf(PreviousWordPosition(At(text, 9))) == 4);
    CHECK(OffsetOf(PreviousWordPosition(At(text, 4))) == 0);

    // Windows moves right to the start of the next word.
    CHECK(OffsetOf(RightWordPosition(At(text, 0), true)) == 4);
    CHECK(OffsetOf(RightWordPosition(At(text, 0), false)) == 3);
    CHECK(OffsetOf(LeftWordPosition(At(text, 9), true)) == 4);
  }

  TEST_CASE("VisibleUnits: words across elements", "[Editing][VisibleUnits]")
  {
    UnitsFixture fixture(u8"<p id=p>wo<b id=b>rd</b> next</p>");
    auto &b = *fixture.ById(u8"b").FirstChild();
    auto start = StartOfWord(At(b, 1));
    CHECK(start.DeepEquivalent().DeprecatedNode() == &fixture.TextIn(u8"p"));
    CHECK(OffsetOf(start) == 0);
    auto end = EndOfWord(At(fixture.TextIn(u8"p"), 1));
    CHECK(end.DeepEquivalent().DeprecatedNode() == &b);
    CHECK(OffsetOf(end) == 2);
  }

  TEST_CASE("VisibleUnits: lines", "[Editing][VisibleUnits]")
  {
    // 60 pixels fits about ten characters, so the text wraps after "aaaa bbbb ".
    UnitsFixture fixture(
      u8"<p id=p style=width:60px;margin:0>aaaa bbbb cccc</p><p id=q style=margin:0>dd<br>ee</p>");
    auto &text = fixture.TextIn(u8"p");

    CHECK(OffsetOf(StartOfLine(At(text, 2))) == 0);
    auto endOfFirstLine = EndOfLine(At(text, 2));
    CHECK(OffsetOf(endOfFirstLine) == 9);
    CHECK(OffsetOf(StartOfLine(At(text, 12))) == 10);
    CHECK(OffsetOf(EndOfLine(At(text, 12))) == 14);
    CHECK(InSameLine(At(text, 1), At(text, 3)));
    CHECK(!InSameLine(At(text, 1), At(text, 12)));
    CHECK(IsStartOfLine(At(text, 0)));
    CHECK(IsEndOfLine(At(text, 14)));
    CHECK(OffsetOf(LogicalStartOfLine(At(text, 12))) == 10);

    // The positions on the lines above and below, at the same horizontal position.
    auto below = NextLinePosition(At(text, 2), At(text, 2).LineDirectionPointForBlockDirectionNavigation());
    CHECK(below.DeepEquivalent().DeprecatedNode() == &text);
    CHECK(OffsetOf(below) == 12);
    auto above =
      PreviousLinePosition(At(text, 12), At(text, 12).LineDirectionPointForBlockDirectionNavigation());
    CHECK(OffsetOf(above) == 2);

    // A line break ends a line: the position before it is the end of the text.
    auto &q = fixture.TextIn(u8"q");
    CHECK(EndOfLine(At(q, 1)) == At(q, 2));
    auto &ee = *q.NextSibling()->NextSibling();
    CHECK(StartOfLine(At(ee, 1)).DeepEquivalent().DeprecatedNode() == &ee);
  }

  TEST_CASE("VisibleUnits: paragraphs, blocks and documents", "[Editing][VisibleUnits]")
  {
    UnitsFixture fixture(u8"<div id=d>one <b>two</b><br>three</div><p id=p>four</p>");
    auto &one = *fixture.ById(u8"d").FirstChild();
    auto &three = *fixture.ById(u8"d").LastChild();

    CHECK(StartOfParagraph(At(one, 2)).DeepEquivalent().DeprecatedNode() == &one);
    CHECK(OffsetOf(StartOfParagraph(At(one, 2))) == 0);
    auto paragraphEnd = EndOfParagraph(At(one, 2));
    CHECK(paragraphEnd.DeepEquivalent().DeprecatedNode()
          == fixture.ById(u8"d").FirstChild()->NextSibling()->FirstChild());
    CHECK(OffsetOf(paragraphEnd) == 3);
    CHECK(OffsetOf(StartOfParagraph(At(three, 2))) == 0);
    CHECK(IsStartOfParagraph(At(three, 0)));
    CHECK(IsEndOfParagraph(At(three, 5)));
    CHECK(!InSameParagraph(At(one, 1), At(three, 1)));

    // Blocks don't end at line breaks.
    CHECK(InSameBlock(At(one, 1), At(three, 1)));
    CHECK(!InSameBlock(At(one, 1), At(fixture.TextIn(u8"p"), 1)));

    auto documentStart = StartOfDocument(&one);
    CHECK(documentStart.DeepEquivalent().DeprecatedNode() == &one);
    CHECK(IsStartOfDocument(documentStart));
    auto documentEnd = EndOfDocument(&one);
    CHECK(documentEnd.DeepEquivalent().DeprecatedNode() == &fixture.TextIn(u8"p"));
    CHECK(OffsetOf(documentEnd) == 4);
    CHECK(IsEndOfDocument(documentEnd));
  }

  TEST_CASE("VisibleUnits: sentences", "[Editing][VisibleUnits]")
  {
    UnitsFixture fixture(u8"<p id=p>One two. Three four. Five.</p>");
    auto &text = fixture.TextIn(u8"p");
    CHECK(OffsetOf(StartOfSentence(At(text, 12))) == 9);
    CHECK(OffsetOf(EndOfSentence(At(text, 12))) == 21);
    CHECK(OffsetOf(StartOfSentence(At(text, 3))) == 0);
  }

  TEST_CASE("VisibleUnits: editable content", "[Editing][VisibleUnits]")
  {
    UnitsFixture fixture(u8"<p>x</p><div id=e contenteditable>ab <i>cd</i></div><p>y</p>");
    auto &ab = *fixture.ById(u8"e").FirstChild();
    auto start = StartOfEditableContent(At(ab, 2));
    CHECK(start.DeepEquivalent().DeprecatedNode() == &ab);
    CHECK(OffsetOf(start) == 0);
    auto end = EndOfEditableContent(At(ab, 2));
    CHECK(end.DeepEquivalent().DeprecatedNode() == fixture.ById(u8"e").LastChild()->FirstChild());
    CHECK(OffsetOf(end) == 2);
    CHECK(IsEndOfEditableOrNonEditableContent(end) == false);

    // The next paragraph's start, skipping over editing boundaries (which needs an editable root).
    UnitsFixture paragraphs(u8"<div id=e contenteditable>one<br>two</div>");
    auto &one = *paragraphs.ById(u8"e").FirstChild();
    CHECK(StartOfNextParagraph(At(one, 1)).DeepEquivalent().DeprecatedNode()
          == paragraphs.ById(u8"e").LastChild());
  }
}
