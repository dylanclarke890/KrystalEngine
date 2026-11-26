#include "Krystal.Web/Page/Frame.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Web/Animation/Timelines/DocumentTimeline.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Dom/Events/EventListener.hpp"
#include "Krystal.Web/Dom/Events/EventListenerHandle.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Events/Pointer/MouseEvent.hpp"
#include "Krystal.Web/Html/Algorithms/RenderBlocking.hpp"
#include "Krystal.Web/Html/Elements/HTMLBodyElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Html/Parser/DocumentParser.hpp"
#include "Krystal.Web/Html/Types/DocumentVisibilityState.hpp"
#include "Krystal.Web/Html/Window.hpp"
#include "Krystal.Web/Input/EventHandler.hpp"
#include "Krystal.Web/Input/PlatformKeyboardEvent.hpp"
#include "Krystal.Web/Input/PlatformMouseEvent.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Page/Settings.hpp"
#include "Krystal.Web/Page/VisualViewport.hpp"
#include "Krystal.Web/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Web/Rendering/RenderText.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"
#include "Krystal.Web/Svg/Animation/SMILTimeContainer.hpp"
#include "Krystal.Web/Svg/Elements/SVGElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGRectElement.hpp"
#include "Krystal.Web/Svg/Elements/SVGSvgElement.hpp"
#include "Krystal.Web/Svg/SVGDocumentExtensions.hpp"

namespace krys::web::tests
{
  namespace
  {
    using namespace std::chrono_literals;

    /// @brief A frame with a 200x100 view, and @p markup parsed into its document.
    struct FrameFixture
    {
      Ref<web::Frame> TestFrame {web::Frame::Create()};
      RefPtr<HTMLDocument> Document;

      explicit FrameFixture(utf8_string &&markup) noexcept
      {
        auto &frame = *TestFrame;
        frame.CreateView({200, 100});
        Document = CreateRef<HTMLDocument>();
        frame.SetDocument(ShareRefPtr<web::Document>(Document.get()));
        auto parser = CreateRef<html::DocumentParser>(*Document);
        parser->InputStream().Append(krys::move(markup), IsEOF::Yes);
        parser->PumpTokenizer();
        Document->StyleScope().FlushPendingUpdate();
        TestFrame->UpdateRendering();
      }

      KRYS_NODISCARD Element &ById(DOMStringView id) const noexcept
      {
        auto element = Document->GetElementById(id);
        REQUIRE(element);
        return *element;
      }

      KRYS_NODISCARD FrameView &View() const noexcept
      {
        return *TestFrame->View();
      }

      KRYS_NODISCARD EventHandler &Handler() const noexcept
      {
        return TestFrame->GetEventHandler();
      }

      void MoveMouse(double x, double y) noexcept
      {
        (void)Handler().MouseMoved(PlatformMouseEvent(PointD(x, y), PointD(x, y), MouseButton::None,
                                                      PlatformEvent::Type::MouseMoved, 0, {},
                                                      MonotonicTime::Now()));
      }

      /// @brief Runs the event loop @p delta after its last run, then the rendering update if one is
      /// scheduled.
      void Frame(MonotonicTime::duration delta = {}) const noexcept
      {
        auto &loop = html::EventLoop::Current();
        loop.Run(loop.Now() + delta);
        if (TestFrame->IsRenderingUpdateScheduled())
        {
          TestFrame->UpdateRendering();
        }
      }
    };

    /// @brief Calls a function for each event it handles.
    struct Listener : EventListenerCallback
    {
      Func<void(Event &)> Function;

      explicit Listener(Func<void(Event &)> &&function) noexcept : Function(krys::move(function))
      {
      }

      void HandleEvent(Event &event) noexcept override
      {
        Function(event);
      }
    };

