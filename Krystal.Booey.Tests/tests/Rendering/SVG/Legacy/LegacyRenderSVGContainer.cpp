#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGHiddenContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGTransformableContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGViewportContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Core/TypeCast.hpp"

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

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGContainer: transforms",
                   "[Rendering][SVG][LegacyRenderSVGContainer]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<g id=g transform="translate(10 20) scale(2)"><g id=inner /></g>)html"
      u8R"html(<g id=css style="transform: translate(1px, 2px)" transform="scale(3)" />)html"
      u8R"html(<svg id=v x=5 y=6 width=50 height=40 viewBox="0 0 10 10"><g/></svg>)html"
      u8R"html(<svg id=w x=1 y=2 width=30 height=30 viewBox="0 0 10 20" preserveAspectRatio="none"/>)html"
      u8R"html(</svg></body>)html"};
    (void)fixture.Document->UpdateLayout();

    // A <g>'s transform maps to its parent's coordinates; the property wins over the attribute.
    auto &g = RendererById<LegacyRenderSVGTransformableContainer>(*fixture.Document, u8"g");
    CHECK(g.LocalToParentTransform() == AffineTransform(2, 0, 0, 2, 10, 20));
    CHECK(AsObject(g).LocalTransform() == g.LocalToParentTransform());
    CHECK(RendererById<LegacyRenderSVGTransformableContainer>(*fixture.Document, u8"inner")
            .LocalToParentTransform()
            .IsIdentity());
    CHECK(
      RendererById<LegacyRenderSVGTransformableContainer>(*fixture.Document, u8"css").LocalToParentTransform()
      == AffineTransform(1, 0, 0, 1, 1, 2));

    // An inner <svg> maps its view box into its viewport, at its x and y.
    auto &v = RendererById<LegacyRenderSVGViewportContainer>(*fixture.Document, u8"v");
    CHECK(v.Viewport() == FloatRect(5, 6, 50, 40));
    CHECK(AsObject(v).LocalToParentTransform() == AffineTransform(4, 0, 0, 4, 10, 6));
    auto &w = RendererById<LegacyRenderSVGViewportContainer>(*fixture.Document, u8"w");
    CHECK(AsObject(w).LocalToParentTransform() == AffineTransform(3, 0, 0, 1.5, 1, 2));

    // Changing the attributes lays the containers out again.
    REQUIRE_FALSE(
      ElementById(*fixture.Document, u8"g").SetAttribute(u8"transform", u8"rotate(90)").HasException());
    REQUIRE_FALSE(ElementById(*fixture.Document, u8"v").SetAttribute(u8"x", u8"15").HasException());
    (void)fixture.Document->UpdateLayout();
    CHECK(g.LocalToParentTransform() == AffineTransform::MakeRotation(90));
    CHECK(AsObject(v).LocalToParentTransform() == AffineTransform(4, 0, 0, 4, 20, 6));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGContainer: bounding boxes",
                   "[Rendering][SVG][LegacyRenderSVGContainer]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body><svg width=200 height=100><g id=g><g id=empty /></g>)html"
      u8R"html(<defs id=defs><g/></defs></svg></body>)html"};
    (void)fixture.Document->UpdateLayout();

    // A container without graphics has no object bounding box, and its children's don't count (containers of
    // shapes are in LegacyRenderSVGShape's tests).
    auto &g = RendererById<LegacyRenderSVGContainer>(*fixture.Document, u8"g");
    CHECK_FALSE(g.IsObjectBoundingBoxValid());
    CHECK(g.ObjectBoundingBox() == FloatRect());
    CHECK(AsObject(g).StrokeBoundingBox() == FloatRect());
    CHECK(AsObject(g).RepaintRectInLocalCoordinates() == FloatRect());
    CHECK(AsObject(g).RepaintRectInLocalCoordinates(RepaintRectCalculation::Accurate) == FloatRect());

    // Hidden containers take up no space.
    auto &defs = RendererById<LegacyRenderSVGHiddenContainer>(*fixture.Document, u8"defs");
    CHECK(defs.IsLegacyRenderSVGHiddenContainer());
    CHECK(defs.AbsoluteClippedOverflowRectForRenderTreeAsText() == LayoutRect());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGRenderSupport: the tree root and overflow",
                   "[Rendering][SVG][SVGRenderSupport]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body><svg id=s width=200 height=100><g id=g><svg id=v><g id=inner /></svg></g>)html"
      u8R"html(<svg id=visible style="overflow: visible" /></svg></body>)html"};
    (void)fixture.Document->UpdateLayout();

    auto &inner = RendererById<LegacyRenderSVGContainer>(*fixture.Document, u8"inner");
    CHECK(static_cast<RenderObject *>(SVGRenderSupport::FindTreeRootObject(inner))
          == ElementById(*fixture.Document, u8"s").Renderer());

    // The first layout updated every transform on the way to the root.
    CHECK(SVGRenderSupport::TransformToRootChanged(&inner));

    // Inner <svg>s clip to their viewport unless their overflow is visible (the user agent sheet hides it).
    CHECK(
      SVGRenderSupport::IsOverflowHidden(RendererById<LegacyRenderSVGContainer>(*fixture.Document, u8"v")));
    CHECK_FALSE(SVGRenderSupport::IsOverflowHidden(
      RendererById<LegacyRenderSVGContainer>(*fixture.Document, u8"visible")));
  }
}
