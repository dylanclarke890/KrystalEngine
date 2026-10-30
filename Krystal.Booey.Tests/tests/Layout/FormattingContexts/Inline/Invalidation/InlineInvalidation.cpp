#include "Krystal.Booey/Layout/FormattingContexts/Inline/Invalidation/InlineInvalidation.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"

namespace krys::boo::layout::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    constexpr auto Style = u8"#d { width: 50px }";

    KRYS_NODISCARD RenderBlockFlow &Block(ResolverFixture &fixture) noexcept
    {
      return Downcast<RenderBlockFlow>(*fixture.ById(u8"d").Renderer());
    }

    KRYS_NODISCARD dom::Text &TextIn(ResolverFixture &fixture) noexcept
    {
      return Downcast<dom::Text>(*fixture.ById(u8"d").FirstChild());
    }

    struct Mutation
    {
      bool KeptLineLayout {false};
      Maybe<InlineDamage::LayoutPosition> StartPosition;
      EnumSet<InlineDamage::Reason> Reasons;
    };

    /// @brief Lays out @p before, applies @p mutate, records the damage, lays out again and checks the result
    /// against a fresh layout of @p after.
    template <typename TMutate>
    KRYS_NODISCARD Mutation Mutate(utf8_stringview before, utf8_stringview after, TMutate mutate)
    {
      ResolverFixture fixture {AhemDocument(Style, before)};
      (void)fixture.Document->UpdateLayout();
      auto *lineLayout = Block(fixture).InlineLayout();
      REQUIRE(lineLayout);

      mutate(fixture);
      fixture.Document->UpdateStyleIfNeeded();
      auto mutation = Mutation {};
      mutation.KeptLineLayout = Block(fixture).InlineLayout() == lineLayout;
      if (mutation.KeptLineLayout)
      {
        if (auto *damage = lineLayout->Damage())
        {
          mutation.StartPosition = damage->LayoutStartPosition();
          mutation.Reasons = damage->Reasons();
        }
      }

      (void)fixture.Document->UpdateLayout();
      ResolverFixture fresh {AhemDocument(Style, after)};
      CHECK(DumpWithIds(fixture) == DumpWithIds(fresh));
      return mutation;
    }

    KRYS_NODISCARD bool StartsAt(const Mutation &mutation, size_t lineIndex, InlineItemPosition position,
                                 float partialContentTop) noexcept
    {
      return mutation.StartPosition && mutation.StartPosition->LineIndex == lineIndex
             && mutation.StartPosition->InlineItemPos == position
             && mutation.StartPosition->PartialContentTop == LayoutUnit {partialContentTop};
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineInvalidation: text changes lay out from the damaged line",
                   "[Layout][InlineInvalidation]")
  {
    // "aa bb" / "cc dd": text inserted in the second line starts layout at the line before it, at the start
    // of the first line's text item.
    SECTION("Appended text dirties the last line")
    {
      auto mutation = Mutate(u8"<div id=d>aa bb cc dd</div>", u8"<div id=d>aa bb cc dd ee</div>",
                             [](ResolverFixture &fixture) { (void)TextIn(fixture).AppendData(u8" ee"); });
      CHECK(mutation.KeptLineLayout);
      CHECK(mutation.Reasons == EnumSet<InlineDamage::Reason> {InlineDamage::Reason::Append});
      // The second line starts at "cc", the fifth inline item.
      CHECK(StartsAt(mutation, 1, {4, 0}, 10.f));
    }

    SECTION("Inserted text")
    {
      auto mutation = Mutate(u8"<div id=d>aa bb cc dd</div>", u8"<div id=d>aa bb xx cc dd</div>",
                             [](ResolverFixture &fixture) { (void)TextIn(fixture).InsertData(6, u8"xx "); });
      CHECK(mutation.KeptLineLayout);
      CHECK(mutation.Reasons == EnumSet<InlineDamage::Reason> {InlineDamage::Reason::ContentChange});
      CHECK(StartsAt(mutation, 0, {0, 0}, 0.f));
    }

    SECTION("Removed text starts at the damaged line itself")
    {
      auto mutation = Mutate(u8"<div id=d>aa bb cc dd ee ff</div>", u8"<div id=d>aa bb cc ee ff</div>",
                             [](ResolverFixture &fixture) { (void)TextIn(fixture).DeleteData(9, 3); });
      CHECK(mutation.KeptLineLayout);
      CHECK(mutation.Reasons == EnumSet<InlineDamage::Reason> {InlineDamage::Reason::Remove});
      // The second line starts at "cc", the fifth inline item.
      CHECK(StartsAt(mutation, 1, {4, 0}, 10.f));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineInvalidation: inserted and removed boxes",
                   "[Layout][InlineInvalidation]")
  {
    SECTION("Appended boxes dirty the last line")
    {
      auto mutation =
        Mutate(u8"<div id=d>aa bb cc dd</div>", u8"<div id=d>aa bb cc dd<br>ee</div>",
               [](ResolverFixture &fixture)
               {
                 auto &block = fixture.ById(u8"d");
                 (void)block.AppendChild(*fixture.Document->CreateElement(u8"br").ReleaseValue());
                 (void)block.AppendChild(*fixture.Document->CreateTextNode(u8"ee"));
               });
      CHECK(mutation.KeptLineLayout);
      CHECK(mutation.Reasons == EnumSet<InlineDamage::Reason> {InlineDamage::Reason::Append});
      CHECK(StartsAt(mutation, 1, {4, 0}, 10.f));
    }

    SECTION("A removed line break")
    {
      // The comment keeps the fresh document's text in two nodes.
      auto mutation = Mutate(u8"<div id=d>aa<br>bb</div>", u8"<div id=d>aa<!---->bb</div>",
                             [](ResolverFixture &fixture)
                             {
                               auto &block = fixture.ById(u8"d");
                               (void)block.RemoveChild(*block.FirstChild()->NextSibling());
                             });
      CHECK(mutation.KeptLineLayout);
      CHECK(mutation.Reasons == EnumSet<InlineDamage::Reason> {InlineDamage::Reason::Remove});
      CHECK(StartsAt(mutation, 0, {0, 0}, 0.f));
    }

    SECTION("Content appended to an inline box that was laid out isn't supported")
    {
      auto mutation = Mutate(
        u8"<div id=d>aa bb<span></span></div>", u8"<div id=d>aa bb<span>cc</span></div>",
        [](ResolverFixture &fixture)
        { (void)fixture.ById(u8"d").LastChild()->AppendChild(*fixture.Document->CreateTextNode(u8"cc")); });
      CHECK_FALSE(mutation.KeptLineLayout);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineInvalidation: changes that need a full layout",
                   "[Layout][InlineInvalidation]")
  {
    SECTION("A style change that affects layout keeps the line layout but lays out everything")
    {
      auto mutation = Mutate(u8"<div id=d>aa <span id=s>bb</span> cc dd</div>",
                             u8"<div id=d>aa <span id=s style=\"font-size: 20px\">bb</span> cc dd</div>",
                             [](ResolverFixture &fixture)
                             { (void)fixture.ById(u8"s").SetAttribute(u8"style", u8"font-size: 20px"); });
      CHECK(mutation.KeptLineLayout);
      CHECK(mutation.Reasons == EnumSet<InlineDamage::Reason> {InlineDamage::Reason::StyleChange});
      CHECK_FALSE(mutation.StartPosition);
    }

    SECTION("A root style change")
    {
      auto mutation =
        Mutate(u8"<div id=d>aa bb cc dd</div>", u8"<div id=d style=\"font-size: 5px\">aa bb cc dd</div>",
               [](ResolverFixture &fixture)
               { (void)fixture.ById(u8"d").SetAttribute(u8"style", u8"font-size: 5px"); });
      CHECK(mutation.KeptLineLayout);
      CHECK_FALSE(mutation.StartPosition);
    }

    SECTION("Content that partial layout doesn't support drops the line layout")
    {
      auto mutation = Mutate(u8"<div id=d style=\"text-wrap: balance\">aa bb cc dd</div>",
                             u8"<div id=d style=\"text-wrap: balance\">aa bb cc dd ee</div>",
                             [](ResolverFixture &fixture) { (void)TextIn(fixture).AppendData(u8" ee"); });
      CHECK_FALSE(mutation.KeptLineLayout);
    }
  }
}
