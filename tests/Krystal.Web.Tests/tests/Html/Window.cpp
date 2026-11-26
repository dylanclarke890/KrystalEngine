#include "Krystal.Web/Html/Window.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Web/Css/CSSOM/CSSStyleDeclaration.hpp"
#include "Krystal.Web/Dom/Algorithms/EventDispatcher.hpp"
#include "Krystal.Web/Dom/Collections/HTMLCollection.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Dom/Events/EventListenerHandle.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Web/Html/Attributes/EventHandlerAttributes.hpp"
#include "Krystal.Web/Html/Elements/HTMLBodyElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Html/Parser/DocumentParser.hpp"
#include "Krystal.Web/Infra/Namespaces.hpp"
#include "Krystal.Web/Input/EventHandler.hpp"
#include "Krystal.Web/Input/PlatformKeyboardEvent.hpp"
#include "Krystal.Web/Input/PlatformMouseEvent.hpp"
#include "Krystal.Web/Page/DOMSelection.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Page/VisualViewport.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"
#include "Krystal.Web/Svg/Elements/SVGElement.hpp"

namespace krys::web::tests
{
  namespace
  {
    /// @brief Counts the page's requests to be brought to the front.
    struct WindowWebClient final : WebClient
    {
      uint32 &FocusRequests;

      explicit WindowWebClient(uint32 &focusRequests) noexcept : FocusRequests(focusRequests)
      {
      }

      void AttachRootGraphicsLayer(web::Frame &, GraphicsLayer *) noexcept override
      {
      }

      void Focus() noexcept override
      {
        ++FocusRequests;
      }
    };

    /// @brief A page with a 200x100 view, and @p markup parsed into its main frame's document.
    struct WindowFixture
    {
      uint32 FocusRequests {0};
      Ref<web::Frame> Frame {CreateFrame()};
      RefPtr<HTMLDocument> Document;

      explicit WindowFixture(utf8_string &&markup = u8"<!DOCTYPE html>") noexcept
      {
        auto &frame = *Frame;
        frame.CreateView({200, 100});
        Document = CreateRef<HTMLDocument>();
        frame.SetDocument(ShareRefPtr<web::Document>(Document.get()));
        auto parser = CreateRef<html::DocumentParser>(*Document);
        parser->InputStream().Append(krys::move(markup), IsEOF::Yes);
        parser->PumpTokenizer();
        Document->StyleScope().FlushPendingUpdate();
        Frame->UpdateRendering();
      }

      KRYS_NODISCARD Ref<web::Frame> CreateFrame() noexcept
      {
        return web::Frame::Create(CreateUnique<WindowWebClient>(FocusRequests), nullptr,
                                  CreateUnique<RecordingFrameLoaderClient>());
      }

      KRYS_NODISCARD web::Window &Window() const noexcept
      {
        REQUIRE(Document->Window());
        return *Document->Window();
      }

      KRYS_NODISCARD Element &ById(DOMStringView id) const noexcept
      {
        auto element = Document->GetElementById(id);
        REQUIRE(element);
        return *element;
      }
    };
  }

  TEST_CASE("A document gets its window as it becomes a frame's document", "[Page][Window]")
  {
    auto document = CreateRef<HTMLDocument>();
    CHECK(document->Window() == nullptr);
    CHECK(document->DefaultView() == nullptr);

    auto frame = web::Frame::Create();
    frame->CreateView({200, 100});
    frame->SetDocument(ShareRefPtr<web::Document>(document.get()));

    auto *window = document->Window();
    REQUIRE(window);
    CHECK(document->DefaultView() == window);
    CHECK(frame->Window() == window);
    CHECK(window->Document() == document.get());
    CHECK(window->Frame() == frame.get());
    CHECK(window->IsCurrentlyDisplayedInFrame());
    CHECK(Is<web::Window>(static_cast<EventTarget &>(*window)));
    CHECK_FALSE(Is<web::Window>(static_cast<EventTarget &>(*document)));

    // The window's custom element registry is the document's.
    CHECK(&window->CustomElements() == document->CustomElementRegistry());

    // Leaving the frame, the document keeps its window, which has no browsing context.
    frame->SetDocument(nullptr);
    CHECK(document->Window() == window);
    CHECK(document->DefaultView() == nullptr);
    CHECK(window->Frame() == nullptr);
  }

