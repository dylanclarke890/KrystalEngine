#include "Krystal.Booey/CSS/Values/Images/Gradient.hpp"
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
    using NLP = LengthPercentage<NonNegative>;
    using Method = GradientColorInterpolationMethod;

    template <typename T>
    CSSOMString SerialiseForTest(const T &value)
    {
      return Serialise(DefaultSerialisationContext(), value);
    }

    Color Red()
    {
      return Color {KeywordColor {ValueId::Red}};
    }

    Color Blue()
    {
      return Color {KeywordColor {ValueId::Blue}};
    }

    Position At(TwoComponentPositionHorizontal x, TwoComponentPositionVertical y)
    {
      return Position {TwoComponentPositionHorizontalVertical {krys::move(x), krys::move(y)}};
    }

    Position LeftTop()
    {
      return At({keywords::Left {}}, {keywords::Top {}});
    }

    Position Centre()
    {
      return At({keywords::Center {}}, {keywords::Center {}});
    }

    Method Standard(ColourInterpolationMethod method = {ColourInterpolationMethod::OKLab {},
                                                        AlphaPremultiplication::Premultiplied})
    {
      return Method {.Method = method, .DefaultMethod = Method::Default::OKLab};
    }

    Method Legacy()
    {
      return Method::LegacyMethod(AlphaPremultiplication::Premultiplied);
    }

    GradientLinearColorStopList RedBlue()
    {
      return {GradientLinearColorStop {.Color = Red(), .Position = {}},
              GradientLinearColorStop {.Color = Blue(), .Position = {}}};
    }

    GradientAngularColorStopList AngularRedBlue()
    {
      return {GradientAngularColorStop {.Color = Red(), .Position = {}},
              GradientAngularColorStop {.Color = Blue(), .Position = {}}};
    }

    GradientDeprecatedColorStopList DeprecatedRedBlue()
    {
      return {GradientDeprecatedColorStop {.Color = Red(), .Position = Number<> {0.0}},
              GradientDeprecatedColorStop {.Color = Blue(), .Position = Number<> {1.0}}};
    }

    Gradient Linear(LinearGradient::GradientLineType line, Method method = Standard(),
                    GradientLinearColorStopList stops = RedBlue())
    {
      return FunctionNotation<ValueId::LinearGradient, LinearGradient> {LinearGradient {
        .ColourInterpolationMethod = method, .GradientLine = krys::move(line), .Stops = krys::move(stops)}};
    }

    Gradient Radial(RadialGradient::GradientBoxType box, Method method = Standard())
    {
      return FunctionNotation<ValueId::RadialGradient, RadialGradient> {RadialGradient {
        .ColourInterpolationMethod = method, .GradientBox = krys::move(box), .Stops = RedBlue()}};
    }

    Gradient PrefixedRadial(PrefixedRadialGradient::GradientBoxType box)
    {
      return FunctionNotation<ValueId::WebkitRadialGradient, PrefixedRadialGradient> {PrefixedRadialGradient {
        .ColourInterpolationMethod = Legacy(), .GradientBox = krys::move(box), .Stops = RedBlue()}};
    }

    Gradient Conic(ConicGradient::GradientBoxType box, Method method = Standard())
    {
      return FunctionNotation<ValueId::ConicGradient, ConicGradient> {ConicGradient {
        .ColourInterpolationMethod = method, .GradientBox = krys::move(box), .Stops = AngularRedBlue()}};
    }

    DeprecatedGradientPosition Point(double x, double y)
    {
      return {NumberOrPercentage<> {Number<> {x}}, NumberOrPercentage<> {Number<> {y}}};
    }
  }

  TEST_CASE("Gradient(Serialise linear)", "[CSS][Images][Gradient]")
  {
    // `to bottom` and `180deg` are the default direction.
    CHECK(SerialiseForTest(Linear(Vertical {keywords::Bottom {}})) == u8"linear-gradient(red, blue)");
    CHECK(SerialiseForTest(Linear(Angle<> {180_css_deg})) == u8"linear-gradient(red, blue)");
    CHECK(SerialiseForTest(Linear(Angle<> {90_css_deg})) == u8"linear-gradient(90deg, red, blue)");
    CHECK(SerialiseForTest(Linear(Vertical {keywords::Top {}})) == u8"linear-gradient(to top, red, blue)");
    CHECK(SerialiseForTest(Linear(Horizontal {keywords::Left {}}))
          == u8"linear-gradient(to left, red, blue)");
    CHECK(SerialiseForTest(
            Linear(SpaceSeparatedTuple<Horizontal, Vertical> {keywords::Right {}, keywords::Top {}}))
          == u8"linear-gradient(to right top, red, blue)");

    // The interpolation method is only written when it isn't the default.
    CHECK(SerialiseForTest(
            Linear(Vertical {keywords::Bottom {}},
                   Standard({ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied})))
          == u8"linear-gradient(in srgb, red, blue)");
    CHECK(SerialiseForTest(Linear(Angle<> {90_css_deg},
                                  Standard({ColourInterpolationMethod::HSL {HueInterpolationMethod::Longer},
                                            AlphaPremultiplication::Premultiplied})))
          == u8"linear-gradient(90deg in hsl longer hue, red, blue)");
    CHECK(SerialiseForTest(Linear(Vertical {keywords::Bottom {}}, Legacy()))
          == u8"linear-gradient(red, blue)");
    CHECK(SerialiseForTest(Linear(
            Vertical {keywords::Bottom {}},
            Method {.Method = {ColourInterpolationMethod::OKLab {}, AlphaPremultiplication::Premultiplied},
                    .DefaultMethod = Method::Default::SRGB}))
          == u8"linear-gradient(in oklab, red, blue)");

    // Stops: colour and position, colour only, and a position-only colour hint.
    CHECK(
      SerialiseForTest(Linear(Vertical {keywords::Bottom {}}, Standard(),
                              {GradientLinearColorStop {.Color = Red(), .Position = LP {10_css_percentage}},
                               GradientLinearColorStop {.Color = {}, .Position = LP {50_css_percentage}},
                               GradientLinearColorStop {.Color = Blue(), .Position = {}}}))
      == u8"linear-gradient(red 10%, 50%, blue)");

    CHECK(SerialiseForTest(Gradient {FunctionNotation<ValueId::RepeatingLinearGradient, LinearGradient> {
            LinearGradient {.ColourInterpolationMethod = Standard(),
                            .GradientLine = Angle<> {45_css_deg},
                            .Stops = RedBlue()}}})
          == u8"repeating-linear-gradient(45deg, red, blue)");
  }

  TEST_CASE("Gradient(Serialise prefixed and deprecated linear)", "[CSS][Images][Gradient]")
  {
    CHECK(SerialiseForTest(Gradient {FunctionNotation<ValueId::WebkitLinearGradient, PrefixedLinearGradient> {
            PrefixedLinearGradient {.ColourInterpolationMethod = Legacy(),
                                    .GradientLine = Horizontal {keywords::Left {}},
                                    .Stops = RedBlue()}}})
          == u8"-webkit-linear-gradient(left, red, blue)");
    CHECK(SerialiseForTest(
            Gradient {FunctionNotation<ValueId::WebkitRepeatingLinearGradient, PrefixedLinearGradient> {
              PrefixedLinearGradient {.ColourInterpolationMethod = Legacy(),
                                      .GradientLine = Angle<> {90_css_deg},
                                      .Stops = RedBlue()}}})
          == u8"-webkit-repeating-linear-gradient(90deg, red, blue)");

    // `from()` at 0, `to()` at 1 and `color-stop()` otherwise; percentages are written as numbers.
    CHECK(SerialiseForTest(Gradient {FunctionNotation<ValueId::WebkitGradient, DeprecatedLinearGradient> {
            DeprecatedLinearGradient {.ColourInterpolationMethod = Legacy(),
                                      .GradientLine = {Point(0, 0), Point(100, 0)},
                                      .Stops = DeprecatedRedBlue()}}})
          == u8"-webkit-gradient(linear, 0 0, 100 0, from(red), to(blue))");
    CHECK(SerialiseForTest(Gradient {
            FunctionNotation<ValueId::WebkitGradient, DeprecatedLinearGradient> {DeprecatedLinearGradient {
              .ColourInterpolationMethod = Legacy(),
              .GradientLine = {Point(0, 0), Point(0, 100)},
              .Stops = {GradientDeprecatedColorStop {.Color = Red(),
                                                     .Position = Percentage<> {50_css_percentage}},
                        GradientDeprecatedColorStop {.Color = Blue(), .Position = Number<> {0.25}}}}}})
          == u8"-webkit-gradient(linear, 0 0, 0 100, color-stop(0.5, red), color-stop(0.25, blue))");
    // No stops: nothing is written after the line.
    CHECK(
      SerialiseForTest(Gradient {
        FunctionNotation<ValueId::WebkitGradient, DeprecatedLinearGradient> {DeprecatedLinearGradient {
          .ColourInterpolationMethod = Legacy(), .GradientLine = {Point(0, 0), Point(1, 1)}, .Stops = {}}}})
      == u8"-webkit-gradient(linear, 0 0, 1 1)");
  }

  TEST_CASE("Gradient(Serialise radial)", "[CSS][Images][Gradient]")
  {
    using Ellipse = RadialGradient::Ellipse;
    using Circle = RadialGradient::Circle;

    // `farthest-corner` and a centred position are the defaults.
    CHECK(SerialiseForTest(Radial(Ellipse {.Size = RadialGradient::Extent {keywords::FarthestCorner {}}}))
          == u8"radial-gradient(red, blue)");
    CHECK(SerialiseForTest(Radial(
            Ellipse {.Size = RadialGradient::Extent {keywords::FarthestCorner {}}, .Position = Centre()}))
          == u8"radial-gradient(red, blue)");
    CHECK(SerialiseForTest(Radial(Ellipse {.Size = RadialGradient::Extent {keywords::ClosestSide {}}}))
          == u8"radial-gradient(closest-side, red, blue)");
    CHECK(SerialiseForTest(Radial(Ellipse {
            .Size = Ellipse::SizeType {NLP {10_css_px}, NLP {20_css_percentage}}, .Position = LeftTop()}))
          == u8"radial-gradient(10px 20% at left top, red, blue)");
    CHECK(SerialiseForTest(Radial(
            Ellipse {.Size = RadialGradient::Extent {keywords::FarthestCorner {}}, .Position = LeftTop()}))
          == u8"radial-gradient(at left top, red, blue)");

    CHECK(SerialiseForTest(Radial(Circle {.Size = RadialGradient::Extent {keywords::FarthestCorner {}}}))
          == u8"radial-gradient(circle, red, blue)");
    CHECK(SerialiseForTest(
            Radial(Circle {.Size = RadialGradient::Extent {keywords::ClosestSide {}}, .Position = LeftTop()}))
          == u8"radial-gradient(circle closest-side at left top, red, blue)");
    CHECK(SerialiseForTest(Radial(Circle {.Size = Circle::Length {10_css_px}, .Position = Centre()}))
          == u8"radial-gradient(10px, red, blue)");

    CHECK(SerialiseForTest(
            Radial(Ellipse {.Size = RadialGradient::Extent {keywords::FarthestCorner {}}},
                   Standard({ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied})))
          == u8"radial-gradient(in srgb, red, blue)");
    CHECK(SerialiseForTest(
            Radial(Circle {.Size = RadialGradient::Extent {keywords::FarthestCorner {}}},
                   Standard({ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied})))
          == u8"radial-gradient(circle in srgb, red, blue)");
  }

  TEST_CASE("Gradient(Serialise prefixed and deprecated radial)", "[CSS][Images][Gradient]")
  {
    using Ellipse = PrefixedRadialGradient::Ellipse;
    using Circle = PrefixedRadialGradient::Circle;

    CHECK(SerialiseForTest(PrefixedRadial(Ellipse {})) == u8"-webkit-radial-gradient(center, red, blue)");
    CHECK(SerialiseForTest(PrefixedRadial(
            Ellipse {.Size = PrefixedRadialGradient::Extent {keywords::Contain {}}, .Position = LeftTop()}))
          == u8"-webkit-radial-gradient(left top, ellipse contain, red, blue)");
    CHECK(
      SerialiseForTest(PrefixedRadial(Ellipse {.Size = Ellipse::SizeType {NLP {10_css_px}, NLP {20_css_px}}}))
      == u8"-webkit-radial-gradient(center, 10px 20px, red, blue)");
    // A circle without a size is `cover`.
    CHECK(SerialiseForTest(PrefixedRadial(Circle {}))
          == u8"-webkit-radial-gradient(center, circle cover, red, blue)");
    CHECK(SerialiseForTest(PrefixedRadial(Circle {
            .Size = PrefixedRadialGradient::Extent {keywords::ClosestSide {}}, .Position = LeftTop()}))
          == u8"-webkit-radial-gradient(left top, circle closest-side, red, blue)");

    CHECK(SerialiseForTest(Gradient {FunctionNotation<ValueId::WebkitGradient, DeprecatedRadialGradient> {
            DeprecatedRadialGradient {.ColourInterpolationMethod = Legacy(),
                                      .GradientBox = {.First = Point(0, 0),
                                                      .FirstRadius = Number<NonNegative> {0.0},
                                                      .Second = Point(10, 10),
                                                      .SecondRadius = Number<NonNegative> {20.0}},
                                      .Stops = DeprecatedRedBlue()}}})
          == u8"-webkit-gradient(radial, 0 0, 0, 10 10, 20, from(red), to(blue))");
  }

  TEST_CASE("Gradient(Serialise conic)", "[CSS][Images][Gradient]")
  {
    // A zero angle and a centred position are the defaults.
    CHECK(SerialiseForTest(Conic({})) == u8"conic-gradient(red, blue)");
    CHECK(SerialiseForTest(Conic({.Angle = Angle<> {0_css_deg}, .Position = Centre()}))
          == u8"conic-gradient(red, blue)");
    CHECK(SerialiseForTest(Conic({.Angle = Angle<> {90_css_deg}}))
          == u8"conic-gradient(from 90deg, red, blue)");
    CHECK(SerialiseForTest(Conic({.Position = LeftTop()})) == u8"conic-gradient(at left top, red, blue)");
    CHECK(SerialiseForTest(Conic({.Angle = Angle<> {90_css_deg}, .Position = LeftTop()}))
          == u8"conic-gradient(from 90deg at left top, red, blue)");
    CHECK(SerialiseForTest(
            Conic({.Angle = Angle<> {90_css_deg}},
                  Standard({ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied})))
          == u8"conic-gradient(from 90deg in srgb, red, blue)");

    CHECK(SerialiseForTest(
            Gradient {FunctionNotation<ValueId::RepeatingConicGradient, ConicGradient> {ConicGradient {
              .ColourInterpolationMethod = Standard(),
              .GradientBox = {},
              .Stops = {GradientAngularColorStop {.Color = Red(), .Position = AnglePercentage<> {10_css_deg}},
                        GradientAngularColorStop {.Color = Blue(),
                                                  .Position = AnglePercentage<> {50_css_percentage}}}}}})
          == u8"repeating-conic-gradient(red 10deg, blue 50%)");
  }

  TEST_CASE("Gradient(Equality)", "[CSS][Images][Gradient]")
  {
    CHECK(Linear(Angle<> {90_css_deg}) == Linear(Angle<> {90_css_deg}));
    CHECK(Linear(Angle<> {90_css_deg}) != Linear(Angle<> {45_css_deg}));
    CHECK(Linear(Angle<> {90_css_deg}) != Linear(Angle<> {90_css_deg}, Legacy()));
    // Same parameters, different function.
    CHECK(
      Linear(Angle<> {90_css_deg})
      != Gradient {FunctionNotation<ValueId::RepeatingLinearGradient, LinearGradient> {LinearGradient {
        .ColourInterpolationMethod = Standard(), .GradientLine = Angle<> {90_css_deg}, .Stops = RedBlue()}}});
    CHECK(Conic({.Position = LeftTop()}) != Conic({.Position = Centre()}));
  }

  TEST_CASE("Gradient(Dependencies and visitation)", "[CSS][Images][Gradient]")
  {
    CHECK(CollectComputedStyleDependencies(Linear(Angle<> {90_css_deg})).Properties.empty());
    CHECK(CollectComputedStyleDependencies(
            Linear(Vertical {keywords::Bottom {}}, Standard(),
                   {GradientLinearColorStop {.Color = Red(), .Position = LP {1_css_em}},
                    GradientLinearColorStop {.Color = Blue(), .Position = {}}}))
            .Properties
          == List<PropertyId> {PropertyId::FontSize});
    CHECK(CollectComputedStyleDependencies(
            Radial(RadialGradient::Circle {.Size = RadialGradient::Circle::Length {2_css_em}}))
            .Properties
          == List<PropertyId> {PropertyId::FontSize});

    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };

    // Raw numerics and keyword colours have no child values.
    CHECK(VisitValueChildren(visitor, Linear(Angle<> {90_css_deg})) == IterationStatus::Continue);
    CHECK(VisitValueChildren(visitor, Conic({.Position = LeftTop()})) == IterationStatus::Continue);
    CHECK(visited == 0);
  }
}
