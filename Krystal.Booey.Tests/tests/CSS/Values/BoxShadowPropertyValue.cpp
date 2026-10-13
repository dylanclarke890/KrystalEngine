#include "Krystal.Booey/CSS/Values/BoxShadowPropertyValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BoxShadowProperty.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using L = Length<AllUnzoomed>;
    using NL = Length<NonNegativeUnzoomed>;

    KRYS_NODISCARD BoxShadow Shadow(L x, L y) noexcept
    {
      return BoxShadow {.Location = {x, y}};
    }

    KRYS_NODISCARD BoxShadow FullShadow() noexcept
    {
      return BoxShadow {.Color = Color {KeywordColor {ValueId::Red}},
                        .Location = {L {1_css_px}, L {2_css_px}},
                        .Blur = NL {3_css_px},
                        .Spread = L {4_css_px},
                        .Inset = keywords::Inset {}};
    }
  }

  TEST_CASE_METHOD(ValueFixture, "BoxShadowPropertyValue::Create", "[CSS][Values][BoxShadowPropertyValue]")
  {
    BoxShadowProperty shadow {BoxShadowProperty::List {FullShadow()}};
    auto value = BoxShadowPropertyValue::Create(shadow);
    CHECK(value->IsBoxShadowPropertyValue());
    CHECK(value->Shadow() == shadow);
  }

  TEST_CASE_METHOD(ValueFixture, "BoxShadowPropertyValue serialisation",
                   "[CSS][Values][BoxShadowPropertyValue]")
  {
    CHECK(CssText(*BoxShadowPropertyValue::Create(BoxShadowProperty {keywords::None {}})) == "none");
    CHECK(CssText(*BoxShadowPropertyValue::Create(
            BoxShadowProperty {BoxShadowProperty::List {Shadow(L {1_css_px}, L {2_css_px}), FullShadow()}}))
          == "1px 2px, red 1px 2px 3px 4px inset");
  }

  TEST_CASE_METHOD(ValueFixture, "BoxShadowPropertyValue equality", "[CSS][Values][BoxShadowPropertyValue]")
  {
    auto a = BoxShadowPropertyValue::Create(BoxShadowProperty {BoxShadowProperty::List {FullShadow()}});
    CHECK(*a == *BoxShadowPropertyValue::Create(BoxShadowProperty {BoxShadowProperty::List {FullShadow()}}));
    CHECK_FALSE(*a
                == *BoxShadowPropertyValue::Create(
                  BoxShadowProperty {BoxShadowProperty::List {Shadow(L {1_css_px}, L {2_css_px})}}));
    CHECK_FALSE(*a == *BoxShadowPropertyValue::Create(BoxShadowProperty {keywords::None {}}));
  }

  TEST_CASE_METHOD(ValueFixture, "BoxShadowPropertyValue has no children",
                   "[CSS][Values][BoxShadowPropertyValue]")
  {
    CHECK(
      Children(*BoxShadowPropertyValue::Create(BoxShadowProperty {BoxShadowProperty::List {FullShadow()}}))
        .empty());
  }
}
