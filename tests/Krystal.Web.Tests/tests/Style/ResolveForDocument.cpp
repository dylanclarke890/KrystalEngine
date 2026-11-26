#include "Krystal.Web/Style/ResolveForDocument.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Web/Css/Font/GenericFontFamilyNames.hpp"
#include "Krystal.Web/Svg/SVGDocument.hpp"

namespace krys::web::style::tests
{
  TEST_CASE_METHOD(css::tests::ValueFixture, "ResolveForDocument", "[Style][ResolveForDocument]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto style = ResolveForDocument(*document);

    CHECK(style.Display() == DisplayType::Block);
    CHECK(style.UserModify() == UserModify::ReadOnly);
    CHECK(style.ComputedFontSize() == 16);
    CHECK(style.GetFontDescription().SpecifiedSize() == 16);
    CHECK(style.GetFontDescription().FirstFamily() == css::generic_font_family_names::StandardFamily());
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
      auto svgDocument = CreateRef<SVGDocument>();
      CHECK(ResolveForDocument(*svgDocument).ComputedFontSize() == 16);
    }
  }
}
