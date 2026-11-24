#include "Krystal.Booey/Page/FrameView.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/MediaQueryList.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2D.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLCanvasElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/Window.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImageSet.hpp"

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;
  using style::tests::WithView;

  namespace
  {
    const boo::Color Blue {boo::Color::blue};
    const boo::Color Black {boo::Color::black};

    /// @brief A parsed document shown in a page's main frame, with an 800x600 view.
    struct FrameViewFixture : ResolverFixture
    {
      Ref<boo::Frame> Frame {boo::Frame::Create()};

      explicit FrameViewFixture(utf8_string &&markup) noexcept
          : ResolverFixture(krys::move(markup), WithView::No)
      {
        Frame->CreateView({800, 600});
        Frame->SetDocument(ShareRefPtr<boo::Document>(Document.get()));
      }

      KRYS_NODISCARD FrameView &View() const noexcept
      {
        return *Frame->View();
      }

      KRYS_NODISCARD const RenderStyle &Rendered(DOMStringView id)
      {
        auto *style = ById(id).GetRenderStyle();
        REQUIRE(style);
        return *style;
      }

      KRYS_NODISCARD std::string Width(DOMStringView id)
      {
        auto &style = Rendered(id);
        return Serialised(style, style.Width());
      }

      KRYS_NODISCARD std::string Height(DOMStringView id)
      {
        auto &style = Rendered(id);
        return Serialised(style, style.Height());
      }
    };

    /// @brief Records the paint, calling back when an image buffer is drawn (as painting a canvas does).
    class ImageBufferCallbackRecorder final : public display_list::RecorderImpl
    {
    private:
      Func<void()> _onDrawImageBuffer;

    public:
      explicit ImageBufferCallbackRecorder(Func<void()> onDrawImageBuffer) noexcept
          : RecorderImpl(SizeF(800, 600)), _onDrawImageBuffer(krys::move(onDrawImageBuffer))
      {
      }

      void DrawImageBuffer(ImageBuffer &buffer, const RectF &destRect, const RectF &srcRect,
                           ImagePaintingOptions options) noexcept override
      {
        _onDrawImageBuffer();
        RecorderImpl::DrawImageBuffer(buffer, destRect, srcRect, options);
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: documents and views", "[Page][Frame]")
  {
    auto document = ResolverFixture::Parse(u8"<!DOCTYPE html><p id=p>x</p>");
    CHECK_FALSE(document->Frame());
    CHECK_FALSE(document->View());
    CHECK_FALSE(document->GetRenderView());

    auto frame = boo::Frame::Create();
    frame->CreateView({800, 600});
    auto &view = *frame->View();
    CHECK(&view.Frame() == frame.get());
    CHECK(view.Size() == SizeI {800, 600});
    CHECK(view.MediaType() == u8"screen");

    frame->SetDocument(ShareRefPtr<Document>(document.get()));
    CHECK(frame->Document() == document.get());
    CHECK(document->Frame() == frame.get());
    CHECK(document->View() == &view);
    REQUIRE(document->HasLivingRenderTree());
    CHECK(view.GetRenderView() == document->GetRenderView());
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
      CHECK(document->View()->Size() == SizeI {400, 300});
    }

    SECTION("Replacing the document")
    {
      auto other = ResolverFixture::Parse(u8"<!DOCTYPE html><p>y</p>");
      frame->SetDocument(ShareRefPtr<Document>(other.get()));
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
      frame = boo::Frame::Create();
      CHECK_FALSE(document->Frame());
      CHECK_FALSE(document->HasLivingRenderTree());
      // Its style is still resolved, without a view, but it has no renderers.
      document->UpdateStyleIfNeeded();
      CHECK_FALSE(document->GetElementById(u8"p")->GetRenderStyle());
      CHECK(document->GetElementById(u8"p")->ComputedStyle());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FrameView: sizes", "[Page][FrameView]")
  {
    FrameViewFixture fixture {u8"<!DOCTYPE html>"};
    auto &view = fixture.View();

    CHECK(view.FrameRect() == RectI {0, 0, 800, 600});
    CHECK(view.GetLayoutSize() == SizeI {800, 600});
    CHECK(view.VisibleContentRect() == RectI {0, 0, 800, 600});
    CHECK(view.SizeForCSSDefaultViewportUnits() == SizeF {800, 600});
    CHECK(view.SizeForCSSSmallViewportUnits() == SizeF {800, 600});
    CHECK(view.SizeForCSSLargeViewportUnits() == SizeF {800, 600});
    CHECK(view.SizeForCSSDynamicViewportUnits() == SizeF {800, 600});

    SECTION("Fixed layout")
    {
      view.SetFixedLayoutSize({1'000, 2'000});
      CHECK(view.GetLayoutSize() == SizeI {800, 600});
      view.SetUseFixedLayout(true);
      CHECK(view.GetLayoutSize() == SizeI {1'000, 2'000});
      CHECK(view.SizeForCSSDefaultViewportUnits() == SizeF {1'000, 2'000});
      CHECK(view.SizeForCSSDynamicViewportUnits() == SizeF {1'000, 2'000});

      // A fixed layout view is laid out at its fixed size whatever its frame.
      view.Resize({100, 100});
      CHECK(view.GetLayoutSize() == SizeI {1'000, 2'000});

      view.SetFixedLayoutSize({});
      CHECK(view.GetLayoutSize() == SizeI {100, 100});
    }

    SECTION("Viewport unit overrides")
    {
      view.SetSizeForCSSSmallViewportUnits({300, 200});
      view.SetSizeForCSSLargeViewportUnits({900, 700});
      CHECK(view.SizeForCSSSmallViewportUnits() == SizeF {300, 200});
      CHECK(view.SizeForCSSLargeViewportUnits() == SizeF {900, 700});
      CHECK(view.SizeForCSSDefaultViewportUnits() == SizeF {800, 600});

      // A width-only override keeps the view's height.
      view.SetOverrideSizeForCSSDefaultViewportUnits({.Width = 500.f});
      CHECK(view.SizeForCSSDefaultViewportUnits() == SizeF {500, 600});

      view.ClearSizeOverrideForCSSDefaultViewportUnits();
      view.ClearSizeOverrideForCSSSmallViewportUnits();
      view.ClearSizeOverrideForCSSLargeViewportUnits();
      CHECK_FALSE(view.OverrideSizeForCSSDefaultViewportUnits());
      CHECK(view.SizeForCSSSmallViewportUnits() == SizeF {800, 600});
      CHECK(view.SizeForCSSLargeViewportUnits() == SizeF {800, 600});
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FrameView: viewport units and the root's scrollbars",
                   "[Page][FrameView][css-values-4:viewport-relative-lengths]")
  {
    // The units leave out the scrollbar the root's overflow always shows (scroll), but not one that comes and
    // goes with the content (auto), as in Chrome.
    auto widthWith = [](utf8_stringview overflow)
    {
      FrameViewFixture fixture {u8"<!DOCTYPE html><style>html { overflow: " + utf8_string(overflow)
                                + u8" } body { margin: 0; height: 2000px } #d { width: 100vw }</style>"
                                  u8"<div id=d></div>"};
      fixture.Document->UpdateLayout();
      fixture.Document->UpdateStyleIfNeeded();
      return Pair {fixture.Width(u8"d"), fixture.Document->DocumentElement()->ClientWidth()};
    };

    auto [scrollWidth, scrollClientWidth] = widthWith(u8"scroll");
    REQUIRE(scrollClientWidth < 800);
    CHECK(scrollWidth == std::to_string(scrollClientWidth) + "px");

    auto [autoWidth, autoClientWidth] = widthWith(u8"auto");
    REQUIRE(autoClientWidth < 800);
    CHECK(autoWidth == "800px");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FrameView: viewport units", "[Page][FrameView]")
  {
    SECTION("Viewport units are zero without a view")
    {
      ResolverFixture fixture {u8"<!DOCTYPE html><div id=d style='width: 50vw'></div>", WithView::No};
      // Without a view there are no renderers: the style is resolved on demand.
      auto *style = fixture.ById(u8"d").ComputedStyle();
      REQUIRE(style);
      CHECK(ResolverFixture::Serialised(*style, style->Width()) == "0px");
    }

    FrameViewFixture fixture {u8R"(<!DOCTYPE html><style>
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

  TEST_CASE_METHOD(css::tests::ValueFixture, "FrameView: registered properties with viewport units",
                   "[Page][FrameView]")
  {
    FrameViewFixture fixture {u8R"(<!DOCTYPE html><style>
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

  TEST_CASE_METHOD(css::tests::ValueFixture, "FrameView: layout re-evaluates media queries",
                   "[Page][FrameView]")
  {
    // The root hides the view's scrollbars, so a resize doesn't lay the document out at once to fit them.
    FrameViewFixture fixture {u8R"(<!DOCTYPE html><style>
      @media (min-width: 1000px) { p { color: rgb(0, 0, 255) } }
      html { overflow: hidden }
    </style><p id=p>x</p>)"};
    auto &document = *fixture.Document;
    auto &layoutContext = fixture.View().LayoutContext();
    document.UpdateLayout();
    CHECK(layoutContext.DidFirstLayout());
    CHECK_FALSE(layoutContext.NeedsLayout());
    CHECK(EqualIgnoringSemanticColor(fixture.Rendered(u8"p").Color(), Black));

    fixture.View().Resize({1'200, 600});
    CHECK(layoutContext.IsLayoutPending());
    CHECK(document.IsLayoutPending());

    // Style updates alone don't re-evaluate media queries for viewport changes; layout does.
    document.UpdateStyleIfNeeded();
    CHECK(EqualIgnoringSemanticColor(fixture.Rendered(u8"p").Color(), Black));

    CHECK(document.UpdateLayout() == Document::UpdateLayoutResult::ChangesDone);
    CHECK_FALSE(layoutContext.IsLayoutPending());
    CHECK(fixture.Rendered(u8"p").Color() == Blue);

    CHECK(document.UpdateLayout() == Document::UpdateLayoutResult::NoChange);

    SECTION("A view that can show scrollbars lays out at once on a resize, to fit them")
    {
      FrameViewFixture scrolling {u8R"(<!DOCTYPE html><style>
        @media (min-width: 1000px) { p { color: rgb(0, 0, 255) } }
      </style><p id=p>x</p>)"};
      scrolling.Document->UpdateLayout();
      scrolling.View().Resize({1'200, 600});
      CHECK_FALSE(scrolling.View().LayoutContext().IsLayoutPending());
      CHECK(scrolling.Rendered(u8"p").Color() == Blue);
    }

    SECTION("Layout isn't scheduled before there's a body")
    {
      auto empty = ResolverFixture::Parse(u8"");
      REQUIRE(empty->DocumentElement());
      REQUIRE(empty->Body());
      CHECK(empty->ShouldScheduleLayout());
      auto frame = boo::Frame::Create();
      frame->CreateView({800, 600});
      REQUIRE_FALSE(empty->Body()->Remove().HasException());
      frame->SetDocument(ShareRefPtr<Document>(empty.get()));
      CHECK_FALSE(empty->ShouldScheduleLayout());
      frame->View()->Resize({1'200, 600});
      CHECK_FALSE(frame->View()->LayoutContext().IsLayoutPending());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FrameView: media type", "[Page][FrameView]")
  {
    FrameViewFixture fixture {u8R"(<!DOCTYPE html><style>
      @media print { p { color: rgb(0, 0, 255) } }
    </style><p id=p>x</p>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    CHECK(EqualIgnoringSemanticColor(fixture.Rendered(u8"p").Color(), Black));

    auto paged = document.Window()->MatchMedia(u8"(overflow-block: paged)");
    CHECK_FALSE(paged->Matches());

    fixture.View().SetMediaType(u8"print");
    document.UpdateStyleIfNeeded();
    CHECK(fixture.Rendered(u8"p").Color() == Blue);
    CHECK(document.GetMediaQueryMatcher().MediaType() == u8"print");

    document.EvaluateMediaQueriesAndReportChanges();
    CHECK(paged->Matches());

    fixture.View().SetMediaType(u8"screen");
    document.UpdateStyleIfNeeded();
    CHECK(EqualIgnoringSemanticColor(fixture.Rendered(u8"p").Color(), Black));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Page: dark appearance and device scale factor", "[Page][Frame]")
  {
    FrameViewFixture fixture {u8R"(<!DOCTYPE html><style>
      @media (prefers-color-scheme: dark) { p { color: rgb(0, 0, 255) } }
      #set { list-style-image: image-set(url(https://example.com/a.png) 1x, url(https://example.com/b.png) 2x) }
    </style><p id=p>x</p><div id=set></div>)"};
    auto &document = *fixture.Document;
    document.UpdateStyleIfNeeded();
    CHECK(EqualIgnoringSemanticColor(fixture.Rendered(u8"p").Color(), Black));
    CHECK(document.RenderViewStyle().DeviceScaleFactor() == 1);
    auto imageSet = [&]() -> StyleImageSet &
    {
      return Downcast<StyleImageSet>(*fixture.Rendered(u8"set").ListStyleImage().TryStyleImage());
    };
    CHECK(imageSet().SelectBestFitImage(document).ScaleFactor == 1);

    auto hiDPI = document.Window()->MatchMedia(u8"(resolution: 2x)");
    CHECK_FALSE(hiDPI->Matches());

    fixture.Frame->SetUseDarkAppearance(true);
    fixture.Frame->SetDeviceScaleFactor(2);
    CHECK(fixture.Frame->UseDarkAppearance());
    CHECK(fixture.Frame->DeviceScaleFactor() == 2);

    document.UpdateStyleIfNeeded();
    CHECK(fixture.Rendered(u8"p").Color() == Blue);
    CHECK(document.DeviceScaleFactor() == 2);
    CHECK(document.RenderViewStyle().DeviceScaleFactor() == 2);
    // An image set picks the image for the device scale factor.
    CHECK(imageSet().SelectBestFitImage(document).ScaleFactor == 2);

    document.EvaluateMediaQueriesAndReportChanges();
    CHECK(hiDPI->Matches());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "FrameView: the style recalc and layout timers",
                   "[Page][FrameView][Timer]")
  {
    FrameViewFixture fixture {u8"<!DOCTYPE html><div id=box style=width:10px;height:10px></div>"};
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

  TEST_CASE_METHOD(css::tests::ValueFixture, "FrameView: no layout while painting", "[Page][FrameView]")
  {
    FrameViewFixture fixture {
      u8"<!DOCTYPE html><div id=box style='width:10px;height:10px'></div><canvas id=canvas></canvas>"};
    html::EventLoop::Current().RunUntilIdle();
    auto &layoutContext = fixture.View().LayoutContext();
    REQUIRE_FALSE(layoutContext.NeedsLayout());
    auto *renderer = fixture.ById(u8"box").Renderer();
    REQUIRE(renderer);
    auto canvasContext = Downcast<HTMLCanvasElement>(fixture.ById(u8"canvas")).GetContext2d();
    REQUIRE(canvasContext);
    canvasContext->FillRect(0, 0, 10, 10);

    // Something that asks for a layout while the view paints gets it after the paint.
    bool painted = false;
    bool neededLayoutInPaint = false;
    ImageBufferCallbackRecorder context {[&]
                                         {
                                           if (painted)
                                           {
                                             return;
                                           }
                                           painted = true;
                                           renderer->SetNeedsLayout();
                                           layoutContext.Layout();
                                           neededLayoutInPaint = layoutContext.NeedsLayout();
                                         }};
    fixture.View().Paint(context, {0, 0, 800, 600});
    REQUIRE(painted);
    CHECK(neededLayoutInPaint);
    CHECK(layoutContext.NeedsLayout());
    layoutContext.Layout();
    CHECK_FALSE(layoutContext.NeedsLayout());
  }
}
