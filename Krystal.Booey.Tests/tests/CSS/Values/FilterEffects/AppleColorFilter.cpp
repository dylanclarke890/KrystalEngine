#include "Krystal.Booey/CSS/Values/FilterEffects/AppleColorFilter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    template <typename T>
    CSSOMString SerialiseForTest(const T &value)
    {
      return Serialise(DefaultSerialisationContext(), value);
    }

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

  TEST_CASE("AppleColorFilter(Serialise)", "[CSS][FilterEffects][AppleColorFilter]")
  {
    CHECK(SerialiseForTest(AppleInvertLightnessFunction {}) == u8"apple-invert-lightness()");
    CHECK(SerialiseForTest(AppleColorFilter {keywords::None {}}) == u8"none");
    CHECK(SerialiseForTest(Filters({AppleInvertLightnessFunction {}})) == u8"apple-invert-lightness()");
    CHECK(SerialiseForTest(Filters({AppleInvertLightnessFunction {}, HalfSepia(), BrightnessFunction {}}))
          == u8"apple-invert-lightness() sepia(50%) brightness()");
    CHECK(SerialiseForTest(Filters({HueRotateFunction {.Parameters = {HueRotate::Parameter {90_css_deg}}}}))
          == u8"hue-rotate(90deg)");
  }

  TEST_CASE("AppleColorFilter(Equality)", "[CSS][FilterEffects][AppleColorFilter]")
  {
    CHECK(AppleInvertLightnessFunction {} == AppleInvertLightnessFunction {});
    CHECK(AppleColorFilterData {AppleInvertLightnessFunction {}}
          == AppleColorFilterData {AppleInvertLightnessFunction {}});
    CHECK(AppleColorFilterData {AppleInvertLightnessFunction {}} != AppleColorFilterData {SepiaFunction {}});
    CHECK(AppleColorFilterData {SepiaFunction {}} != AppleColorFilterData {HalfSepia()});

    CHECK(AppleColorFilter {keywords::None {}} == AppleColorFilter {keywords::None {}});
    CHECK(AppleColorFilter {keywords::None {}} != Filters({AppleInvertLightnessFunction {}}));
    CHECK(Filters({HalfSepia(), AppleInvertLightnessFunction {}})
          != Filters({AppleInvertLightnessFunction {}, HalfSepia()}));
  }

  TEST_CASE("AppleColorFilter(SwitchOn)", "[CSS][FilterEffects][AppleColorFilter]")
  {
    AppleColorFilterData data {AppleInvertLightnessFunction {}};
    auto isInvertLightness = data.SwitchOn([](const AppleInvertLightnessFunction &) { return true; },
                                           [](const auto &) { return false; });
    CHECK(isInvertLightness);
  }

  TEST_CASE("AppleColorFilter(Dependencies and visitation)", "[CSS][FilterEffects][AppleColorFilter]")
  {
    CHECK(CollectComputedStyleDependencies(AppleColorFilter {keywords::None {}}).Properties.empty());
    CHECK(CollectComputedStyleDependencies(Filters({AppleInvertLightnessFunction {}, HalfSepia()}))
            .Properties.empty());

    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };

    CHECK(VisitValueChildren(visitor, Filters({AppleInvertLightnessFunction {}, HalfSepia()}))
          == IterationStatus::Continue);
    CHECK(visited == 0);
  }
}
