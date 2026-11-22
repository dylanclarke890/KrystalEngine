#include "Krystal.Booey/CSS/Values/TextShadowPropertyValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/TextDecoration/TextShadowProperty.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using L = Length<AllUnzoomed>;
    using NL = Length<NonNegativeUnzoomed>;

    KRYS_NODISCARD TextShadow FullTextShadow() noexcept
    {
      return TextShadow {.Color = Color {KeywordColor {ValueId::Blue}},
                         .Location = {L {1_css_px}, L {-2_css_px}},
                         .Blur = NL {3_css_px}};
    }
  }

  TEST_CASE_METHOD(ValueFixture, "TextShadowPropertyValue::Create", "[CSS][Values][TextShadowPropertyValue]")
  {
    TextShadowProperty shadow {TextShadowProperty::List {FullTextShadow()}};
    auto value = TextShadowPropertyValue::Create(shadow);
    CHECK(Is<TextShadowPropertyValue>(*value));
    CHECK(value->Shadow() == shadow);
  }

  TEST_CASE_METHOD(ValueFixture, "TextShadowPropertyValue serialisation",
                   "[CSS][Values][TextShadowPropertyValue]")
  {
    CHECK(CssText(*TextShadowPropertyValue::Create(TextShadowProperty {keywords::None {}})) == "none");
    CHECK(CssText(*TextShadowPropertyValue::Create(TextShadowProperty {TextShadowProperty::List {
            FullTextShadow(), TextShadow {.Location = {L {0_css_px}, L {0_css_px}}}}}))
          == "blue 1px -2px 3px, 0px 0px");
  }

  TEST_CASE_METHOD(ValueFixture, "TextShadowPropertyValue equality", "[CSS][Values][TextShadowPropertyValue]")
  {
    auto a =
      TextShadowPropertyValue::Create(TextShadowProperty {TextShadowProperty::List {FullTextShadow()}});
    CHECK(
      *a
      == *TextShadowPropertyValue::Create(TextShadowProperty {TextShadowProperty::List {FullTextShadow()}}));

    auto noBlur = FullTextShadow();
    noBlur.Blur = {};
    CHECK_FALSE(*a
                == *TextShadowPropertyValue::Create(TextShadowProperty {TextShadowProperty::List {noBlur}}));
    CHECK_FALSE(*a == *TextShadowPropertyValue::Create(TextShadowProperty {keywords::None {}}));
  }

  TEST_CASE_METHOD(ValueFixture, "TextShadowPropertyValue has no children",
                   "[CSS][Values][TextShadowPropertyValue]")
  {
    CHECK(Children(*TextShadowPropertyValue::Create(
                     TextShadowProperty {TextShadowProperty::List {FullTextShadow()}}))
            .empty());
  }
}
