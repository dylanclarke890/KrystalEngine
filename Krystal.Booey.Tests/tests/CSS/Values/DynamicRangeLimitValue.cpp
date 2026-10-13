#include "Krystal.Booey/CSS/Values/DynamicRangeLimitValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimitMix.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    DynamicRangeLimit Mix()
    {
      DynamicRangeLimitMixParameters parameters;
      parameters.Value.emplace_back(DynamicRangeLimitMixComponent {
        DynamicRangeLimit {keywords::Standard {}}, DynamicRangeLimitMixPercentage {40_css_percentage}});
      parameters.Value.emplace_back(DynamicRangeLimitMixComponent {
        DynamicRangeLimit {keywords::NoLimit {}}, DynamicRangeLimitMixPercentage {60_css_percentage}});
      return DynamicRangeLimit {
        DynamicRangeLimitMixFunction {DynamicRangeLimitMixFunctionValue {krys::move(parameters)}}};
    }
  }

  TEST_CASE_METHOD(ValueFixture, "DynamicRangeLimitValue::Create", "[CSS][Values][DynamicRangeLimitValue]")
  {
    auto value = DynamicRangeLimitValue::Create(DynamicRangeLimit {keywords::Standard {}});
    CHECK(value->IsDynamicRangeLimitValue());
    CHECK(value->GetDynamicRangeLimit() == DynamicRangeLimit {keywords::Standard {}});
  }

  TEST_CASE_METHOD(ValueFixture, "DynamicRangeLimitValue serialisation",
                   "[CSS][Values][DynamicRangeLimitValue]")
  {
    CHECK(CssText(*DynamicRangeLimitValue::Create(DynamicRangeLimit {keywords::Standard {}})) == "standard");
    CHECK(CssText(*DynamicRangeLimitValue::Create(DynamicRangeLimit {keywords::NoLimit {}})) == "no-limit");
    CHECK(CssText(*DynamicRangeLimitValue::Create(Mix()))
          == "dynamic-range-limit-mix(standard 40%, no-limit 60%)");
  }

  TEST_CASE_METHOD(ValueFixture, "DynamicRangeLimitValue equality and hashing",
                   "[CSS][Values][DynamicRangeLimitValue]")
  {
    auto standard = DynamicRangeLimitValue::Create(DynamicRangeLimit {keywords::Standard {}});
    CHECK(*standard == *DynamicRangeLimitValue::Create(DynamicRangeLimit {keywords::Standard {}}));
    CHECK_FALSE(*standard == *DynamicRangeLimitValue::Create(DynamicRangeLimit {keywords::NoLimit {}}));
    CHECK_FALSE(*standard == *DynamicRangeLimitValue::Create(Mix()));
    CHECK(*DynamicRangeLimitValue::Create(Mix()) == *DynamicRangeLimitValue::Create(Mix()));

    // `DynamicRangeLimitValue` has no hash of its own.
    size_t seed = 0uz;
    CHECK_FALSE(standard->AddHash(seed));
  }

  TEST_CASE_METHOD(ValueFixture, "DynamicRangeLimitValue visitation", "[CSS][Values][DynamicRangeLimitValue]")
  {
    CHECK(Children(*DynamicRangeLimitValue::Create(Mix())).empty());
  }
}
