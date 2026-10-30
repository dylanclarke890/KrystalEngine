#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceFilter.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
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
    KRYS_NODISCARD bool Contains(utf8_stringview text, utf8_stringview part) noexcept
    {
      return text.find(part) != utf8_stringview::npos;
    }

    constexpr const char8_t *FilteredShape =
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<filter id=f x=0 y=0 width=1 height=1><feOffset dx=2 dy=3 /></filter>)html"
      u8R"html(<rect id=r x=10 y=10 width=40 height=20 fill=green filter="url(#f)" /></svg></body>)html";
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceFilter: the render tree",
                   "[Rendering][SVG][LegacyRenderSVGResourceFilter]")
  {
    ResolverFixture fixture {FilteredShape};

    // A filter dumps its units and its graph; its primitives dump nothing; a filtered shape dumps the filter
    // it uses, with the filter region.
    auto text = ExternalRepresentation(fixture.Frame.get());
    CAPTURE(css::tests::ToNarrow(text));
    CHECK(Contains(
      text,
      u8R"(RenderSVGResourceFilter {filter} [id="f"] [filterUnits=objectBoundingBox] [primitiveUnits=userSpaceOnUse]
          [feOffset dx="2.00" dy="3.00"]
            [SourceGraphic]
)"));
    CHECK_FALSE(Contains(text, u8"RenderSVGResourceFilterPrimitive"));
    CHECK(Contains(text, u8R"([filter="f"] RenderSVGResourceFilter {filter} at (10,10) size 40x20)"));

    auto *renderer = fixture.ById(u8"r").Renderer();
    REQUIRE(renderer);
    auto *resources = SVGResourcesCache::CachedResourcesForRenderer(*renderer);
    REQUIRE(resources);
    CHECK(resources->Filter() == fixture.ById(u8"f").Renderer());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceFilter: painting",
                   "[Rendering][SVG][LegacyRenderSVGResourceFilter]")
  {
    // A filtered shape is drawn into an image buffer, which is drawn through the filter.
    PaintFixture fixture(FilteredShape, {200, 100}, true);
    auto painted = fixture.Painted();
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8R"((draw-filtered-image-buffer
  (source-image-rect at (10,10) size 40x20)))"));

    // A filter that does nothing isn't applied.
    auto identity =
      PaintFixture(u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
                   u8R"html(<filter id=f><feOffset /></filter>)html"
                   u8R"html(<rect x=10 y=10 width=40 height=20 filter="url(#f)" /></svg></body>)html",
                   {200, 100}, true)
        .Painted();
    CAPTURE(css::tests::ToNarrow(identity));
    CHECK_FALSE(Contains(identity, u8"draw-filtered-image-buffer"));
    CHECK(Contains(identity, u8"(fill-rect"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceFilter: filtering boxes",
                   "[Rendering][SVG][LegacyRenderSVGResourceFilter]")
  {
    // A box whose filter is a url() is painted through the SVG filter it refers to, and the filter's layer
    // watches the filter element.
    PaintFixture fixture(
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=0 height=0>)html"
      u8R"html(<filter id=f><feGaussianBlur stdDeviation=2 /></filter></svg>)html"
      u8R"html(<div id=box style="width: 40px; height: 30px; background: green; filter: url(#f)"></div></body>)html",
      {200, 100}, true);
    auto painted = fixture.Painted();
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8"(draw-filtered-image-buffer"));

    auto *layer = Downcast<RenderLayerModelObject>(*fixture.ById(u8"box").Renderer()).Layer();
    REQUIRE(layer);
    CHECK(layer->FilterMovesPixels());
    CHECK(layer->HasFilterOutsets());
  }
}
