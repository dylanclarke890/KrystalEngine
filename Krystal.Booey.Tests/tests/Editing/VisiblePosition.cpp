#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/RenderedPosition.hpp"
#include "Krystal.Booey/Rendering/CaretRectComputation.hpp"
#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief A laid out document in a 400x300 view, with 10px text on 20px lines.
    struct VisibleFixture : PaintFixture
    {
      explicit VisibleFixture(utf8_stringview markup) noexcept
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

      /// @brief The visible position for the absolute @p point, through the render view.
      KRYS_NODISCARD VisiblePosition PositionAt(float x, float y) noexcept
      {
        return Document->GetRenderView()->VisiblePositionForPoint(LayoutPoint(FloatPoint(x, y)),
                                                                  HitTestSource::User);
      }
    };

    KRYS_NODISCARD dom::Position At(dom::Node &node, uint32 offset) noexcept
    {
      return dom::MakeContainerOffsetPosition(ShareRefPtr(&node), offset);
    }

    KRYS_NODISCARD uint32 OffsetOf(const VisiblePosition &position) noexcept
    {
      return static_cast<uint32>(position.DeepEquivalent().DeprecatedEditingOffset());
    }
  }

  TEST_CASE("VisiblePosition: canonical positions", "[Editing][VisiblePosition]")
  {
    VisibleFixture fixture(u8"<p id=p>foo     bar</p><div id=d><b id=b>ab</b>cd</div>");
    auto &text = fixture.TextIn(u8"p");

    // Every position in the collapsed run is the same visible position, the upstream one.
    CHECK(VisiblePosition(At(text, 6)) == VisiblePosition(At(text, 4)));
    CHECK(VisiblePosition(At(text, 8)) == VisiblePosition(At(text, 4)));
    CHECK(OffsetOf(VisiblePosition(At(text, 8))) == 4);
    CHECK(VisiblePosition(At(text, 3)) != VisiblePosition(At(text, 4)));

    // A position between elements canonicalises into text.
    auto &d = fixture.ById(u8"d");
    auto between = VisiblePosition(At(d, 1));
    CHECK(between.DeepEquivalent().DeprecatedNode() == fixture.ById(u8"b").FirstChild());
    CHECK(OffsetOf(between) == 2);

    CHECK(VisiblePosition().IsNull());
    CHECK(VisiblePosition(At(text, 1)) < VisiblePosition(At(text, 2)));
  }

  TEST_CASE("VisiblePosition: next, previous and characters", "[Editing][VisiblePosition]")
  {
    VisibleFixture fixture(u8"<p id=p>ab  c</p><p id=q>éé</p>");
    auto &text = fixture.TextIn(u8"p");

    VisiblePosition position(At(text, 0));
    CHECK(position.CharacterAfter() == 'a');
    position = position.Next();
    CHECK(OffsetOf(position) == 1);
    position = position.Next().Next();
    CHECK(OffsetOf(position) == 3);
    CHECK(position.CharacterBefore() == ' ');
    position = position.Next();
    CHECK(position.DeepEquivalent().Downstream().DeprecatedEditingOffset() == 5);
    position = position.Next();
    // Into the next paragraph.
    CHECK(position.DeepEquivalent().DeprecatedNode() == &fixture.TextIn(u8"q"));
    CHECK(position.CharacterAfter() == 0xE9);
    position = position.Next();
    CHECK(OffsetOf(position) == 2);
    position = position.Next();
    CHECK(OffsetOf(position) == 5);
    CHECK(position.Next().IsNull());
    CHECK(OffsetOf(position.Previous()) == 2);
    CHECK(VisiblePosition(At(text, 0)).Previous().IsNull());

    // Left and right in left-to-right text are previous and next.
    CHECK(VisiblePosition(At(text, 1)).Right() == VisiblePosition(At(text, 2)));
    CHECK(VisiblePosition(At(text, 1)).Left() == VisiblePosition(At(text, 0)));
  }

  TEST_CASE("VisiblePosition: editing boundaries", "[Editing][VisiblePosition]")
  {
    VisibleFixture fixture(u8"<p id=a>x</p><div id=e contenteditable>ab</div><p id=b>y</p>");
    auto &editable = fixture.TextIn(u8"e");
    VisiblePosition end(At(editable, 2));
    CHECK(end.Next(EditingBoundaryCrossingRule::CannotCrossEditingBoundary).IsNull());
    CHECK(end.Next().DeepEquivalent().DeprecatedNode() == &fixture.TextIn(u8"b"));
    VisiblePosition start(At(editable, 0));
    CHECK(start.Previous(EditingBoundaryCrossingRule::CannotCrossEditingBoundary).IsNull());
    CHECK(end.RootEditableElement() == &fixture.ById(u8"e"));
  }

  TEST_CASE("RenderObject::PositionForPoint", "[Editing][VisiblePosition]")
  {
    VisibleFixture fixture(u8"<p id=p style=margin:0>abcdefghij</p><p id=q style=margin:0>klmno</p>"
                           u8"<div id=empty style=height:40px></div>"
                           u8"<p style=margin:0><img id=img style=width:40px;height:20px></p>");
    auto &first = fixture.TextIn(u8"p");
    auto &second = fixture.TextIn(u8"q");

    // Each line is 20 pixels tall; characters are about 5 pixels wide.
    auto start = fixture.PositionAt(0, 10);
    CHECK(start.DeepEquivalent().DeprecatedNode() == &first);
    CHECK(OffsetOf(start) == 0);

    auto middle = fixture.PositionAt(26, 10);
    CHECK(middle.DeepEquivalent().DeprecatedNode() == &first);
    auto caret = middle.LocalCaretRect().Rect;
    CHECK(std::abs(caret.X().ToFloat() - 26) <= 3);

    // Past the end of a line is its end; below a line is the next one.
    auto end = fixture.PositionAt(300, 10);
    CHECK(OffsetOf(end) == 10);
    auto next = fixture.PositionAt(2, 30);
    CHECK(next.DeepEquivalent().DeprecatedNode() == &second);
    CHECK(OffsetOf(next) == 0);

    // An image's left half is before it and its right half after it.
    auto &img = fixture.ById(u8"img");
    auto *imgBox = img.Renderer();
    REQUIRE(imgBox);
    auto imageTop = imgBox->LocalToAbsolute().Y();
    auto beforeImage = fixture.PositionAt(5, imageTop + 10);
    CHECK(beforeImage.DeepEquivalent() == dom::PositionBeforeNode(&img));
    auto afterImage = fixture.PositionAt(35, imageTop + 10);
    CHECK(afterImage.DeepEquivalent() == dom::PositionAfterNode(&img));
  }

  TEST_CASE("VisiblePosition: caret rects", "[Editing][VisiblePosition]")
  {
    VisibleFixture fixture(
      u8"<p id=p style=margin:0>abc def</p><div id=e contenteditable style=height:30px></div>");
    auto &text = fixture.TextIn(u8"p");

    auto start = VisiblePosition(At(text, 0)).LocalCaretRect();
    CHECK(start.Renderer == text.Renderer());
    CHECK(start.Rect.X() == 0);
    CHECK(start.Rect.Width() == CaretWidth());
    CHECK(start.Rect.Height() > 0);
    auto later = VisiblePosition(At(text, 4)).LocalCaretRect();
    CHECK(later.Rect.X() > start.Rect.X());
    CHECK(later.Rect.Y() == start.Rect.Y());

    auto absolute = VisiblePosition(At(text, 4)).AbsoluteCaretBounds();
    CHECK(absolute.X() == later.Rect.X().ToInt());
    CHECK(absolute.Width() == CaretWidth());

    // An empty editable block has a caret at its start, as tall as the font.
    auto &editable = fixture.ById(u8"e");
    VisiblePosition inEmpty(dom::FirstPositionInNode(&editable));
    REQUIRE(inEmpty.IsNotNull());
    auto emptyCaret = inEmpty.LocalCaretRect();
    CHECK(emptyCaret.Renderer == editable.Renderer());
    CHECK(emptyCaret.Rect.X() == 0);
    CHECK(emptyCaret.Rect.Height() > 0);

    RenderedPosition rendered(VisiblePosition(At(text, 4)));
    CHECK(!rendered.IsNull());
    CHECK(rendered.Offset() == 4);
    CHECK(rendered.LineBox());
    CHECK(rendered.AbsoluteRect() == absolute);
  }
}
