#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSOM/CSSStyleDeclaration.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Scheduling/IdleDeadline.hpp"
#include "Krystal.Booey/HTML/Attributes/EventHandlerAttributes.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/DocumentParser.hpp"
#include "Krystal.Booey/Page/DOMSelection.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/VisualViewport.hpp"
#include "Krystal.Booey/Platform/Events/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/Events/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Counts the page's requests to be brought to the front.
    struct WindowChromeClient final : ChromeClient
    {
      uint32 &FocusRequests;

      explicit WindowChromeClient(uint32 &focusRequests) noexcept : FocusRequests(focusRequests)
      {
      }

      void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept override
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
      Ref<Page> TestPage {CreatePage()};
      RefPtr<dom::HTMLDocument> Document;

      explicit WindowFixture(utf8_string &&markup = u8"<!DOCTYPE html>") noexcept
      {
        auto &frame = TestPage->MainFrame();
        frame.CreateView({200, 100});
        Document = CreateRef<dom::HTMLDocument>();
        frame.SetDocument(ShareRefPtr<dom::Document>(Document.get()));
        auto parser = CreateRef<html::DocumentParser>(*Document);
        parser->InputStream().Append(krys::move(markup), IsEOF(true));
        parser->PumpTokenizer();
        Document->StyleScope().FlushPendingUpdate();
        TestPage->UpdateRendering();
      }

      KRYS_NODISCARD Ref<Page> CreatePage() noexcept
      {
        return Page::Create(CreateUnique<WindowChromeClient>(FocusRequests), nullptr,
                            CreateUnique<RecordingFrameLoaderClient>());
      }

      KRYS_NODISCARD LocalDOMWindow &Window() const noexcept
      {
        REQUIRE(Document->Window());
        return *Document->Window();
      }

      KRYS_NODISCARD dom::Element &ById(dom::DOMStringView id) noexcept
      {
        auto element = Document->GetElementById(id);
        REQUIRE(element);
        return *element;
      }
    };
  }

  TEST_CASE("A document gets its window as it becomes a frame's document", "[Page][LocalDOMWindow]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    CHECK(document->Window() == nullptr);
    CHECK(document->DefaultView() == nullptr);

    auto frame = LocalFrame::Create();
    frame->CreateView({200, 100});
    frame->SetDocument(ShareRefPtr<dom::Document>(document.get()));

    auto *window = document->Window();
    REQUIRE(window);
    CHECK(document->DefaultView() == window);
    CHECK(frame->Window() == window);
    CHECK(window->Document() == document.get());
    CHECK(window->Frame() == frame.get());
    CHECK(window->IsCurrentlyDisplayedInFrame());
    CHECK(Is<LocalDOMWindow>(static_cast<dom::EventTarget &>(*window)));
    CHECK_FALSE(Is<LocalDOMWindow>(static_cast<dom::EventTarget &>(*document)));

    // The window's custom element registry is the document's.
    CHECK(&window->CustomElements() == document->CustomElementRegistry().get());

    // Leaving the frame, the document keeps its window, which has no browsing context.
    frame->SetDocument(nullptr);
    CHECK(document->Window() == window);
    CHECK(document->DefaultView() == nullptr);
    CHECK(window->Frame() == nullptr);
  }

  TEST_CASE("Events bubble from the document to the window, except load", "[Page][LocalDOMWindow]")
  {
    WindowFixture fixture(u8"<!DOCTYPE html><button id=button>Go</button>");
    auto &window = fixture.Window();
    auto &button = fixture.ById(u8"button");

    List<std::string> order;
    auto windowCapture = window.AddEventListener(
      u8"click",
      [&](dom::Event &event)
      {
        CHECK(event.CurrentTarget().get() == &window);
        CHECK(event.Target().get() == &button);
        order.push_back("window capture");
      },
      true);
    auto documentCapture = fixture.Document->AddEventListener(
      u8"click", [&](dom::Event &) { order.push_back("document capture"); }, true);
    auto documentBubble =
      fixture.Document->AddEventListener(u8"click", [&](dom::Event &) { order.push_back("document"); });
    auto windowBubble = window.AddEventListener(u8"click", [&](dom::Event &) { order.push_back("window"); });

    button.DispatchSimulatedClick(nullptr);
    CHECK(order == List<std::string> {"window capture", "document capture", "document", "window"});

    SECTION("load stops at the document")
    {
      bool documentHeard = false;
      bool windowHeard = false;
      auto documentLoad =
        fixture.Document->AddEventListener(u8"load", [&](dom::Event &) { documentHeard = true; });
      auto windowLoad = window.AddEventListener(u8"load", [&](dom::Event &) { windowHeard = true; });

      auto load = dom::Event::Create(dom::DOMStringAtom(EventNames::Load), true, false);
      (void)dom::EventDispatcher::DispatchToTarget(*load, &button);
      CHECK(documentHeard);
      CHECK_FALSE(windowHeard);
    }

    // The legacy target override flag of dispatch.
    SECTION("load at the window with the legacy target override targets the document")
    {
      bool windowHeard = false;
      auto windowLoad = window.AddEventListener(u8"load",
                                                [&](dom::Event &event)
                                                {
                                                  windowHeard = true;
                                                  CHECK(event.CurrentTarget().get() == &window);
                                                  CHECK(event.Target().get() == fixture.Document.get());
                                                });
      (void)dom::EventDispatcher::FireEvent<dom::Event>(dom::DOMStringAtom(EventNames::Load), window, {},
                                                        true);
      CHECK(windowHeard);
    }

    SECTION("a document without a browsing context doesn't reach its window, nor run its own listeners")
    {
      // Its window's document isn't fully active, so inner invoke skips the listeners.
      fixture.TestPage->MainFrame().SetDocument(nullptr);
      order.clear();
      auto event = dom::Event::Create(dom::DOMStringAtom(EventNames::Click), true, false);
      (void)dom::EventDispatcher::DispatchToTarget(*event, &button);
      CHECK(order.empty());
    }
  }

  TEST_CASE("resize fires at the window when the viewport resizes", "[Page][LocalDOMWindow]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();

    uint32 resizes = 0;
    auto listener = window.AddEventListener(u8"resize",
                                            [&](dom::Event &event)
                                            {
                                              CHECK(event.IsTrusted());
                                              CHECK_FALSE(event.Bubbles());
                                              ++resizes;
                                            });

    // A rendering update without a change fires nothing.
    fixture.TestPage->UpdateRendering();
    CHECK(resizes == 0);

    fixture.TestPage->MainFrame().View()->Resize({300, 100});
    fixture.TestPage->UpdateRendering();
    CHECK(resizes == 1);

    // Once per change.
    fixture.TestPage->UpdateRendering();
    CHECK(resizes == 1);

    // A zoom changes the viewport too.
    fixture.TestPage->MainFrame().SetPageZoomFactor(2);
    fixture.TestPage->UpdateRendering();
    CHECK(resizes == 2);
  }

  TEST_CASE("A window asks its host to focus it", "[Page][LocalDOMWindow][html:dom-window-focus]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();

    window.Focus();
    CHECK(fixture.FocusRequests == 1);
  }

  TEST_CASE("A window's named objects",
            "[Page][LocalDOMWindow][html:named-access-on-the-window-object][html:dom-window-nameditem-filter]"
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

    auto single = [&](dom::DOMStringView name) -> dom::Element *
    {
      auto item = window.NamedItem(name);
      if (!item || !std::holds_alternative<RefPtr<dom::Element>>(*item))
      {
        return nullptr;
      }
      return std::get<RefPtr<dom::Element>>(*item).get();
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
    REQUIRE(std::holds_alternative<RefPtr<dom::HTMLCollection>>(*twice));
    auto collection = std::get<RefPtr<dom::HTMLCollection>>(*twice);
    CHECK(collection->Length() == 2);
    REQUIRE_FALSE(fixture.ById(u8"box").SetAttribute(u8"id", u8"twice").HasException());
    CHECK(collection->Length() == 3);

    CHECK(window.SupportedPropertyNames()
          == List<dom::DOMString> {u8"picture", u8"twice", u8"order", u8"orderform"});
  }

  TEST_CASE("A window's viewport, scrolling and zoom", "[Page][LocalDOMWindow]")
  {
    WindowFixture fixture(u8"<!DOCTYPE html><style>body { margin: 0 } div { width: 1000px; height: 1000px }"
                          u8"</style><div></div>");
    auto &window = fixture.Window();
    auto &view = *fixture.TestPage->MainFrame().View();

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
    fixture.TestPage->MainFrame().SetPageZoomFactor(2);
    (void)fixture.Document->UpdateLayoutIgnorePendingStylesheets();
    window.ScrollTo(0, 50);
    CHECK(view.ContentsScrollPosition().Y() == 100);
    CHECK(window.ScrollY() == 50);
    CHECK(window.InnerWidth() == 100);
    CHECK(window.DevicePixelRatio() == 2);

    auto environment = fixture.TestPage->MainFrame().GetMediaEnvironment();
    environment.DeviceScaleFactor = 1.5f;
    fixture.TestPage->MainFrame().SetMediaEnvironment(environment);
    CHECK(window.DevicePixelRatio() == 3);
  }

  TEST_CASE("A right-to-left window's scrolling", "[Page][LocalDOMWindow]")
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

  TEST_CASE("A window's visual viewport is its layout viewport", "[Page][LocalDOMWindow]")
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
    auto resizeListener = visualViewport.AddEventListener(u8"resize", [&](dom::Event &) { ++resizes; });
    auto scrollListener = visualViewport.AddEventListener(u8"scroll", [&](dom::Event &) { ++scrolls; });

    fixture.TestPage->MainFrame().View()->Resize({300, 150});
    fixture.TestPage->UpdateRendering();
    CHECK(resizes == 1);
    // Scrolling moves the layout viewport, and the visual viewport with it.
    window.ScrollTo(0, 80);
    fixture.TestPage->UpdateRendering();
    CHECK(scrolls == 0);
  }

  TEST_CASE("A window's selection and computed styles", "[Page][LocalDOMWindow]")
  {
    WindowFixture fixture(u8"<!DOCTYPE html><p id=text style='color: rgb(1, 2, 3)'>Text</p>");
    auto &window = fixture.Window();

    REQUIRE(window.GetSelection());
    CHECK(window.GetSelection() == fixture.Document->GetSelection());

    auto style = window.GetComputedStyle(fixture.ById(u8"text"));
    CHECK(style->GetPropertyValue(u8"color") == u8"rgb(1, 2, 3)");

    fixture.TestPage->MainFrame().SetDocument(nullptr);
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

  TEST_CASE(
    "A window's timers",
    "[Page][LocalDOMWindow][html:timers][html:dom-settimeout][html:dom-setinterval][html:dom-cleartimeout]"
    "[html:dom-clearinterval][html:timer-initialisation-steps][html:run-steps-after-a-timeout]"
    "[html:timer-nesting-level][html:map-of-settimeout-and-setinterval-ids][html:microtask-queuing]"
    "[html:dom-queuemicrotask]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();
    html::EventLoop::Current().RunUntilIdle();

    List<std::string> order;

    SECTION("timeouts run as tasks after their delay, in order")
    {
      auto late = window.SetTimeout([&] { order.push_back("late"); }, 20);
      auto early = window.SetTimeout([&] { order.push_back("early"); }, 10);
      auto now = window.SetTimeout([&] { order.push_back("now"); }, -5);
      CHECK(late > 0);
      CHECK(early > 0);
      CHECK(now > 0);
      CHECK(late != early);
      CHECK(order.empty());

      Advance(0ms);
      CHECK(order == List<std::string> {"now"});
      Advance(10ms);
      CHECK(order == List<std::string> {"now", "early"});
      Advance(10ms);
      CHECK(order == List<std::string> {"now", "early", "late"});
    }

    SECTION("a cleared timeout doesn't run")
    {
      auto id = window.SetTimeout([&] { order.push_back("cleared"); }, 5);
      window.ClearTimeout(id);
      window.ClearTimeout(0);
      window.ClearTimeout(12'345);
      Advance(10ms);
      CHECK(order.empty());
    }

    SECTION("intervals repeat until cleared, even from their own handler")
    {
      int32 id = 0;
      int32 count = 0;
      id = window.SetInterval(
        [&]
        {
          if (++count == 3)
          {
            window.ClearInterval(id);
          }
        },
        10);
      for (int32 i = 0; i < 6; ++i)
      {
        Advance(10ms);
      }
      CHECK(count == 3);
    }

    SECTION("deeply nested timeouts are clamped to 4ms")
    {
      int32 runs = 0;
      Func<void()> nest;
      nest = [&]
      {
        ++runs;
        (void)window.SetTimeout(Func<void()>(nest), 0);
      };
      (void)window.SetTimeout(Func<void()>(nest), 0);

      // Ten levels run at once; the eleventh waits for the minimum interval.
      html::EventLoop::Current().RunUntilIdle();
      CHECK(runs == 10);
      Advance(4ms);
      CHECK(runs == 11);
      Advance(4ms);
      CHECK(runs == 12);
      fixture.TestPage->MainFrame().SetDocument(nullptr);
    }

    SECTION("microtasks run before the next task")
    {
      window.SetTimeout([&] { order.push_back("task"); }, 0);
      window.QueueMicrotask([&] { order.push_back("microtask"); });
      Advance(0ms);
      CHECK(order == List<std::string> {"microtask", "task"});
    }

    SECTION("a document leaving its frame stops its timers")
    {
      (void)window.SetTimeout([&] { order.push_back("timeout"); }, 5);
      (void)window.SetInterval([&] { order.push_back("interval"); }, 5);
      fixture.TestPage->MainFrame().SetDocument(nullptr);
      Advance(10ms);
      CHECK(order.empty());
    }
  }

  TEST_CASE(
    "A window's animation frame callbacks run in the rendering update",
    "[Page][LocalDOMWindow][html:animation-frames][html:dom-animationframeprovider-requestanimationframe]"
    "[html:animationframeprovider-cancelanimationframe][html:run-the-animation-frame-callbacks]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();
    auto &page = *fixture.TestPage;

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

  TEST_CASE("A window's idle callbacks run in idle periods, or after their timeout", "[Page][LocalDOMWindow]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();
    auto &page = *fixture.TestPage;

    double remaining = -1;
    bool timedOut = true;
    int32 calls = 0;
    (void)window.RequestIdleCallback(
      [&](dom::IdleDeadline &deadline)
      {
        ++calls;
        remaining = deadline.TimeRemaining();
        timedOut = deadline.DidTimeoutValue();
      });
    auto cancelled = window.RequestIdleCallback([&](dom::IdleDeadline &) { calls += 100; });
    window.CancelIdleCallback(cancelled);

    // An idle period that's already over runs nothing.
    page.OpportunisticallyRunIdleCallbacks(MonotonicTime::Now());
    CHECK(calls == 0);

    page.OpportunisticallyRunIdleCallbacks(MonotonicTime::Now() + 1s);
    CHECK(calls == 1);
    CHECK(remaining > 0);
    CHECK(remaining <= 1'000);
    CHECK_FALSE(timedOut);

    SECTION("a timeout calls it without an idle period")
    {
      (void)window.RequestIdleCallback(
        [&](dom::IdleDeadline &deadline)
        {
          ++calls;
          remaining = deadline.TimeRemaining();
          timedOut = deadline.DidTimeoutValue();
        },
        {50});
      Advance(40ms);
      CHECK(calls == 1);
      Advance(10ms);
      CHECK(calls == 2);
      CHECK(timedOut);
      CHECK(remaining == 0);
    }
  }

  TEST_CASE("A window's user activation",
            "[Page][LocalDOMWindow][html:user-activation-data-model]"
            "[html:activation-expiry][html:activation-notification]"
            "[html:activation-triggering-input-event][html:consume-user-activation]"
            "[html:consume-history-action-user-activation]")
  {
    WindowFixture fixture(
      u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }</style><div></div>");
    auto &window = fixture.Window();
    auto &handler = fixture.TestPage->MainFrame().GetEventHandler();

    CHECK_FALSE(window.HasStickyActivation());
    CHECK_FALSE(window.HasTransientActivation());

    SECTION("a press activates the window, transiently for 5s")
    {
      PlatformMouseEvent press(DoublePoint(10, 10), DoublePoint(10, 10), MouseButton::Left,
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
        return PlatformKeyboardEvent(PlatformEvent::Type::KeyDown, u8"", u8"", krys::move(name), u8"", u8"",
                                     0, false, false, false, {}, MonotonicTime::Now());
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
            "[Page][LocalDOMWindow][EventHandlers][html:event-handler-attributes]"
            "[html:the-event-handler-processing-algorithm][html:event-handler-idl-attributes]"
            "[html:window-reflecting-body-element-event-handler-set]")
  {
    WindowFixture fixture(u8"<!DOCTYPE html><body><button id=button>Go</button></body>");
    auto &window = fixture.Window();
    auto &button = Downcast<html::HTMLElement>(fixture.ById(u8"button"));

    List<std::string> order;
    CHECK(button.OnClick() == nullptr);

    SECTION("a handler keeps its listener's place as its value changes")
    {
      auto before = button.AddEventListener(u8"click", [&](dom::Event &) { order.push_back("before"); });
      button.SetOnClick([&](dom::Event &) { order.push_back("handler"); });
      auto after = button.AddEventListener(u8"click", [&](dom::Event &) { order.push_back("after"); });
      REQUIRE(button.OnClick() != nullptr);

      button.DispatchSimulatedClick(nullptr);
      CHECK(order == List<std::string> {"before", "handler", "after"});

      order.clear();
      button.SetOnClick([&](dom::Event &) { order.push_back("replaced"); });
      button.DispatchSimulatedClick(nullptr);
      CHECK(order == List<std::string> {"before", "replaced", "after"});

      // Unsetting removes its listener; setting it again adds one at the end.
      order.clear();
      button.SetOnClick({});
      CHECK(button.OnClick() == nullptr);
      button.DispatchSimulatedClick(nullptr);
      CHECK(order == List<std::string> {"before", "after"});

      order.clear();
      button.SetOnClick([&](dom::Event &) { order.push_back("again"); });
      button.DispatchSimulatedClick(nullptr);
      CHECK(order == List<std::string> {"before", "after", "again"});
    }

    SECTION("a handler cancels by preventing the default")
    {
      button.SetOnClick([](dom::Event &event) { event.PreventDefault(); });
      auto click = dom::Event::Create(dom::DOMStringAtom(EventNames::Click), true, true);
      CHECK_FALSE(dom::EventDispatcher::DispatchToTarget(*click, &button));
    }

    SECTION("documents and windows have handlers too")
    {
      fixture.Document->SetOnClick([&](dom::Event &) { order.push_back("document"); });
      window.SetOnClick([&](dom::Event &) { order.push_back("window"); });
      fixture.Document->SetOnCopy([&](dom::Event &) {});
      CHECK(fixture.Document->OnCopy() != nullptr);
      button.DispatchSimulatedClick(nullptr);
      CHECK(order == List<std::string> {"document", "window"});
    }

    SECTION("the body's window handlers are the window's")
    {
      auto &body = *fixture.Document->Body();
      body.SetOnResize([&](dom::Event &) { order.push_back("resize"); });
      body.SetOnPageShow([&](dom::Event &) {});
      CHECK(window.OnResize() == body.OnResize());
      CHECK(window.OnResize() != nullptr);
      CHECK(window.OnPageShow() != nullptr);
      // Other handlers are the body's own.
      body.SetOnClick([&](dom::Event &) {});
      CHECK(window.OnClick() == nullptr);

      fixture.TestPage->MainFrame().View()->Resize({300, 150});
      fixture.TestPage->UpdateRendering();
      CHECK(order == List<std::string> {"resize"});

      // Without a browsing context, they're nowhere.
      fixture.TestPage->MainFrame().SetDocument(nullptr);
      CHECK(body.OnResize() == nullptr);
      body.SetOnLoad([](dom::Event &) {});
      CHECK(window.OnLoad() == nullptr);
    }
  }
}
