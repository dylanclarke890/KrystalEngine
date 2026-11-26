#include "Krystal.Web/Rendering/RenderObject.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Core/Geometry/Quad.hpp"
#include "Krystal.Web.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Web.Tests/Rendering/RenderTestUtils.hpp"
#include "Krystal.Web.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Web/Rendering/RenderBox.hpp"
#include "Krystal.Web/Rendering/RenderInline.hpp"

namespace krys::web::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderObject: geometry mapping", "[Rendering][RenderObject]")
  {
    ResolverFixture fixture {layout::tests::AhemDocument(
      u8R"(
        #outer { margin: 10px 0 0 20px; border: 3px solid; width: 200px; height: 100px }
        #inner { margin-left: 7px; width: 50px; height: 20px }
        #rel { position: relative; left: 5px; top: 6px; width: 10px; height: 10px }
        #moved { transform: translate(30px, 40px); width: 10px; height: 10px }
        #w { width: 30px }
      )",
      u8R"(<div id=outer><div id=inner></div><div id=rel></div><div id=moved></div></div>)"
      u8R"(<div id=w><span id=inl>aa bb</span></div>)")};

    SECTION("Points")
    {
      // The outer box's border box starts at (20, 10); the inner box is inside its border.
      auto &inner = LaidOutRendererOf(fixture, u8"inner");
      CHECK(inner.LocalToAbsolute() == PointF(30, 13));
      CHECK(inner.LocalToAbsolute(PointF(1, 2)) == PointF(31, 15));
      CHECK(inner.AbsoluteToLocal(PointF(31, 15)) == PointF(1, 2));

      // Relative positioning offsets the box.
      auto &rel = LaidOutRendererOf(fixture, u8"rel");
      CHECK(rel.LocalToAbsolute() == PointF(28, 39));
    }

    SECTION("Transforms")
    {
      auto &moved = LaidOutRendererOf(fixture, u8"moved");
      CHECK(moved.LocalToAbsolute(PointF(), MapCoordinatesMode::UseTransforms) == PointF(53, 83));
      // Without UseTransforms, the transform is ignored.
      CHECK(moved.LocalToAbsolute() == PointF(23, 43));
      CHECK(moved.AbsoluteToLocal(PointF(53, 83), MapCoordinatesMode::UseTransforms) == PointF());
      CHECK(moved.AbsoluteBoundingBoxRect() == RectI(53, 83, 10, 10));
    }

    SECTION("Quads")
    {
      List<QuadF> quads;
      Downcast<RenderBox>(LaidOutRendererOf(fixture, u8"outer")).AbsoluteQuads(quads);
      REQUIRE(quads.size() == 1);
      CHECK(quads[0].BoundingBox() == RectF(20, 10, 206, 106));

      // An inline has a quad per line.
      quads.clear();
      Downcast<RenderInline>(LaidOutRendererOf(fixture, u8"inl")).AbsoluteQuads(quads);
      REQUIRE(quads.size() == 2);
      CHECK(quads[0].BoundingBox() == RectF(0, 116, 20, 10));
      CHECK(quads[1].BoundingBox() == RectF(0, 126, 20, 10));
    }
  }
}
