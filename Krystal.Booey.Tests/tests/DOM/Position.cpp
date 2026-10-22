#include "Krystal.Booey/DOM/Position.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/PositionIterator.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/InlineRunAndOffset.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    /// @brief A laid out document in a 200x100 view.
    struct PositionFixture : boo::tests::PaintFixture
    {
      explicit PositionFixture(utf8_stringview markup) noexcept
          : PaintFixture(u8"<!DOCTYPE html><style>body{margin:0;font:10px/20px sans-serif}</style>"
                         + utf8_string(markup))
      {
        (void)Document->UpdateLayout();
      }

      /// @brief The first child of the element with @p id, which must be text.
      KRYS_NODISCARD Text &TextIn(DOMStringView id) noexcept
      {
        return Downcast<Text>(*ById(id).FirstChild());
      }
    };

    KRYS_NODISCARD Position At(Node &node, uint32 offset) noexcept
    {
      return MakeContainerOffsetPosition(ShareRefPtr(&node), offset);
    }
  }

  TEST_CASE("Position: anchors and containers", "[DOM][Position]")
  {
    PositionFixture fixture(u8"<div id=d><b id=b>ab</b><i id=i>cd</i><img id=img></div>");
    auto &d = fixture.ById(u8"d");
    auto &b = fixture.ById(u8"b");
    auto &i = fixture.ById(u8"i");
    auto &img = fixture.ById(u8"img");

    SECTION("Before and after a node are offsets in its parent")
    {
      auto before = PositionBeforeNode(&i);
      CHECK(before.GetAnchorType() == Position::AnchorType::PositionIsBeforeAnchor);
      CHECK(before.GetContainerNode() == &d);
      CHECK(before.ComputeOffsetInContainerNode() == 1);
      CHECK(before.ComputeNodeAfterPosition() == &i);
      CHECK(before.ComputeNodeBeforePosition() == &b);
      CHECK(PositionAfterNode(&i).ComputeOffsetInContainerNode() == 2);

      // Before a node is offset 0 in it, unless editing ignores its content.
      auto equivalent = before.ParentAnchoredEquivalent();
      CHECK(equivalent.GetAnchorType() == Position::AnchorType::PositionIsOffsetInAnchor);
      CHECK(equivalent.GetContainerNode() == &i);
      CHECK(equivalent.OffsetInContainerNode() == 0);
      auto afterImage = PositionAfterNode(&img).ParentAnchoredEquivalent();
      CHECK(afterImage.GetContainerNode() == &d);
      CHECK(afterImage.OffsetInContainerNode() == 3);
    }

    SECTION("First and last positions in a node")
    {
      CHECK(FirstPositionInNode(&d).GetAnchorType() == Position::AnchorType::PositionIsBeforeChildren);
      CHECK(LastPositionInNode(&d).GetAnchorType() == Position::AnchorType::PositionIsAfterChildren);
      CHECK(LastPositionInNode(&d).ComputeOffsetInContainerNode() == 3);
      auto &text = *b.FirstChild();
      CHECK(FirstPositionInNode(&text).GetAnchorType() == Position::AnchorType::PositionIsOffsetInAnchor);
      CHECK(LastPositionInNode(&text).OffsetInContainerNode() == 2);
      CHECK(PositionBeforeNode(&b).Equals(FirstPositionInNode(&d)));
      CHECK(!(PositionBeforeNode(&b) == FirstPositionInNode(&d)));
    }

    SECTION("Legacy positions in nodes editing ignores the content of are before or after them")
    {
      CHECK(EditingIgnoresContent(img));
      CHECK(!EditingIgnoresContent(d));
      CHECK(MakeDeprecatedLegacyPosition(ShareRefPtr<Node>(&img), 0).GetAnchorType()
            == Position::AnchorType::PositionIsBeforeAnchor);
      CHECK(MakeDeprecatedLegacyPosition(ShareRefPtr<Node>(&img), 1).GetAnchorType()
            == Position::AnchorType::PositionIsAfterAnchor);
      CHECK(MakeDeprecatedLegacyPosition(ShareRefPtr<Node>(&d), 1).GetAnchorType()
            == Position::AnchorType::PositionIsOffsetInAnchor);
      CHECK(LastOffsetForEditing(img) == 1);
      CHECK(LastOffsetForEditing(d) == 3);
    }

    SECTION("Tree order")
    {
      auto &bText = *b.FirstChild();
      auto &iText = *i.FirstChild();
      CHECK(At(bText, 2) < At(iText, 0));
      CHECK(At(iText, 1) > At(bText, 0));
      CHECK(std::is_eq(PositionBeforeNode(&b) <=> FirstPositionInNode(&d)));
      CHECK(PositionAfterNode(&img) > At(iText, 2));
      CHECK(std::is_eq(Position() <=> Position()));
      CHECK(CommonInclusiveAncestor(At(bText, 0), At(iText, 0)) == &d);
    }

    SECTION("Boundary points")
    {
      auto point = MakeBoundaryPoint(PositionAfterNode(&b));
      REQUIRE(point);
      CHECK(point->Container.get() == &d);
      CHECK(point->Offset == 1);
      CHECK(MakeContainerOffsetPosition(*point) == At(d, 1));
    }
  }

  TEST_CASE("Position: moving by code points and characters", "[DOM][Position]")
  {
    // "é" is two bytes, and "e" with a combining acute is one character of three bytes.
    PositionFixture fixture(u8"<p id=p>aééb</p>");
    auto &text = fixture.TextIn(u8"p");

    CHECK(At(text, 1).Next().DeprecatedEditingOffset() == 2);
    CHECK(At(text, 1).Next(PositionMoveType::Character).DeprecatedEditingOffset() == 3);
    CHECK(At(text, 3).Next(PositionMoveType::Character).DeprecatedEditingOffset() == 6);
    CHECK(At(text, 6).Previous(PositionMoveType::Character).DeprecatedEditingOffset() == 3);
    CHECK(At(text, 3).Previous(PositionMoveType::Character).DeprecatedEditingOffset() == 1);
    // Deletion goes back by characters too.
    CHECK(At(text, 6).Previous(PositionMoveType::BackwardDeletion).DeprecatedEditingOffset() == 3);

    auto &renderer = *text.Renderer();
    CHECK(renderer.ContainsCaretOffset(3));
    CHECK(!renderer.ContainsCaretOffset(4));
    CHECK(!renderer.ContainsCaretOffset(2));
    CHECK(renderer.CaretMinOffset() == 0);
    CHECK(renderer.CaretMaxOffset() == 7);
  }

  TEST_CASE("Position: candidates and equivalent positions", "[DOM][Position]")
  {
    SECTION("Collapsed whitespace")
    {
      // "foo     bar": the spaces collapse to one, rendered between offsets 3 and 4.
      PositionFixture fixture(u8"<p id=p>foo     bar</p>");
      auto &text = fixture.TextIn(u8"p");

      CHECK(At(text, 3).IsCandidate());
      CHECK(At(text, 4).IsCandidate());
      CHECK(!At(text, 5).IsCandidate());
      CHECK(!At(text, 7).IsCandidate());
      CHECK(At(text, 8).IsCandidate());

      // Inside or at the ends of the collapsed run, upstream is after the rendered space and downstream
      // before "bar".
      for (uint32 offset = 4; offset <= 8; ++offset)
      {
        CHECK(At(text, offset).Upstream().DeprecatedEditingOffset() == 4);
        CHECK(At(text, offset).Downstream().DeprecatedEditingOffset() == 8);
      }
      CHECK(At(text, 2).Upstream() == At(text, 2));

      CHECK(text.Renderer()->CountRenderedCharacterOffsetsUntil(8) == 4);
      CHECK(At(text, 4).RendersInDifferentPosition(At(text, 3)));
      CHECK(!At(text, 4).RendersInDifferentPosition(At(text, 8)));
    }

    SECTION("Unrendered and invisible content has no candidates")
    {
      PositionFixture fixture(u8"<p id=a>one</p><p id=hidden style=display:none>two</p><p id=invisible "
                              u8"style=visibility:hidden>three</p>");
      CHECK(At(fixture.TextIn(u8"a"), 1).IsCandidate());
      CHECK(!At(fixture.TextIn(u8"hidden"), 1).IsCandidate());
      CHECK(!At(fixture.TextIn(u8"invisible"), 1).IsCandidate());
      CHECK(!At(fixture.ById(u8"a"), 0).IsCandidate());
    }

    SECTION("Line breaks and images are candidates before and after them")
    {
      PositionFixture fixture(u8"<p id=p>a<br id=br>b<img id=img style=width:10px;height:10px></p>");
      auto &br = fixture.ById(u8"br");
      auto &img = fixture.ById(u8"img");
      CHECK(PositionBeforeNode(&br).IsCandidate());
      CHECK(!PositionAfterNode(&br).IsCandidate());
      CHECK(PositionBeforeNode(&img).IsCandidate());
      CHECK(PositionAfterNode(&img).IsCandidate());
    }

    SECTION("An empty block is a candidate at its start")
    {
      PositionFixture fixture(u8"<div id=d style=height:20px></div>");
      CHECK(FirstPositionInNode(&fixture.ById(u8"d")).IsCandidate());
    }

    SECTION("user-select: none")
    {
      PositionFixture fixture(u8"<p id=p style=-webkit-user-select:none>abc</p>");
      CHECK(Position::NodeIsUserSelectNone(&fixture.TextIn(u8"p")));
      CHECK(!At(fixture.TextIn(u8"p"), 1).IsCandidate());
    }
  }

  TEST_CASE("Position: editable content", "[DOM][Position]")
  {
    PositionFixture fixture(
      u8"<p id=before>x</p><div id=e contenteditable><span id=s>ab</span></div><p id=after>y</p>");
    auto &editable = fixture.ById(u8"e");
    auto &span = fixture.ById(u8"s");
    auto &text = Downcast<Text>(*span.FirstChild());

    CHECK(editable.HasEditableStyle());
    CHECK(text.HasEditableStyle());
    CHECK(!fixture.TextIn(u8"before").HasEditableStyle());
    CHECK(editable.IsRootEditableElement());
    CHECK(!span.IsRootEditableElement());
    CHECK(text.RootEditableElement() == &editable);
    CHECK(fixture.TextIn(u8"before").RootEditableElement() == nullptr);
    CHECK(IsEditablePosition(At(text, 1)));
    CHECK(!IsEditablePosition(At(fixture.TextIn(u8"before"), 1)));
    CHECK(HighestEditableRoot(At(text, 1)).get() == &editable);
    CHECK(At(text, 0).ParentEditingBoundary().get() == &editable);

    // Moving into editable text stops at its boundary unless crossing is allowed.
    PositionFixture mixed(u8"<p id=p>a<span id=s contenteditable>b</span>c</p>");
    auto &a = mixed.TextIn(u8"p");
    CHECK(At(a, 1).Downstream() == At(a, 1));
    auto crossed = At(a, 1).Downstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary);
    CHECK(crossed.DeprecatedNode() == &mixed.ById(u8"s"));
    CHECK(crossed.DeprecatedNode()->HasEditableStyle());
    CHECK(At(a, 1).AtEditingBoundary());

    // contenteditable=false makes a non-editable island.
    PositionFixture island(u8"<div contenteditable><b id=b contenteditable=false>no</b></div>");
    CHECK(!island.ById(u8"b").HasEditableStyle());
    CHECK(island.ById(u8"b").ParentNode()->HasEditableStyle());
  }

  TEST_CASE("PositionIterator", "[DOM][Position]")
  {
    PositionFixture fixture(u8"<div id=d>ab<b id=b>c</b></div>");
    auto &d = fixture.ById(u8"d");
    auto &ab = *d.FirstChild();

    PositionIterator iterator(At(ab, 0));
    CHECK(iterator.AtStartOfNode());
    iterator.Increment();
    CHECK(iterator.GetNode() == &ab);
    CHECK(iterator.OffsetInLeafNode() == 1);
    CHECK(iterator.IsCandidate());
    iterator.Increment();
    CHECK(iterator.AtEndOfNode());
    iterator.Increment();
    // Past the text, before <b>.
    CHECK(iterator.GetNode() == &d);
    CHECK(Position(iterator).ComputeOffsetInContainerNode() == 1);
    iterator.Increment();
    CHECK(iterator.GetNode() == &fixture.ById(u8"b"));
    iterator.Decrement();
    CHECK(iterator.GetNode() == &d);
    iterator.Decrement();
    CHECK(iterator.GetNode() == &ab);
    CHECK(iterator.OffsetInLeafNode() == 2);

    CHECK(NextCandidate(At(ab, 2)).DeprecatedNode() == fixture.ById(u8"b").FirstChild());
    CHECK(PreviousCandidate(At(ab, 1)) == MakeDeprecatedLegacyPosition(ShareRefPtr(&ab), 0));
  }

  TEST_CASE("Position: inline boxes and directions", "[DOM][Position]")
  {
    PositionFixture fixture(u8"<p id=p>one two</p><p id=r dir=rtl>abc</p>");
    auto &text = fixture.TextIn(u8"p");
    auto [box, offset] = At(text, 2).GetInlineBoxAndOffset(Affinity::Downstream);
    REQUIRE(box);
    CHECK(box->IsText());
    CHECK(offset == 2);
    CHECK(At(text, 2).PrimaryDirection() == TextDirection::LTR);
    CHECK(At(fixture.TextIn(u8"r"), 1).PrimaryDirection() == TextDirection::RTL);
  }
}
