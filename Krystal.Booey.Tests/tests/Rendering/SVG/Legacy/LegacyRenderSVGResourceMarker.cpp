#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMarker.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResources.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/SVGResourcesCache.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"

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

    KRYS_NODISCARD bool Contains(utf8_stringview text, utf8_stringview part) noexcept
    {
      return text.find(part) != utf8_stringview::npos;
    }

    /// @brief A page with a path marked at every vertex by a 4x4 marker with the given attributes.
    KRYS_NODISCARD utf8_string MarkedPath(utf8_stringview markerAttributes,
                                          utf8_stringview pathAttributes = {})
    {
      return u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100><marker id=k markerWidth=4 markerHeight=4 refX=2 refY=2 )html"
             + utf8_string(markerAttributes)
             + u8R"html(><rect width=4 height=4 fill=blue /></marker><path id=p d="M10 10 L50 10 L50 50" stroke=black )html"
               u8R"html(marker-start="url(#k)" marker-mid="url(#k)" marker-end="url(#k)" )html"
             + utf8_string(pathAttributes) + u8" /></svg></body>";
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceMarker: the render tree",
                   "[Rendering][SVG][LegacyRenderSVGResourceMarker]")
  {
    ResolverFixture fixture {MarkedPath(u8"orient=auto")};

    // A marker dumps its id, units, reference point and angle; a marked path's bounds include its markers,
    // and it dumps the markers it uses.
    CHECK(ExternalRepresentation(fixture.Frame.get()) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x104
  RenderBlock {HTML} at (0,0) size 800x104
    RenderBody {BODY} at (0,0) size 800x104
      RenderSVGRoot {svg} at (8,7) size 45x45
        RenderSVGResourceMarker {marker} [id="k"] [markerUnits=strokeWidth] [ref at (2,2)] [angle=auto]
          RenderSVGRect {rect} at (0,0) size 4x4 [fill={[type=SOLID] [color=#0000FF]}] [x=0.00] [y=0.00] [width=4.00] [height=4.00]
        RenderSVGPath {path} at (8,7) size 45x45 [stroke={[type=SOLID] [color=#000000]}] [fill={[type=SOLID] [color=#000000]}] [start marker=k] [middle marker=k] [end marker=k] [data="M 10 10 L 50 10 L 50 50"]
)");

    auto *renderer = ElementById(*fixture.Document, u8"p").Renderer();
    REQUIRE(renderer);
    auto *resources = SVGResourcesCache::CachedResourcesForRenderer(*renderer);
    REQUIRE(resources);
    auto *marker = ElementById(*fixture.Document, u8"k").Renderer();
    CHECK(resources->MarkerStart() == marker);
    CHECK(resources->MarkerMid() == marker);
    CHECK(resources->MarkerEnd() == marker);
    CHECK_FALSE(resources->MarkerReverseStart());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceMarker: painting",
                   "[Rendering][SVG][LegacyRenderSVGResourceMarker]")
  {
    // Markers are drawn after the stroke at each vertex, with their reference point on it, clipped to their
    // viewport and turned along the path: the middle one bisects the angle between its segments.
    auto painted = PaintFixture(MarkedPath(u8"orient=auto")).Painted();
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8"(stroke-path"));
    CHECK(Contains(painted, u8"(ctm {m=((1.00,0.00)(0.00,1.00)) t=(8.00,8.00)})"));
    CHECK(Contains(painted, u8"(ctm {m=((0.71,0.71)(-0.71,0.71)) t=(50.00,7.17)})"));
    CHECK(Contains(painted, u8"(ctm {m=((0.00,1.00)(-1.00,0.00)) t=(52.00,48.00)})"));
    CHECK(Contains(painted, u8R"((clip
  (rect at (0,0) size 4x4)))"));

    // A fixed angle turns every marker by it.
    painted = PaintFixture(MarkedPath(u8"orient=90")).Painted();
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8"(ctm {m=((0.00,1.00)(-1.00,0.00)) t=(12.00,8.00)})"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceMarker: auto-start-reverse",
                   "[Rendering][SVG][LegacyRenderSVGResourceMarker]")
  {
    // The start marker points the other way.
    PaintFixture fixture {MarkedPath(u8"orient=auto-start-reverse")};
    auto painted = fixture.Painted();
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8"t=(12.00,12.00)})"));

    auto *renderer = ElementById(*fixture.Document, u8"p").Renderer();
    REQUIRE(renderer);
    auto *resources = SVGResourcesCache::CachedResourcesForRenderer(*renderer);
    REQUIRE(resources);
    CHECK(resources->MarkerReverseStart());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceMarker: units",
                   "[Rendering][SVG][LegacyRenderSVGResourceMarker]")
  {
    // Markers in stroke width units are scaled by the stroke's width; ones in user space aren't.
    auto painted = PaintFixture(MarkedPath(u8"", u8"stroke-width=2")).Painted();
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8"(ctm {m=((2.00,0.00)(0.00,2.00)) t=(6.00,6.00)})"));

    painted = PaintFixture(MarkedPath(u8"markerUnits=userSpaceOnUse", u8"stroke-width=2")).Painted();
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8"(ctm {m=((1.00,0.00)(0.00,1.00)) t=(8.00,8.00)})"));
  }
}
