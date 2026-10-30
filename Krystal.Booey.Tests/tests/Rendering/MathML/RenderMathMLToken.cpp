#include "Krystal.Booey/Rendering/MathML/RenderMathMLToken.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey/MathML/MathMLTokenElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD RenderMathMLToken &TokenById(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      auto *renderer = fixture.ById(id).Renderer();
      REQUIRE(renderer);
      return Downcast<RenderMathMLToken>(*renderer);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderMathMLToken: its text",
                   "[Rendering][MathML][RenderMathMLToken]")
  {
    ResolverFixture fixture {layout::tests::AhemDocument(
      u8"mn, mtext { font: 10px Ahem }",
      u8"<math><mn id=n>12</mn><mtext id=t>a<span>b</span></mtext><ms id=s></ms></math>")};

    SECTION("It's sized to its text")
    {
      auto &token = TokenById(fixture, u8"n");
      CHECK(token.LogicalWidth() == 20_lu);
      CHECK(token.MaxPreferredLogicalWidth() == 20_lu);
      CHECK(token.GetElement().HasTagName(html::TagName::mn));
    }

    SECTION("Its children can be any renderer")
    {
      auto &token = TokenById(fixture, u8"t");
      CHECK(token.LogicalWidth() == 20_lu);
      // An empty token has no size.
      CHECK(TokenById(fixture, u8"s").Size() == LayoutSize());
    }

    SECTION("Changing its text lays it out again")
    {
      CHECK(TokenById(fixture, u8"n").LogicalWidth() == 20_lu);
      (void)fixture.ById(u8"n").TextContent(u8"12345");
      CHECK(TokenById(fixture, u8"n").LogicalWidth() == 50_lu);
    }
  }
}