  TEST_CASE("Events bubble from the document to the window, except load", "[Page][Window]")
  {
    WindowFixture fixture(u8"<!DOCTYPE html><button id=button>Go</button>");
    auto &window = fixture.Window();
    auto &button = fixture.ById(u8"button");

    List<std::string> order;
    auto windowCapture = window.AddEventListener(
      u8"click",
      [&](Event &event)
      {
        CHECK(event.CurrentTarget().get() == &window);
        CHECK(event.Target().get() == &button);
        order.emplace_back("window capture");
      },
      true);
    auto documentCapture = fixture.Document->AddEventListener(
      u8"click", [&](Event &) { order.emplace_back("document capture"); }, true);
    auto documentBubble =
      fixture.Document->AddEventListener(u8"click", [&](Event &) { order.emplace_back("document"); });
    auto windowBubble = window.AddEventListener(u8"click", [&](Event &) { order.emplace_back("window"); });

    button.DispatchSimulatedClick(nullptr);
    CHECK(order == List<std::string> {"window capture", "document capture", "document", "window"});

    SECTION("load stops at the document")
    {
      bool documentHeard = false;
      bool windowHeard = false;
      auto documentLoad =
        fixture.Document->AddEventListener(u8"load", [&](Event &) { documentHeard = true; });
      auto windowLoad = window.AddEventListener(u8"load", [&](Event &) { windowHeard = true; });

      auto load = Event::Create(event_names::Load, true, false);
      (void)dom::EventDispatcher::DispatchToTarget(*load, &button);
      CHECK(documentHeard);
      CHECK_FALSE(windowHeard);
    }

    // The legacy target override flag of dispatch.
    SECTION("load at the window with the legacy target override targets the document")
    {
      bool windowHeard = false;
      auto windowLoad = window.AddEventListener(u8"load",
                                                [&](Event &event)
                                                {
                                                  windowHeard = true;
                                                  CHECK(event.CurrentTarget().get() == &window);
                                                  CHECK(event.Target().get() == fixture.Document.get());
                                                });
      (void)dom::EventDispatcher::FireEvent<Event>(event_names::Load, window, {}, true);
      CHECK(windowHeard);
    }

    SECTION("a document without a browsing context doesn't reach its window, nor run its own listeners")
    {
      // Its window's document isn't fully active, so inner invoke skips the listeners.
      fixture.Frame->SetDocument(nullptr);
      order.clear();
      auto event = Event::Create(event_names::Click, true, false);
      (void)dom::EventDispatcher::DispatchToTarget(*event, &button);
      CHECK(order.empty());
    }
  }

  TEST_CASE("resize fires at the window when the viewport resizes", "[Page][Window]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();

    uint32 resizes = 0;
    auto listener = window.AddEventListener(u8"resize",
                                            [&](Event &event)
                                            {
                                              CHECK(event.IsTrusted());
                                              CHECK_FALSE(event.Bubbles());
                                              ++resizes;
                                            });

    // A rendering update without a change fires nothing.
    fixture.Frame->UpdateRendering();
    CHECK(resizes == 0);

    fixture.Frame->View()->Resize({300, 100});
    fixture.Frame->UpdateRendering();
    CHECK(resizes == 1);

    // Once per change.
    fixture.Frame->UpdateRendering();
    CHECK(resizes == 1);

    // A zoom changes the viewport too.
    fixture.Frame->SetPageZoomFactor(2);
    fixture.Frame->UpdateRendering();
    CHECK(resizes == 2);
  }

  TEST_CASE("A window asks its host to focus it", "[Page][Window][html:dom-window-focus]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();

    window.Focus();
    CHECK(fixture.FocusRequests == 1);
  }

