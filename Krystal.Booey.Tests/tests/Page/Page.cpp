#include "Krystal.Booey/Page/Page.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/VisualViewport.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief A page with a 200x100 view, and @p markup parsed into its main frame's document.
    struct PageFixture
    {
      Ref<Page> TestPage {Page::Create()};
      RefPtr<dom::HTMLDocument> Document;

      explicit PageFixture(utf8_string &&markup) noexcept
      {
        auto &frame = TestPage->MainFrame();
        frame.CreateView({200, 100});
        Document = CreateRef<dom::HTMLDocument>();
        frame.SetDocument(ShareRefPtr<dom::Document>(Document.get()));
        auto parser = CreateRef<html::HTMLDocumentParser>(*Document);
        parser->InputStream().Append(krys::move(markup), IsEOF(true));
        parser->PumpTokenizer();
        Document->StyleScope().FlushPendingUpdate();
        TestPage->UpdateRendering();
      }

      KRYS_NODISCARD dom::Element &ById(dom::DOMStringView id) noexcept
      {
        auto element = Document->GetElementById(id);
        REQUIRE(element);
        return *element;
      }

      KRYS_NODISCARD LocalFrameView &View() noexcept
      {
        return *TestPage->MainFrame().View();
      }

      KRYS_NODISCARD EventHandler &Handler() noexcept
      {
        return TestPage->MainFrame().GetEventHandler();
      }

      void MoveMouse(double x, double y) noexcept
      {
        (void)Handler().MouseMoved(PlatformMouseEvent(DoublePoint(x, y), DoublePoint(x, y), MouseButton::None,
                                                      PlatformEvent::Type::MouseMoved, 0, {},
                                                      MonotonicTime::Now()));
      }

      /// @brief Runs the event loop @p delta after its last run, then the rendering update if one is
      /// scheduled.
      void Frame(MonotonicTime::duration delta = {}) noexcept
      {
        auto &loop = html::EventLoop::Current();
        loop.Run(loop.Now() + delta);
        if (TestPage->IsRenderingUpdateScheduled())
        {
          TestPage->UpdateRendering();
        }
      }
    };

    /// @brief Calls a function for each event it handles.
    struct Listener : dom::EventListenerCallback
    {
      Func<void(dom::Event &)> Function;

      explicit Listener(Func<void(dom::Event &)> &&function) noexcept : Function(krys::move(function))
      {
      }

      void HandleEvent(dom::Event &event) noexcept override
      {
        Function(event);
      }
    };

    constexpr utf8_stringview TallPage =
      u8"<!DOCTYPE html><style>body{margin:0}#page{height:600px}</style><div id=page>"
      u8"<div id=top style=height:50px></div><div id=target style=margin-top:250px;height:20px></div></div>";
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Page: the main frame", "[Page][RenderingUpdate]")
  {
    auto page = Page::Create();
    CHECK(page->MainFrame().GetPage() == page.get());
    CHECK_FALSE(page->MainFrame().Document());
    CHECK_FALSE(LocalFrame::Create()->GetPage());

    // A page without a document has nothing to update.
    page->UpdateRendering();
    CHECK_FALSE(page->IsRenderingUpdateScheduled());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Page: scheduling the rendering update",
                   "[Page][RenderingUpdate]")
  {
    PageFixture fixture {utf8_string(TallPage)};
    auto &page = *fixture.TestPage;
    CHECK_FALSE(page.IsRenderingUpdateScheduled());

    SECTION("a style change schedules one, which lays out")
    {
      REQUIRE_FALSE(fixture.ById(u8"top").SetAttribute(u8"style", u8"height:80px").HasException());
      CHECK(page.IsRenderingUpdateScheduled());
      page.UpdateRendering();
      CHECK_FALSE(page.IsRenderingUpdateScheduled());
      CHECK_FALSE(fixture.Document->HasPendingStyleRecalc());
      CHECK_FALSE(fixture.View().LayoutContext().NeedsLayout());
      CHECK(page.LastRenderingUpdateTimestamp() == html::EventLoop::Current().Now());
    }

    SECTION("a scroll schedules one, which fires the scroll events")
    {
      List<std::string> records;
      Ref<dom::EventListenerCallback> recorder = CreateRef<Listener>(
        [&](dom::Event &event)
        { records.emplace_back(event.Type().View().begin(), event.Type().View().end()); });
      fixture.Document->AddEventListener(dom::DOMStringAtom(u8"scroll"), Ref {recorder}, false);

      fixture.View().SetScrollPosition({0, 100});
      CHECK(page.IsRenderingUpdateScheduled());
      CHECK(records.empty());
      page.UpdateRendering();
      CHECK(records == List<std::string> {"scroll"});
    }

    SECTION("a step asked for after it ran runs in the next update")
    {
      // A `scroll` listener scrolls again: the scroll steps already ran, so the next update fires it.
      int32 scrolls = 0;
      Ref<dom::EventListenerCallback> scroller = CreateRef<Listener>(
        [&](dom::Event &)
        {
          if (++scrolls == 1)
          {
            fixture.View().SetScrollPosition({0, 200});
          }
        });
      fixture.Document->AddEventListener(dom::DOMStringAtom(u8"scroll"), Ref {scroller}, false);

      fixture.View().SetScrollPosition({0, 100});
      page.UpdateRendering();
      CHECK(scrolls == 1);
      CHECK(page.IsRenderingUpdateScheduled());
      page.UpdateRendering();
      CHECK(scrolls == 2);
      CHECK_FALSE(page.IsRenderingUpdateScheduled());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Page: autofocus", "[Page][RenderingUpdate][Focus]")
  {
    SECTION("the first focusable candidate is focused in the rendering update")
    {
      PageFixture fixture {
        u8"<!DOCTYPE html><div id=hidden autofocus tabindex=0 style=display:none></div>"
        u8"<div id=first autofocus tabindex=0></div><div id=second autofocus tabindex=0></div>"};
      // The constructor's rendering update focused it.
      CHECK(fixture.Document->FocusedElement() == &fixture.ById(u8"first"));
      CHECK(fixture.Document->AutofocusProcessed());
      CHECK(fixture.Document->AutofocusCandidates().empty());

      // Autofocus is processed once.
      auto late = fixture.Document->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(late->SetAttribute(u8"autofocus", u8"").HasException());
      REQUIRE_FALSE(late->SetAttribute(u8"tabindex", u8"0").HasException());
      REQUIRE_FALSE(fixture.Document->Body()->AppendChild(*late).HasException());
      CHECK(fixture.Document->AutofocusCandidates().empty());
    }

    SECTION("an element focused first wins")
    {
      auto page = Page::Create();
      page->MainFrame().CreateView({200, 100});
      auto document = CreateRef<dom::HTMLDocument>();
      page->MainFrame().SetDocument(ShareRefPtr<dom::Document>(document.get()));
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(
        u8"<!DOCTYPE html><div id=auto autofocus tabindex=0></div><div id=other tabindex=0></div>",
        IsEOF(true));
      parser->PumpTokenizer();
      CHECK(document->AutofocusCandidates().size() == 1);
      CHECK(page->IsRenderingUpdateScheduled());

      Downcast<html::HTMLElement>(*document->GetElementById(u8"other")).Focus();
      page->UpdateRendering();
      CHECK(document->FocusedElement() == document->GetElementById(u8"other").get());
      CHECK(document->AutofocusProcessed());
    }

    SECTION("not when the document has a target element")
    {
      auto page = Page::Create();
      page->MainFrame().CreateView({200, 100});
      auto document = CreateRef<dom::HTMLDocument>();
      page->MainFrame().SetDocument(ShareRefPtr<dom::Document>(document.get()));
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(
        u8"<!DOCTYPE html><div id=auto autofocus tabindex=0></div><div id=target></div>", IsEOF(true));
      parser->PumpTokenizer();
      REQUIRE(document->AutofocusCandidates().size() == 1);

      // As when the URL's fragment names an element.
      document->SetCSSTarget(document->GetElementById(u8"target").get());
      page->UpdateRendering();
      CHECK_FALSE(document->FocusedElement());
      CHECK(document->AutofocusProcessed());
      CHECK(document->AutofocusCandidates().empty());
    }

    SECTION("only in a document shown in a frame")
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto parser = CreateRef<html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(u8"<!DOCTYPE html><div autofocus tabindex=0></div>", IsEOF(true));
      parser->PumpTokenizer();
      CHECK(document->AutofocusCandidates().empty());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Page: focus fixup", "[Page][RenderingUpdate][Focus]")
  {
    PageFixture fixture {u8"<!DOCTYPE html><div id=box tabindex=0></div>"};
    auto &box = fixture.ById(u8"box");
    Downcast<html::HTMLElement>(box).Focus();
    REQUIRE(fixture.Document->FocusedElement() == &box);

    // Hidden, it stops being a focusable area, and the rendering update moves the focus to the viewport.
    REQUIRE_FALSE(box.SetAttribute(u8"style", u8"display:none").HasException());
    CHECK(fixture.Document->FocusedElement() == &box);
    fixture.TestPage->UpdateRendering();
    CHECK_FALSE(fixture.Document->FocusedElement());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Page: the cursor follows the page under a still mouse",
                   "[Page][RenderingUpdate]")
  {
    PageFixture fixture {u8"<!DOCTYPE html><style>body{margin:0}</style>"
                         u8"<div id=box style=width:100px;height:50px;cursor:pointer></div>"};
    fixture.MoveMouse(10, 10);
    REQUIRE(fixture.Handler().CurrentMouseCursor().GetType() == Cursor::Type::Hand);

    // A style change to the cursor updates it in the rendering update.
    REQUIRE_FALSE(fixture.ById(u8"box")
                    .SetAttribute(u8"style", u8"width:100px;height:50px;cursor:crosshair")
                    .HasException());
    fixture.Frame();
    CHECK(fixture.Handler().CurrentMouseCursor().GetType() == Cursor::Type::Cross);
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "Page: hover follows the content under a still mouse after a scroll",
                   "[Page][RenderingUpdate]")
  {
    PageFixture fixture {utf8_string(TallPage)};
    fixture.MoveMouse(10, 5);
    REQUIRE(fixture.Handler().ElementUnderMouse() == &fixture.ById(u8"top"));

    // The target scrolls under the mouse; a fake mouse move soon after (100 or 250 milliseconds) finds it.
    fixture.View().SetScrollPosition({0, 300});
    fixture.Frame();
    CHECK(fixture.Handler().ElementUnderMouse() == &fixture.ById(u8"top"));
    fixture.Frame(250ms);
    CHECK(fixture.Handler().ElementUnderMouse() == &fixture.ById(u8"target"));
    CHECK(fixture.ById(u8"target").Hovered());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Page: page scale", "[Page]")
  {
    PageFixture fixture(u8"<!DOCTYPE html><style>body{margin:0}div{width:100px;height:50px}</style>"
                        u8"<div id=a></div><div id=b></div><div style=height:300px></div>");
    auto &view = fixture.View();
    auto &frame = fixture.TestPage->MainFrame();
    CHECK(fixture.TestPage->PageScaleFactor() == 1);
    // The view is 200x100, less the vertical scrollbar.
    CHECK(view.ContentsSize() == IntSize(179, 400));

    auto hit = [&](int32 x, int32 y) -> std::string
    {
      auto result = fixture.Handler().HitTestResultAtPoint(
        LayoutPoint(view.WindowToContents(IntPoint(x, y))),
        {HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active});
      auto *element = result.TargetElement();
      auto id = element ? element->GetAttribute(u8"id").value_or(dom::DOMString()) : dom::DOMString();
      return std::string(id.begin(), id.end());
    };
    CHECK(hit(50, 75) == "b");

    // Scaled about the document's origin, the content is twice as big, and scrolls to the origin given.
    fixture.TestPage->SetPageScaleFactor(2, IntPoint(100, 40));
    fixture.Frame();
    CHECK(fixture.TestPage->PageScaleFactor() == 2);
    CHECK(frame.FrameScaleFactor() == 2);
    CHECK(view.ContentsSize() == IntSize(358, 800));
    CHECK(view.GetScrollPosition() == IntPoint(100, 40));

    // The viewport's (10, 70) is the content's (110, 110), the document's (55, 55); its (10, 35) the
    // document's (55, 37.5).
    CHECK(hit(10, 70) == "b");
    CHECK(hit(10, 35) == "a");
    // The client's (10, 35), in the document's pixels, is the content's (120, 110).
    auto fromPoint = fixture.Document->ElementFromPoint(10, 35);
    REQUIRE(fromPoint);
    CHECK(fromPoint->GetAttribute(u8"id") == dom::DOMString(u8"b"));

    // Script sees the scale in devicePixelRatio and the visual viewport, which is half the view (less the
    // scrollbar).
    auto &window = *fixture.Document->Window();
    CHECK(window.DevicePixelRatio() == 2);
    CHECK(window.GetVisualViewport().Scale() == 2);
    CHECK(window.GetVisualViewport().Width() == 89.5);
    CHECK(window.GetVisualViewport().PageLeft() == 50);

    // Back to no scale.
    fixture.TestPage->SetPageScaleFactor(1, IntPoint());
    fixture.Frame();
    CHECK(view.ContentsSize() == IntSize(179, 400));
    CHECK(view.GetScrollPosition() == IntPoint());
    CHECK(hit(50, 75) == "b");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Page: page scale paints the content scaled", "[Page]")
  {
    PaintFixture fixture(
      u8"<!DOCTYPE html><style>body{margin:0}div{width:10px;height:10px;background:red}</style>"
      u8"<div></div>",
      {200, 100}, true);
    fixture.TestPage->SetPageScaleFactor(2, IntPoint());
    auto painted = fixture.Painted();
    // The view's content (half of it in view) is scaled about the origin.
    CHECK(painted.starts_with(u8"\n(concatenate-ctm\n  (ctm {m=((2.00,0.00)(0.00,2.00)) t=(0.00,0.00)}))"));
    CHECK(painted.contains(u8"(rect at (0,0) size 100x50)"));
    CHECK(painted.contains(u8"(rect at (0,0) size 10x10)\n  (color #FF0000)"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Page: a renderer-only change schedules a rendering update",
                   "[Page][RenderingUpdate]")
  {
    PageFixture fixture {u8"<!DOCTYPE html><p id=p>old</p><svg><clipPath id=clip><rect width=10 height=10>"
                         u8"</rect></clipPath></svg>"};
    REQUIRE_FALSE(fixture.TestPage->IsRenderingUpdateScheduled());

    SECTION("A text node's data")
    {
      auto &text = Downcast<dom::Text>(*fixture.ById(u8"p").FirstChild());
      REQUIRE_FALSE(text.ReplaceData(0, 3, u8"new").HasException());
      CHECK(fixture.TestPage->IsRenderingUpdateScheduled());

      fixture.Frame();
      CHECK_FALSE(fixture.TestPage->IsRenderingUpdateScheduled());
      REQUIRE(text.Renderer());
      CHECK(text.Renderer()->Text() == u8"new");
    }

    SECTION("An SVG element's renderer")
    {
      auto &clip = fixture.ById(u8"clip");
      REQUIRE_FALSE(clip.SetAttribute(u8"clipPathUnits", u8"objectBoundingBox").HasException());
      CHECK(fixture.TestPage->IsRenderingUpdateScheduled());
    }
  }
}
