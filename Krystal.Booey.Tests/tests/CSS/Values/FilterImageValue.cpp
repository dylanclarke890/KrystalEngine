#include "Krystal.Booey/CSS/Values/FilterImageValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/Filter.hpp"
#include "Krystal.Booey/CSS/Values/NamedImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    Filter BlurFilter(Blur::Parameter radius)
    {
      return Filter {FilterValueList {FilterValueList::Container {BlurFunction {.Parameters = {radius}}}}};
    }

    Filter SepiaFilter()
    {
      return Filter {FilterValueList {FilterValueList::Container {SepiaFunction {}}}};
    }

    Ref<FilterImageValue> FilterImage(utf8_stringview name, Filter filter)
    {
      return FilterImageValue::Create(NamedImageValue::Create(CSSOMString {name}), krys::move(filter));
    }
  }

  TEST_CASE_METHOD(ValueFixture, "FilterImageValue::Create", "[CSS][Values][FilterImageValue]")
  {
    auto value = FilterImage(u8"a", SepiaFilter());
    CHECK(value->IsFilterImageValue());
    CHECK(value->IsImageGeneratorValue());
    CHECK(value->IsImage());
  }

  TEST_CASE_METHOD(ValueFixture, "FilterImageValue serialisation", "[CSS][Values][FilterImageValue]")
  {
    CHECK(CssText(*FilterImage(u8"a", SepiaFilter())) == "filter(-webkit-named-image(a), sepia())");
    CHECK(CssText(*FilterImage(u8"a", BlurFilter(2_css_px))) == "filter(-webkit-named-image(a), blur(2px))");

    // The image can be `none`.
    CHECK(CssText(*FilterImageValue::Create(PrimitiveValue::Create(ValueId::None), SepiaFilter()))
          == "filter(none, sepia())");
  }

  TEST_CASE_METHOD(ValueFixture, "FilterImageValue equality", "[CSS][Values][FilterImageValue]")
  {
    auto a = FilterImage(u8"a", BlurFilter(2_css_px));
    CHECK(*a == *FilterImage(u8"a", BlurFilter(2_css_px)));
    CHECK_FALSE(*a == *FilterImage(u8"b", BlurFilter(2_css_px)));
    CHECK_FALSE(*a == *FilterImage(u8"a", BlurFilter(3_css_px)));
    CHECK_FALSE(*a == *FilterImage(u8"a", SepiaFilter()));

    CHECK(a->EqualInputImages(*FilterImage(u8"a", SepiaFilter())));
    CHECK_FALSE(a->EqualInputImages(*FilterImage(u8"b", BlurFilter(2_css_px))));

    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "FilterImageValue children", "[CSS][Values][FilterImageValue]")
  {
    // The filter has no values to visit, so only the image is visited.
    auto value = FilterImage(u8"a", BlurFilter(2_css_px));
    auto children = Children(*value);
    REQUIRE(children.size() == 1uz);
    CHECK(CssText(*children[0uz]) == "-webkit-named-image(a)");

    CHECK_FALSE(value->MayDependOnBaseUrl());
  }
}
