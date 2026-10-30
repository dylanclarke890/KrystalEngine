#include "Krystal.Booey/Style/ContainerQueryEvaluator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    KRYS_NODISCARD std::string HeightOf(ResolverFixture &fixture, dom::DOMStringView id)
    {
      (void)fixture.Document->UpdateLayout();
      auto *style = fixture.ById(id).GetRenderStyle();
      REQUIRE(style);
      return ResolverFixture::Serialised(*style, style->Height());
    }

    KRYS_NODISCARD std::string WidthOf(ResolverFixture &fixture, dom::DOMStringView id)
    {
      (void)fixture.Document->UpdateLayout();
      auto *style = fixture.ById(id).GetRenderStyle();
      REQUIRE(style);
      return ResolverFixture::Serialised(*style, style->Width());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "ContainerQueryEvaluator: size queries",
                   "[Style][ContainerQueryEvaluator]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      body { margin: 0 }
      #c { container-type: inline-size; width: 50% }
      #s { container-type: size; width: 100px; height: 50px }
      @container (width > 150px) { #p { height: 5px } }
      @container (width <= 150px) { #p { height: 7px } }
      @container (orientation: landscape) and (aspect-ratio > 1) { #q { height: 3px } }
      @container (height > 100px) { #q { width: 1px } }
      @container (inline-size > 0px) { #i { height: 1px } }
      #u { width: 25cqw; height: 10cqh }
    </style><div id=c><p id=p></p><p id=u></p></div><div id=s><p id=q></p></div><div><p id=i></p></div>)"};

    // The container is half the viewport's width.
    CHECK(HeightOf(fixture, u8"p") == "5px");
    CHECK(WidthOf(fixture, u8"u") == "100px");

    // Container units on an axis with no container use the small viewport's size.
    CHECK(HeightOf(fixture, u8"u") == "60px");

    // A size container answers height, orientation and aspect-ratio queries.
    CHECK(HeightOf(fixture, u8"q") == "3px");
    CHECK(WidthOf(fixture, u8"q") == "auto");

    // Without a container, a size query is unknown, so it doesn't match.
    CHECK(HeightOf(fixture, u8"i") == "auto");

    SECTION("The container's style changes")
    {
      REQUIRE_FALSE(fixture.ById(u8"c").SetAttribute(u8"style", u8"width: 100px").HasException());
      CHECK(HeightOf(fixture, u8"p") == "7px");
      CHECK(WidthOf(fixture, u8"u") == "25px");
    }

    SECTION("The container's size changes with the viewport")
    {
      fixture.Frame->View()->Resize({200, 600});
      CHECK(HeightOf(fixture, u8"p") == "7px");
      CHECK(WidthOf(fixture, u8"u") == "25px");
    }
  }
}
