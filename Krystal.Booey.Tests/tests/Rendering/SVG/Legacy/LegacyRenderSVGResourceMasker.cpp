#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceMasker.hpp"
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

    constexpr const char8_t *MaskedShape =
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<mask id=m><rect width=20 height=20 fill=white /></mask>)html"
      u8R"html(<rect id=r width=40 height=40 fill=red mask="url(#m)" /></svg></body>)html";
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceMasker: the render tree",
                   "[Rendering][SVG][LegacyRenderSVGResourceMasker]")
  {
    ResolverFixture fixture {MaskedShape};

    // A mask dumps its id and units; a masked shape's bounds are clipped to the mask's content, and it dumps
    // the mask it uses.
    CHECK(ExternalRepresentation(fixture.Frame.get()) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x104
  RenderBlock {HTML} at (0,0) size 800x104
    RenderBody {BODY} at (0,0) size 800x104
      RenderSVGRoot {svg} at (0,0) size 20x20
        RenderSVGResourceMasker {mask} [id="m"] [maskUnits=objectBoundingBox] [maskContentUnits=userSpaceOnUse]
          RenderSVGRect {rect} at (0,0) size 20x20 [fill={[type=SOLID] [color=#FFFFFF]}] [x=0.00] [y=0.00] [width=20.00] [height=20.00]
        RenderSVGRect {rect} at (0,0) size 20x20 [fill={[type=SOLID] [color=#FF0000]}] [x=0.00] [y=0.00] [width=40.00] [height=40.00]
          [masker="m"] RenderSVGResourceMasker {mask} at (0,0) size 20x20
)");

    auto *renderer = ElementById(*fixture.Document, u8"r").Renderer();
    REQUIRE(renderer);
    auto *resources = SVGResourcesCache::CachedResourcesForRenderer(*renderer);
    REQUIRE(resources);
    CHECK(resources->Masker() == ElementById(*fixture.Document, u8"m").Renderer());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceMasker: painting",
                   "[Rendering][SVG][LegacyRenderSVGResourceMasker]")
  {
    // A masked shape is clipped to an image of the mask's content.
    auto painted = PaintFixture(MaskedShape).Painted();
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8R"((clip-to-image-buffer
  (dest-rect at (0,0) size 20x20))
(set-inline-fill-color
  (color #FF0000))
(fill-rect
  (rect at (0,0) size 40x40))"));

    // A mask with no content masks everything out.
    auto empty =
      PaintFixture(
        u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
        u8R"html(<mask id=m /><rect width=40 height=40 fill=red mask="url(#m)" /></svg></body>)html")
        .Painted();
    CAPTURE(css::tests::ToNarrow(empty));
    CHECK_FALSE(Contains(empty, u8"#FF0000"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceMasker: masking boxes",
                   "[Rendering][SVG][LegacyRenderSVGResourceMasker]")
  {
    // A box whose mask image is a mask draws the mask's content in a layer that keeps only the box's pixels
    // under it.
    auto painted =
      PaintFixture(
        u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=0 height=0>)html"
        u8R"html(<mask id=m><rect width=20 height=20 fill=white /></mask></svg>)html"
        u8R"html(<div style="width: 40px; height: 40px; background: green; mask-image: url(#m)"></div></body>)html")
        .Painted();
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8"(composite-operation destination-in)"));
    CHECK(Contains(painted, u8R"((set-inline-fill-color
  (color #FFFFFF))
(fill-rect
  (rect at (0,0) size 20x20))"));
  }
}
