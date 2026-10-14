#include "Krystal.Booey/CSS/CSSOM/CSSFontFeatureValuesRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSFontFeatureValuesRule", "[CSS][CSSOM][CSSFontFeatureValuesRule]")
  {
    auto sheet =
      SheetFromText(u8"@font-feature-values Font One, \"Font Two\" { "
                    u8"@styleset { nice: 2 4; } @swash { fancy: 1; } @annotation { circled: 3; } } "
                    u8"@font-feature-values a { }");
    auto &rule = RuleAt<CSSFontFeatureValuesRule>(*sheet, 0);

    CHECK(TypeOf(rule) == RuleType::FontFeatureValues);
    REQUIRE(rule.FontFamilies().size() == 2);
    CHECK(ToNarrow(CSSOMString(rule.FontFamily().View())) == "Font One, Font Two");
    // Each block is written once, in Servo's order.
    CHECK(
      RuleText(rule)
      == "@font-feature-values \"Font One\", \"Font Two\" { @swash { fancy: 1; } @annotation { circled: 3; } "
         "@styleset { nice: 2 4; } }");

    CHECK(RuleText(*sheet->Item(1)) == "@font-feature-values a { }");
  }

  TEST_CASE("CSSFontFeatureValuesBlockRule", "[CSS][CSSOM][CSSFontFeatureValuesRule]")
  {
    auto block = FontFeatureValuesBlockRule::Create(FontFeatureValuesType::Styleset, {});
    auto wrapper = block->CreateCSSOMWrapper();
    CHECK(TypeOf(*wrapper) == RuleType::FontFeatureValuesBlock);
    CHECK(Is<CSSFontFeatureValuesBlockRule>(*wrapper));
  }
}