    constexpr utf8_stringview TallPage =
      u8"<!DOCTYPE html><style>body{margin:0}#page{height:600px}</style><div id=page>"
      u8"<div id=top style=height:50px></div><div id=target style=margin-top:250px;height:20px></div></div>";
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: a new frame", "[Page][RenderingUpdate]")
  {
    auto frame = web::Frame::Create();
    CHECK_FALSE(frame->Document());

    // A frame without a document has nothing to update.
    frame->UpdateRendering();
    CHECK_FALSE(frame->IsRenderingUpdateScheduled());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: scheduling the rendering update",
                   "[Page][RenderingUpdate]")
  {
    FrameFixture fixture {utf8_string(TallPage)};
    auto &frame = *fixture.TestFrame;
    CHECK_FALSE(frame.IsRenderingUpdateScheduled());

    SECTION("a style change schedules one, which lays out")
    {
      REQUIRE_FALSE(fixture.ById(u8"top").SetAttribute(u8"style", u8"height:80px").HasException());
      CHECK(frame.IsRenderingUpdateScheduled());
      frame.UpdateRendering();
      CHECK_FALSE(frame.IsRenderingUpdateScheduled());
      CHECK_FALSE(fixture.Document->HasPendingStyleRecalc());
      CHECK_FALSE(fixture.View().LayoutContext().NeedsLayout());
      CHECK(frame.LastRenderingUpdateTimestamp() == html::EventLoop::Current().Now());
    }

    SECTION("a scroll schedules one, which fires the scroll events")
    {
      List<std::string> records;
      Ref<EventListenerCallback> recorder = CreateRef<Listener>(
        [&](Event &event) { records.emplace_back(event.Type().View().begin(), event.Type().View().end()); });
      fixture.Document->AddEventListener(u8"scroll", Ref {recorder}, false);

      fixture.View().SetScrollPosition({0, 100});
      CHECK(frame.IsRenderingUpdateScheduled());
      CHECK(records.empty());
      frame.UpdateRendering();
      CHECK(records == List<std::string> {"scroll"});
    }

    SECTION("a step asked for after it ran runs in the next update")
    {
      // A `scroll` listener scrolls again: the scroll steps already ran, so the next update fires it.
      int32 scrolls = 0;
      Ref<EventListenerCallback> scroller = CreateRef<Listener>(
        [&](Event &)
        {
          if (++scrolls == 1)
          {
            fixture.View().SetScrollPosition({0, 200});
          }
        });
      fixture.Document->AddEventListener(u8"scroll", Ref {scroller}, false);

      fixture.View().SetScrollPosition({0, 100});
      frame.UpdateRendering();
      CHECK(scrolls == 1);
      CHECK(frame.IsRenderingUpdateScheduled());
      frame.UpdateRendering();
      CHECK(scrolls == 2);
      CHECK_FALSE(frame.IsRenderingUpdateScheduled());
    }

    SECTION("a style change during the update is laid out by it")
    {
      // The request carries no steps; the update's last layout takes the change in, so none is left over.
      Ref<EventListenerCallback> restyler = CreateRef<Listener>(
        [&](Event &) { (void)fixture.ById(u8"top").SetAttribute(u8"style", u8"height:80px"); });
      fixture.Document->AddEventListener(u8"scroll", Ref {restyler}, false);

      fixture.View().SetScrollPosition({0, 100});
      frame.UpdateRendering();
      CHECK_FALSE(fixture.Document->HasPendingStyleRecalc());
      CHECK_FALSE(fixture.View().LayoutContext().NeedsLayout());
      CHECK_FALSE(frame.IsRenderingUpdateScheduled());
    }
  }

