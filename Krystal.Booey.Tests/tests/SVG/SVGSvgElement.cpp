#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey.Tests/SVG/TestSVGElement.hpp"
#include "Krystal.Booey/CSS/DOMMatrix2DInit.hpp"
#include "Krystal.Booey/DOM/DOMRect.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/SVG/SVGAngle.hpp"
#include "Krystal.Booey/SVG/SVGLength.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGMatrix.hpp"
#include "Krystal.Booey/SVG/SVGNumber.hpp"
#include "Krystal.Booey/SVG/SVGRect.hpp"
#include "Krystal.Booey/SVG/SVGTransform.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::svg::tests
{
  using style::tests::ResolverFixture;

  namespace
  {
    KRYS_NODISCARD SVGSvgElement &SvgById(dom::Document &document, dom::DOMStringView id) noexcept
    {
      auto element = document.GetElementById(id);
      krys_assert(element);
      return Downcast<SVGSvgElement>(Downcast<SVGElement>(*element));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGSVGElement: attributes", "[SVG][SVGSVGElement]")
  {
    auto document = ResolverFixture::Parse(u8"<svg id=s></svg>");
    auto &svg = SvgById(*document, u8"s");

    SECTION("x, y, width and height")
    {
      CHECK(svg.X().ValueAsString() == u8"0");
      CHECK(svg.Width().ValueAsString() == u8"100%");
      CHECK(svg.Height().ValueAsString() == u8"100%");

      REQUIRE_FALSE(svg.SetAttribute(u8"x", u8"10").HasException());
      REQUIRE_FALSE(svg.SetAttribute(u8"width", u8"200").HasException());
      REQUIRE_FALSE(svg.SetAttribute(u8"height", u8"-5").HasException());
      CHECK(svg.X().ValueAsString() == u8"10");
      CHECK(svg.Width().ValueAsString() == u8"200");
      // Negative sizes are errors, which leave the initial value.
      CHECK(svg.Height().ValueAsString() == u8"100%");

      svg.RemoveAttribute(u8"width");
      CHECK(svg.Width().ValueAsString() == u8"100%");

      // A change to the property is written to the attribute.
      REQUIRE_FALSE(svg.XAnimated().BaseVal()->SetValueAsString(u8"3px").HasException());
      CHECK(svg.GetAttribute(u8"x") == u8"3px");
    }

    SECTION("viewBox")
    {
      CHECK_FALSE(svg.HasValidViewBox());

      REQUIRE_FALSE(svg.SetAttribute(u8"viewBox", u8" 0,0 100 50 ").HasException());
      CHECK(svg.HasValidViewBox());
      CHECK_FALSE(svg.HasEmptyViewBox());
      CHECK(svg.ViewBox() == FloatRect(0, 0, 100, 50));
      CHECK(svg.ViewBoxString() == u8"0 0 100 50");

      // Errors leave no view box.
      for (auto value : {u8"0 0 -1 5", u8"0 0 5 -1", u8"1 2 3", u8"1 2 3 4 5", u8"a"})
      {
        REQUIRE_FALSE(svg.SetAttribute(u8"viewBox", value).HasException());
        CHECK_FALSE(svg.HasValidViewBox());
        CHECK(svg.ViewBox() == FloatRect());
      }

      REQUIRE_FALSE(svg.SetAttribute(u8"viewBox", u8"0 0 0 0").HasException());
      CHECK(svg.HasValidViewBox());
      CHECK(svg.HasEmptyViewBox());

      svg.RemoveAttribute(u8"viewBox");
      CHECK_FALSE(svg.HasValidViewBox());

      // A change to the property is written to the attribute when it's next read.
      REQUIRE_FALSE(svg.SetAttribute(u8"viewBox", u8"0 0 100 50").HasException());
      REQUIRE_FALSE(svg.ViewBoxAnimated().BaseVal()->SetX(5).HasException());
      CHECK(svg.GetAttribute(u8"viewBox") == u8"5 0 100 50");
    }

    SECTION("preserveAspectRatio")
    {
      CHECK(svg.PreserveAspectRatioString() == u8"xMidYMid meet");
      REQUIRE_FALSE(svg.SetAttribute(u8"preserveAspectRatio", u8"xMinYMin slice").HasException());
      CHECK(svg.PreserveAspectRatioString() == u8"xMinYMin slice");
      svg.RemoveAttribute(u8"preserveAspectRatio");
      CHECK(svg.PreserveAspectRatioString() == u8"xMidYMid meet");
    }

    SECTION("zoomAndPan")
    {
      CHECK(svg.ZoomAndPan() == SVGZoomAndPanMagnify);
      REQUIRE_FALSE(svg.SetAttribute(u8"zoomAndPan", u8"disable").HasException());
      CHECK(svg.ZoomAndPan() == SVGZoomAndPanDisable);
      REQUIRE_FALSE(svg.SetAttribute(u8"zoomAndPan", u8"other").HasException());
      CHECK(svg.ZoomAndPan() == SVGZoomAndPanUnknown);

      // An absent attribute is magnify.
      svg.RemoveAttribute(u8"zoomAndPan");
      CHECK(svg.ZoomAndPan() == SVGZoomAndPanMagnify);

      // It can't be set through the DOM.
      auto result = svg.SetZoomAndPan(uint16 {SVGZoomAndPan::SVG_ZOOMANDPAN_DISABLE});
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == dom::ExceptionCode::NoModificationAllowedError);
      CHECK(svg.ZoomAndPan() == SVGZoomAndPanMagnify);
    }

    SECTION("transform")
    {
      CHECK(svg.Transform().IsEmpty());
      CHECK_FALSE(svg.HasTransformRelatedAttributes());

      REQUIRE_FALSE(svg.SetAttribute(u8"transform", u8"translate(5 6) scale(2)").HasException());
      CHECK(svg.Transform().NumberOfItems() == 2);
      CHECK(svg.Transform().Concatenate() == AffineTransform(2, 0, 0, 2, 5, 6));
      CHECK(svg.HasTransformRelatedAttributes());

      svg.RemoveAttribute(u8"transform");
      CHECK(svg.Transform().IsEmpty());
      CHECK_FALSE(svg.HasTransformRelatedAttributes());

      svg.EnsureSupplementalTransform()->Translate(1, 0);
      CHECK(svg.HasTransformRelatedAttributes());
    }

    SECTION("Conditional processing")
    {
      CHECK(static_cast<SVGElement &>(svg).IsValid());
      REQUIRE_FALSE(svg.SetAttribute(u8"requiredExtensions", u8"http://example.com/other").HasException());
      CHECK_FALSE(static_cast<SVGElement &>(svg).IsValid());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGSVGElement: viewports", "[SVG][SVGSVGElement]")
  {
    auto document = ResolverFixture::Parse(
      u8"<svg id=a width=200 height=100 viewBox='0 0 400 200'>"
      u8"<svg id=b x=10 y=20 width=50 height=50 viewBox='0 0 100 100'><svg id=c></svg></svg>"
      u8"<svg id=d width=20 height=10></svg></svg>");
    auto &a = SvgById(*document, u8"a");
    auto &b = SvgById(*document, u8"b");
    auto &c = SvgById(*document, u8"c");
    auto &d = SvgById(*document, u8"d");

    CHECK(a.IsOutermostSVGSVGElement());
    CHECK(a.IsReplaced());
    CHECK_FALSE(b.IsReplaced());

    // Without renderers, the viewport is the intrinsic size.
    CHECK(a.HasIntrinsicWidth());
    CHECK(a.IntrinsicWidth() == 200);
    CHECK(a.IntrinsicHeight() == 100);
    CHECK(a.CurrentViewportSizeExcludingZoom() == FloatSize(200, 100));
    CHECK_FALSE(c.HasIntrinsicWidth());
    CHECK(c.IntrinsicWidth() == 0);
    CHECK(c.CurrentViewportSizeExcludingZoom() == FloatSize());

    CHECK(a.CurrentViewBoxRect() == FloatRect(0, 0, 400, 200));
    CHECK(d.CurrentViewBoxRect() == FloatRect());
    CHECK(a.ViewBoxToViewTransform(200, 100) == AffineTransform(0.5, 0, 0, 0.5, 0, 0));
    CHECK(d.ViewBoxToViewTransform(200, 100).IsIdentity());

    // Viewport elements, including <foreignObject>s.
    CHECK(c.NearestViewportElement() == &b);
    CHECK(c.FarthestViewportElement() == &a);
    CHECK(a.NearestViewportElement() == nullptr);
    CHECK(a.FarthestViewportElement() == nullptr);

    // A view box makes lengths relative, as do the default width and height.
    CHECK(b.HasRelativeLengths());
    CHECK_FALSE(d.HasRelativeLengths());
    CHECK(c.HasRelativeLengths());

    // x and y are transform-related, as is a view box that isn't empty.
    CHECK(b.HasTransformRelatedAttributes());
    CHECK_FALSE(d.HasTransformRelatedAttributes());

    // The CTM of an inner <svg> goes up to its nearest viewport: b's place and view box, then a's view box.
    CHECK(b.GetCTM(SVGLocatable::DisallowStyleUpdate) == AffineTransform(0.25, 0, 0, 0.25, 5, 10));
    CHECK(b.GetCTMForBindings()->Value() == AffineTransform(0.25, 0, 0, 0.25, 5, 10));
    CHECK(b.GetScreenCTM(SVGLocatable::DisallowStyleUpdate) == AffineTransform(0.25, 0, 0, 0.25, 5, 10));
    CHECK(a.GetCTM(SVGLocatable::DisallowStyleUpdate) == AffineTransform(0.5, 0, 0, 0.5, 0, 0));
    // c has no place or view box of its own, so its CTM is b's local transform.
    CHECK(c.GetCTM(SVGLocatable::DisallowStyleUpdate) == AffineTransform(0.5, 0, 0, 0.5, 10, 20));

    // Percentages resolve against the nearest <svg>'s view box, or else its viewport.
    auto inB = CreateTestSVGElement(*document);
    auto inD = CreateTestSVGElement(*document);
    REQUIRE_FALSE(b.AppendChild(*inB).HasException());
    REQUIRE_FALSE(d.AppendChild(*inD).HasException());
    REQUIRE_FALSE(inB->SetAttribute(u8"x", u8"50%").HasException());
    REQUIRE_FALSE(inD->SetAttribute(u8"x", u8"50%").HasException());
    CHECK(inB->X->CurrentValue().Value(SVGLengthContext(inB.get())) == 50);
    CHECK(inD->X->CurrentValue().Value(SVGLengthContext(inD.get())) == 10);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGSVGElement: transforms with a renderer",
                   "[SVG][SVGSVGElement]")
  {
    ResolverFixture fixture {
      u8"<!DOCTYPE html><body><svg id=s transform='translate(5 6)'></svg>"
      u8"<svg id=t transform='translate(5 6)' style='transform: scale(2)'></svg></body>"};
    (void)fixture.Document->UpdateLayout();
    auto &s = SvgById(*fixture.Document, u8"s");
    auto &t = SvgById(*fixture.Document, u8"t");
    REQUIRE(s.Renderer() != nullptr);
    REQUIRE(t.Renderer() != nullptr);

    // The attribute's transform, about the transform origin.
    CHECK(s.AnimatedLocalTransform() == AffineTransform(1, 0, 0, 1, 5, 6));
    // The transform property wins over the attribute, about the transform origin: the centre of the
    // 300x150 box (the user agent sheet's 0 0 is only for an <svg> inside SVG).
    CHECK(t.AnimatedLocalTransform() == AffineTransform(2, 0, 0, 2, -150, -75));

    // The supplemental transform comes first.
    s.EnsureSupplementalTransform()->Scale(3);
    CHECK(s.AnimatedLocalTransform() == AffineTransform(3, 0, 0, 3, 15, 18));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGSVGElement: DOM", "[SVG][SVGSVGElement]")
  {
    auto document = ResolverFixture::Parse(u8"<p id=out></p><svg id=s><svg id=in></svg></svg>");
    auto &svg = SvgById(*document, u8"s");

    CHECK(SVGSvgElement::CreateSVGNumber()->ValueForBindings() == 0);
    CHECK(SVGSvgElement::CreateSVGLength()->ValueAsString() == u8"0");
    CHECK(SVGSvgElement::CreateSVGAngle()->ValueAsString() == u8"0");
    CHECK(SVGSvgElement::CreateSVGPoint()->Value() == FloatPoint());
    CHECK(SVGSvgElement::CreateSVGMatrix()->Value().IsIdentity());
    CHECK(SVGSvgElement::CreateSVGRect()->Value() == FloatRect());
    CHECK(SVGSvgElement::CreateSVGTransform()->Value().Type() == SVGTransformValue::SVG_TRANSFORM_MATRIX);
    auto fromMatrix = SVGSvgElement::CreateSVGTransformFromMatrix({.A = 2, .F = 3});
    CHECK(fromMatrix->Value().Matrix().Value() == AffineTransform(2, 0, 0, 1, 0, 3));

    // Only the <svg>'s subtree is searched.
    CHECK(svg.GetElementById(u8"in") == document->GetElementById(u8"in"));
    CHECK(svg.GetElementById(u8"out") == nullptr);
    CHECK(svg.GetElementById(u8"") == nullptr);

    svg.SetCurrentTranslate({1, 2});
    CHECK(svg.CurrentTranslateValue() == FloatPoint(1, 2));
    CHECK(svg.CurrentTranslate().X() == 1);

    // Only the document element is scaled.
    CHECK(svg.CurrentScale() == 1);
    svg.SetCurrentScale(2);
    CHECK(svg.CurrentScale() == 1);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGSVGElement: the current scale of the document element",
                   "[SVG][SVGSVGElement]")
  {
    ResolverFixture fixture {u8"<svg id=s></svg>"};
    auto svg = ShareRef(SvgById(*fixture.Document, u8"s"));
    auto documentElement = fixture.Document->DocumentElement();
    REQUIRE_FALSE(fixture.Document->RemoveChild(*documentElement).HasException());
    REQUIRE_FALSE(fixture.Document->AppendChild(*svg).HasException());
    REQUIRE(fixture.Document->DocumentElement() == svg.get());

    CHECK(svg->CurrentScale() == 1);
    svg->SetCurrentScale(2);
    CHECK(fixture.Frame->PageZoomFactor() == 2);
    CHECK(svg->CurrentScale() == 2);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGLengthContext: font-relative lengths exclude the zoom",
                   "[SVG][SVGLengthContext]")
  {
    // An SVG element's own font size is never zoomed, but one inherited from HTML is.
    ResolverFixture fixture {u8"<!DOCTYPE html><body><div style='zoom: 2; font-size: 10px'><svg id=s>"
                             u8"<svg id=in></svg></svg></div></body>"};
    (void)fixture.Document->UpdateLayout();
    auto &inner = SvgById(*fixture.Document, u8"in");
    REQUIRE(SvgById(*fixture.Document, u8"s").Renderer() != nullptr);

    SVGLengthContext context(&inner);
    REQUIRE(inner.Renderer()->Style().ComputedFontSize() == 20);
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"2em").Value(context) == 20);
    CHECK(SVGLengthValue(SVGLengthMode::Other, u8"1rem").Value(context) == 16);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "SVGSVGElement: renderer queries", "[SVG][SVGSVGElement]")
  {
    ResolverFixture fixture {
      u8R"html(<!DOCTYPE html><body style="margin: 0"><div style="height: 20px"></div>)html"
      u8R"html(<svg id=s width=200 height=100 viewBox="0 0 100 50" style="padding: 5px; display: block">)html"
      u8R"html(<rect id=a x=10 y=10 width=20 height=20 /><rect id=b x=50 y=10 width=20 height=20 />)html"
      u8R"html(<svg id=inner x=60 y=0 width=40 height=30><g id=g><rect id=c width=5 height=5 /></g></svg>)html"
      u8R"html(</svg></body>)html"};
    (void)fixture.Document->UpdateLayout();
    auto &s = SvgById(*fixture.Document, u8"s");
    auto &inner = SvgById(*fixture.Document, u8"inner");

    // The viewport is the root's content box, or an inner <svg>'s viewport.
    CHECK(s.CurrentViewportSizeExcludingZoom() == FloatSize(200, 100));
    CHECK(inner.CurrentViewportSizeExcludingZoom() == FloatSize(40, 30));

    // The screen CTM of the outermost <svg> maps through its place on the page and its view box.
    CHECK(s.GetScreenCTM() == AffineTransform(2, 0, 0, 2, 5, 25));
    auto &rect = Downcast<SVGGraphicsElement>(Downcast<SVGElement>(*fixture.Document->GetElementById(u8"c")));
    CHECK(rect.GetScreenCTM() == AffineTransform(2, 0, 0, 2, 125, 25));

    // The graphics elements whose repaint rects intersect or are enclosed by a rect. Each rect is mapped by
    // the transforms up to and including its nearest viewport element's: for the outermost <svg>, through its
    // place on the page (a at (25,45) size 40x40, b at (105,45)), and for the inner one, only by its x and y
    // (c at (60,0) size 5x5).
    auto query = SVGRect::Create(FloatRect(0, 40, 70, 60));
    auto intersecting = s.GetIntersectionList(*query, nullptr);
    REQUIRE(intersecting->Length() == 1);
    CHECK(intersecting->Item(0).get() == fixture.Document->GetElementById(u8"a").get());
    auto wide = SVGRect::Create(FloatRect(20, 0, 140, 90));
    CHECK(s.GetIntersectionList(*wide, nullptr)->Length() == 3);
    CHECK(s.GetEnclosureList(*wide, nullptr)->Length() == 3);
    auto narrow = SVGRect::Create(FloatRect(30, 0, 140, 90));
    CHECK(s.GetEnclosureList(*narrow, nullptr)->Length() == 2);
    auto &g = Downcast<SVGElement>(*fixture.Document->GetElementById(u8"g"));
    CHECK(s.GetIntersectionList(*wide, &g)->Length() == 1);
    CHECK(SVGSvgElement::CheckEnclosure(
      ShareRef(Downcast<SVGElement>(*fixture.Document->GetElementById(u8"b"))), *wide));
    auto far = SVGRect::Create(FloatRect(300, 300, 5, 5));
    CHECK_FALSE(SVGSvgElement::CheckIntersection(
      ShareRef(Downcast<SVGElement>(*fixture.Document->GetElementById(u8"a"))), *far));

    // Elements' boxes are their object bounding boxes, mapped to the page.
    auto clientRect = fixture.Document->GetElementById(u8"b")->GetBoundingClientRect();
    CHECK(clientRect->X() == 105);
    CHECK(clientRect->Y() == 45);
    CHECK(clientRect->Width() == 40);
    CHECK(clientRect->Height() == 40);
    // The outermost <svg>'s is its CSS box.
    auto svgRect = s.GetBoundingClientRect();
    CHECK(svgRect->Width() == 210);
    CHECK(svgRect->Height() == 110);
  }
}
