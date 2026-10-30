#include "Krystal.Booey/CSS/Values/Shapes/ShapeFunction.hpp"
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

    Position LeftTop()
    {
      return At({keywords::Left {}}, {keywords::Top {}});
    }

    Position Centre()
    {
      return At({keywords::Center {}}, {keywords::Center {}});
    }

    CoordinatePair Coordinates(LP x, LP y)
    {
      return CoordinatePair {krys::move(x), krys::move(y)};
    }

    ArcCommand Arc(Variant<ArcCommand::To, ArcCommand::By> toBy, ArcCommand::SizeOfEllipse size,
                   ArcSweep sweep = keywords::Ccw {}, ArcSize arcSize = keywords::Small {},
                   Angle<> rotation = 0_css_deg)
    {
      return ArcCommand {.ToBy = krys::move(toBy),
                         .Size = krys::move(size),
                         .ArcSweep = sweep,
                         .ArcSize = arcSize,
                         .Rotation = rotation};
    }

    ShapeFunction MakeShape(Shape::CommandsType commands, Maybe<FillRule> fillRule = {})
    {
      return ShapeFunction {Shape {
        .FillRule = krys::move(fillRule), .StartingPoint = LeftTop(), .Commands = krys::move(commands)}};
    }
  }

  TEST_CASE("ShapeFunction(Serialise move and line)", "[CSS][Shapes][ShapeFunction]")
  {
    CHECK(SerialiseForTest(MoveCommand {ToPosition {Centre()}}) == u8"move to center center");
    CHECK(SerialiseForTest(MoveCommand {ByCoordinatePair {Coordinates(LP {1_css_px}, LP {2_css_percentage})}})
          == u8"move by 1px 2%");
    CHECK(SerialiseForTest(LineCommand {ToPosition {LeftTop()}}) == u8"line to left top");
    CHECK(SerialiseForTest(LineCommand {ByCoordinatePair {Coordinates(LP {-1_css_px}, LP {0_css_px})}})
          == u8"line by -1px 0px");
  }

  TEST_CASE("ShapeFunction(Serialise hline and vline)", "[CSS][Shapes][ShapeFunction]")
  {
    CHECK(
      SerialiseForTest(HLineCommand {HLineCommand::To {TwoComponentPositionHorizontal {keywords::XEnd {}}}})
      == u8"hline to x-end");
    CHECK(SerialiseForTest(HLineCommand {HLineCommand::To {TwoComponentPositionHorizontal {LP {10_css_px}}}})
          == u8"hline to 10px");
    CHECK(SerialiseForTest(HLineCommand {HLineCommand::By {LP {5_css_percentage}}}) == u8"hline by 5%");
    CHECK(
      SerialiseForTest(VLineCommand {VLineCommand::To {TwoComponentPositionVertical {keywords::Bottom {}}}})
      == u8"vline to bottom");
    CHECK(SerialiseForTest(VLineCommand {VLineCommand::By {LP {-5_css_px}}}) == u8"vline by -5px");
  }

  TEST_CASE("ShapeFunction(Serialise curve and smooth)", "[CSS][Shapes][ShapeFunction]")
  {
    CHECK(SerialiseForTest(CurveCommand {CurveCommand::To {
            .Offset = Centre(), .ControlPoint1 = AbsoluteControlPoint {.Offset = LeftTop()}}})
          == u8"curve to center center with left top");
    CHECK(SerialiseForTest(CurveCommand {CurveCommand::To {
            .Offset = Centre(),
            .ControlPoint1 = AbsoluteControlPoint {.Offset = LeftTop(), .Anchor = keywords::Start {}},
            .ControlPoint2 = AbsoluteControlPoint {.Offset = Centre(), .Anchor = keywords::End {}}}})
          == u8"curve to center center with left top from start / center center from end");
    CHECK(SerialiseForTest(CurveCommand {CurveCommand::By {
            .Offset = Coordinates(LP {1_css_px}, LP {2_css_px}),
            .ControlPoint1 = RelativeControlPoint {.Offset = Coordinates(LP {3_css_px}, LP {4_css_px})},
            .ControlPoint2 = RelativeControlPoint {.Offset = Coordinates(LP {5_css_px}, LP {6_css_px}),
                                                   .Anchor = keywords::Origin {}}}})
          == u8"curve by 1px 2px with 3px 4px / 5px 6px from origin");

    CHECK(SerialiseForTest(SmoothCommand {SmoothCommand::To {.Offset = LeftTop()}})
          == u8"smooth to left top");
    CHECK(SerialiseForTest(SmoothCommand {SmoothCommand::To {
            .Offset = LeftTop(), .ControlPoint = AbsoluteControlPoint {.Offset = Centre()}}})
          == u8"smooth to left top with center center");
    CHECK(SerialiseForTest(SmoothCommand {SmoothCommand::By {
            .Offset = Coordinates(LP {1_css_px}, LP {2_css_px}),
            .ControlPoint = RelativeControlPoint {.Offset = Coordinates(LP {3_css_px}, LP {4_css_px}),
                                                  .Anchor = keywords::Start {}}}})
          == u8"smooth by 1px 2px with 3px 4px from start");
  }

  TEST_CASE("ShapeFunction(Serialise arc)", "[CSS][Shapes][ShapeFunction]")
  {
    // `ccw`, `small` and a zero rotation are the defaults, so they're omitted.
    CHECK(SerialiseForTest(Arc(ArcCommand::To {Centre()}, {LP {10_css_px}}))
          == u8"arc to center center of 10px");
    // Equal radii are written once.
    CHECK(SerialiseForTest(Arc(ArcCommand::To {Centre()}, {LP {10_css_px}, LP {10_css_px}}))
          == u8"arc to center center of 10px");
    CHECK(SerialiseForTest(Arc(ArcCommand::By {Coordinates(LP {1_css_px}, LP {2_css_px})},
                               {LP {10_css_px}, LP {20_css_percentage}}))
          == u8"arc by 1px 2px of 10px 20%");
    CHECK(SerialiseForTest(Arc(ArcCommand::To {Centre()}, {LP {10_css_px}}, keywords::Cw {},
                               keywords::Large {}, Angle<> {45_css_deg}))
          == u8"arc to center center of 10px cw large rotate 45deg");
    CHECK(
      SerialiseForTest(Arc(ArcCommand::To {Centre()}, {LP {10_css_px}}, keywords::Ccw {}, keywords::Large {}))
      == u8"arc to center center of 10px large");
  }

  TEST_CASE("ShapeFunction(Serialise)", "[CSS][Shapes][ShapeFunction]")
  {
    CHECK(SerialiseForTest(MakeShape({MoveCommand {ToPosition {Centre()}}}))
          == u8"shape(from left top, move to center center)");
    CHECK(SerialiseForTest(
            MakeShape({LineCommand {ByCoordinatePair {Coordinates(LP {10_css_px}, LP {0_css_px})}},
                       VLineCommand {VLineCommand::By {LP {10_css_px}}},
                       HLineCommand {HLineCommand::To {TwoComponentPositionHorizontal {keywords::Left {}}}},
                       CloseCommand {}}))
          == u8"shape(from left top, line by 10px 0px, vline by 10px, hline to left, close)");

    // `nonzero` is the default, so it's omitted.
    CHECK(SerialiseForTest(MakeShape({CloseCommand {}}, FillRule {keywords::Nonzero {}}))
          == u8"shape(from left top, close)");
    CHECK(SerialiseForTest(MakeShape({CloseCommand {}}, FillRule {keywords::Evenodd {}}))
          == u8"shape(evenodd from left top, close)");
  }

  TEST_CASE("ShapeFunction(Equality)", "[CSS][Shapes][ShapeFunction]")
  {
    CHECK(MakeShape({CloseCommand {}}) == MakeShape({CloseCommand {}}));
    CHECK(MakeShape({CloseCommand {}}) != MakeShape({CloseCommand {}}, FillRule {keywords::Nonzero {}}));
    CHECK(MakeShape({MoveCommand {ToPosition {Centre()}}})
          != MakeShape({LineCommand {ToPosition {Centre()}}}));
    // `to` and `by` are different commands, even if they reach the same point.
    CHECK(MakeShape({LineCommand {ToPosition {LeftTop()}}})
          != MakeShape({LineCommand {ByCoordinatePair {Coordinates(LP {0_css_px}, LP {0_css_px})}}}));
    CHECK(Arc(ArcCommand::To {Centre()}, {LP {10_css_px}})
          == Arc(ArcCommand::To {Centre()}, {LP {10_css_px}, LP {10_css_px}}));
    CHECK(Arc(ArcCommand::To {Centre()}, {LP {10_css_px}})
          != Arc(ArcCommand::To {Centre()}, {LP {10_css_px}}, keywords::Cw {}));
  }

  TEST_CASE("ShapeFunction(Dependencies and visitation)", "[CSS][Shapes][ShapeFunction]")
  {
    CHECK(CollectComputedStyleDependencies(MakeShape({MoveCommand {ToPosition {Centre()}}, CloseCommand {}}))
            .Properties.empty());
    CHECK(CollectComputedStyleDependencies(
            MakeShape({SmoothCommand {SmoothCommand::By {
              .Offset = Coordinates(LP {1_css_px}, LP {2_css_px}),
              .ControlPoint = RelativeControlPoint {.Offset = Coordinates(LP {3_css_em}, LP {4_css_px})}}}}))
            .Properties
          == List<PropertyId> {PropertyId::FontSize});
    CHECK(CollectComputedStyleDependencies(MakeShape({Arc(ArcCommand::To {Centre()}, {LP {1_css_em}})}))
            .Properties
          == List<PropertyId> {PropertyId::FontSize});

    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };
    CHECK(VisitValueChildren(visitor, MakeShape({MoveCommand {ToPosition {Centre()}}, CloseCommand {}}))
          == IterationStatus::Continue);
    CHECK(visited == 0);
  }
}
