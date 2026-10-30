#include "Krystal.Booey/Rendering/MathML/RenderMathMLSpace.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD RenderBox &SpaceBox(ResolverFixture &fixture) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      auto *renderer = fixture.ById(u8"s").Renderer();
      REQUIRE(renderer);
      return Downcast<RenderMathMLSpace>(*renderer);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLSpace: its size and baseline",
                   "[Rendering][MathML][RenderMathMLSpace]")
  {
    SECTION("Width, height and depth")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"", u8"<math><mspace id=s width=2em height=10px depth=5px></mspace></math>")};
      auto &space = SpaceBox(fixture);
      // The em is the font size the formula inherits, 10px.
      CHECK(space.Size() == LayoutSize(20, 15));
      CHECK(space.FirstLineBaseline() == 10_lu);
      CHECK(space.MaxPreferredLogicalWidth() == 20_lu);
      CHECK(space.MinPreferredLogicalWidth() == 20_lu);
    }

    SECTION("No attributes, or invalid ones, give an empty space")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"", u8"<math><mspace id=s width=10 height=\"5 px\" depth=-></mspace></math>")};
      auto &space = SpaceBox(fixture);
      CHECK(space.Size() == LayoutSize());
      CHECK(space.FirstLineBaseline() == 0_lu);
    }

    SECTION("Negative widths are ignored, and negative totals zero the height")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"", u8"<math><mspace id=s width=-10px height=5px depth=-10px></mspace></math>")};
      auto &space = SpaceBox(fixture);
      CHECK(space.Size() == LayoutSize());
      CHECK(space.FirstLineBaseline() == 0_lu);
    }

    SECTION("A negative depth shortens the height")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"", u8"<math><mspace id=s width=10px height=20px depth=-5px></mspace></math>")};
      auto &space = SpaceBox(fixture);
      CHECK(space.Size() == LayoutSize(10, 15));
      CHECK(space.FirstLineBaseline() == 20_lu);
    }

    SECTION("Border and padding surround the space")
    {
      ResolverFixture fixture {layout::tests::AhemDocument(
        u8"",
        u8"<math><mspace id=s width=10px height=20px depth=5px style=\"padding: 1px; border: 2px solid\">"
        u8"</mspace></math>")};
      auto &space = SpaceBox(fixture);
      CHECK(space.Size() == LayoutSize(16, 31));
      CHECK(space.FirstLineBaseline() == 23_lu);
      CHECK(space.MaxPreferredLogicalWidth() == 16_lu);
    }

    SECTION("Changing an attribute lays the space out again")
    {
      ResolverFixture fixture {
        layout::tests::AhemDocument(u8"", u8"<math><mspace id=s width=10px height=20px></mspace></math>")};
      CHECK(SpaceBox(fixture).Size() == LayoutSize(10, 20));
      fixture.ById(u8"s").SetAttribute(u8"width", u8"25px");
      fixture.ById(u8"s").SetAttribute(u8"depth", u8"4px");
      CHECK(SpaceBox(fixture).Size() == LayoutSize(25, 24));
    }
  }
}