  TEST_CASE("A window's named objects",
            "[Page][Window][html:named-access-on-the-window-object][html:dom-window-nameditem-filter]"
            "[html:document-tree-child-navigable-target-name-property-set]")
  {
    WindowFixture fixture(u8"<!DOCTYPE html>"
                          u8"<img name=picture>"
                          u8"<div id=box></div>"
                          u8"<form name=order id=orderform></form>"
                          u8"<p id=twice></p><span id=twice></span>"
                          u8"<span name=label></span>"
                          u8"<img name=''>");
    auto &window = fixture.Window();

    auto single = [&](DOMStringView name) -> Element *
    {
      auto item = window.NamedItem(name);
      if (!item || !std::holds_alternative<RefPtr<Element>>(*item))
      {
        return nullptr;
      }
      return std::get<RefPtr<Element>>(*item).get();
    };

    CHECK(single(u8"picture") == fixture.Document->QuerySelector(u8"img").Value().get());
    CHECK(single(u8"box") == &fixture.ById(u8"box"));
    // A form by its name and by its ID.
    CHECK(single(u8"order") == &fixture.ById(u8"orderform"));
    CHECK(single(u8"orderform") == &fixture.ById(u8"orderform"));
    // Only embed, form, img and object elements are named by their name.
    CHECK_FALSE(window.NamedItem(u8"label"));
    CHECK_FALSE(window.NamedItem(u8""));
    CHECK_FALSE(window.NamedItem(u8"missing"));

    // Several are a live collection.
    auto twice = window.NamedItem(u8"twice");
    REQUIRE(twice);
    REQUIRE(std::holds_alternative<RefPtr<HTMLCollection>>(*twice));
    auto collection = std::get<RefPtr<HTMLCollection>>(*twice);
    CHECK(collection->Length() == 2);
    REQUIRE_FALSE(fixture.ById(u8"box").SetAttribute(u8"id", u8"twice").HasException());
    CHECK(collection->Length() == 3);

    CHECK(window.SupportedPropertyNames()
          == List<DOMString> {u8"picture", u8"twice", u8"order", u8"orderform"});
  }

  TEST_CASE("A window's viewport, scrolling and zoom", "[Page][Window]")
  {
    WindowFixture fixture(u8"<!DOCTYPE html><style>body { margin: 0 } div { width: 1000px; height: 1000px }"
                          u8"</style><div></div>");
    auto &window = fixture.Window();
    auto &view = *fixture.Frame->View();

    // The viewport with its scrollbars.
    CHECK(window.InnerWidth() == 200);
    CHECK(window.InnerHeight() == 100);
    CHECK(window.ScrollX() == 0);
    CHECK(window.ScrollY() == 0);

    window.ScrollTo(30, 40);
    CHECK(window.ScrollX() == 30);
    CHECK(window.ScrollY() == 40);
    CHECK(window.PageXOffset() == 30);
    CHECK(window.PageYOffset() == 40);

    window.ScrollBy(5, 10);
    CHECK(window.ScrollX() == 35);
    CHECK(window.ScrollY() == 50);

    // A missing coordinate stays where it is; a non-finite one is 0.
    ScrollToOptions onlyTop;
    onlyTop.Top = 0;
    window.Scroll(onlyTop);
    CHECK(window.ScrollX() == 35);
    CHECK(window.ScrollY() == 0);
    window.ScrollTo(std::numeric_limits<double>::infinity(), 20);
    CHECK(window.ScrollX() == 0);
    CHECK(window.ScrollY() == 20);

    // The document element scrolls the viewport through the window.
    fixture.Document->DocumentElement()->ScrollTo(ScrollToOptions(0, 60));
    CHECK(window.ScrollY() == 60);

    // Zoom maps CSS pixels to layout units.
    fixture.Frame->SetPageZoomFactor(2);
    (void)fixture.Document->UpdateLayoutIgnorePendingStylesheets();
    window.ScrollTo(0, 50);
    CHECK(view.ContentsScrollPosition().Y() == 100);
    CHECK(window.ScrollY() == 50);
    CHECK(window.InnerWidth() == 100);
    CHECK(window.DevicePixelRatio() == 2);

    fixture.Frame->SetDeviceScaleFactor(1.5f);
    CHECK(window.DevicePixelRatio() == 3);
  }

