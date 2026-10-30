#include "Krystal.Booey/CSS/Values/AppleColorFilterValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    AppleColorFilter Filters(AppleColorFilterValueList::Container &&values)
    {
      return AppleColorFilter {AppleColorFilterValueList {krys::move(values)}};
    }

    SepiaFunction HalfSepia()
    {
      return SepiaFunction {
        .Parameters = {Sepia::Parameter {Percentage<ClosedPercentageRangeClampUpper> {50.0}}}};
    }
  }

  TEST_CASE_METHOD(ValueFixture, "AppleColorFilterValue::Create", "[CSS][Values][AppleColorFilterValue]")
  {
    auto value = AppleColorFilterValue::Create(Filters({AppleInvertLightnessFunction {}}));
    CHECK(value->IsAppleColorFilterValue());
    CHECK(value->GetFilter() == Filters({AppleInvertLightnessFunction {}}));
  }

  TEST_CASE_METHOD(ValueFixture, "AppleColorFilterValue serialisation",
                   "[CSS][Values][AppleColorFilterValue]")
  {
    CHECK(CssText(*AppleColorFilterValue::Create(AppleColorFilter {keywords::None {}})) == "none");
    CHECK(CssText(*AppleColorFilterValue::Create(Filters({AppleInvertLightnessFunction {}})))
          == "apple-invert-lightness()");
    CHECK(CssText(*AppleColorFilterValue::Create(Filters({AppleInvertLightnessFunction {}, HalfSepia()})))
          == "apple-invert-lightness() sepia(50%)");
  }

  TEST_CASE_METHOD(ValueFixture, "AppleColorFilterValue equality and hashing",
                   "[CSS][Values][AppleColorFilterValue]")
  {
    auto a = AppleColorFilterValue::Create(Filters({AppleInvertLightnessFunction {}, HalfSepia()}));
    auto b = AppleColorFilterValue::Create(Filters({AppleInvertLightnessFunction {}, HalfSepia()}));
    auto reordered = AppleColorFilterValue::Create(Filters({HalfSepia(), AppleInvertLightnessFunction {}}));
    auto none = AppleColorFilterValue::Create(AppleColorFilter {keywords::None {}});

    CHECK(*a == *b);
    CHECK_FALSE(*a == *reordered);
    CHECK_FALSE(*a == *none);

    // `AppleColorFilterValue` has no hash of its own.
    size_t seed = 0uz;
    CHECK_FALSE(a->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "AppleColorFilterValue visitation", "[CSS][Values][AppleColorFilterValue]")
  {
    auto value = AppleColorFilterValue::Create(Filters({AppleInvertLightnessFunction {}, HalfSepia()}));
    CHECK(Children(*value).empty());
  }
}
