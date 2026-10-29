#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImageSet.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;
  using style::tests::WithView;

  namespace
  {
    const boo::Color Blue {boo::Color::blue};
    const boo::Color Black {boo::Color::black};

    /// @brief A parsed document shown in a frame with an 800x600 view.
    struct FrameFixture : ResolverFixture
    {
      Ref<LocalFrame> Frame {LocalFrame::Create()};

      explicit FrameFixture(utf8_string &&markup) noexcept : ResolverFixture(krys::move(markup), WithView::No)
      {
        Frame->CreateView({800, 600});
        Frame->SetDocument(ShareRefPtr<dom::Document>(Document.get()));
      }

      KRYS_NODISCARD LocalFrameView &View() noexcept
      {
        return *Frame->View();
      }

      KRYS_NODISCARD const RenderStyle &Rendered(dom::DOMStringView id)
      {
        auto *style = ById(id).GetRenderStyle();
        REQUIRE(style);
        return *style;
      }

      KRYS_NODISCARD std::string Width(dom::DOMStringView id)
      {
        auto &style = Rendered(id);
        return Serialised(style, style.Width());
      }

      KRYS_NODISCARD std::string Height(dom::DOMStringView id)
      {
        auto &style = Rendered(id);
        return Serialised(style, style.Height());
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrame: documents and views", "[Page][LocalFrame]")
  {
    auto document = ResolverFixture::Parse(u8"<!DOCTYPE html><p id=p>x</p>");
    CHECK_FALSE(document->Frame());
    CHECK_FALSE(document->View());
    CHECK_FALSE(document->GetRenderView());

    auto frame = LocalFrame::Create();
    frame->CreateView({800, 600});
    auto &view = *frame->View();
    CHECK(&view.Frame() == frame.get());
    CHECK(view.Size() == IntSize {800, 600});
    CHECK(view.MediaType() == css::CSSOMStringAtom {u8"screen"});

    frame->SetDocument(ShareRefPtr<dom::Document>(document.get()));
    CHECK(frame->Document() == document.get());
    CHECK(document->Frame() == frame.get());
    CHECK(document->View() == &view);
    REQUIRE(document->HasLivingRenderTree());
    CHECK(view.GetRenderView() == document->GetRenderView());
    CHECK(RenderViewOf(*document) == document->GetRenderView());
    CHECK(&document->GetRenderView()->GetDocument() == document.get());
    // Creating the render tree resolves the document's style.
    CHECK_FALSE(document->NeedsStyleRecalc());

    SECTION("The render tree belongs to the view")
    {
      frame->SetView(nullptr);
      CHECK_FALSE(document->View());
      CHECK_FALSE(document->HasLivingRenderTree());
      CHECK(document->Frame() == frame.get());

      frame->CreateView({400, 300});
      CHECK(document->HasLivingRenderTree());
      CHECK(document->View()->Size() == IntSize {400, 300});
    }

    SECTION("Replacing the document")
    {
      auto other = ResolverFixture::Parse(u8"<!DOCTYPE html><p>y</p>");
      frame->SetDocument(ShareRefPtr<dom::Document>(other.get()));
      CHECK_FALSE(document->Frame());
      CHECK_FALSE(document->HasLivingRenderTree());
      CHECK(other->Frame() == frame.get());
      CHECK(other->HasLivingRenderTree());

      frame->SetDocument(nullptr);
      CHECK_FALSE(other->Frame());
      CHECK_FALSE(frame->Document());
    }

    SECTION("A document outlives its frame")
    {
      frame = LocalFrame::Create();
      CHECK_FALSE(document->Frame());
      CHECK_FALSE(document->HasLivingRenderTree());
      // Its style is still resolved, without a view, but it has no renderers.
      document->UpdateStyleIfNeeded();
      CHECK_FALSE(document->GetElementById(u8"p")->GetRenderStyle());
      CHECK(document->GetElementById(u8"p")->ComputedStyle());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: sizes", "[Page][LocalFrameView]")
  {
    FrameFixture fixture {u8"<!DOCTYPE html>"};
    auto &view = fixture.View();

    CHECK(view.FrameRect() == IntRect {0, 0, 800, 600});
    CHECK(view.GetLayoutSize() == IntSize {800, 600});
    CHECK(view.VisibleContentRect() == IntRect {0, 0, 800, 600});
    CHECK(view.SizeForCSSDefaultViewportUnits() == FloatSize {800, 600});
    CHECK(view.SizeForCSSSmallViewportUnits() == FloatSize {800, 600});
    CHECK(view.SizeForCSSLargeViewportUnits() == FloatSize {800, 600});
    CHECK(view.SizeForCSSDynamicViewportUnits() == FloatSize {800, 600});

    SECTION("Fixed layout")
    {
      view.SetFixedLayoutSize({1'000, 2'000});
      CHECK(view.GetLayoutSize() == IntSize {800, 600});
      view.SetUseFixedLayout(true);
      CHECK(view.GetLayoutSize() == IntSize {1'000, 2'000});
      CHECK(view.SizeForCSSDefaultViewportUnits() == FloatSize {1'000, 2'000});
      CHECK(view.SizeForCSSDynamicViewportUnits() == FloatSize {1'000, 2'000});

      // A fixed layout view is laid out at its fixed size whatever its frame.
      view.Resize({100, 100});
      CHECK(view.GetLayoutSize() == IntSize {1'000, 2'000});

      view.SetFixedLayoutSize({});
      CHECK(view.GetLayoutSize() == IntSize {100, 100});
    }

    SECTION("Viewport unit overrides")
    {
      view.SetSizeForCSSSmallViewportUnits({300, 200});
      view.SetSizeForCSSLargeViewportUnits({900, 700});
      CHECK(view.SizeForCSSSmallViewportUnits() == FloatSize {300, 200});
      CHECK(view.SizeForCSSLargeViewportUnits() == FloatSize {900, 700});
      CHECK(view.SizeForCSSDefaultViewportUnits() == FloatSize {800, 600});

      // A width-only override keeps the view's height.
      view.SetOverrideSizeForCSSDefaultViewportUnits({.Width = 500.f});
      CHECK(view.SizeForCSSDefaultViewportUnits() == FloatSize {500, 600});

      view.ClearSizeOverrideForCSSDefaultViewportUnits();
      view.ClearSizeOverrideForCSSSmallViewportUnits();
      view.ClearSizeOverrideForCSSLargeViewportUnits();
      CHECK_FALSE(view.OverrideSizeForCSSDefaultViewportUnits());
      CHECK(view.SizeForCSSSmallViewportUnits() == FloatSize {800, 600});
      CHECK(view.SizeForCSSLargeViewportUnits() == FloatSize {800, 600});
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: viewport units", "[Page][LocalFrameView]")
  {
    SECTION("Viewport units are zero without a view")
    {
      ResolverFixture fixture {u8"<!DOCTYPE html><div id=d style='width: 50vw'></div>", WithView::No};
      // Without a view there are no renderers: the style is resolved on demand.
      auto *style = fixture.ById(u8"d").ComputedStyle();
      REQUIRE(style);
      CHECK(ResolverFixture::Serialised(*style, style->Width()) == "0px");
    }

    FrameFixture fixture {u8R"(<!DOCTYPE html><style>
      #d { width: 50vw; height: 10vh }
      #s { width: 10svw; height: 10lvh }
      #m { width: 10vmin; height: 10dvh }
      #f { width: 10px }
      html { overflow: hidden }
    </style><div id=d></div><div id=s></div><div id=m></div><div id=f></div>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();

    CHECK(fixture.Width(u8"d") == "400px");
    CHECK(fixture.Height(u8"d") == "60px");
    CHECK(fixture.Width(u8"s") == "80px");
    CHECK(fixture.Height(u8"s") == "60px");
    CHECK(fixture.Width(u8"m") == "60px");
    CHECK(fixture.Height(u8"m") == "60px");
    CHECK(document.HasStyleWithViewportUnits());

    SECTION("Resizing the view invalidates the styles that use viewport units")
    {
      // The root hides the view's scrollbars (as of a layout), so a resize doesn't lay the document out at
      // once to fit them.
      document.UpdateLayout();
      fixture.View().Resize({1'000, 500});
      CHECK(fixture.ById(u8"d").NeedsStyleRecalc());
      CHECK_FALSE(fixture.ById(u8"f").NeedsStyleRecalc());

      document.UpdateStyleIfNeeded();
      CHECK(fixture.Width(u8"d") == "500px");
      CHECK(fixture.Height(u8"d") == "50px");
      CHECK(fixture.Width(u8"m") == "50px");
    }

    SECTION("Overriding the sizes")
    {
      fixture.View().SetSizeForCSSDefaultViewportUnits({200, 100});
      fixture.View().SetSizeForCSSSmallViewportUnits({100, 100});
      document.UpdateStyleIfNeeded();
      CHECK(fixture.Width(u8"d") == "100px");
      CHECK(fixture.Height(u8"d") == "10px");
      CHECK(fixture.Width(u8"s") == "10px");
    }

    SECTION("Page zoom")
    {
      // Media query lengths are unzoomed, so the 800px wide view is 400px wide at a zoom of 2.
      auto wide = document.Window()->MatchMedia(u8"(width: 400px)");
      CHECK_FALSE(wide->Matches());
      fixture.Frame->SetPageZoomFactor(2);
      CHECK(document.RenderViewStyle().UsedZoom() == 2);
      CHECK(fixture.Rendered(u8"f").UsedZoom() == 2);
      CHECK(fixture.Rendered(u8"f").ComputedFontSize() == 32);
      document.EvaluateMediaQueriesAndReportChanges();
      CHECK(wide->Matches());
    }

    SECTION("Text zoom")
    {
      fixture.Frame->SetTextZoomFactor(1.5f);
      CHECK(fixture.Rendered(u8"f").UsedZoom() == 1);
      CHECK(fixture.Rendered(u8"f").ComputedFontSize() == 24);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: registered properties with viewport units",
                   "[Page][LocalFrameView]")
  {
    FrameFixture fixture {u8R"(<!DOCTYPE html><style>
      @property --size { syntax: '<length>'; inherits: false; initial-value: 10vw }
      #d { width: var(--size) }
    </style><div id=d></div>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    CHECK(document.HasStyleWithViewportUnits());
    CHECK(fixture.Width(u8"d") == "80px");

    fixture.View().Resize({1'000, 600});
    document.UpdateStyleIfNeeded();
    CHECK(fixture.Width(u8"d") == "100px");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: layout re-evaluates media queries",
                   "[Page][LocalFrameView]")
  {
    // The root hides the view's scrollbars, so a resize doesn't lay the document out at once to fit them.
    FrameFixture fixture {u8R"(<!DOCTYPE html><style>
      @media (min-width: 1000px) { p { color: rgb(0, 0, 255) } }
      html { overflow: hidden }
    </style><p id=p>x</p>)"};
    auto &document = *fixture.Document;
    auto &layoutContext = fixture.View().LayoutContext();
    document.UpdateLayout();
    CHECK(layoutContext.DidFirstLayout());
    CHECK_FALSE(layoutContext.NeedsLayout());
    CHECK(EqualIgnoringSemanticColor(fixture.Rendered(u8"p").GetColor(), Black));

    fixture.View().Resize({1'200, 600});
    CHECK(layoutContext.IsLayoutPending());
    CHECK(document.IsLayoutPending());

    // Style updates alone don't re-evaluate media queries for viewport changes; layout does.
    document.UpdateStyleIfNeeded();
    CHECK(EqualIgnoringSemanticColor(fixture.Rendered(u8"p").GetColor(), Black));

    CHECK(document.UpdateLayout() == dom::Document::UpdateLayoutResult::ChangesDone);
    CHECK_FALSE(layoutContext.IsLayoutPending());
    CHECK(fixture.Rendered(u8"p").GetColor() == Blue);

    CHECK(document.UpdateLayout() == dom::Document::UpdateLayoutResult::NoChange);

    SECTION("A view that can show scrollbars lays out at once on a resize, to fit them")
    {
      FrameFixture scrolling {u8R"(<!DOCTYPE html><style>
        @media (min-width: 1000px) { p { color: rgb(0, 0, 255) } }
      </style><p id=p>x</p>)"};
      scrolling.Document->UpdateLayout();
      scrolling.View().Resize({1'200, 600});
      CHECK_FALSE(scrolling.View().LayoutContext().IsLayoutPending());
      CHECK(scrolling.Rendered(u8"p").GetColor() == Blue);
    }

    SECTION("Layout isn't scheduled before there's a body")
    {
      auto empty = ResolverFixture::Parse(u8"");
      REQUIRE(empty->DocumentElement());
      REQUIRE(empty->Body());
      CHECK(empty->ShouldScheduleLayout());
      auto frame = LocalFrame::Create();
      frame->CreateView({800, 600});
      REQUIRE_FALSE(empty->Body()->Remove().HasException());
      frame->SetDocument(ShareRefPtr<dom::Document>(empty.get()));
      CHECK_FALSE(empty->ShouldScheduleLayout());
      frame->View()->Resize({1'200, 600});
      CHECK_FALSE(frame->View()->LayoutContext().IsLayoutPending());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: media type and pagination",
                   "[Page][LocalFrameView]")
  {
    FrameFixture fixture {u8R"(<!DOCTYPE html><style>
      @media print { p { color: rgb(0, 0, 255) } }
    </style><p id=p>x</p>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    CHECK(EqualIgnoringSemanticColor(fixture.Rendered(u8"p").GetColor(), Black));

    auto paged = document.Window()->MatchMedia(u8"(overflow-block: paged)");
    CHECK_FALSE(paged->Matches());

    fixture.View().SetMediaType(css::CSSOMStringAtom {u8"print"});
    document.UpdateStyleIfNeeded();
    CHECK(fixture.Rendered(u8"p").GetColor() == Blue);
    CHECK(document.GetMediaQueryMatcher().MediaType() == css::CSSOMStringAtom {u8"print"});

    fixture.View().SetMediaType(css::CSSOMStringAtom {u8"screen"});
    fixture.View().SetPagination({.Mode = PaginationMode::TopToBottomPaginated});
    document.UpdateStyleIfNeeded();
    CHECK(EqualIgnoringSemanticColor(fixture.Rendered(u8"p").GetColor(), Black));
    document.EvaluateMediaQueriesAndReportChanges();
    CHECK(paged->Matches());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrame: media environment", "[Page][LocalFrame]")
  {
    FrameFixture fixture {u8R"(<!DOCTYPE html><style>
      @media (prefers-color-scheme: dark) { p { color: rgb(0, 0, 255) } }
      #set { list-style-image: image-set(url(https://example.com/a.png) 1x, url(https://example.com/b.png) 2x) }
    </style><p id=p>x</p><div id=set></div>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    CHECK(EqualIgnoringSemanticColor(fixture.Rendered(u8"p").GetColor(), Black));
    CHECK(document.RenderViewStyle().DeviceScaleFactor() == 1);
    auto imageSet = [&]() -> StyleImageSet &
    {
      return Downcast<StyleImageSet>(*fixture.Rendered(u8"set").ListStyleImage().TryStyleImage());
    };
    CHECK(imageSet().SelectBestFitImage(document).ScaleFactor == 1);

    auto hiDPI = document.Window()->MatchMedia(u8"(resolution: 2x)");
    CHECK_FALSE(hiDPI->Matches());

    css::mq::MediaEnvironment environment;
    environment.UseDarkAppearance = true;
    environment.DeviceScaleFactor = 2;
    fixture.Frame->SetMediaEnvironment(environment);
    CHECK(fixture.Frame->GetMediaEnvironment() == environment);

    document.UpdateStyleIfNeeded();
    CHECK(fixture.Rendered(u8"p").GetColor() == Blue);
    CHECK(document.DeviceScaleFactor() == 2);
    CHECK(document.RenderViewStyle().DeviceScaleFactor() == 2);
    // An image set picks the image for the device scale factor.
    CHECK(imageSet().SelectBestFitImage(document).ScaleFactor == 2);

    document.EvaluateMediaQueriesAndReportChanges();
    CHECK(hiDPI->Matches());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "LocalFrameView: the style recalc and layout timers",
                   "[Page][LocalFrameView][Timer]")
  {
    FrameFixture fixture {u8"<!DOCTYPE html><div id=box style=width:10px;height:10px></div>"};
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    REQUIRE_FALSE(fixture.Document->HasPendingStyleRecalc());
    REQUIRE_FALSE(fixture.View().LayoutContext().NeedsLayout());

    // A change schedules a style recalc, which schedules a layout; both run when the loop does.
    auto &box = fixture.ById(u8"box");
    REQUIRE_FALSE(box.SetAttribute(u8"style", u8"width:20px;height:10px").HasException());
    CHECK(fixture.Document->HasPendingStyleRecalc());
    loop.RunUntilIdle();
    CHECK_FALSE(fixture.Document->HasPendingStyleRecalc());
    CHECK_FALSE(fixture.View().LayoutContext().NeedsLayout());
    auto *renderer = DynamicDowncast<RenderBox>(box.Renderer());
    REQUIRE(renderer);
    CHECK(renderer->Width() == 20_lu);

    SECTION("a document taken out of its frame stops its style recalc timer")
    {
      REQUIRE_FALSE(box.SetAttribute(u8"style", u8"width:30px").HasException());
      fixture.Frame->SetDocument(nullptr);
      loop.RunUntilIdle();
      CHECK(fixture.Document->HasPendingStyleRecalc());
    }
  }
}