  TEST_CASE("A right-to-left window's scrolling", "[Page][Window]")
  {
    // As Chrome: the viewport starts at its right edge, which is 0, and scrolls left to negative positions;
    // its vertical scrollbar stays on the right.
    WindowFixture fixture(
      u8"<!DOCTYPE html><html dir=rtl><style>body { margin: 0 }"
      u8" #wide { width: 1000px; height: 1000px } #start { width: 10px; height: 10px }</style>"
      u8"<div id=wide><div id=start></div></div></html>");
    auto &window = fixture.Window();
    auto &root = *fixture.Document->DocumentElement();
    auto start = [&]
    {
      return fixture.Document->GetElementById(u8"start")->GetBoundingClientRect()->X();
    };

    auto clientWidth = root.ClientWidth();
    CHECK(clientWidth < 200);
    CHECK(window.ScrollX() == 0);
    CHECK(start() == clientWidth - 10);
    CHECK(root.ScrollWidth() == 1'000);

    window.ScrollTo(-500, 0);
    CHECK(window.ScrollX() == -500);
    CHECK(start() == clientWidth - 10 + 500);
    window.ScrollTo(-100'000, 0);
    CHECK(window.ScrollX() == -(1'000 - clientWidth));
    CHECK(root.ScrollLeft() == -(1'000 - clientWidth));
    CHECK(start() == 990);
    window.ScrollTo(100, 0);
    CHECK(window.ScrollX() == 0);
  }

  TEST_CASE("A window's visual viewport is its layout viewport", "[Page][Window]")
  {
    WindowFixture fixture(u8"<!DOCTYPE html><style>body { margin: 0 } div { width: 1000px; height: 1000px }"
                          u8"</style><div></div>");
    auto &window = fixture.Window();
    auto &visualViewport = window.GetVisualViewport();

    CHECK(visualViewport.Scale() == 1);
    CHECK(visualViewport.OffsetLeft() == 0);
    CHECK(visualViewport.OffsetTop() == 0);
    // Less the scrollbars.
    CHECK(visualViewport.Width() < 200);
    CHECK(visualViewport.Height() < 100);

    window.ScrollTo(0, 40);
    CHECK(visualViewport.PageTop() == 40);
    CHECK(visualViewport.OffsetTop() == 0);

    uint32 resizes = 0;
    uint32 scrolls = 0;
    auto resizeListener = visualViewport.AddEventListener(u8"resize", [&](Event &) { ++resizes; });
    auto scrollListener = visualViewport.AddEventListener(u8"scroll", [&](Event &) { ++scrolls; });

    fixture.Frame->View()->Resize({300, 150});
    fixture.Frame->UpdateRendering();
    CHECK(resizes == 1);
    // Scrolling moves the layout viewport, and the visual viewport with it.
    window.ScrollTo(0, 80);
    fixture.Frame->UpdateRendering();
    CHECK(scrolls == 0);
  }

  TEST_CASE("A window's selection and computed styles", "[Page][Window]")
  {
    WindowFixture fixture(u8"<!DOCTYPE html><p id=text style='color: rgb(1, 2, 3)'>Text</p>");
    auto &window = fixture.Window();

    REQUIRE(window.GetSelection());
    CHECK(window.GetSelection() == fixture.Document->GetSelection());

    auto style = window.GetComputedStyle(fixture.ById(u8"text"));
    CHECK(style->GetPropertyValue(u8"color") == u8"rgb(1, 2, 3)");

    fixture.Frame->SetDocument(nullptr);
    CHECK(window.GetSelection() == nullptr);
  }

  namespace
  {
    using namespace std::chrono_literals;

    /// @brief Runs the event loop with the host's time moved on by @p delta.
    void Advance(MonotonicTime::duration delta) noexcept
    {
      auto &eventLoop = html::EventLoop::Current();
      eventLoop.Run(eventLoop.Now() + delta);
    }
  }

  TEST_CASE("A window's microtasks run before the next task",
            "[Page][Window][html:microtask-queuing][html:dom-queuemicrotask]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();
    html::EventLoop::Current().RunUntilIdle();

    List<std::string> order;
    auto timer = fixture.Document->StartTimer(0ms, [&] { order.emplace_back("task"); });
    window.QueueMicrotask([&] { order.emplace_back("microtask"); });
    Advance(0ms);
    CHECK(order == List<std::string> {"microtask", "task"});
  }

  TEST_CASE("A window's animation frame callbacks run in the rendering update",
            "[Page][Window][html:animation-frames][html:dom-animationframeprovider-requestanimationframe]"
            "[html:animationframeprovider-cancelanimationframe][html:run-the-animation-frame-callbacks]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();
    auto &page = *fixture.Frame;

    List<double> timestamps;
    int32 nested = 0;
    auto first = window.RequestAnimationFrame(
      [&](double timestamp)
      {
        timestamps.push_back(timestamp);
        // Callbacks requested during a frame run in the next one.
        (void)window.RequestAnimationFrame([&](double) { ++nested; });
      });
    auto second = window.RequestAnimationFrame([&](double timestamp) { timestamps.push_back(timestamp); });
    auto cancelled = window.RequestAnimationFrame([&](double) { timestamps.push_back(-1); });
    CHECK(first > 0);
    CHECK(second == first + 1);
    window.CancelAnimationFrame(cancelled);
    CHECK(page.IsRenderingUpdateScheduled());

    Advance(16ms);
    page.UpdateRendering();
    REQUIRE(timestamps.size() == 2);
    // Both see the frame's frozen timestamp: milliseconds since the window's time origin.
    CHECK(timestamps[0] == timestamps[1]);
    CHECK(timestamps[0] >= 16);
    CHECK(nested == 0);
    CHECK(page.IsRenderingUpdateScheduled());

    page.UpdateRendering();
    CHECK(nested == 1);
    CHECK(timestamps.size() == 2);
  }

  TEST_CASE("A window's user activation",
            "[Page][Window][html:user-activation-data-model]"
            "[html:activation-expiry][html:activation-notification]"
            "[html:activation-triggering-input-event][html:consume-user-activation]"
            "[html:consume-history-action-user-activation]")
  {
    WindowFixture fixture(
      u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }</style><div></div>");
    auto &window = fixture.Window();
    auto &handler = fixture.Frame->GetEventHandler();

    CHECK_FALSE(window.HasStickyActivation());
    CHECK_FALSE(window.HasTransientActivation());

    SECTION("a press activates the window, transiently for 5s")
    {
      PlatformMouseEvent press(PointD(10, 10), PointD(10, 10), MouseButton::Left,
                               PlatformEvent::Type::MousePressed, 1, {}, MonotonicTime::Now());
      (void)handler.HandleMousePressEvent(press);
      CHECK(window.HasStickyActivation());
      CHECK(window.HasTransientActivation());

      Advance(4s);
      CHECK(window.HasTransientActivation());
      Advance(1s);
      CHECK_FALSE(window.HasTransientActivation());
      CHECK(window.HasStickyActivation());
    }

    SECTION("a key press activates it, but not Escape")
    {
      auto key = [](utf8_string name)
      {
        return PlatformKeyboardEvent(PlatformEvent::Type::KeyDown, u8"", u8"", krys::move(name), u8"", 0,
                                     false, false, false, {}, MonotonicTime::Now());
      };
      (void)handler.KeyEvent(key(u8"Escape"));
      CHECK_FALSE(window.HasStickyActivation());
      (void)handler.KeyEvent(key(u8"a"));
      CHECK(window.HasTransientActivation());
    }

    SECTION("consuming it ends the transient activation, keeping the sticky one")
    {
      window.NotifyActivated();
      CHECK(window.ConsumeTransientActivation());
      CHECK_FALSE(window.HasTransientActivation());
      CHECK(window.HasStickyActivation());
      CHECK_FALSE(window.ConsumeTransientActivation());

      // Close watchers consume the history-action activation.
      CHECK(window.ConsumeHistoryActionUserActivation());
      CHECK_FALSE(window.HasHistoryActionActivation());
      CHECK_FALSE(window.ConsumeHistoryActionUserActivation());
    }
  }

  TEST_CASE("Event handler attributes",
            "[Page][Window][EventHandlers][html:event-handler-attributes]"
            "[html:the-event-handler-processing-algorithm][html:event-handler-idl-attributes]"
            "[html:window-reflecting-body-element-event-handler-set]")
  {
    WindowFixture fixture(u8"<!DOCTYPE html><body><button id=button>Go</button></body>");
    auto &window = fixture.Window();
    auto &button = Downcast<HTMLElement>(fixture.ById(u8"button"));

    List<std::string> order;
    CHECK(button.OnClick() == nullptr);

    SECTION("a handler keeps its listener's place as its value changes")
    {
      auto before = button.AddEventListener(u8"click", [&](Event &) { order.emplace_back("before"); });
      button.SetOnClick([&](Event &) { order.emplace_back("handler"); });
      auto after = button.AddEventListener(u8"click", [&](Event &) { order.emplace_back("after"); });
      REQUIRE(button.OnClick() != nullptr);

      button.DispatchSimulatedClick(nullptr);
      CHECK(order == List<std::string> {"before", "handler", "after"});

      order.clear();
      button.SetOnClick([&](Event &) { order.emplace_back("replaced"); });
      button.DispatchSimulatedClick(nullptr);
      CHECK(order == List<std::string> {"before", "replaced", "after"});

      // Unsetting removes its listener; setting it again adds one at the end.
      order.clear();
      button.SetOnClick({});
      CHECK(button.OnClick() == nullptr);
      button.DispatchSimulatedClick(nullptr);
      CHECK(order == List<std::string> {"before", "after"});

      order.clear();
      button.SetOnClick([&](Event &) { order.emplace_back("again"); });
      button.DispatchSimulatedClick(nullptr);
      CHECK(order == List<std::string> {"before", "after", "again"});
    }

    SECTION("a handler cancels by preventing the default")
    {
      button.SetOnClick([](Event &event) { event.PreventDefault(); });
      auto click = Event::Create(event_names::Click, true, true);
      CHECK_FALSE(dom::EventDispatcher::DispatchToTarget(*click, &button));
    }

    SECTION("documents and windows have handlers too")
    {
      fixture.Document->SetOnClick([&](Event &) { order.emplace_back("document"); });
      window.SetOnClick([&](Event &) { order.emplace_back("window"); });
      fixture.Document->SetOnCopy([&](Event &) {});
      CHECK(fixture.Document->OnCopy() != nullptr);
      button.DispatchSimulatedClick(nullptr);
      CHECK(order == List<std::string> {"document", "window"});
    }

    SECTION("pointer and touch events have handlers")
    {
      uint32 calls = 0;
      auto count = [&](Event &)
      {
        ++calls;
      };
      button.SetOnGotPointerCapture(count);
      button.SetOnLostPointerCapture(count);
      button.SetOnPointerCancel(count);
      button.SetOnPointerDown(count);
      button.SetOnPointerEnter(count);
      button.SetOnPointerLeave(count);
      button.SetOnPointerMove(count);
      button.SetOnPointerOut(count);
      button.SetOnPointerOver(count);
      button.SetOnPointerRawUpdate(count);
      button.SetOnPointerUp(count);
      button.SetOnTouchCancel(count);
      button.SetOnTouchEnd(count);
      button.SetOnTouchMove(count);
      button.SetOnTouchStart(count);
      for (DOMStringAtom type :
           {event_names::GotPointerCapture, event_names::LostPointerCapture, event_names::PointerCancel,
            event_names::PointerDown, event_names::PointerEnter, event_names::PointerLeave,
            event_names::PointerMove, event_names::PointerOut, event_names::PointerOver, u8"pointerrawupdate",
            event_names::PointerUp, event_names::TouchCancel, event_names::TouchEnd, event_names::TouchMove,
            event_names::TouchStart})
      {
        dom::EventDispatcher::FireEvent(type, button);
      }
      CHECK(calls == 15);
    }

    SECTION("the document's own handlers")
    {
      uint32 calls = 0;
      auto count = [&](Event &)
      {
        ++calls;
      };
      fixture.Document->SetOnReadyStateChange(count);
      fixture.Document->SetOnPointerLockChange(count);
      fixture.Document->SetOnPointerLockError(count);
      for (auto type :
           {event_names::ReadyStateChange, event_names::PointerLockChange, event_names::PointerLockError})
      {
        dom::EventDispatcher::FireEvent(type, *fixture.Document);
      }
      CHECK(calls == 3);
    }

    SECTION("an svg element's handlers are its own")
    {
      auto svg = fixture.Document->CreateElementNS(infra::namespaces::SVG, u8"svg").ReleaseValue();
      auto &svgElement = Downcast<SVGElement>(*svg);
      svgElement.SetOnResize([](Event &) {});
      CHECK(svgElement.OnResize() != nullptr);
      CHECK(window.OnResize() == nullptr);
    }

    SECTION("the body's window handlers are the window's")
    {
      auto &body = *fixture.Document->Body();
      body.SetOnResize([&](Event &) { order.emplace_back("resize"); });
      body.SetOnPageShow([&](Event &) {});
      CHECK(window.OnResize() == body.OnResize());
      CHECK(window.OnResize() != nullptr);
      CHECK(window.OnPageShow() != nullptr);
      // Other handlers are the body's own.
      body.SetOnClick([&](Event &) {});
      CHECK(window.OnClick() == nullptr);

      fixture.Frame->View()->Resize({300, 150});
      fixture.Frame->UpdateRendering();
      CHECK(order == List<std::string> {"resize"});

      // Without a browsing context, they're nowhere.
      fixture.Frame->SetDocument(nullptr);
      CHECK(body.OnResize() == nullptr);
      body.SetOnLoad([](Event &) {});
      CHECK(window.OnLoad() == nullptr);
    }
  }
}
