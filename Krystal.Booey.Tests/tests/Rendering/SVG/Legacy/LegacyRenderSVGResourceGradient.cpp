#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceGradient.hpp"
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
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceGradient: the render tree",
                   "[Rendering][SVG][LegacyRenderSVGResourceGradient]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100><defs>)html"
      u8R"html(<linearGradient id=lg x2=0 y2=1><stop offset=0 stop-color=red /><stop offset=50% stop-color=blue stop-opacity=0.5 /></linearGradient>)html"
      u8R"html(<linearGradient id=inherited href="#lg" gradientUnits=userSpaceOnUse x1=10 spreadMethod=reflect gradientTransform="rotate(90)" />)html"
      u8R"html(<radialGradient id=rg cx=0.25 fr=0.1><stop offset=1 stop-color=green /></radialGradient>)html"
      u8R"html(<radialGradient id=userSpace gradientUnits=userSpaceOnUse spreadMethod=repeat />)html"
      u8R"html(</defs><rect width=10 height=10 fill="url(#lg)" stroke="url(#rg)" /></svg></body>)html"};

    // A gradient dumps its id and the attributes it paints with, collected from the gradients it references
    // (the second takes its end from the first): the units, spread method and transform, then its geometry,
    // resolved in its units (bounding box units are fractions). Its stops dump their offset and colour (with
    // the stop's opacity). An unspecified focal point is the centre; the default centre and radius of a
    // gradient in user space units are 50% of the viewport's width, height and normalised diagonal. Shapes
    // painted with a gradient dump its type and id.
    CHECK(ExternalRepresentation(fixture.Frame.get()) == u8R"(layer at (0,0) size 800x600
  RenderView at (0,0) size 800x600
layer at (0,0) size 800x104
  RenderBlock {HTML} at (0,0) size 800x104
    RenderBody {BODY} at (0,0) size 800x104
      RenderSVGRoot {svg} at (0,0) size 11x11
        RenderSVGHiddenContainer {defs} at (0,0) size 0x0
          RenderSVGResourceLinearGradient {linearGradient} [id="lg"] [gradientUnits=objectBoundingBox] [start=(0,0)] [end=(0,1)]
            RenderSVGGradientStop {stop} [offset=0.00] [color=#FF0000]
            RenderSVGGradientStop {stop} [offset=0.50] [color=#0000FF80]
          RenderSVGResourceLinearGradient {linearGradient} [id="inherited"] [gradientUnits=userSpaceOnUse] [spreadMethod=REFLECT] [gradientTransform={m=((0.00,1.00)(-1.00,0.00)) t=(0.00,0.00)}] [start=(10,0)] [end=(0,1)]
          RenderSVGResourceRadialGradient {radialGradient} [id="rg"] [gradientUnits=objectBoundingBox] [center=(0.25,0.50)] [focal=(0.25,0.50)] [radius=0.50] [focalRadius=0.10]
            RenderSVGGradientStop {stop} [offset=1.00] [color=#008000]
          RenderSVGResourceRadialGradient {radialGradient} [id="userSpace"] [gradientUnits=userSpaceOnUse] [spreadMethod=REPEAT] [center=(100,50)] [focal=(100,50)] [radius=79.06] [focalRadius=0.00]
        RenderSVGRect {rect} at (0,0) size 11x11 [stroke={[type=RADIAL-GRADIENT] [id="rg"]}] [fill={[type=LINEAR-GRADIENT] [id="lg"]}] [x=0.00] [y=0.00] [width=10.00] [height=10.00]
)");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceGradient: painting",
                   "[Rendering][SVG][LegacyRenderSVGResourceGradient]")
  {
    // A gradient in bounding box units is mapped onto the shape's bounding box; its stops' offsets only ever
    // increase, and one past 1 is clamped. The fill opacity is the context's alpha.
    auto painted = Painted(
      u8R"html(<body style="margin: 0"><svg width=100 height=50>)html"
      u8R"html(<linearGradient id=g><stop offset=0.6 stop-color=red /><stop offset=0.2 stop-color=blue /><stop offset=2 stop-color=lime /></linearGradient>)html"
      u8R"html(<rect x=10 y=10 width=40 height=20 fill="url(#g)" fill-opacity=0.5 /></svg></body>)html");
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8"(gradient-space-transform {m=((40.00,0.00)(0.00,20.00)) t=(10.00,10.00)})"));
    CHECK(Contains(painted, u8"(stops [#FF0000 at 0.60, #0000FF at 0.60, #00FF00 at 1.00])"));
    CHECK(Contains(painted, u8"(alpha 0.50)"));
    CHECK(Contains(painted, u8"(fill-rect\n  (rect at (10,10) size 40x20)"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceGradient: fallbacks",
                   "[Rendering][SVG][LegacyRenderSVGResourceGradient]")
  {
    // A paint that references nothing paints its fallback colour, or nothing with `none` or without one
    // (SVG 2); so does a gradient in bounding box units on a shape without a width or height.
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
                             u8R"html(<linearGradient id=g><stop stop-color=red /></linearGradient>)html"
                             u8R"html(<rect width=10 height=10 fill="url(#missing) green" />)html"
                             u8R"html(<rect width=10 height=10 fill="url(#missing) none" />)html"
                             u8R"html(<rect width=10 height=10 fill="url(#missing)" />)html"
                             u8R"html(<line x2=10 stroke="url(#g) blue" /></svg></body>)html"};

    auto text = ExternalRepresentation(fixture.Frame.get());
    CAPTURE(css::tests::ToNarrow(text));
    CHECK(Contains(text, u8"RenderSVGRect {rect} at (0,0) size 10x10 [fill={[type=SOLID] [color=#008000]}]"));
    CHECK(Contains(text, u8"RenderSVGRect {rect} at (0,0) size 10x10 [x=0.00]"));
    CHECK(Contains(text, u8"[stroke={[type=LINEAR-GRADIENT] [id=\"g\"]}]"));

    // The line's stroke paints in the fallback colour.
    auto painted =
      PaintFixture(u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
                   u8R"html(<linearGradient id=g><stop stop-color=red /></linearGradient>)html"
                   u8R"html(<line x2=10 stroke="url(#g) blue" /></svg></body>)html")
        .Painted();
    CAPTURE(css::tests::ToNarrow(painted));
    CHECK(Contains(painted, u8"(color #0000FF)"));
    CHECK_FALSE(Contains(painted, u8"gradient"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceGradient: resources and invalidation",
                   "[Rendering][SVG][LegacyRenderSVGResourceGradient]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<linearGradient id=g><stop id=s stop-color=red /></linearGradient>)html"
      u8R"html(<linearGradient id=linked href="#g" />)html"
      u8R"html(<rect id=r width=10 height=10 fill="url(#g)" stroke="url(#later)" /></svg></body>)html"};
    (void)fixture.Document->UpdateLayout();

    // The rect's resources are the gradient it fills with; a gradient's resource is the gradient it links to.
    auto *resources = ResourcesById(*fixture.Document, u8"r");
    REQUIRE(resources);
    CHECK(resources->Fill() == ElementById(*fixture.Document, u8"g").Renderer());
    CHECK(resources->Stroke() == nullptr);
    auto *linkedResources = ResourcesById(*fixture.Document, u8"linked");
    REQUIRE(linkedResources);
    CHECK(linkedResources->LinkedResource() == ElementById(*fixture.Document, u8"g").Renderer());

    // A stop's colour changes the gradient.
    (void)ElementById(*fixture.Document, u8"s").SetAttribute(u8"stop-color", u8"blue");
    auto text = ExternalRepresentation(fixture.Frame.get());
    CHECK(Contains(text, u8"RenderSVGGradientStop {stop} [offset=0.00] [color=#0000FF]"));

    // The stroke waits for its gradient, and takes it once there's one with the id.
    auto gradient = fixture.Document->CreateElementNS(dom::DOMStringAtom(infra::Namespaces::SVG),
                                                      dom::DOMStringAtom(u8"radialGradient"));
    REQUIRE_FALSE(gradient.HasException());
    (void)gradient.Value()->SetAttribute(u8"id", u8"later");
    (void)ElementById(*fixture.Document, u8"g").ParentNode()->AppendChild(*gradient.Value());
    (void)fixture.Document->UpdateLayout();
    resources = ResourcesById(*fixture.Document, u8"r");
    REQUIRE(resources);
    CHECK(resources->Stroke() == gradient.Value()->Renderer());

    // Removing the gradient drops it from the rect's resources, and the rect waits for the id again.
    (void)ElementById(*fixture.Document, u8"g").Remove();
    (void)fixture.Document->UpdateLayout();
    resources = ResourcesById(*fixture.Document, u8"r");
    CHECK((!resources || !resources->Fill()));
    CHECK_FALSE(Contains(ExternalRepresentation(fixture.Frame.get()), u8"LINEAR-GRADIENT"));
    CHECK(fixture.Document->IsIdOfPendingSVGResource(dom::DOMStringAtom(u8"g")));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LegacyRenderSVGResourceGradient: cycles",
                   "[Rendering][SVG][LegacyRenderSVGResourceGradient]")
  {
    // Gradients that reference each other in a cycle take the attributes along it once, and the link that
    // closes the cycle is dropped from their resources.
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><svg width=200 height=100>)html"
      u8R"html(<linearGradient id=a href="#b" x1=0.5 />)html"
      u8R"html(<linearGradient id=b href="#a" spreadMethod=repeat><stop stop-color=red /></linearGradient>)html"
      u8R"html(<rect width=10 height=10 fill="url(#a)" /></svg></body>)html"};

    auto text = ExternalRepresentation(fixture.Frame.get());
    CAPTURE(css::tests::ToNarrow(text));
    CHECK(Contains(text,
                   u8"[id=\"a\"] [gradientUnits=objectBoundingBox] [spreadMethod=REPEAT] [start=(0.50,0)]"));
    auto *a = ResourcesById(*fixture.Document, u8"a");
    auto *b = ResourcesById(*fixture.Document, u8"b");
    CHECK(((a && a->LinkedResource()) != (b && b->LinkedResource())));
  }
}
