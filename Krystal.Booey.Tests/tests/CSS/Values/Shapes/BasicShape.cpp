#include "Krystal.Booey/CSS/Values/Shapes/BasicShape.hpp"
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

    template <typename T>
    CSSOMString SerialiseForTest(const T &value)
    {
      return Serialise(DefaultSerialisationContext(), value);
    }

    Position At(TwoComponentPositionHorizontal x, TwoComponentPositionVertical y)
    {
      return Position {TwoComponentPositionHorizontalVertical {krys::move(x), krys::move(y)}};
    }

    Position Centre()
    {
      return At({keywords::Center {}}, {keywords::Center {}});
    }

    BorderRadius Radii(NLP all)
    {
      return BorderRadius {.Horizontal = {all, all, all, all}, .Vertical = {all, all, all, all}};
    }
  }

  TEST_CASE("CircleFunction(Serialise)", "[CSS][Shapes][CircleFunction]")
  {
    // `closest-side` is the default radius, so it's omitted.
    CHECK(SerialiseForTest(CircleFunction {Circle {.Radius = Circle::Extent {keywords::ClosestSide {}}}})
          == u8"circle()");
    CHECK(SerialiseForTest(CircleFunction {Circle {.Radius = Circle::Extent {keywords::FarthestCorner {}}}})
          == u8"circle(farthest-corner)");
    CHECK(SerialiseForTest(CircleFunction {Circle {.Radius = NLP {10_css_px}}}) == u8"circle(10px)");
    CHECK(SerialiseForTest(CircleFunction {Circle {.Radius = NLP {50_css_percentage}}}) == u8"circle(50%)");

    // The position is always written when present, even at its default.
    CHECK(SerialiseForTest(CircleFunction {
            Circle {.Radius = Circle::Extent {keywords::ClosestSide {}}, .Position = Centre()}})
          == u8"circle(at center center)");
    CHECK(SerialiseForTest(CircleFunction {Circle {
            .Radius = NLP {10_css_px}, .Position = At({keywords::Left {}}, {LP {20_css_percentage}})}})
          == u8"circle(10px at left 20%)");
  }

  TEST_CASE("EllipseFunction(Serialise)", "[CSS][Shapes][EllipseFunction]")
  {
    Ellipse::RadialSize closestSide = Ellipse::Extent {keywords::ClosestSide {}};

    CHECK(SerialiseForTest(EllipseFunction {Ellipse {.Radii = {closestSide, closestSide}}}) == u8"ellipse()");
    // Both radii are written if either isn't the default.
    CHECK(SerialiseForTest(EllipseFunction {Ellipse {.Radii = {NLP {10_css_px}, closestSide}}})
          == u8"ellipse(10px closest-side)");
    CHECK(SerialiseForTest(
            EllipseFunction {Ellipse {.Radii = {closestSide, Ellipse::Extent {keywords::FarthestSide {}}}}})
          == u8"ellipse(closest-side farthest-side)");
    CHECK(SerialiseForTest(
            EllipseFunction {Ellipse {.Radii = {NLP {10_css_px}, NLP {20_css_px}}, .Position = Centre()}})
          == u8"ellipse(10px 20px at center center)");
    CHECK(
      SerialiseForTest(EllipseFunction {Ellipse {.Radii = {closestSide, closestSide}, .Position = Centre()}})
      == u8"ellipse(at center center)");
  }

  TEST_CASE("InsetFunction(Serialise)", "[CSS][Shapes][InsetFunction]")
  {
    CHECK(SerialiseForTest(
            InsetFunction {Inset {.Insets = {LP {10_css_px}}, .Radii = BorderRadius::DefaultValue()}})
          == u8"inset(10px)");
    CHECK(SerialiseForTest(
            InsetFunction {Inset {.Insets = {LP {1_css_px}, LP {2_css_px}, LP {3_css_px}, LP {2_css_px}},
                                  .Radii = BorderRadius::DefaultValue()}})
          == u8"inset(1px 2px 3px)");
    CHECK(SerialiseForTest(InsetFunction {Inset {.Insets = {LP {10_css_px}}, .Radii = Radii(NLP {5_css_px})}})
          == u8"inset(10px round 5px)");
  }

  TEST_CASE("PolygonFunction(Serialise)", "[CSS][Shapes][PolygonFunction]")
  {
    Polygon::VerticesType vertices {{LP {0_css_px}, LP {0_css_px}},
                                    {LP {100_css_percentage}, LP {0_css_px}},
                                    {LP {50_css_percentage}, LP {10_css_px}}};

    CHECK(SerialiseForTest(PolygonFunction {Polygon {.Vertices = vertices}})
          == u8"polygon(0px 0px, 100% 0px, 50% 10px)");
    // `nonzero` is the default, so it's omitted.
    CHECK(SerialiseForTest(
            PolygonFunction {Polygon {.FillRule = FillRule {keywords::Nonzero {}}, .Vertices = vertices}})
          == u8"polygon(0px 0px, 100% 0px, 50% 10px)");
    CHECK(SerialiseForTest(
            PolygonFunction {Polygon {.FillRule = FillRule {keywords::Evenodd {}}, .Vertices = vertices}})
          == u8"polygon(evenodd, 0px 0px, 100% 0px, 50% 10px)");
  }

  TEST_CASE("RectFunction(Serialise)", "[CSS][Shapes][RectFunction]")
  {
    SpaceSeparatedRectEdges<RectShape::Edge> edges {
      RectShape::Edge {LP {10_css_px}}, RectShape::Edge {keywords::Auto {}},
      RectShape::Edge {LP {20_css_percentage}}, RectShape::Edge {keywords::Auto {}}};

    CHECK(SerialiseForTest(RectFunction {RectShape {.Edges = edges, .Radii = BorderRadius::DefaultValue()}})
          == u8"rect(10px auto 20% auto)");
    // All four edges are always written.
    CHECK(SerialiseForTest(RectFunction {
            RectShape {.Edges = {RectShape::Edge {LP {1_css_px}}}, .Radii = BorderRadius::DefaultValue()}})
          == u8"rect(1px 1px 1px 1px)");
    CHECK(SerialiseForTest(RectFunction {RectShape {.Edges = edges, .Radii = Radii(NLP {5_css_px})}})
          == u8"rect(10px auto 20% auto round 5px)");
  }

  TEST_CASE("XywhFunction(Serialise)", "[CSS][Shapes][XywhFunction]")
  {
    CHECK(SerialiseForTest(XywhFunction {Xywh {.Location = {LP {10_css_px}, LP {20_css_px}},
                                               .Size = {NLP {30_css_px}, NLP {40_css_percentage}},
                                               .Radii = BorderRadius::DefaultValue()}})
          == u8"xywh(10px 20px 30px 40%)");
    // The size is always written in full.
    CHECK(SerialiseForTest(XywhFunction {Xywh {.Location = {LP {0_css_px}, LP {0_css_px}},
                                               .Size = {NLP {5_css_px}, NLP {5_css_px}},
                                               .Radii = BorderRadius::DefaultValue()}})
          == u8"xywh(0px 0px 5px 5px)");
    CHECK(SerialiseForTest(XywhFunction {Xywh {
            .Location = {LP {10_css_px}, LP {20_css_px}},
            .Size = {NLP {30_css_px}, NLP {40_css_px}},
            .Radii =
              BorderRadius {.Horizontal = {NLP {1_css_px}, NLP {1_css_px}, NLP {1_css_px}, NLP {1_css_px}},
                            .Vertical = {NLP {2_css_px}, NLP {2_css_px}, NLP {2_css_px}, NLP {2_css_px}}}}})
          == u8"xywh(10px 20px 30px 40px round 1px / 2px)");
  }

  TEST_CASE("BasicShape(Serialise)", "[CSS][Shapes][BasicShape]")
  {
    CHECK(SerialiseForTest(BasicShape {CircleFunction {Circle {.Radius = NLP {1_css_px}}}})
          == u8"circle(1px)");
    CHECK(SerialiseForTest(BasicShape {
            InsetFunction {Inset {.Insets = {LP {1_css_px}}, .Radii = BorderRadius::DefaultValue()}}})
          == u8"inset(1px)");
    CHECK(SerialiseForTest(BasicShape {PathFunction {Path {.Data = {svg::SVGPathByteStream(u8"M 0 0")}}}})
          == u8"path(\"M 0 0\")");
  }

  TEST_CASE("BasicShape(Equality)", "[CSS][Shapes][BasicShape]")
  {
    BasicShape circle = CircleFunction {Circle {.Radius = NLP {1_css_px}, .Position = Centre()}};

    CHECK(circle == BasicShape {CircleFunction {Circle {.Radius = NLP {1_css_px}, .Position = Centre()}}});
    CHECK(circle != BasicShape {CircleFunction {Circle {.Radius = NLP {1_css_px}}}});
    CHECK(circle != BasicShape {CircleFunction {Circle {.Radius = NLP {2_css_px}, .Position = Centre()}}});
    CHECK(circle
          != BasicShape {
            EllipseFunction {Ellipse {.Radii = {NLP {1_css_px}, NLP {1_css_px}}, .Position = Centre()}}});

    // `inset(10px)` and `inset(10px round 0px)` are the same value.
    CHECK(Inset {.Insets = {LP {10_css_px}}, .Radii = BorderRadius::DefaultValue()}
          == Inset {.Insets = {LP {10_css_px}}, .Radii = Radii(NLP {0_css_px})});
    CHECK(Inset {.Insets = {LP {10_css_px}}, .Radii = BorderRadius::DefaultValue()}
          != Inset {.Insets = {LP {10_css_px}}, .Radii = Radii(NLP {1_css_px})});
  }

  TEST_CASE("BasicShape(Dependencies and visitation)", "[CSS][Shapes][BasicShape]")
  {
    CHECK(CollectComputedStyleDependencies(BasicShape {CircleFunction {Circle {.Radius = NLP {1_css_px}}}})
            .Properties.empty());
    CHECK(CollectComputedStyleDependencies(BasicShape {CircleFunction {Circle {.Radius = NLP {1_css_em}}}})
            .Properties
          == List<PropertyId> {PropertyId::FontSize});
    CHECK(CollectComputedStyleDependencies(
            BasicShape {XywhFunction {Xywh {.Location = {LP {1_css_px}, LP {1_css_px}},
                                            .Size = {NLP {1_css_px}, NLP {1_css_px}},
                                            .Radii = Radii(NLP {1_css_em})}}})
            .Properties
          == List<PropertyId> {PropertyId::FontSize});

    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };
    CHECK(VisitValueChildren(
            visitor, BasicShape {PolygonFunction {Polygon {.Vertices = {{LP {0_css_px}, LP {0_css_px}}}}}})
          == IterationStatus::Continue);
    CHECK(visited == 0);
  }
}
