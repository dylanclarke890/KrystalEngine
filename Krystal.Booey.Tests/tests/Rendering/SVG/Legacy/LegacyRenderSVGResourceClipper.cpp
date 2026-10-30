#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceClipper.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD dom::Element &ElementById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto element = document.GetElementById(id);
      krys_assert(element);
      return *element;
    }

    KRYS_NODISCARD SVGResources *ResourcesById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto *renderer = ElementById(document, id).Renderer();
      krys_assert(renderer);
      return SVGResourcesCache::CachedResourcesForRenderer(*renderer);
    }

    KRYS_NODISCARD bool Contains(utf8_stringview text, utf8_stringview part) noexcept
    {
      return text.find(part) != utf8_stringview::npos;
    }

    constexpr const char8_t *ClippedShapes =
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<clipPath id=c><circle cx=20 cy=20 r=10 /></clipPath>)html"
      u8R"html(<clipPath id=d clipPathUnits=objectBoundingBox><rect width=.5 height=.5 /><rect x=.5 y=.5 width=.5 height=.5 /></clipPath>)html"
      u8R"html(<rect id=r width=40 height=40 fill=red clip-path="url(#c)" /><rect id=s x=50 width=40 height=40 clip-path="url(#d)" /></svg></body>)html";

    constexpr const char8_t *ClippedBoxes =
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=0 height=0><clipPath id=c><circle cx=20 cy=20 r=10 /></clipPath></svg>)html"
      u8R"html(<div id=box style="width: 40px; height: 40px; background: green; clip-path: url(#c)"></div></body>)html";
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceClipper: the render tree",
                   "[Rendering][SVG][LegacyRenderSVGResourceClipper]")
  {
    ResolverFixture fixture {ClippedShapes};

    // A clip path dumps its id and units; a clipped shape's bounds are clipped to the clip path's, in its
    // units, and it dumps the clip path it uses.
    CHECK(ExternalRepresentation(fixture.Frame.get()) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x104
  RenderBlock {HTML} at (0,0) size 800x104
    RenderBody {BODY} at (0,0) size 800x104
      RenderSVGRoot {svg} at (10,0) size 80x40
        RenderSVGResourceClipper {clipPath} [id="c"] [clipPathUnits=userSpaceOnUse]
          RenderSVGEllipse {circle} at (10,10) size 20x20 [fill={[type=SOLID] [color=#000000]}] [cx=20.00] [cy=20.00] [r=10.00]
        RenderSVGResourceClipper {clipPath} [id="d"] [clipPathUnits=objectBoundingBox]
          RenderSVGRect {rect} at (0,0) size 1x1 [fill={[type=SOLID] [color=#000000]}] [x=0.00] [y=0.00] [width=0.50] [height=0.50]
          RenderSVGRect {rect} at (0,0) size 1x1 [fill={[type=SOLID] [color=#000000]}] [x=0.50] [y=0.50] [width=0.50] [height=0.50]
        RenderSVGRect {rect} at (10,10) size 20x20 [fill={[type=SOLID] [color=#FF0000]}] [x=0.00] [y=0.00] [width=40.00] [height=40.00]
          [clipPath="c"] RenderSVGResourceClipper {clipPath} at (10,10) size 20x20
        RenderSVGRect {rect} at (50,0) size 40x40 [fill={[type=SOLID] [color=#000000]}] [x=50.00] [y=0.00] [width=40.00] [height=40.00]
          [clipPath="d"] RenderSVGResourceClipper {clipPath} at (50,0) size 40x40
)");

    auto *resources = ResourcesById(*fixture.Document, u8"r");
    REQUIRE(resources);
    CHECK(resources->Clipper() == ElementById(*fixture.Document, u8"c").Renderer());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceClipper: painting",
                   "[Rendering][SVG][LegacyRenderSVGResourceClipper]")
  {
    // A clip path of one shape clips to its path; one of several shapes is drawn into an image that the shape
    // is clipped to.
    auto painted = PaintFixture(ClippedShapes).Painted();
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8R"((clip-path
  (path add ellipse in rect at (10,10) size 20x20)
  (wind-rule NON-ZERO)))"));
    CHECK(Contains(painted, u8R"((clip-to-image-buffer
  (dest-rect at (50,0) size 40x40))
(fill-rect
  (rect at (50,0) size 40x40))"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceClipper: hit testing",
                   "[Rendering][SVG][LegacyRenderSVGResourceClipper]")
  {
    ResolverFixture fixture {ClippedShapes};
    (void)fixture.Document->UpdateLayout();
    auto &r = ElementById(*fixture.Document, u8"r");
    auto &s = ElementById(*fixture.Document, u8"s");

    // Only the parts of a shape inside its clip path are hit.
    CHECK(fixture.Document->ElementFromPoint(20, 20).get() == &r);
    CHECK(fixture.Document->ElementFromPoint(12, 12).get() != &r);
    CHECK(fixture.Document->ElementFromPoint(60, 10).get() == &s);
    CHECK(fixture.Document->ElementFromPoint(80, 30).get() == &s);
    CHECK(fixture.Document->ElementFromPoint(80, 10).get() != &s);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceClipper: clipping boxes",
                   "[Rendering][SVG][LegacyRenderSVGResourceClipper]")
  {
    // A box clipped by a clip path paints inside its path, in the box's coordinates.
    PaintFixture fixture {ClippedBoxes};
    auto painted = fixture.Painted();
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8R"((translate
  (x 0.00)
  (y 18.00))
(clip-path
  (path add ellipse in rect at (10,10) size 20x20)
  (wind-rule NON-ZERO))
(translate
  (x 0.00)
  (y -18.00))
(fill-composited-rect
  (rect at (0,18) size 40x40))"));

    // And is only hit inside it.
    auto &box = ElementById(*fixture.Document, u8"box");
    CHECK(fixture.Document->ElementFromPoint(20, 38).get() == &box);
    CHECK(fixture.Document->ElementFromPoint(2, 20).get() != &box);

    // Changing the clip path repaints the box with the new one.
    (void)ElementById(*fixture.Document, u8"c").FirstElementChild()->SetAttribute(u8"r", u8"5");
    painted = fixture.Painted();
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8"(path add ellipse in rect at (15,15) size 10x10)"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceClipper: cycles",
                   "[Rendering][SVG][LegacyRenderSVGResourceClipper]")
  {
    // Clip paths that clip each other drop the reference that closes the cycle, and still paint.
    PaintFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<clipPath id=a clip-path="url(#b)"><rect width=10 height=10 /></clipPath>)html"
      u8R"html(<clipPath id=b clip-path="url(#a)"><rect width=20 height=20 /></clipPath>)html"
      u8R"html(<rect width=40 height=40 clip-path="url(#a)" /></svg></body>)html"};
    auto painted = fixture.Painted();
    CHECK(Contains(painted, u8"(rect at (0,0) size 40x40)"));

    auto *a = ResourcesById(*fixture.Document, u8"a");
    auto *b = ResourcesById(*fixture.Document, u8"b");
    CHECK(((a && a->Clipper()) != (b && b->Clipper())));
  }
}
