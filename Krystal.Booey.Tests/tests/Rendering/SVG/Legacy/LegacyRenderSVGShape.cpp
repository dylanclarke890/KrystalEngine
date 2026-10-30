#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Graphics/TestPathStrokeStrategy.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGPath.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRect.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include <numbers>

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    // The renderers' overrides are private; tests call them through RenderObject.
    KRYS_NODISCARD const RenderObject &AsObject(const RenderObject &renderer) noexcept
    {
      return renderer;
    }

    KRYS_NODISCARD dom::Element &ElementById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto element = document.GetElementById(id);
      krys_assert(element);
      return *element;
    }

    template <typename T>
    KRYS_NODISCARD T &RendererById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto *renderer = ElementById(document, id).Renderer();
      krys_assert(renderer);
      return Downcast<T>(*renderer);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGShape: the render tree",
                   "[Rendering][SVG][LegacyRenderSVGShape]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<rect x=10 y=20 width=30 height=40 fill=green stroke=blue stroke-width=2 />)html"
      u8R"html(<circle cx=50 cy=50 r=10 fill-opacity=0.5 fill-rule=evenodd />)html"
      u8R"html(<ellipse cx=60 cy=40 rx=20 ry=10 fill=none stroke=red stroke-dasharray="4 2" stroke-linecap=round />)html"
      u8R"html(<line x1=0 y1=0 x2=100 y2=50 stroke=black />)html"
      u8R"html(<polyline points="0,0 10,10 20,0" fill=none stroke=black stroke-linejoin=bevel />)html"
      u8R"html(<polygon points="0,0 10,10 20,0" clip-rule=evenodd />)html"
      u8R"html(<path d="M 10 10 h 20 v 20 z" transform="translate(5 5)" pathLength=100 stroke=black />)html"
      u8R"html(<rect width=0 height=10 /></svg></body>)html"};

    // Each shape dumps its painting resources (a solid colour, with the stroke's or fill's properties that
    // aren't their initial values) and its geometry. Rects, circles and ellipses have renderers of their own;
    // lines, polylines, polygons and paths are paths. The dump's rects are the accurate repaint rects: the
    // stroke bounding boxes, which the host's stroker would compute (without it, the path's bounds grown by
    // half the stroke).
    CHECK(ExternalRepresentation(fixture.Frame.get()) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x104
  RenderBlock {HTML} at (0,0) size 800x104
    RenderBody {BODY} at (0,0) size 800x104
      RenderSVGRoot {svg} at (0,0) size 101x61
        RenderSVGRect {rect} at (9,19) size 32x42 [stroke={[type=SOLID] [color=#0000FF] [stroke width=2.00]}] [fill={[type=SOLID] [color=#008000]}] [x=10.00] [y=20.00] [width=30.00] [height=40.00]
        RenderSVGEllipse {circle} at (40,40) size 20x20 [fill={[type=SOLID] [color=#000000] [opacity=0.50] [fill rule=EVEN-ODD]}] [cx=50.00] [cy=50.00] [r=10.00]
        RenderSVGEllipse {ellipse} at (39,29) size 42x22 [stroke={[type=SOLID] [color=#FF0000] [line cap=ROUND] [dash array={4.00, 2.00}]}] [cx=60.00] [cy=40.00] [rx=20.00] [ry=10.00]
        RenderSVGPath {line} at (0,0) size 101x51 [stroke={[type=SOLID] [color=#000000]}] [fill={[type=SOLID] [color=#000000]}] [x1=0.00] [y1=0.00] [x2=100.00] [y2=50.00]
        RenderSVGPath {polyline} at (0,0) size 21x11 [stroke={[type=SOLID] [color=#000000] [line join=BEVEL]}] [points="0 0 10 10 20 0"]
        RenderSVGPath {polygon} at (0,0) size 20x10 [fill={[type=SOLID] [color=#000000]}] [clip rule=EVEN-ODD] [points="0 0 10 10 20 0"]
        RenderSVGPath {path} at (14,14) size 22x22 [transform={m=((1.00,0.00)(0.00,1.00)) t=(5.00,5.00)}] [stroke={[type=SOLID] [color=#000000] [path length=100.00]}] [fill={[type=SOLID] [color=#000000]}] [data="M 10 10 L 30 10 L 30 30 Z"]
        RenderSVGRect {rect} at (0,0) size 0x0 [fill={[type=SOLID] [color=#000000]}] [x=0.00] [y=0.00] [width=0.00] [height=10.00]
)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGShape: bounding boxes",
                   "[Rendering][SVG][LegacyRenderSVGShape]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<g id=g><rect id=rect x=10 y=20 width=30 height=40 stroke=blue stroke-width=4 />)html"
      u8R"html(<rect id=rounded x=10 y=20 width=30 height=40 rx=5 />)html"
      u8R"html(<circle id=circle cx=50 cy=50 r=10 stroke=black stroke-width=2 />)html"
      u8R"html(<path id=path d="M 0 0 L 20 0 L 20 20" stroke=black stroke-width=2 stroke-linecap=square />)html"
      u8R"html(<line id=line x1=0 y1=0 x2=10 y2=0 stroke=black stroke-width=2 stroke-linecap=square />)html"
      u8R"html(<path id=dot d="M 70 70 z" stroke=black stroke-width=4 stroke-linecap=round />)html"
      u8R"html(<rect id=empty width=0 height=10 /></g></svg></body>)html"};
    (void)fixture.Document->UpdateLayout();

    // A plain rect's boxes need no path: the stroke grows it by half its width.
    auto &rect = RendererById<LegacyRenderSVGRect>(*fixture.Document, u8"rect");
    CHECK(rect.GetShapeType() == LegacyRenderSVGShape::ShapeType::Rectangle);
    CHECK_FALSE(rect.HasPath());
    CHECK(AsObject(rect).ObjectBoundingBox() == FloatRect(10, 20, 30, 40));
    CHECK(AsObject(rect).StrokeBoundingBox() == FloatRect(8, 18, 34, 44));
    CHECK(AsObject(rect).RepaintRectInLocalCoordinates() == FloatRect(8, 18, 34, 44));

    // A rounded rect is a path.
    auto &rounded = RendererById<LegacyRenderSVGRect>(*fixture.Document, u8"rounded");
    CHECK(rounded.GetShapeType() == LegacyRenderSVGShape::ShapeType::RoundedRectangle);
    CHECK(rounded.HasPath());
    CHECK(AsObject(rounded).ObjectBoundingBox() == FloatRect(10, 20, 30, 40));

    auto &circle = RendererById<LegacyRenderSVGShape>(*fixture.Document, u8"circle");
    CHECK(circle.GetShapeType() == LegacyRenderSVGShape::ShapeType::Circle);
    CHECK(AsObject(circle).ObjectBoundingBox() == FloatRect(40, 40, 20, 20));
    CHECK(AsObject(circle).StrokeBoundingBox() == FloatRect(39, 39, 22, 22));

    // A path's approximate stroke box (its repaint rect) allows for miter joins (the miter limit, 4, times
    // half the stroke); a line's for square caps (half the stroke times the square root of 2).
    auto &path = RendererById<LegacyRenderSVGPath>(*fixture.Document, u8"path");
    CHECK(path.GetShapeType() == LegacyRenderSVGShape::ShapeType::Path);
    CHECK(AsObject(path).ObjectBoundingBox() == FloatRect(0, 0, 20, 20));
    CHECK(AsObject(path).RepaintRectInLocalCoordinates() == FloatRect(-4, -4, 28, 28));
    auto &line = RendererById<LegacyRenderSVGPath>(*fixture.Document, u8"line");
    CHECK(line.GetShapeType() == LegacyRenderSVGShape::ShapeType::Line);
    auto lineRepaintRect = AsObject(line).RepaintRectInLocalCoordinates();
    CHECK(lineRepaintRect.X() == Catch::Approx(-std::numbers::sqrt2_v<float>));
    CHECK(lineRepaintRect.Width() == Catch::Approx(10 + 2 * std::numbers::sqrt2_v<float>));

    // The accurate stroke box comes from the host's stroker.
    {
      PathStrokeStrategyScope scope;
      fixture.Document->GetElementById(u8"path")->Renderer()->SetNeedsBoundariesUpdate();
      fixture.Document->GetElementById(u8"path")->Renderer()->SetNeedsLayout();
      (void)fixture.Document->UpdateLayout();
      CHECK(AsObject(path).StrokeBoundingBox() == FloatRect(-1, -1, 22, 22));
      CHECK(scope.Strategy.LastStyle.Thickness == 2);
      CHECK(scope.Strategy.LastStyle.Cap == LineCap::Square);
    }

    // A zero length subpath with round caps is drawn as a dot, which counts in the stroke box.
    auto &dot = RendererById<LegacyRenderSVGPath>(*fixture.Document, u8"dot");
    CHECK(AsObject(dot).ObjectBoundingBox() == FloatRect(70, 70, 0, 0));
    CHECK(AsObject(dot).RepaintRectInLocalCoordinates().Contains(FloatRect(68, 68, 4, 4)));

    // A shape with a zero width or height isn't rendered, and doesn't count in its container's boxes.
    auto &empty = RendererById<LegacyRenderSVGShape>(*fixture.Document, u8"empty");
    CHECK(empty.IsRenderingDisabled());
    auto &g = RendererById<LegacyRenderSVGContainer>(*fixture.Document, u8"g");
    CHECK(g.IsObjectBoundingBoxValid());
    CHECK(g.ObjectBoundingBox() == FloatRect(0, 0, 70, 70));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGShape: hit testing",
                   "[Rendering][SVG][LegacyRenderSVGShape]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=400 height=100>)html"
      u8R"html(<rect id=rect x=10 y=10 width=40 height=40 stroke=black stroke-width=10 />)html"
      u8R"html(<rect id=hollow x=60 y=10 width=40 height=40 fill=none stroke=black stroke-width=10 />)html"
      u8R"html(<circle id=circle cx=150 cy=30 r=20 />)html"
      u8R"html(<path id=path d="M 200 10 L 240 50 L 200 50 z" fill-rule=evenodd />)html"
      u8R"html(<rect id=none x=260 y=10 width=40 height=40 fill=none pointer-events=fill />)html"
      u8R"html(<rect id=hidden x=310 y=10 width=40 height=40 visibility=hidden />)html"
      u8R"html(<rect id=painted x=360 y=10 width=30 height=30 fill=none pointer-events=painted /></svg></body>)html"};
    (void)fixture.Document->UpdateLayout();

    auto hit = [&](float x, float y) -> dom::Element *
    {
      auto element = fixture.Document->ElementFromPoint(x, y);
      return element.get();
    };
    auto *svg = fixture.Document->QuerySelector(u8"svg").ReleaseValue().get();

    // A filled rect is hit inside it, and on its stroke (rects with miter joins need no stroker).
    CHECK(hit(30, 30) == &ElementById(*fixture.Document, u8"rect"));
    CHECK(hit(7, 30) == &ElementById(*fixture.Document, u8"rect"));
    // An unfilled one only on its stroke; inside it, the <svg> is hit.
    CHECK(hit(62, 30) == &ElementById(*fixture.Document, u8"hollow"));
    CHECK(hit(80, 30) == svg);

    // Circles are hit inside the circle, not its bounding box's corners.
    CHECK(hit(150, 30) == &ElementById(*fixture.Document, u8"circle"));
    CHECK(hit(132, 12) == svg);

    // Paths by their fill rule.
    CHECK(hit(210, 45) == &ElementById(*fixture.Document, u8"path"));
    CHECK(hit(235, 15) == svg);

    // `pointer-events: fill` hits the fill even when there's none; `painted` doesn't.
    CHECK(hit(280, 30) == &ElementById(*fixture.Document, u8"none"));
    CHECK(hit(375, 25) == svg);

    // Hidden shapes aren't hit with the initial `pointer-events`.
    CHECK(hit(330, 30) == svg);

    SECTION("strokes of paths are hit as the host's stroker strokes them")
    {
      ResolverFixture paths {
        u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
        u8R"html(<path id=open d="M 10 50 L 190 50" stroke=black stroke-width=10 /></svg></body>)html"};
      (void)paths.Document->UpdateLayout();
      auto *pathsSvg = paths.Document->QuerySelector(u8"svg").ReleaseValue().get();

      // Without the host's stroker, nothing is inside a stroke.
      CHECK(paths.Document->ElementFromPoint(100, 52).get() == pathsSvg);

      PathStrokeStrategyScope scope;
      CHECK(paths.Document->ElementFromPoint(100, 52).get()
            == paths.Document->GetElementById(u8"open").get());
      CHECK(paths.Document->ElementFromPoint(100, 58).get() == pathsSvg);
      CHECK(scope.Strategy.LastStyle.Thickness == 10);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGShape: painting",
                   "[Rendering][SVG][LegacyRenderSVGShape]")
  {
    // The root clips to its viewport. A plain rect fills and strokes without a path, the fill with its colour
    // and the stroke with its colour, width, caps, joins and miter limit; a circle fills as an ellipse, at
    // its fill opacity. `paint-order` picks the order, and `stroke: none` paints no stroke.
    CHECK(Painted(u8R"html(<body style="margin: 0"><svg width=100 height=50>)html"
                  u8R"html(<rect x=10 y=10 width=20 height=20 fill=green stroke=blue stroke-width=2 />)html"
                  u8R"html(<circle cx=60 cy=20 r=10 fill=red fill-opacity=0.5 paint-order=stroke />)html"
                  u8R"html(</svg></body>)html")
          == Canvas()
               + u8R"(
(save)
(clip
  (rect at (0,0) size 100x50))
(save)
(set-inline-fill-color
  (color #008000))
(fill-rect
  (rect at (10,10) size 20x20)
  (requiresClipToRect 1))
(set-line-cap
  (line-cap BUTT))
(set-line-join
  (line-join MITER))
(set-miter-limit
  (mitre-limit 4.00))
(set-inline-stroke
  (color #0000FF)
  (thickness 2.00))
(stroke-rect
  (rect at (10,10) size 20x20)
  (line-width 2.00))
(restore)
(save)
(set-state
  (change-flags [fill-brush, alpha])
  (fill-brush)"
                 u8" "
                 u8R"(
    (color #FF0000))
  (alpha 0.50))
(fill-ellipse
  (rect at (50,10) size 20x20))
(restore)
(restore))");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGShape: non-scaling strokes",
                   "[Rendering][SVG][LegacyRenderSVGShape]")
  {
    // The path is mapped to the screen and stroked there, under the inverse of the transform to the screen,
    // so the stroke's width isn't scaled.
    auto painted =
      Painted(u8R"html(<body style="margin: 0"><svg width=200 height=100><g transform="scale(3 1)">)html"
              u8R"html(<rect x=10 y=8 width=20 height=48 fill=none stroke=blue stroke-width=2 )html"
              u8R"html(vector-effect=non-scaling-stroke /></g></svg></body>)html");
    CHECK(painted.find(u8R"((concatenate-ctm
  (ctm {m=((0.33,0.00)(0.00,1.00)) t=(0.00,0.00)})))")
          != utf8_string::npos);
    CHECK(painted.find(u8R"((stroke-path
  (path move to (30,8), add line to (90,8), add line to (90,56), add line to (30,56), close subpath)))")
          != utf8_string::npos);
  }
}
