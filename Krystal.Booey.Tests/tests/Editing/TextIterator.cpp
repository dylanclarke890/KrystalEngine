#include "Krystal.Booey/Editing/TextIterator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include <string>

namespace krys::boo::tests
{
  namespace
  {
    struct TextFixture : PaintFixture
    {
      explicit TextFixture(utf8_stringview markup) noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}</style>"
                           + utf8_string(markup),
                         {400, 300})
      {
        (void)Document->UpdateLayout();
      }

      KRYS_NODISCARD std::string Text(TextIteratorBehaviors behaviors = {}) noexcept
      {
        auto &body = *Document->Body();
        auto text = PlainText(dom::MakeRangeSelectingNodeContents(body), behaviors);
        return std::string(text.begin(), text.end());
      }
    };
  }

  TEST_CASE("TextIterator: plain text", "[Editing][TextIterator]")
  {
    CHECK(TextFixture(u8"<p>one   two\nthree</p>").Text() == "one two three");
    CHECK(TextFixture(u8"<div>a</div><div>b</div>").Text() == "a\nb");
    // Paragraphs with a large enough bottom margin get an extra newline, as headings do.
    CHECK(TextFixture(u8"<p>a</p><p>b</p>").Text() == "a\n\nb");
    CHECK(TextFixture(u8"a<br>b").Text() == "a\nb");
    CHECK(TextFixture(u8"a<span style=display:none>hidden</span>b").Text() == "ab");
    CHECK(TextFixture(u8"a<b>b</b><i>c</i>").Text() == "abc");
    CHECK(TextFixture(u8"<pre>a  b\nc</pre>").Text() == "a  b\nc");
    CHECK(TextFixture(u8"x <span style=visibility:hidden>y</span> z").Text() == "x  z");
    CHECK(TextFixture(u8"<table><tr><td>a</td><td>b</td></tr><tr><td>c</td><td>d</td></tr></table>").Text()
          == "a\tb\nc\td");
    CHECK(TextFixture(u8"<p style=text-transform:uppercase>abc</p>").Text() == "ABC");
  }

  TEST_CASE("TextIterator: replaced elements", "[Editing][TextIterator]")
  {
    TextFixture fixture(u8"a<img alt=pic style=width:10px;height:10px>b");
    CHECK(fixture.Text() == "ab");
    CHECK(fixture.Text(TextIteratorBehavior::EmitsObjectReplacementCharacters)
          == "a\xEF\xBF\xBC"
             "b");
    CHECK(fixture.Text(TextIteratorBehavior::EmitsCharactersBetweenAllVisiblePositions) == "a,b");
  }

  TEST_CASE("TextIterator: ranges and character offsets", "[Editing][TextIterator]")
  {
    TextFixture fixture(u8"<p id=p>hello  world</p>");
    auto &text = Downcast<dom::Text>(*fixture.ById(u8"p").FirstChild());
    auto scope = dom::MakeRangeSelectingNodeContents(*fixture.Document->Body());

    TextIterator iterator(scope);
    REQUIRE(!iterator.AtEnd());
    CHECK(std::string(iterator.Text().begin(), iterator.Text().end()) == "hello ");
    CHECK(iterator.Range().Start.Container.get() == &text);
    CHECK(iterator.Range().StartOffset() == 0);
    CHECK(iterator.Range().EndOffset() == 6);
    iterator.Advance();
    REQUIRE(!iterator.AtEnd());
    CHECK(std::string(iterator.Text().begin(), iterator.Text().end()) == "world");
    CHECK(iterator.Range().StartOffset() == 7);

    // Characters count the collapsed text once.
    CHECK(CharacterCount(scope) == 11);
    auto location = ResolveCharacterLocation(scope, 6);
    CHECK(location.Container.get() == &text);
    // A location at the end of a run is there, not at the start of the next.
    CHECK(location.Offset == 6);
    CHECK(ResolveCharacterLocation(scope, 7).Offset == 8);
    auto range = ResolveCharacterRange(scope, {1, 3});
    CHECK(range.StartOffset() == 1);
    CHECK(range.EndOffset() == 4);
    CHECK(GetCharacterRange(scope, range) == CharacterRange {1, 3});

    CharacterIterator characters(scope);
    characters.Advance(7);
    CHECK(characters.Range().StartOffset() == 8);
    CHECK(characters.CharacterOffset() == 7);
  }

  TEST_CASE("TextIterator: backwards", "[Editing][TextIterator]")
  {
    TextFixture fixture(u8"<div id=d>ab<b>cd</b></div><div>ef</div>");
    std::string text;
    for (SimplifiedBackwardsTextIterator it(dom::MakeRangeSelectingNodeContents(*fixture.Document->Body()));
         !it.AtEnd(); it.Advance())
    {
      text.insert(0, std::string(it.Text().begin(), it.Text().end()));
    }
    // The newline for the first block's start is emitted too.
    CHECK(text == "\nabcd\nef");

    BackwardsCharacterIterator characters(dom::MakeRangeSelectingNodeContents(fixture.ById(u8"d")));
    characters.Advance(1);
    CHECK(characters.Range().Start.Container.get() == fixture.ById(u8"d").LastChild()->FirstChild());
    CHECK(characters.Range().EndOffset() == 1);
  }

  TEST_CASE("TextIterator: word aware chunks", "[Editing][TextIterator]")
  {
    TextFixture fixture(u8"<p>ab<b>cd</b> ef</p>");
    WordAwareIterator iterator(dom::MakeRangeSelectingNodeContents(*fixture.Document->Body()));
    REQUIRE(!iterator.AtEnd());
    CHECK(std::string(iterator.Text().begin(), iterator.Text().end()) == "abcd");
  }
}
