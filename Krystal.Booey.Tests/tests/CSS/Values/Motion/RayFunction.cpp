#include "Krystal.Booey/CSS/Values/Motion/RayFunction.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using LP = LengthPercentage<>;

    template <typename T>
    CSSOMString SerialiseForTest(const T &value)
    {
      return Serialise(DefaultSerialisationContext(), value);
    }

    Position At(TwoComponentPositionHorizontal x, TwoComponentPositionVertical y)
    {
      return Position {TwoComponentPositionHorizontalVertical {krys::move(x), krys::move(y)}};
    }

    RayFunction MakeRay(Angle<> angle, RaySize size = keywords::ClosestSide {},
                        Maybe<keywords::Contain> contain = {}, Maybe<Position> position = {})
    {
      return RayFunction {
        .Parameters = Ray {
          .Angle = krys::move(angle), .Size = size, .Contain = contain, .Position = krys::move(position)}};
    }
  }

  TEST_CASE("RayFunction(Serialise)", "[CSS][Motion][RayFunction]")
  {
    // `closest-side` is the default size, so it's omitted.
    CHECK(SerialiseForTest(MakeRay(45_css_deg)) == u8"ray(45deg)");
    CHECK(SerialiseForTest(MakeRay(45_css_deg, keywords::FarthestCorner {}))
          == u8"ray(45deg farthest-corner)");
    CHECK(SerialiseForTest(MakeRay(45_css_deg, keywords::ClosestCorner {})) == u8"ray(45deg closest-corner)");
    CHECK(SerialiseForTest(MakeRay(45_css_deg, keywords::FarthestSide {})) == u8"ray(45deg farthest-side)");
    CHECK(SerialiseForTest(MakeRay(45_css_deg, keywords::Sides {})) == u8"ray(45deg sides)");
    CHECK(SerialiseForTest(MakeRay(45_css_deg, keywords::ClosestSide {}, keywords::Contain {}))
          == u8"ray(45deg contain)");
    CHECK(SerialiseForTest(MakeRay(45_css_deg, keywords::ClosestSide {}, {},
                                   At({keywords::Left {}}, {LP {20_css_percentage}})))
          == u8"ray(45deg at left 20%)");
    CHECK(SerialiseForTest(MakeRay(1_css_turn, keywords::Sides {}, keywords::Contain {},
                                   At({keywords::Center {}}, {keywords::Center {}})))
          == u8"ray(1turn sides contain at center center)");
  }

  TEST_CASE("RayFunction(Equality)", "[CSS][Motion][RayFunction]")
  {
    CHECK(MakeRay(45_css_deg) == MakeRay(45_css_deg));
    CHECK(MakeRay(45_css_deg) != MakeRay(90_css_deg));
    CHECK(MakeRay(45_css_deg) != MakeRay(45_css_deg, keywords::Sides {}));
    CHECK(MakeRay(45_css_deg) != MakeRay(45_css_deg, keywords::ClosestSide {}, keywords::Contain {}));
    CHECK(
      MakeRay(45_css_deg)
      != MakeRay(45_css_deg, keywords::ClosestSide {}, {}, At({keywords::Center {}}, {keywords::Center {}})));
  }

  TEST_CASE("RayFunction(Dependencies and visitation)", "[CSS][Motion][RayFunction]")
  {
    CHECK(CollectComputedStyleDependencies(MakeRay(45_css_deg)).Properties.empty());
    CHECK(CollectComputedStyleDependencies(
            MakeRay(45_css_deg, keywords::ClosestSide {}, {}, At({LP {1_css_em}}, {keywords::Center {}})))
            .Properties
          == List<PropertyId> {PropertyId::FontSize});

    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };
    CHECK(VisitValueChildren(visitor, MakeRay(45_css_deg, keywords::Sides {}, keywords::Contain {},
                                              At({keywords::Center {}}, {keywords::Center {}})))
          == IterationStatus::Continue);
    CHECK(visited == 0);
  }
}
