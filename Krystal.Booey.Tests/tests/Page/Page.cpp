#include "Krystal.Booey/Page/Page.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

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
}