  TEST_CASE_METHOD(
    css::tests::ValueFixture, "Frame: autofocus",
    "[Page][RenderingUpdate][Focus][html:the-autofocus-attribute][html:autofocus-processed-flag]"
    "[html:flush-autofocus-candidates]")
  {
    SECTION("the first focusable candidate is focused in the rendering update")
    {
      FrameFixture fixture {
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

    SECTION("SVG and MathML elements are candidates too")
    {
      auto markup = GENERATE(as<utf8_string> {},
                             u8"<!DOCTYPE html><svg id=target autofocus tabindex=0 width=9 height=9></svg>",
                             u8"<!DOCTYPE html><svg width=9 height=9><rect id=target autofocus tabindex=0 "
                             u8"width=9 height=9 /></svg>",
                             u8"<!DOCTYPE html><math id=target autofocus tabindex=0><mi>x</mi></math>",
                             u8"<!DOCTYPE html><math><mi id=target autofocus tabindex=0>x</mi></math>");
      FrameFixture fixture {krys::move(markup)};
      CHECK(fixture.Document->FocusedElement() == &fixture.ById(u8"target"));
      CHECK(fixture.Document->AutofocusProcessed());
    }

    SECTION("an SVG or MathML element that can't be focused leaves the next candidate")
    {
      auto markup = GENERATE(as<utf8_string> {}, u8"<svg autofocus width=9 height=9></svg>",
                             u8"<math autofocus><mi>x</mi></math>");
      FrameFixture fixture {u8"<!DOCTYPE html>" + markup + u8"<input id=target autofocus>"};
      CHECK(fixture.Document->FocusedElement() == &fixture.ById(u8"target"));
    }

    SECTION("an SVG or MathML element is a candidate once inserted")
    {
      auto ns = GENERATE(as<const char8_t *> {}, u8"http://www.w3.org/2000/svg",
                         u8"http://www.w3.org/1998/Math/MathML");
      FrameFixture fixture {u8"<!DOCTYPE html><body>"};
      auto &document = *fixture.Document;
      auto element = document.CreateElementNS(DOMStringAtom(ns), u8"g").ReleaseValue();
      REQUIRE_FALSE(element->SetAttribute(u8"autofocus", u8"").HasException());
      REQUIRE_FALSE(element->SetAttribute(u8"tabindex", u8"0").HasException());
      CHECK(document.AutofocusCandidates().empty());
      REQUIRE_FALSE(document.Body()->AppendChild(*element).HasException());
      CHECK(document.AutofocusCandidates().size() == 1);
    }

    SECTION("a candidate that isn't a focusable area focuses its focusable area")
    {
      // A shadow host that delegates focus: its focus delegate.
      FrameFixture fixture {u8"<!DOCTYPE html><body>"};
      auto &document = *fixture.Document;
      auto host = document.CreateElement(u8"div").ReleaseValue();
      auto shadow = host->AttachShadow({.DelegatesFocus = dom::DelegatesFocus::Yes}).ReleaseValue();
      auto inner = document.CreateElement(u8"input").ReleaseValue();
      REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());
      REQUIRE_FALSE(host->SetAttribute(u8"autofocus", u8"").HasException());
      REQUIRE_FALSE(document.Body()->AppendChild(*host).HasException());
      fixture.TestFrame->UpdateRendering();
      CHECK(document.FocusedElement() == inner.get());
      CHECK(document.AutofocusProcessed());
    }

    SECTION("an element focused first wins")
    {
      auto frame = web::Frame::Create();
      frame->CreateView({200, 100});
      auto document = CreateRef<HTMLDocument>();
      frame->SetDocument(ShareRefPtr<Document>(document.get()));
      auto parser = CreateRef<html::DocumentParser>(*document);
      parser->InputStream().Append(
        u8"<!DOCTYPE html><div id=auto autofocus tabindex=0></div><div id=other tabindex=0></div>",
        IsEOF::Yes);
      parser->PumpTokenizer();
      CHECK(document->AutofocusCandidates().size() == 1);
      CHECK(frame->IsRenderingUpdateScheduled());

      Downcast<HTMLElement>(*document->GetElementById(u8"other")).Focus();
      frame->UpdateRendering();
      CHECK(document->FocusedElement() == document->GetElementById(u8"other").get());
      CHECK(document->AutofocusProcessed());
    }

    SECTION("not when the document has a target element")
    {
      auto frame = web::Frame::Create();
      frame->CreateView({200, 100});
      auto document = CreateRef<HTMLDocument>();
      frame->SetDocument(ShareRefPtr<Document>(document.get()));
      auto parser = CreateRef<html::DocumentParser>(*document);
      parser->InputStream().Append(
        u8"<!DOCTYPE html><div id=auto autofocus tabindex=0></div><div id=target></div>", IsEOF::Yes);
      parser->PumpTokenizer();
      REQUIRE(document->AutofocusCandidates().size() == 1);

      // As when the URL's fragment names an element.
      document->SetCSSTarget(document->GetElementById(u8"target").get());
      frame->UpdateRendering();
      CHECK_FALSE(document->FocusedElement());
      CHECK(document->AutofocusProcessed());
      CHECK(document->AutofocusCandidates().empty());
    }

    SECTION("only in a document shown in a frame")
    {
      auto document = CreateRef<HTMLDocument>();
      auto parser = CreateRef<html::DocumentParser>(*document);
      parser->InputStream().Append(u8"<!DOCTYPE html><div autofocus tabindex=0></div>", IsEOF::Yes);
      parser->PumpTokenizer();
      CHECK(document->AutofocusCandidates().empty());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "Frame: render-blocked documents are left out of the rendering update",
                   "[Page][RenderingUpdate][html:render-blocked][html:update-the-rendering]")
  {
    // An HTML document with no body yet is render-blocked.
    auto frame = web::Frame::Create();
    frame->CreateView({200, 100});
    auto document = CreateRef<HTMLDocument>();
    auto html = document->CreateElement(u8"html").ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*html).HasException());
    frame->SetDocument(ShareRefPtr<Document>(document.get()));
    REQUIRE(html::RenderBlocking::IsRenderBlocked(*document));

    uint32 calls = 0;
    (void)document->RequestAnimationFrame([&](dom::DOMHighResTimeStamp) { ++calls; });
    frame->UpdateRendering();
    CHECK(calls == 0);
    // The next rendering update tries again.
    CHECK(frame->IsRenderingUpdateScheduled());

    SECTION("until it's unblocked")
    {
      REQUIRE_FALSE(html->AppendChild(*document->CreateElement(u8"body").ReleaseValue()).HasException());
      frame->UpdateRendering();
      CHECK(calls == 1);
    }

    SECTION("or the block times out")
    {
      auto &loop = html::EventLoop::Current();
      loop.Run(loop.Now() + html::RenderBlocking::Timeout + 1ms);
      CHECK_FALSE(html::RenderBlocking::IsRenderBlocked(*document));
      frame->UpdateRendering();
      CHECK(calls == 1);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: focus fixup", "[Page][RenderingUpdate][Focus]")
  {
    FrameFixture fixture {u8"<!DOCTYPE html><div id=box tabindex=0></div>"};
    auto &box = fixture.ById(u8"box");
    Downcast<HTMLElement>(box).Focus();
    REQUIRE(fixture.Document->FocusedElement() == &box);

    // Hidden, it stops being a focusable area, and the rendering update moves the focus to the viewport.
    REQUIRE_FALSE(box.SetAttribute(u8"style", u8"display:none").HasException());
    CHECK(fixture.Document->FocusedElement() == &box);
    fixture.TestFrame->UpdateRendering();
    CHECK_FALSE(fixture.Document->FocusedElement());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: the cursor follows the page under a still mouse",
                   "[Page][RenderingUpdate]")
  {
    FrameFixture fixture {u8"<!DOCTYPE html><style>body{margin:0}</style>"
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

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: cursor images",
                   "[Page][Cursor][css-ui:cursor][css-ui:typedef-cursor-cursor-image]")
  {
    constexpr utf8_stringview Small = u8"data:image/"
                                      u8"png;base64,iVBORw0KGgoAAAANSUhEUgAAAAQAAAAECAYAAACp8Z5+"
                                      u8"AAAAEklEQVR4nGP4z8DwHxkzkC4AADxAH+HggXe0AAAAAElFTkSuQmCC";
    constexpr utf8_stringview Eight =
      u8"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAAgAAAAICAYAAADED76LAAAAEUlEQVR4nGNgYPj/Hz8eEQoAQ1d/"
      u8"gZtbdTsAAAAASUVORK5CYII=";
    constexpr utf8_stringview Big = u8"data:image/"
                                    u8"png;base64,"
                                    u8"iVBORw0KGgoAAAANSUhEUgAAAIIAAAABCAYAAAASJ3aLAAAAEklEQVR4nGP4z8DwfxQzjH"
                                    u8"gMAJrgAwzQzIqPAAAAAElFTkSuQmCC";
    auto page = [](utf8_stringview cursor)
    {
      return u8"<!DOCTYPE html><style>body{margin:0}</style><div id=box "
             u8"style='width:100px;height:50px;cursor:"
             + utf8_string(cursor) + u8"'></div>";
    };
    auto cursorAt = [](FrameFixture &fixture)
    {
      // The images load as tasks.
      fixture.Frame();
      fixture.MoveMouse(10, 10);
      return fixture.Handler().CurrentMouseCursor();
    };

    SECTION("the first image that loads is the cursor, with its hot spot")
    {
      FrameFixture fixture {page(u8"url(" + utf8_string(Small) + u8") 3 2, crosshair")};
      auto cursor = cursorAt(fixture);
      REQUIRE(cursor.GetType() == Cursor::Type::Custom);
      REQUIRE(cursor.GetImage());
      CHECK(cursor.GetImage()->Size() == SizeF(4, 4));
      CHECK(cursor.HotSpot() == PointI(3, 2));
      CHECK(cursor.ImageScaleFactor() == 1);
    }

    SECTION("a hot spot outside the image is its top left")
    {
      FrameFixture fixture {page(u8"url(" + utf8_string(Small) + u8") 4 1, crosshair")};
      auto cursor = cursorAt(fixture);
      REQUIRE(cursor.GetType() == Cursor::Type::Custom);
      CHECK(cursor.HotSpot() == PointI(0, 0));
    }

    SECTION("an image that fails or is too big is passed over")
    {
      FrameFixture fixture {page(u8"url(data:image/png;base64,AAAA), url(" + utf8_string(Big) + u8"), url("
                                 + utf8_string(Small) + u8"), crosshair")};
      auto cursor = cursorAt(fixture);
      REQUIRE(cursor.GetType() == Cursor::Type::Custom);
      CHECK(cursor.GetImage()->Size() == SizeF(4, 4));
    }

    SECTION("with no image to show, the predefined cursor")
    {
      FrameFixture fixture {page(u8"url(" + utf8_string(Big) + u8"), crosshair")};
      CHECK(cursorAt(fixture).GetType() == Cursor::Type::Cross);
    }

    SECTION("an image-set() image has its resolution, and its hot spot is in its natural size")
    {
      FrameFixture fixture {page(u8"image-set(url(" + utf8_string(Eight) + u8") 2x) 3 3, auto")};
      auto cursor = cursorAt(fixture);
      REQUIRE(cursor.GetType() == Cursor::Type::Custom);
      CHECK(cursor.ImageScaleFactor() == 2);
      CHECK(cursor.HotSpot() == PointI(3, 3));
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture,
                   "Frame: hover follows the content under a still mouse after a scroll",
                   "[Page][RenderingUpdate]")
  {
    FrameFixture fixture {utf8_string(TallPage)};
    fixture.MoveMouse(10, 5);
    REQUIRE(fixture.Handler().ElementUnderMouse() == &fixture.ById(u8"top"));

    // The target scrolls under the mouse; a fake mouse move soon after (100 or 250 milliseconds) finds it.
    fixture.View().SetScrollPosition({0, 300});
    fixture.Frame();
    CHECK(fixture.Handler().ElementUnderMouse() == &fixture.ById(u8"top"));
    fixture.Frame(250ms);
    CHECK(fixture.Handler().ElementUnderMouse() == &fixture.ById(u8"target"));
    CHECK(fixture.ById(u8"target").Hovered());

    // The fake move has the modifier keys of the latest event, a key press here.
    Maybe<bool> shiftKey;
    auto listener = fixture.ById(u8"top").AddEventListener(
      u8"mousemove", [&](Event &event) { shiftKey = Downcast<MouseEvent>(event).ShiftKey(); });
    (void)fixture.Handler().KeyEvent(
      PlatformKeyboardEvent(PlatformEvent::Type::KeyDown, u8"", u8"", u8"Shift", u8"ShiftLeft", 16, false,
                            false, false, {PlatformEvent::Modifier::ShiftKey}, MonotonicTime::Now()));
    fixture.View().SetScrollPosition({0, 0});
    fixture.Frame(250ms);
    CHECK(fixture.Handler().ElementUnderMouse() == &fixture.ById(u8"top"));
    CHECK(shiftKey == true);

    // A hidden page gets no fake moves.
    fixture.TestFrame->SetIsVisible(false);
    fixture.View().SetScrollPosition({0, 300});
    fixture.Frame(250ms);
    CHECK(fixture.Handler().ElementUnderMouse() == &fixture.ById(u8"top"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: page scale", "[Page]")
  {
    FrameFixture fixture(u8"<!DOCTYPE html><style>body{margin:0}div{width:100px;height:50px}</style>"
                         u8"<div id=a></div><div id=b></div><div style=height:300px></div>");
    auto &view = fixture.View();
    auto &frame = *fixture.TestFrame;
    CHECK(fixture.TestFrame->PageScaleFactor() == 1);
    // The view is 200x100, less the vertical scrollbar.
    CHECK(view.ContentsSize() == SizeI(179, 400));

    auto hit = [&](int32 x, int32 y) -> std::string
    {
      auto result = fixture.Handler().HitTestResultAtPoint(
        LayoutPoint(view.WindowToContents(PointI(x, y))),
        {HitTestRequest::Type::ReadOnly, HitTestRequest::Type::Active});
      auto *element = result.TargetElement();
      auto id = element ? element->GetAttribute(u8"id").value_or(DOMString()) : DOMString();
      return std::string(id.begin(), id.end());
    };
    CHECK(hit(50, 75) == "b");

    // Scaled about the document's origin, the content is twice as big, and scrolls to the origin given.
    fixture.TestFrame->SetPageScaleFactor(2, PointI(100, 40));
    fixture.Frame();
    CHECK(fixture.TestFrame->PageScaleFactor() == 2);
    CHECK(frame.FrameScaleFactor() == 2);
    CHECK(view.ContentsSize() == SizeI(358, 800));
    CHECK(view.GetScrollPosition() == PointI(100, 40));

    // The viewport's (10, 70) is the content's (110, 110), the document's (55, 55); its (10, 35) the
    // document's (55, 37.5).
    CHECK(hit(10, 70) == "b");
    CHECK(hit(10, 35) == "a");
    // The client's (10, 35), in the document's pixels, is the content's (120, 110).
    auto fromPoint = fixture.Document->ElementFromPoint(10, 35);
    REQUIRE(fromPoint);
    CHECK(fromPoint->GetAttribute(u8"id") == DOMString(u8"b"));

    // Script sees the scale in devicePixelRatio and the visual viewport, which is half the view (less the
    // scrollbar).
    auto &window = *fixture.Document->Window();
    CHECK(window.DevicePixelRatio() == 2);
    CHECK(window.GetVisualViewport().Scale() == 2);
    CHECK(window.GetVisualViewport().Width() == 89.5);
    CHECK(window.GetVisualViewport().PageLeft() == 50);

    // Back to no scale.
    fixture.TestFrame->SetPageScaleFactor(1, PointI());
    fixture.Frame();
    CHECK(view.ContentsSize() == SizeI(179, 400));
    CHECK(view.GetScrollPosition() == PointI());
    CHECK(hit(50, 75) == "b");
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: page scale paints the content scaled", "[Page]")
  {
    PaintFixture fixture(
      u8"<!DOCTYPE html><style>body{margin:0}div{width:10px;height:10px;background:red}</style>"
      u8"<div></div>");
    fixture.Frame->SetPageScaleFactor(2, PointI());
    auto painted = fixture.Painted();
    // The view's content (half of it in view) is scaled about the origin.
    CHECK(painted.starts_with(u8"\n(concatenate-ctm\n  (ctm {m=((2.00,0.00)(0.00,2.00)) t=(0.00,0.00)}))"));
    CHECK(painted.contains(u8"(rect at (0,0) size 100x50)"));
    CHECK(painted.contains(u8"(rect at (0,0) size 10x10)\n  (color #FF0000)"));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: a renderer-only change schedules a rendering update",
                   "[Page][RenderingUpdate]")
  {
    FrameFixture fixture {u8"<!DOCTYPE html><p id=p>old</p><svg><clipPath id=clip><rect width=10 height=10>"
                          u8"</rect></clipPath></svg>"};
    REQUIRE_FALSE(fixture.TestFrame->IsRenderingUpdateScheduled());

    SECTION("A text node's data")
    {
      auto &text = Downcast<Text>(*fixture.ById(u8"p").FirstChild());
      REQUIRE_FALSE(text.ReplaceData(0, 3, u8"new").HasException());
      CHECK(fixture.TestFrame->IsRenderingUpdateScheduled());

      fixture.Frame();
      CHECK_FALSE(fixture.TestFrame->IsRenderingUpdateScheduled());
      REQUIRE(text.Renderer());
      CHECK(text.Renderer()->Text() == u8"new");
    }

    SECTION("An SVG element's renderer")
    {
      auto &clip = fixture.ById(u8"clip");
      REQUIRE_FALSE(clip.SetAttribute(u8"clipPathUnits", u8"objectBoundingBox").HasException());
      CHECK(fixture.TestFrame->IsRenderingUpdateScheduled());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: visibility", "[Page][Visibility]")
  {
    FrameFixture fixture {u8"<!DOCTYPE html><p>text</p>"};
    auto &frame = *fixture.TestFrame;
    auto &document = *fixture.Document;
    CHECK(frame.IsVisible());
    CHECK(document.VisibilityState() == html::DocumentVisibilityState::Visible);
    CHECK_FALSE(document.Hidden());

    List<bool> events;
    auto handle = document.AddEventListener(event_names::VisibilityChange,
                                            [&](Event &event) { events.push_back(event.Bubbles()); });

    SECTION("Hiding and showing the page fire visibilitychange in a task")
    {
      frame.SetIsVisible(false);
      CHECK(document.VisibilityState() == html::DocumentVisibilityState::Hidden);
      CHECK(document.Hidden());
      CHECK(events.empty());
      html::EventLoop::Current().RunUntilIdle();
      CHECK(events == List<bool> {true});

      // Hiding a hidden page changes nothing.
      frame.SetIsVisible(false);
      html::EventLoop::Current().RunUntilIdle();
      CHECK(events.size() == 1);

      frame.SetIsVisible(true);
      CHECK(document.VisibilityState() == html::DocumentVisibilityState::Visible);
      html::EventLoop::Current().RunUntilIdle();
      CHECK(events == List<bool> {true, true});
    }

    // https://html.spec.whatwg.org/multipage/webappapis.html#update-the-rendering (hidden documents aren't
    // rendered)
    SECTION("A hidden page's rendering updates wait for it to be shown")
    {
      frame.SetIsVisible(false);
      frame.ScheduleRenderingUpdate();
      frame.UpdateRendering();
      CHECK(frame.IsRenderingUpdateScheduled());

      frame.SetIsVisible(true);
      CHECK(frame.IsRenderingUpdateScheduled());
      frame.UpdateRendering();
      CHECK_FALSE(frame.IsRenderingUpdateScheduled());

      // Shown, a page is updated even if nothing asked while it was hidden.
      frame.SetIsVisible(false);
      frame.SetIsVisible(true);
      CHECK(frame.IsRenderingUpdateScheduled());
    }

    SECTION("A hidden page's animation frame callbacks wait for it to be shown")
    {
      uint32 calls = 0;
      (void)document.RequestAnimationFrame([&](dom::DOMHighResTimeStamp) { ++calls; });
      frame.SetIsVisible(false);
      fixture.Frame(16ms);
      frame.UpdateRendering();
      CHECK(calls == 0);

      // A callback asked for while hidden waits too.
      (void)document.RequestAnimationFrame([&](dom::DOMHighResTimeStamp) { ++calls; });
      frame.UpdateRendering();
      CHECK(calls == 0);

      frame.SetIsVisible(true);
      fixture.Frame(16ms);
      frame.UpdateRendering();
      CHECK(calls == 2);
    }

    SECTION("A hidden page's document timeline stops")
    {
      fixture.Frame(16ms);
      frame.UpdateRendering();
      frame.SetIsVisible(false);
      auto hiddenAt = document.Timeline().CurrentTime();
      REQUIRE(hiddenAt);
      fixture.Frame(500ms);
      frame.UpdateRendering();
      CHECK(document.Timeline().CurrentTime() == hiddenAt);

      frame.SetIsVisible(true);
      fixture.Frame(16ms);
      frame.UpdateRendering();
      auto shownAt = document.Timeline().CurrentTime();
      REQUIRE(shownAt);
      CHECK(*shownAt->Time() > *hiddenAt->Time());
    }

    SECTION("Unless the setting is off")
    {
      SetForScope suspension(document.GetSettings().HiddenPageCSSAnimationSuspensionEnabled, false);
      fixture.Frame(16ms);
      frame.UpdateRendering();
      frame.SetIsVisible(false);
      auto hiddenAt = document.Timeline().CurrentTime();
      REQUIRE(hiddenAt);
      fixture.Frame(500ms);
      frame.UpdateRendering();
      auto later = document.Timeline().CurrentTime();
      REQUIRE(later);
      CHECK(*later->Time() > *hiddenAt->Time());
      frame.SetIsVisible(true);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: a hidden frame hides its view", "[Page][Visibility]")
  {
    FrameFixture fixture {u8"<!DOCTYPE html><p id=p>text</p>"};
    auto &frame = *fixture.TestFrame;
    auto &renderer = *fixture.ById(u8"p").Renderer();
    CHECK(fixture.View().IsVisible());
    CHECK_FALSE(fixture.View().IsOffscreen());
    CHECK(renderer.IsVisibleIgnoringGeometry());

    frame.SetIsVisible(false);
    CHECK_FALSE(fixture.View().IsVisible());
    CHECK(fixture.View().IsOffscreen());
    CHECK_FALSE(renderer.IsVisibleIgnoringGeometry());

    frame.SetIsVisible(true);
    CHECK(fixture.View().IsVisible());
    CHECK(renderer.IsVisibleIgnoringGeometry());

    SECTION("A view made while the frame is hidden starts hidden")
    {
      frame.SetIsVisible(false);
      frame.CreateView({200, 100});
      CHECK_FALSE(fixture.View().IsVisible());
      frame.SetIsVisible(true);
      CHECK(fixture.View().IsVisible());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: a hidden page pauses its SMIL animations",
                   "[Page][Visibility]")
  {
    FrameFixture fixture {u8"<!DOCTYPE html><svg id=svg><rect width=10 height=10><animate attributeName=x "
                          u8"from=0 to=10 dur=1s></animate></rect></svg>"};
    auto &svg = Downcast<SVGSvgElement>(Downcast<SVGElement>(fixture.ById(u8"svg")));
    CHECK_FALSE(svg.AnimationsPaused());
    fixture.TestFrame->SetIsVisible(false);
    CHECK(svg.AnimationsPaused());
    fixture.TestFrame->SetIsVisible(true);
    CHECK_FALSE(svg.AnimationsPaused());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: holding image animations pauses SMIL animations",
                   "[Page][SVG]")
  {
    FrameFixture fixture {u8"<!DOCTYPE html><svg id=svg><rect width=10 height=10><animate attributeName=x "
                          u8"from=0 to=10 dur=1s></animate></rect></svg>"};
    auto &svg = Downcast<SVGSvgElement>(Downcast<SVGElement>(fixture.ById(u8"svg")));
    fixture.TestFrame->SetImageAnimationEnabled(false);
    CHECK(svg.AnimationsPaused());

    // Shown again, a page keeps them held.
    fixture.TestFrame->SetIsVisible(false);
    fixture.TestFrame->SetIsVisible(true);
    CHECK(svg.AnimationsPaused());

    fixture.TestFrame->SetImageAnimationEnabled(true);
    CHECK_FALSE(svg.AnimationsPaused());
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: animations follow the host's frame rate",
                   "[Page][Animation]")
  {
    FrameFixture fixture {u8"<!DOCTYPE html>"};
    auto &timeline = fixture.Document->Timeline();
    CHECK(fixture.TestFrame->DisplayNominalFramesPerSecond() == 60u);
    CHECK(timeline.AnimationInterval().count() == Catch::Approx(1.0 / 60));

    fixture.TestFrame->SetDisplayNominalFramesPerSecond(144);
    CHECK(timeline.AnimationInterval().count() == Catch::Approx(1.0 / 144));
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Frame: SMIL animations are sampled in the rendering update",
                   "[Page][SVG]")
  {
    FrameFixture fixture {
      u8"<!DOCTYPE html><svg id=svg><rect id=r width=10 height=10><animate attributeName=x "
      u8"from=0 to=100 dur=1s></animate></rect></svg>"};
    auto &svg = Downcast<SVGSvgElement>(Downcast<SVGElement>(fixture.ById(u8"svg")));
    auto &rect = Downcast<SVGRectElement>(Downcast<SVGElement>(fixture.ById(u8"r")));
    // As the document's load does.
    fixture.Document->SVGExtensions().StartAnimations();
    REQUIRE(svg.TimeContainer().IsActive());

    // An animation changing every frame asks for each rendering update, and is sampled in it.
    for (int i = 0; i < 2; ++i)
    {
      CHECK(fixture.TestFrame->IsRenderingUpdateScheduled());
      auto x = rect.X().ValueInSpecifiedUnits();
      fixture.TestFrame->UpdateRendering();
      CHECK(rect.X().ValueInSpecifiedUnits() > x);
    }
  }
}
