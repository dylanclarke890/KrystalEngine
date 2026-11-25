#include "Krystal.Booey/CSS/Highlights/Highlight.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/CSS/Geometry/DOMRectList.hpp"
#include "Krystal.Booey/CSS/Highlights/HighlightRegistry.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/DOM/Ranges/Range.hpp"
#include "Krystal.Booey/DOM/Ranges/StaticRange.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<StaticRange> MakeStaticRange(Node &startContainer, uint64 startOffset,
                                                    Node &endContainer, uint64 endOffset) noexcept
    {
      auto init =
        StaticRangeInit::Create({ShareRef(startContainer), startOffset}, {ShareRef(endContainer), endOffset});
      REQUIRE_FALSE(init.HasException());
      return CreateRef<StaticRange>(init.ReleaseValue());
    }

    /// @brief Whether @p painted has @p fragment.
    KRYS_NODISCARD bool Has(const utf8_string &painted, utf8_stringview fragment) noexcept
    {
      return painted.contains(fragment);
    }
  }

  TEST_CASE("Highlight: a set of ranges",
            "[CSS][Highlight][css-highlight-api-1:creation][css-highlight-api-1:dom-highlight-highlight]"
            "[css-highlight-api-1:dom-highlight-priority][css-highlight-api-1:dom-highlight-type]"
            "[css-highlight-api-1:highlight-types][css-highlight-api-1:custom-highlight]"
            "[css-highlight-api-1:highlight][css-highlight-api-1:priority]")
  {
    auto document = CreateRef<Document>();
    auto text = document->CreateTextNode(u8"Some text");
    auto first = MakeStaticRange(*text, 0, *text, 4);
    auto second = MakeStaticRange(*text, 5, *text, 9);

    // The initial ranges are added as the set's add() adds them, so a repeat is dropped.
    auto highlight = Highlight::Create({first, second, first});
    CHECK(highlight->Size() == 2);
    CHECK(highlight->Has(*first));
    CHECK(highlight->Priority() == 0);
    CHECK(highlight->GetType() == Highlight::Type::Highlight);

    // Adding a range already there leaves it where it is.
    CHECK_FALSE(highlight->Add(*first));
    CHECK(&highlight->HighlightRanges().front()->Range() == first.get());
    CHECK(&highlight->HighlightRanges().back()->Range() == second.get());

    CHECK(highlight->Delete(*second));
    CHECK_FALSE(highlight->Delete(*second));
    CHECK(highlight->Size() == 1);

    highlight->SetPriority(3);
    CHECK(highlight->Priority() == 3);
    highlight->SetType(Highlight::Type::SpellingError);
    CHECK(highlight->GetType() == Highlight::Type::SpellingError);

    highlight->Clear();
    CHECK(highlight->Size() == 0);
  }

  TEST_CASE("HighlightRegistry: highlights by name, in order",
            "[CSS][Highlight][css-highlight-api-1:registration][css-highlight-api-1:registered]"
            "[css-highlight-api-1:highlight-registry][css-highlight-api-1:custom-highlight-name]")
  {
    auto document = CreateRef<Document>();
    CHECK_FALSE(document->HasHighlight());
    auto &registry = document->GetHighlightRegistry();
    CHECK_FALSE(document->HasHighlight());

    auto text = document->CreateTextNode(u8"Text");
    registry.Set(u8"b", Highlight::Create({MakeStaticRange(*text, 0, *text, 1)}));
    registry.Set(u8"a", Highlight::Create());
    CHECK(document->HasHighlight());
    REQUIRE(registry.HighlightNames().size() == 2);
    CHECK(registry.HighlightNames()[0] == u8"b");

    // Setting a name again keeps its place.
    auto replacement = Highlight::Create();
    registry.Set(u8"b", Ref<Highlight>(replacement));
    CHECK(registry.HighlightNames()[0] == u8"b");
    CHECK(registry.Get(u8"b") == replacement.get());

    CHECK(registry.Delete(u8"b"));
    CHECK_FALSE(registry.Delete(u8"b"));
    CHECK_FALSE(registry.Has(u8"b"));
    CHECK(registry.Size() == 1);

    // Clearing the registry unregisters the highlights, which keep their ranges.
    auto kept = Highlight::Create({MakeStaticRange(*text, 1, *text, 2)});
    registry.Set(u8"kept", Ref<Highlight>(kept));
    registry.Clear();
    CHECK(registry.IsEmpty());
    CHECK(registry.HighlightNames().empty());
    CHECK(kept->Size() == 1);
    CHECK_FALSE(document->HasHighlight());
  }

  TEST_CASE("Highlight: ::highlight() paints its ranges",
            "[CSS][Highlight][Rendering][css-highlight-api-1:custom-highlight-pseudo]"
            "[css-highlight-api-1:custom-highlight-pseudo-element][css-highlight-api-1:repaint]")
  {
    boo::tests::PaintFixture fixture(
      u8"<!DOCTYPE html><style>body{margin:0;font:16px sans-serif} ::highlight(mark) { background-color: "
      u8"#ffd166; color: #073b4c }</style><p id=p>Hello highlighted world</p>");
    auto &text = *fixture.ById(u8"p").FirstChild();

    auto unhighlighted = fixture.Painted();
    CHECK_FALSE(Has(unhighlighted, u8"#FFD166"));

    fixture.Document->GetHighlightRegistry().Set(u8"mark",
                                                 Highlight::Create({MakeStaticRange(text, 6, text, 17)}));
    fixture.Document->UpdateHighlightPositions();
    auto painted = fixture.Painted();
    CHECK(Has(painted, u8"#FFD166"));
    CHECK(Has(painted, u8"#073B4C"));

    // A highlight without a ::highlight() style isn't painted.
    fixture.Document->GetHighlightRegistry().Clear();
    fixture.Document->GetHighlightRegistry().Set(u8"other",
                                                 Highlight::Create({MakeStaticRange(text, 0, text, 5)}));
    fixture.Document->UpdateHighlightPositions();
    CHECK(fixture.Painted() == unhighlighted);
  }

  TEST_CASE("Highlight: a live range's highlight follows it",
            "[CSS][Highlight][Rendering][css-highlight-api-1:range-invalidation]")
  {
    boo::tests::PaintFixture fixture(
      u8"<!DOCTYPE html><style>body{margin:0;font:16px sans-serif} "
      u8"::highlight(live) { background-color: #ef476f }</style><p id=p>Live</p>");
    auto &text = *fixture.ById(u8"p").FirstChild();
    auto range = CreateRef<Range>(BoundaryPoint {ShareRef(text), 0}, BoundaryPoint {ShareRef(text), 0});
    fixture.Document->GetHighlightRegistry().Set(u8"live", Highlight::Create({range}));
    fixture.Document->UpdateHighlightPositions();
    // A collapsed range highlights nothing.
    CHECK_FALSE(Has(fixture.Painted(), u8"#EF476F"));

    REQUIRE_FALSE(range->SetEnd(text, 4).HasException());
    fixture.Document->UpdateHighlightPositions();
    CHECK(Has(fixture.Painted(), u8"#EF476F"));
  }

  TEST_CASE("Highlight: the text fragments' highlight paints with ::target-text",
            "[CSS][Highlight][Rendering]")
  {
    boo::tests::PaintFixture fixture(
      u8"<!DOCTYPE html><style>body{margin:0;font:16px sans-serif} "
      u8"::target-text { background-color: #06d6a0 }</style><p id=p>Target</p>");
    auto &text = *fixture.ById(u8"p").FirstChild();
    fixture.Document->FragmentHighlightRegistry().AddAnnotationHighlightWithRange(
      MakeStaticRange(text, 0, text, 6));
    fixture.Document->UpdateHighlightPositions();
    CHECK(Has(fixture.Painted(), u8"#06D6A0"));
  }

  TEST_CASE("Highlight: overlapping highlights paint by priority, then registration",
            "[CSS][Highlight][Rendering][css-highlight-api-1:priorities]")
  {
    boo::tests::PaintFixture fixture(
      u8"<!DOCTYPE html><style>body{margin:0;font:16px sans-serif} ::highlight(first) { background-color: "
      u8"#118ab2 } ::highlight(second) { background-color: #8338ec }</style><p id=p>Overlap</p>");
    auto &text = *fixture.ById(u8"p").FirstChild();
    auto first = Highlight::Create({MakeStaticRange(text, 0, text, 7)});
    auto second = Highlight::Create({MakeStaticRange(text, 0, text, 7)});
    auto &registry = fixture.Document->GetHighlightRegistry();
    registry.Set(u8"first", Ref<Highlight>(first));
    registry.Set(u8"second", Ref<Highlight>(second));

    // The highlights cover the same text, so only the background of the one on top is painted.
    auto paintedOnTop = [&](utf8_stringview upper, utf8_stringview lower)
    {
      fixture.Document->UpdateHighlightPositions();
      auto painted = fixture.Painted();
      return Has(painted, upper) && !Has(painted, lower);
    };

    // Of equal priorities, the one registered last is on top; a higher priority is on top whatever the order.
    CHECK(paintedOnTop(u8"#8338EC", u8"#118AB2"));
    first->SetPriority(1);
    CHECK(paintedOnTop(u8"#118AB2", u8"#8338EC"));
  }

  TEST_CASE("Highlight: ranges outside the document aren't painted",
            "[CSS][Highlight][Rendering][css-highlight-api-1:range-invalidation]")
  {
    boo::tests::PaintFixture fixture(
      u8"<!DOCTYPE html><style>body{margin:0;font:16px sans-serif} "
      u8"::highlight(elsewhere) { background-color: #fb5607 }</style><p id=p>Here</p>");
    auto unhighlighted = fixture.Painted();

    // A range in a node that isn't in the document.
    auto detached = fixture.Document->CreateTextNode(u8"There");
    fixture.Document->GetHighlightRegistry().Set(
      u8"elsewhere", Highlight::Create({MakeStaticRange(*detached, 0, *detached, 5)}));
    fixture.Document->UpdateHighlightPositions();
    CHECK(fixture.Painted() == unhighlighted);
  }

  TEST_CASE("HighlightRegistry: highlights from a point",
            "[CSS][Highlight][Rendering][css-highlight-api-1:dom-highlightregistry-highlightsfrompoint]"
            "[css-highlight-api-1:interactions]")
  {
    boo::tests::PaintFixture fixture(
      u8"<!DOCTYPE html><style>body{margin:0;font:16px sans-serif}</style><p id=p>abcd efgh</p>"
      u8"<div id=host></div>");
    auto &text = *fixture.ById(u8"p").FirstChild();
    auto shadowRoot = fixture.ById(u8"host").AttachShadow({}).ReleaseValue();
    auto shadowText = fixture.Document->CreateTextNode(u8"Shadow text");
    REQUIRE_FALSE(shadowRoot->AppendChild(*shadowText).HasException());
    (void)fixture.Painted();

    auto ab = MakeStaticRange(text, 0, text, 2);
    auto b = MakeStaticRange(text, 1, text, 2);
    auto efgh = MakeStaticRange(text, 5, text, 9);
    auto lower = Highlight::Create({ab, efgh});
    auto upper = Highlight::Create({b});
    lower->SetPriority(1);
    upper->SetPriority(2);
    auto &registry = fixture.Document->GetHighlightRegistry();
    registry.Set(u8"lower", Ref<Highlight>(lower));
    registry.Set(u8"upper", Ref<Highlight>(upper));

    // The middle of a range's first box.
    auto centre = [](const AbstractRange &range)
    {
      auto rects = CreateRef<Range>(BoundaryPoint {ShareRef(*range.StartContainer()), range.StartOffset()},
                                    BoundaryPoint {ShareRef(*range.EndContainer()), range.EndOffset()})
                     ->GetClientRects();
      REQUIRE(rects->Length() > 0);
      auto &rect = *rects->Item(0);
      return std::pair {static_cast<float>(rect.X() + rect.Width() / 2),
                        static_cast<float>(rect.Y() + rect.Height() / 2)};
    };

    // Each highlight with a range at the point, highest priority first, with only its ranges there.
    auto [bx, by] = centre(*b);
    auto atB = registry.HighlightsFromPoint(bx, by);
    REQUIRE(atB.size() == 2);
    CHECK(atB[0].Highlight.get() == upper.get());
    CHECK(atB[1].Highlight.get() == lower.get());
    REQUIRE(atB[1].Ranges.size() == 1);
    CHECK(atB[1].Ranges[0].get() == ab.get());

    auto [ex, ey] = centre(*efgh);
    auto atEfgh = registry.HighlightsFromPoint(ex, ey);
    REQUIRE(atEfgh.size() == 1);
    CHECK(atEfgh[0].Ranges[0].get() == efgh.get());

    // Nothing outside the viewport.
    CHECK(registry.HighlightsFromPoint(-1, by).empty());

    // A point on a shadow tree finds nothing unless its shadow root is given.
    auto shadowRange = MakeStaticRange(*shadowText, 0, *shadowText, 6);
    registry.Set(u8"shadow", Highlight::Create({shadowRange}));
    auto [sx, sy] = centre(*shadowRange);
    CHECK(registry.HighlightsFromPoint(sx, sy).empty());
    auto withShadowRoot = registry.HighlightsFromPoint(sx, sy, {.ShadowRoots = {shadowRoot}});
    REQUIRE(withShadowRoot.size() == 1);
    CHECK(withShadowRoot[0].Ranges[0].get() == shadowRange.get());
  }
}
