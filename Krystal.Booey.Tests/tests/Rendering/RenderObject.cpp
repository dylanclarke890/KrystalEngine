#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD RenderElement &RendererOf(ResolverFixture &fixture, dom::DOMStringView id) noexcept
    {
      (void)fixture.Document->UpdateLayout();
      return *fixture.ById(id).Renderer();
    }
  }

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
      auto &inner = RendererOf(fixture, u8"inner");
      CHECK(inner.LocalToAbsolute() == FloatPoint(30, 13));
      CHECK(inner.LocalToAbsolute(FloatPoint(1, 2)) == FloatPoint(31, 15));
      CHECK(inner.AbsoluteToLocal(FloatPoint(31, 15)) == FloatPoint(1, 2));

      // Relative positioning offsets the box.
      auto &rel = RendererOf(fixture, u8"rel");
      CHECK(rel.LocalToAbsolute() == FloatPoint(28, 39));
    }

    SECTION("Transforms")
    {
      auto &moved = RendererOf(fixture, u8"moved");
      CHECK(moved.LocalToAbsolute(FloatPoint(), MapCoordinatesMode::UseTransforms) == FloatPoint(53, 83));
      // Without UseTransforms, the transform is ignored.
      CHECK(moved.LocalToAbsolute() == FloatPoint(23, 43));
      CHECK(moved.AbsoluteToLocal(FloatPoint(53, 83), MapCoordinatesMode::UseTransforms) == FloatPoint());
      CHECK(moved.AbsoluteBoundingBoxRect() == IntRect(53, 83, 10, 10));
    }

    SECTION("Quads")
    {
      List<FloatQuad> quads;
      Downcast<RenderBox>(RendererOf(fixture, u8"outer")).AbsoluteQuads(quads);
      REQUIRE(quads.size() == 1);
      CHECK(quads[0].BoundingBox() == FloatRect(20, 10, 206, 106));

      // An inline has a quad per line.
      quads.clear();
      Downcast<RenderInline>(RendererOf(fixture, u8"inl")).AbsoluteQuads(quads);
      REQUIRE(quads.size() == 2);
      CHECK(quads[0].BoundingBox() == FloatRect(0, 116, 20, 10));
      CHECK(quads[1].BoundingBox() == FloatRect(0, 126, 20, 10));
    }
  }
}
