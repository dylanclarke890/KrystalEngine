#include "Krystal.Booey/DOM/Highlight.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/HighlightRegistry.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/StaticRange.hpp"
#include "Krystal.Booey/DOM/Text.hpp"

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
      return painted.find(fragment) != utf8_string::npos;
    }
  }

  TEST_CASE("Highlight: a set of ranges", "[DOM][Highlight]")
  {
    auto document = CreateRef<Document>();
    auto text = document->CreateTextNode(u8"Some text");
    auto first = MakeStaticRange(*text, 0, *text, 4);
    auto second = MakeStaticRange(*text, 5, *text, 9);

    auto highlight = Highlight::Create({first, second});
    CHECK(highlight->Size() == 2);
    CHECK(highlight->Has(*first));

    // Adding a range already there moves it to the end.
    CHECK_FALSE(highlight->Add(*first));
    CHECK(&highlight->HighlightRanges().back()->Range() == first.get());

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

  TEST_CASE("HighlightRegistry: highlights by name, in order", "[DOM][Highlight]")
  {
    auto document = CreateRef<Document>();
    CHECK_FALSE(document->HasHighlight());
    auto &registry = document->GetHighlightRegistry();
    CHECK_FALSE(document->HasHighlight());

    auto text = document->CreateTextNode(u8"Text");
    registry.Set(DOMStringAtom(u8"b"), Highlight::Create({MakeStaticRange(*text, 0, *text, 1)}));
    registry.Set(DOMStringAtom(u8"a"), Highlight::Create());
    CHECK(document->HasHighlight());
    REQUIRE(registry.HighlightNames().size() == 2);
    CHECK(registry.HighlightNames()[0] == DOMStringAtom(u8"b"));

    // Setting a name again keeps its place.
    auto replacement = Highlight::Create();
    registry.Set(DOMStringAtom(u8"b"), Ref<Highlight>(replacement));
    CHECK(registry.HighlightNames()[0] == DOMStringAtom(u8"b"));
    CHECK(registry.Get(DOMStringAtom(u8"b")) == replacement.get());

    CHECK(registry.Delete(DOMStringAtom(u8"b")));
    CHECK_FALSE(registry.Has(DOMStringAtom(u8"b")));
    CHECK(registry.Size() == 1);
    registry.Clear();
    CHECK(registry.IsEmpty());
    CHECK_FALSE(document->HasHighlight());
  }

  TEST_CASE("Highlight: ::highlight() paints its ranges", "[DOM][Highlight][Rendering]")
  {
    boo::tests::PaintFixture fixture(
      u8"<!DOCTYPE html><style>body{margin:0;font:16px sans-serif} ::highlight(mark) { background-color: "
      u8"#ffd166; color: #073b4c }</style><p id=p>Hello highlighted world</p>");
    auto &text = *fixture.ById(u8"p").FirstChild();

    auto unhighlighted = fixture.Painted();
    CHECK_FALSE(Has(unhighlighted, u8"#FFD166"));

    fixture.Document->GetHighlightRegistry().Set(DOMStringAtom(u8"mark"),
                                                 Highlight::Create({MakeStaticRange(text, 6, text, 17)}));
    fixture.Document->UpdateHighlightPositions();
    auto painted = fixture.Painted();
    CHECK(Has(painted, u8"#FFD166"));
    CHECK(Has(painted, u8"#073B4C"));

    // A highlight without a ::highlight() style isn't painted.
    fixture.Document->GetHighlightRegistry().Clear();
    fixture.Document->GetHighlightRegistry().Set(DOMStringAtom(u8"other"),
                                                 Highlight::Create({MakeStaticRange(text, 0, text, 5)}));
    fixture.Document->UpdateHighlightPositions();
    CHECK(fixture.Painted() == unhighlighted);
  }

  TEST_CASE("Highlight: a live range's highlight follows it", "[DOM][Highlight][Rendering]")
  {
    boo::tests::PaintFixture fixture(
      u8"<!DOCTYPE html><style>body{margin:0;font:16px sans-serif} "
      u8"::highlight(live) { background-color: #ef476f }</style><p id=p>Live</p>");
    auto &text = *fixture.ById(u8"p").FirstChild();
    auto range = CreateRef<Range>(BoundaryPoint {ShareRef(text), 0}, BoundaryPoint {ShareRef(text), 0});
    fixture.Document->GetHighlightRegistry().Set(DOMStringAtom(u8"live"), Highlight::Create({range}));
    fixture.Document->UpdateHighlightPositions();
    // A collapsed range highlights nothing.
    CHECK_FALSE(Has(fixture.Painted(), u8"#EF476F"));

    REQUIRE_FALSE(range->SetEnd(text, 4).HasException());
    fixture.Document->UpdateHighlightPositions();
    CHECK(Has(fixture.Painted(), u8"#EF476F"));
  }

  TEST_CASE("Highlight: the text fragments' highlight paints with ::target-text",
            "[DOM][Highlight][Rendering]")
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
}
