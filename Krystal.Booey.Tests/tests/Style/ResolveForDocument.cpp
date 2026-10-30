#include "Krystal.Booey/Style/ResolveForDocument.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/SVG/SVGDocument.hpp"

namespace krys::boo::style::tests
{
  TEST_CASE_METHOD(css::tests::ValueFixture, "ResolveForDocument", "[Style][ResolveForDocument]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto style = ResolveForDocument(*document);

    CHECK(style.Display() == DisplayType::Block);
    CHECK(style.RtlOrdering() == boo::Order::Logical);
    CHECK(style.GetUserModify() == UserModify::ReadOnly);
    CHECK(style.ComputedFontSize() == 16);
    CHECK(style.GetFontDescription().SpecifiedSize() == 16);
    CHECK(style.GetFontDescription().FirstFamily() == css::WebKitFontFamilyNames::StandardFamily());
    CHECK(style.GetFontDescription().KeywordSizeAsIdentifier() == css::ValueId::Medium);

    SECTION("The document's initial containing block style")
    {
      auto *initialContainingBlockStyle = document->InitialContainingBlockStyle();
      REQUIRE(initialContainingBlockStyle);
      CHECK(document->InitialContainingBlockStyle() == initialContainingBlockStyle);
      CHECK(*initialContainingBlockStyle == style);
    }

    SECTION("SVG documents use the SVG zoom rules")
    {
      auto svgDocument = CreateRef<svg::SVGDocument>();
      CHECK(ResolveForDocument(*svgDocument).ComputedFontSize() == 16);
    }
  }
}
