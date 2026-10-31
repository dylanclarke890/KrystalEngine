#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Loader/NavigationTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/CSSStyleDeclaration.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/IdleDeadline.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/ErrorEvent.hpp"
#include "Krystal.Booey/HTML/EventHandlerAttributes.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/MessageEvent.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Page/BarProp.hpp"
#include "Krystal.Booey/Page/DOMSelection.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/External.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Navigator.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Screen.hpp"
#include "Krystal.Booey/Page/UserActivation.hpp"
#include "Krystal.Booey/Page/VisualViewport.hpp"
#include "Krystal.Booey/Platform/Language.hpp"
#include "Krystal.Booey/Platform/PlatformKeyboardEvent.hpp"
#include "Krystal.Booey/Platform/PlatformMouseEvent.hpp"
#include "Krystal.Booey/Plugins/DOMMimeTypeArray.hpp"
#include "Krystal.Booey/Plugins/DOMPluginArray.hpp"
#include "Krystal.Booey/Storage/Storage.hpp"
#include "Krystal.Booey/Storage/StorageEvent.hpp"
#include "Krystal.Booey/Style/Scope.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Counts what the page asks of its host.
    struct WindowChromeClient final : ChromeClient
    {
      uint32 &FocusRequests;
      uint32 &CloseRequests;

      WindowChromeClient(uint32 &focusRequests, uint32 &closeRequests) noexcept
          : FocusRequests(focusRequests), CloseRequests(closeRequests)
      {
      }

      void AttachRootGraphicsLayer(LocalFrame &, GraphicsLayer *) noexcept override
      {
      }

      void Focus() noexcept override
      {
        ++FocusRequests;
      }

      void CloseWindow() noexcept override
      {
        ++CloseRequests;
      }

      FloatRect Rect {100, 50, 640, 480};
      List<std::string> Dialogs;
      bool ConfirmAnswer {true};
      Maybe<dom::DOMString> PromptAnswer;
      uint32 PrintRequests {0};
      List<std::string> ConsoleMessages;
      List<std::string> ProtocolHandlers;

      void RegisterProtocolHandler(const dom::DOMString &scheme, const dom::DOMString &url) noexcept override
      {
        ProtocolHandlers.push_back("+" + Str(scheme) + " " + Str(url));
      }

      void UnregisterProtocolHandler(const dom::DOMString &scheme,
                                     const dom::DOMString &url) noexcept override
      {
        ProtocolHandlers.push_back("-" + Str(scheme) + " " + Str(url));
      }

      void RunJavaScriptAlert(LocalFrame &, const dom::DOMString &message) noexcept override
      {
        Dialogs.push_back("alert:" + Str(message));
      }

      KRYS_NODISCARD bool RunJavaScriptConfirm(LocalFrame &, const dom::DOMString &message) noexcept override
      {
        Dialogs.push_back("confirm:" + Str(message));
        return ConfirmAnswer;
      }

      KRYS_NODISCARD Maybe<dom::DOMString>
        RunJavaScriptPrompt(LocalFrame &, const dom::DOMString &message,
                            const dom::DOMString &defaultValue) noexcept override
      {
        Dialogs.push_back("prompt:" + Str(message) + ":" + Str(defaultValue));
        return PromptAnswer;
      }

      void Print(LocalFrame &) noexcept override
      {
        ++PrintRequests;
      }

      void AddMessageToConsole(MessageSource, MessageLevel level, const dom::DOMString &message, uint32,
                               uint32, const dom::DOMString &) noexcept override
      {
        ConsoleMessages.push_back((level == MessageLevel::Error ? "error:" : "other:") + Str(message));
      }

      KRYS_NODISCARD FloatRect WindowRect() const noexcept override
      {
        return Rect;
      }

      void SetWindowRect(const FloatRect &rect) noexcept override
      {
        Rect = rect;
      }
    };

    /// @brief A page with a 200x100 view, and @p markup parsed into its main frame's document.
    struct WindowFixture
    {
      uint32 FocusRequests {0};
      uint32 CloseRequests {0};
      WindowChromeClient *Chrome {nullptr};
      RecordingFrameLoaderClient *Loader {nullptr};
      Ref<Page> TestPage {CreatePage()};
      RefPtr<dom::HTMLDocument> Document;

      explicit WindowFixture(utf8_string &&markup = u8"<!DOCTYPE html>") noexcept
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

      KRYS_NODISCARD Ref<Page> CreatePage() noexcept
      {
        auto chrome = CreateUnique<WindowChromeClient>(FocusRequests, CloseRequests);
        Chrome = chrome.get();
        auto loader = CreateUnique<RecordingFrameLoaderClient>();
        Loader = loader.get();
        return Page::Create(krys::move(chrome), nullptr, krys::move(loader));
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

    // The browsing context members: the window is the only one in its tree.
    CHECK(&window->Window() == window);
    CHECK(&window->Self() == window);
    CHECK(&window->Frames() == window);
    CHECK(window->Top() == window);
    CHECK(window->Parent() == window);
    CHECK(window->Opener() == nullptr);
    CHECK(window->FrameElement() == nullptr);
    CHECK(window->Length() == 0);
    CHECK_FALSE(window->Closed());

    // The window's custom element registry is the document's.
    CHECK(&window->CustomElements() == document->CustomElementRegistry().get());

    // The bars are visible, as no window is a popup; status is only kept.
    CHECK(window->Locationbar().Visible());
    CHECK(window->Menubar().Visible());
    CHECK(window->Personalbar().Visible());
    CHECK(window->Scrollbars().Visible());
    CHECK(window->Statusbar().Visible());
    CHECK(window->Toolbar().Visible());
    CHECK(&window->Toolbar() == &window->Toolbar());
    window->SetStatus(u8"Ready");
    CHECK(window->Status() == u8"Ready");
    CHECK(window->GetExternal().IsSearchProviderInstalled() == 0);
    CHECK_FALSE(window->OriginAgentCluster());

    // Leaving the frame, the document keeps its window, which has no browsing context.
    frame->SetDocument(nullptr);
    CHECK(document->Window() == window);
    CHECK(document->DefaultView() == nullptr);
    CHECK(window->Frame() == nullptr);
    CHECK(window->Closed());
    CHECK(window->Top() == nullptr);
    CHECK(window->Parent() == nullptr);
  }

  TEST_CASE("A window's name is its frame's, which links can target", "[Page][LocalDOMWindow]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();
    auto &loader = fixture.TestPage->MainFrame().Loader();

    CHECK(window.Name().empty());
    CHECK(loader.FindFrameForNavigation(u8"main") == nullptr);

    window.SetName(u8"main");
    CHECK(window.Name() == u8"main");
    CHECK(fixture.TestPage->MainFrame().SpecifiedName() == u8"main");
    CHECK(loader.FindFrameForNavigation(u8"main") == &fixture.TestPage->MainFrame());
    // Names are matched exactly.
    CHECK(loader.FindFrameForNavigation(u8"Main") == nullptr);
    CHECK(loader.FindFrameForNavigation(u8"_blank") == nullptr);
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
      (void)dom::EventDispatcher::FireEvent<dom::Event>(dom::DOMStringAtom(EventNames::Load), window, nullptr,
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

  TEST_CASE("Window.event: the window's current event, the event its listeners handle",
            "[Page][LocalDOMWindow][dom:dom-window-event][dom:window-current-event]")
  {
    WindowFixture fixture(u8"<!DOCTYPE html><div id=host></div><p id=text>Text</p>");
    auto &window = fixture.Window();
    CHECK(window.CurrentEvent() == nullptr);

    dom::Event *inText = nullptr;
    dom::Event *inWindow = nullptr;
    auto textListener = fixture.ById(u8"text").AddEventListener(u8"click", [&](dom::Event &)
                                                                { inText = window.CurrentEvent(); });
    auto windowListener =
      window.AddEventListener(u8"click", [&](dom::Event &) { inWindow = window.CurrentEvent(); });

    fixture.ById(u8"text").DispatchSimulatedClick(nullptr);
    CHECK(inText != nullptr);
    CHECK(inWindow == inText);
    CHECK(window.CurrentEvent() == nullptr);

    SECTION("but not for listeners in shadow trees")
    {
      auto shadow = fixture.ById(u8"host").AttachShadow({}).ReleaseValue();
      auto inner = fixture.Document->CreateElement(u8"span").ReleaseValue();
      (void)shadow->AppendChild(*inner);

      bool shadowHeard = false;
      dom::Event *inShadow = nullptr;
      auto innerListener = inner->AddEventListener(u8"click",
                                                   [&](dom::Event &)
                                                   {
                                                     shadowHeard = true;
                                                     inShadow = window.CurrentEvent();
                                                   });
      inWindow = nullptr;

      inner->DispatchSimulatedClick(nullptr);
      CHECK(shadowHeard);
      CHECK(inShadow == nullptr);
      CHECK(inWindow != nullptr);
    }

    SECTION("and an event dispatched from a listener is current only while its own listeners run")
    {
      auto nested = dom::Event::Create(dom::DOMStringAtom(u8"nested"), false, false);
      dom::Event *inNested = nullptr;
      dom::Event *afterNested = nullptr;
      auto nestedListener = fixture.ById(u8"text").AddEventListener(u8"nested", [&](dom::Event &)
                                                                    { inNested = window.CurrentEvent(); });
      auto outerListener =
        fixture.ById(u8"host").AddEventListener(u8"click",
                                                [&](dom::Event &)
                                                {
                                                  (void)fixture.ById(u8"text").DispatchEvent(*nested);
                                                  afterNested = window.CurrentEvent();
                                                });

      fixture.ById(u8"host").DispatchSimulatedClick(nullptr);
      CHECK(inNested == nested.get());
      CHECK(afterNested != nullptr);
      CHECK(afterNested != nested.get());
      CHECK(window.CurrentEvent() == nullptr);
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

  TEST_CASE("A window asks its host to focus and close it", "[Page][LocalDOMWindow]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();

    window.Focus();
    CHECK(fixture.FocusRequests == 1);
    window.Blur();
    CHECK(fixture.FocusRequests == 1);

    window.Close();
    CHECK(window.Closed());
    // Closing is a task after the request.
    CHECK(fixture.CloseRequests == 0);
    html::EventLoop::Current().RunUntilIdle();
    CHECK(fixture.CloseRequests == 1);

    // A page that's closing doesn't ask again.
    window.Close();
    html::EventLoop::Current().RunUntilIdle();
    CHECK(fixture.CloseRequests == 1);
  }

  TEST_CASE("A window's named objects", "[Page][LocalDOMWindow]")
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

  TEST_CASE("A window's screen and the host's window", "[Page][LocalDOMWindow]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();

    auto environment = fixture.TestPage->MainFrame().GetMediaEnvironment();
    environment.ScreenSize = FloatSize(1'920, 1'080);
    fixture.TestPage->MainFrame().SetMediaEnvironment(environment);

    auto &screen = window.GetScreen();
    CHECK(&screen == &window.GetScreen());
    CHECK(screen.Width() == 1'920);
    CHECK(screen.Height() == 1'080);
    // The whole screen is available unless the host says otherwise.
    CHECK(screen.AvailWidth() == 1'920);
    CHECK(screen.AvailHeight() == 1'080);
    CHECK(screen.ColorDepth() == 24);
    CHECK(screen.PixelDepth() == 24);

    environment.ScreenAvailableRect = FloatRect(0, 0, 1'920, 1'040);
    fixture.TestPage->MainFrame().SetMediaEnvironment(environment);
    CHECK(screen.AvailHeight() == 1'040);

    CHECK(window.ScreenX() == 100);
    CHECK(window.ScreenY() == 50);
    CHECK(window.ScreenLeft() == 100);
    CHECK(window.ScreenTop() == 50);
    CHECK(window.OuterWidth() == 640);
    CHECK(window.OuterHeight() == 480);

    window.MoveTo(200, 300);
    CHECK(fixture.Chrome->Rect == FloatRect(200, 300, 640, 480));
    window.MoveBy(-50, 10);
    CHECK(fixture.Chrome->Rect == FloatRect(150, 310, 640, 480));
    window.ResizeTo(800, 600);
    CHECK(fixture.Chrome->Rect == FloatRect(150, 310, 800, 600));
    window.ResizeBy(-10, -20);
    CHECK(fixture.Chrome->Rect == FloatRect(150, 310, 790, 580));

    // The window stays on the available screen, and at least its minimum size.
    window.MoveTo(5'000, -100);
    CHECK(fixture.Chrome->Rect == FloatRect(1'130, 0, 790, 580));
    window.ResizeTo(10, 5'000);
    CHECK(fixture.Chrome->Rect == FloatRect(1'130, 0, 100, 1'040));
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

  TEST_CASE("A window's timers", "[Page][LocalDOMWindow]")
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

  TEST_CASE("A window's animation frame callbacks run in the rendering update", "[Page][LocalDOMWindow]")
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

  namespace
  {
    /// @brief A value the host passes as a message or an error.
    struct TestValue final : html::SerializedScriptValue
    {
      std::string Text;
      bool IsClone {false};

      explicit TestValue(std::string text, bool isClone = false) noexcept
          : Text(krys::move(text)), IsClone(isClone)
      {
      }

      KRYS_NODISCARD Ref<html::SerializedScriptValue> Clone() noexcept override
      {
        return CreateRef<TestValue>(Text, true);
      }
    };
  }

  TEST_CASE("A window's user prompts go to the host", "[Page][LocalDOMWindow]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();

    window.Alert(u8"Hello\r\nthere\rfriend");
    CHECK(window.Confirm(u8"Sure?"));
    fixture.Chrome->ConfirmAnswer = false;
    CHECK_FALSE(window.Confirm());
    CHECK_FALSE(window.Prompt(u8"Name?", u8"Ada"));
    fixture.Chrome->PromptAnswer = dom::DOMString(u8"Grace");
    CHECK(window.Prompt(u8"Name?") == dom::DOMString(u8"Grace"));
    // Newlines are normalised.
    CHECK(fixture.Chrome->Dialogs
          == List<std::string> {"alert:Hello\nthere\nfriend", "confirm:Sure?", "confirm:", "prompt:Name?:Ada",
                                "prompt:Name?:"});

    List<std::string> events;
    auto before = window.AddEventListener(u8"beforeprint", [&](dom::Event &) { events.push_back("before"); });
    auto after = window.AddEventListener(u8"afterprint", [&](dom::Event &) { events.push_back("after"); });
    window.Print();
    CHECK(fixture.Chrome->PrintRequests == 1);
    CHECK(events == List<std::string> {"before", "after"});

    // A window without a browsing context shows nothing.
    fixture.TestPage->MainFrame().SetDocument(nullptr);
    window.Alert(u8"Gone");
    CHECK(fixture.Chrome->Dialogs.size() == 5);
  }

  TEST_CASE("A window's user activation", "[Page][LocalDOMWindow]")
  {
    WindowFixture fixture(
      u8"<!DOCTYPE html><style>body { margin: 0 } div { height: 50px }</style><div></div>");
    auto &window = fixture.Window();
    auto &handler = fixture.TestPage->MainFrame().GetEventHandler();

    CHECK_FALSE(window.HasStickyActivation());
    CHECK_FALSE(window.HasTransientActivation());
    CHECK_FALSE(window.HasHistoryActionActivation());

    SECTION("a press activates the window, transiently for 5s")
    {
      PlatformMouseEvent press(DoublePoint(10, 10), DoublePoint(10, 10), MouseButton::Left,
                               PlatformEvent::Type::MousePressed, 1, {}, MonotonicTime::Now());
      (void)handler.HandleMousePressEvent(press);
      CHECK(window.HasStickyActivation());
      CHECK(window.HasTransientActivation());
      CHECK(window.HasHistoryActionActivation());

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

      CHECK(window.ConsumeHistoryActionUserActivation());
      CHECK_FALSE(window.HasHistoryActionActivation());
      CHECK_FALSE(window.ConsumeHistoryActionUserActivation());
    }
  }

  TEST_CASE("A window opens windows through the host", "[Page][LocalDOMWindow]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();
    fixture.Document->SetURL(url::URL(u8"https://example.com/dir/page.html"));
    auto &navigations = fixture.Loader->Navigations;

    // Without transient activation, popups are blocked.
    auto blocked = window.Open(u8"other.html");
    REQUIRE_FALSE(blocked.HasException());
    CHECK(blocked.Value() == nullptr);
    CHECK(navigations.empty());

    window.NotifyActivated();
    auto opened =
      window.Open(u8"other.html", dom::DOMStringAtom(u8"_blank"), u8"width=300,height=200,noopener");
    REQUIRE_FALSE(opened.HasException());
    CHECK(opened.Value() == nullptr);
    REQUIRE(navigations.size() == 1);
    CHECK(navigations[0].IsNewWindow);
    CHECK(navigations[0].URL == "https://example.com/dir/other.html");
    CHECK(navigations[0].NewWindowName == "_blank");
    REQUIRE(navigations[0].Action.GetWindowFeatures());
    CHECK(navigations[0].Action.GetWindowFeatures()->Width == 300.f);
    CHECK(navigations[0].Action.GetWindowFeatures()->WantsPopup());
    CHECK(navigations[0].Action.GetNewFrameOpenerPolicy() == NewFrameOpenerPolicy::Suppress);

    // An existing browsing context is navigated, without activation.
    (void)window.ConsumeTransientActivation();
    auto self = window.Open(u8"#here", dom::DOMStringAtom(u8"_self"));
    REQUIRE_FALSE(self.HasException());
    CHECK(self.Value() == &window);
    REQUIRE(navigations.size() == 2);
    CHECK_FALSE(navigations[1].IsNewWindow);
    CHECK(navigations[1].URL == "https://example.com/dir/page.html#here");

    // A URL that can't be parsed is a SyntaxError.
    auto invalid = window.Open(u8"https://exa mple.com/", dom::DOMStringAtom(u8"_self"));
    CHECK(invalid.HasException());
  }

  TEST_CASE("Window features", "[Page][LocalDOMWindow]")
  {
    auto features = ParseWindowFeatures(u8" left = 10 , top=20,width=300 height=200, popup");
    CHECK(features.X == 10.f);
    CHECK(features.Y == 20.f);
    CHECK(features.Width == 300.f);
    CHECK(features.Height == 200.f);
    CHECK(features.Popup == true);
    CHECK(features.WantsPopup());

    CHECK_FALSE(ParseWindowFeatures(u8"").WantsPopup());
    CHECK_FALSE(ParseWindowFeatures(u8"noopener,noreferrer").WantsPopup());
    CHECK(ParseWindowFeatures(u8"noopener").WantsNoOpener());
    CHECK(ParseWindowFeatures(u8"noreferrer").WantsNoReferrer());
    CHECK(ParseWindowFeatures(u8"noreferrer").WantsNoOpener());
    CHECK_FALSE(ParseWindowFeatures(u8"noopener=0").WantsNoOpener());
    // A window with its bars isn't a popup; one without them is.
    CHECK_FALSE(ParseWindowFeatures(u8"location,menubar,resizable,scrollbars,status").WantsPopup());
    CHECK(ParseWindowFeatures(u8"location,menubar").WantsPopup());
    CHECK(ParseWindowFeatures(u8"popup=yes").Popup == true);
    CHECK(ParseWindowFeatures(u8"popup=no").Popup == false);
  }

  TEST_CASE("A window's global scope", "[Page][LocalDOMWindow]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();

    SECTION("origin and secure contexts")
    {
      fixture.Document->SetURL(url::URL(u8"https://example.com:8080/page.html"));
      CHECK(window.Origin() == u8"https://example.com:8080");
      CHECK(window.IsSecureContext());
      CHECK_FALSE(window.CrossOriginIsolated());

      fixture.Document->SetURL(url::URL(u8"http://example.com/"));
      CHECK_FALSE(window.IsSecureContext());
      fixture.Document->SetURL(url::URL(u8"http://localhost/"));
      CHECK(window.IsSecureContext());
      fixture.Document->SetURL(url::URL(u8"file:///C:/game/ui.html"));
      CHECK(window.Origin() == u8"file://");
      CHECK(window.IsSecureContext());
      fixture.Document->SetURL(url::URL(u8"data:text/html,hi"));
      CHECK(window.Origin() == u8"null");
      CHECK_FALSE(window.IsSecureContext());
    }

    SECTION("btoa() and atob()")
    {
      CHECK(window.Btoa(u8"Hello").Value() == u8"SGVsbG8=");
      CHECK(window.Btoa(u8"").Value() == u8"");
      // Code points up to U+00FF are bytes.
      CHECK(window.Btoa(u8"\u00ff").Value() == u8"/w==");
      CHECK(window.Btoa(u8"\u0100").HasException());

      CHECK(window.Atob(u8"SGVsbG8=").Value() == u8"Hello");
      // Forgiving: whitespace and missing padding.
      CHECK(window.Atob(u8" SGVs\tbG8 ").Value() == u8"Hello");
      CHECK(window.Atob(u8"/w==").Value() == u8"\u00ff");
      CHECK(window.Atob(u8"SGVsbG8=?").HasException());
      CHECK(window.Atob(u8"SGVsbG8==").HasException());
      CHECK(window.Atob(u8"S").HasException());
    }

    SECTION("reportError() fires error, and reaches the console unless cancelled")
    {
      std::string seen;
      bool cancel = true;
      auto listener = window.AddEventListener<html::ErrorEvent>(u8"error",
                                                                [&](html::ErrorEvent &event)
                                                                {
                                                                  seen = Str(event.Message());
                                                                  CHECK(event.IsTrusted());
                                                                  CHECK(event.Cancelable());
                                                                  REQUIRE(event.Error());
                                                                  if (cancel)
                                                                  {
                                                                    event.PreventDefault();
                                                                  }
                                                                  // Reporting from a report does nothing.
                                                                  window.ReportError(u8"Nested");
                                                                });
      window.ReportError(u8"Broken", CreateRef<TestValue>("value"));
      CHECK(seen == "Broken");
      CHECK(fixture.Chrome->ConsoleMessages.empty());

      cancel = false;
      window.ReportError(u8"Unhandled", CreateRef<TestValue>("value"));
      CHECK(fixture.Chrome->ConsoleMessages == List<std::string> {"error:Unhandled"});
    }

    SECTION("structuredClone() clones the value")
    {
      auto value = CreateRef<TestValue>("value");
      auto clone = window.StructuredClone(value.get());
      REQUIRE(clone);
      CHECK(clone.get() != value.get());
      CHECK(static_cast<TestValue &>(*clone).IsClone);
      CHECK(window.StructuredClone(nullptr) == nullptr);
    }

    SECTION("postMessage() queues a message from the window")
    {
      fixture.Document->SetURL(url::URL(u8"https://example.com/page.html"));
      List<std::string> messages;
      auto listener = window.AddEventListener<html::MessageEvent>(
        u8"message",
        [&](html::MessageEvent &event)
        {
          CHECK(event.IsTrusted());
          CHECK(event.Source() == &window);
          CHECK(event.Origin() == u8"https://example.com");
          messages.push_back(event.Data() ? static_cast<TestValue *>(event.Data())->Text : "null");
        });

      REQUIRE_FALSE(window.PostMessage(CreateRef<TestValue>("first")).HasException());
      REQUIRE_FALSE(window.PostMessage(nullptr, u8"*").HasException());
      REQUIRE_FALSE(
        window.PostMessage(CreateRef<TestValue>("same"), u8"https://example.com/other").HasException());
      // Another origin isn't the window's: the message is dropped.
      REQUIRE_FALSE(
        window.PostMessage(CreateRef<TestValue>("other"), u8"https://other.example").HasException());
      CHECK(window.PostMessage(nullptr, u8"not a url").HasException());
      CHECK(messages.empty());

      html::EventLoop::Current().RunUntilIdle();
      CHECK(messages == List<std::string> {"first", "null", "same"});
    }
  }

  TEST_CASE("A window's navigator", "[Page][LocalDOMWindow]")
  {
    WindowFixture fixture;
    auto &window = fixture.Window();
    auto &navigator = window.GetNavigator();
    CHECK(&window.ClientInformation() == &navigator);

    SECTION("NavigatorID")
    {
      CHECK(navigator.AppCodeName() == u8"Mozilla");
      CHECK(navigator.AppName() == u8"Netscape");
      CHECK(navigator.Product() == u8"Gecko");
      CHECK(navigator.ProductSub() == u8"20030107");
      CHECK(navigator.Platform() == u8"Win32");
      CHECK(navigator.Vendor().empty());
      CHECK_FALSE(navigator.TaintEnabled());
      CHECK(navigator.UserAgent() == LocalFrameLoaderClient::DefaultUserAgent);
      CHECK(navigator.AppVersion()
            == utf8_stringview(LocalFrameLoaderClient::DefaultUserAgent)
                 .substr(utf8_stringview(u8"Mozilla/").size()));
    }

    SECTION("NavigatorLanguage, with languagechange")
    {
      uint32 changes = 0;
      auto listener = window.AddEventListener(u8"languagechange", [&](dom::Event &) { ++changes; });
      OverrideUserPreferredLanguages({u8"fr-CA", u8"en-GB"});
      CHECK(navigator.Language() == u8"fr-CA");
      // Only the most preferred language is exposed.
      CHECK(navigator.Languages() == List<dom::DOMString> {u8"fr-CA"});
      CHECK(changes == 0);
      html::EventLoop::Current().RunUntilIdle();
      CHECK(changes == 1);
      OverrideUserPreferredLanguages({});
      html::EventLoop::Current().RunUntilIdle();
      CHECK(changes == 2);
      CHECK_FALSE(navigator.Language().empty());
    }

    SECTION("NavigatorOnLine, with online and offline")
    {
      CHECK(navigator.OnLine());
      List<std::string> events;
      auto online = window.AddEventListener(u8"online", [&](dom::Event &) { events.push_back("online"); });
      auto offline = window.AddEventListener(u8"offline", [&](dom::Event &) { events.push_back("offline"); });
      fixture.TestPage->NetworkStateChanged(false);
      fixture.TestPage->NetworkStateChanged(true);
      CHECK(events == List<std::string> {"offline", "online"});
    }

    SECTION("plugins, cookies, hardware and activation")
    {
      CHECK(navigator.Plugins().Length() == 0);
      CHECK(&navigator.Plugins() == &navigator.Plugins());
      CHECK_FALSE(navigator.Plugins().HasNamedItem(u8"PDF Viewer"));
      CHECK(navigator.MimeTypes().Length() == 0);
      CHECK_FALSE(navigator.PdfViewerEnabled());
      CHECK_FALSE(navigator.JavaEnabled());
      CHECK_FALSE(navigator.CookieEnabled());
      CHECK((navigator.HardwareConcurrency() == 4 || navigator.HardwareConcurrency() == 8));

      auto &activation = navigator.GetUserActivation();
      CHECK_FALSE(activation.HasBeenActive());
      CHECK_FALSE(activation.IsActive());
      window.NotifyActivated();
      CHECK(activation.HasBeenActive());
      CHECK(activation.IsActive());
    }

    SECTION("protocol handlers")
    {
      fixture.Document->SetURL(url::URL(u8"https://example.com/app/page.html"));
      REQUIRE_FALSE(navigator.RegisterProtocolHandler(u8"MAILTO", u8"compose?to=%s").HasException());
      REQUIRE_FALSE(navigator.RegisterProtocolHandler(u8"web+game", u8"/play?u=%s").HasException());
      REQUIRE_FALSE(navigator.UnregisterProtocolHandler(u8"web+game", u8"/play?u=%s").HasException());
      CHECK(fixture.Chrome->ProtocolHandlers
            == List<std::string> {"+mailto https://example.com/app/compose?to=%s",
                                  "+web+game https://example.com/play?u=%s",
                                  "-web+game https://example.com/play?u=%s"});

      // Schemes must be safelisted or web+ and letters; the URL must have %s and be the page's origin.
      CHECK(navigator.RegisterProtocolHandler(u8"http", u8"x?%s").HasException());
      CHECK(navigator.RegisterProtocolHandler(u8"web+", u8"x?%s").HasException());
      CHECK(navigator.RegisterProtocolHandler(u8"web+1", u8"x?%s").HasException());
      CHECK(navigator.RegisterProtocolHandler(u8"mailto", u8"x").HasException());
      CHECK(navigator.RegisterProtocolHandler(u8"mailto", u8"https://other.example/?%s").HasException());
      CHECK(fixture.Chrome->ProtocolHandlers.size() == 3);
    }
  }

  TEST_CASE("Storage maps keep their order and quota", "[Page][LocalDOMWindow][Storage]")
  {
    StorageMap map(20);
    Maybe<dom::DOMString> oldValue;
    CHECK(map.SetItem(u8"b", u8"one", oldValue));
    CHECK_FALSE(oldValue);
    CHECK(map.SetItem(u8"a", u8"two", oldValue));
    CHECK(map.Key(0) == dom::DOMString(u8"b"));
    CHECK(map.Key(1) == dom::DOMString(u8"a"));
    CHECK_FALSE(map.Key(2));
    // Sizes are UTF-16 bytes: b + one + a + two is 16 bytes, so 4 more fit.
    CHECK(map.SetItem(u8"b", u8"three", oldValue));
    CHECK(oldValue == dom::DOMString(u8"one"));
    CHECK_FALSE(map.SetItem(u8"c", u8"x", oldValue));
    CHECK(map.GetItem(u8"b") == dom::DOMString(u8"three"));
    CHECK(map.RemoveItem(u8"b") == dom::DOMString(u8"three"));
    CHECK(map.Length() == 1);
    CHECK(StorageMap::SizeInBytes(u8"\U0001F600") == 4);
  }

  TEST_CASE("A window's session and local storage", "[Page][LocalDOMWindow][Storage]")
  {
    WindowFixture first;
    WindowFixture second;
    first.Document->SetURL(url::URL(u8"https://example.com/one.html"));
    second.Document->SetURL(url::URL(u8"https://example.com/two.html"));

    auto local = first.Window().LocalStorage();
    REQUIRE_FALSE(local.HasException());
    auto &storage = *local.Value();
    CHECK(first.Window().LocalStorage().Value() == &storage);
    CHECK(first.Window().OptionalLocalStorage() == &storage);
    REQUIRE_FALSE(storage.Clear().HasException());

    REQUIRE_FALSE(storage.SetItem(u8"score", u8"10").HasException());
    CHECK(storage.GetItem(u8"score") == dom::DOMString(u8"10"));
    CHECK(storage.Length() == 1);
    CHECK(storage.Key(0) == dom::DOMString(u8"score"));
    CHECK(storage.SupportedPropertyNames() == List<dom::DOMString> {u8"score"});

    SECTION("local storage is shared by an origin's windows, which hear of changes")
    {
      auto &other = *second.Window().LocalStorage().Value();
      CHECK(other.GetItem(u8"score") == dom::DOMString(u8"10"));

      List<std::string> heard;
      auto otherListener = second.Window().AddEventListener<StorageEvent>(
        u8"storage",
        [&](StorageEvent &event)
        {
          CHECK(event.IsTrusted());
          CHECK(event.StorageArea() == &other);
          CHECK(event.URL() == u8"https://example.com/one.html");
          heard.push_back((event.Key() ? Str(*event.Key()) : "null") + ":"
                          + (event.OldValue() ? Str(*event.OldValue()) : "null") + ":"
                          + (event.NewValue() ? Str(*event.NewValue()) : "null"));
        });
      // The window that made the change isn't told.
      bool sourceHeard = false;
      auto sourceListener =
        first.Window().AddEventListener(u8"storage", [&](dom::Event &) { sourceHeard = true; });

      REQUIRE_FALSE(storage.SetItem(u8"score", u8"20").HasException());
      // Setting the same value changes nothing.
      REQUIRE_FALSE(storage.SetItem(u8"score", u8"20").HasException());
      REQUIRE_FALSE(storage.RemoveItem(u8"score").HasException());
      REQUIRE_FALSE(storage.RemoveItem(u8"missing").HasException());
      REQUIRE_FALSE(storage.SetItem(u8"level", u8"3").HasException());
      REQUIRE_FALSE(storage.Clear().HasException());
      REQUIRE_FALSE(storage.Clear().HasException());
      CHECK(heard.empty());

      html::EventLoop::Current().RunUntilIdle();
      CHECK(heard == List<std::string> {"score:10:20", "score:20:null", "level:null:3", "null:null:null"});
      CHECK_FALSE(sourceHeard);
    }

    SECTION("session storage is the page's")
    {
      auto &session = *first.Window().SessionStorage().Value();
      REQUIRE_FALSE(session.SetItem(u8"tab", u8"inventory").HasException());
      CHECK(second.Window().SessionStorage().Value()->GetItem(u8"tab") == null);
      CHECK(&session.Area() != &storage.Area());
    }

    SECTION("the quota")
    {
      dom::DOMString big(2 * 1'024 * 1'024, u8'x');
      REQUIRE_FALSE(storage.SetItem(u8"big", big).HasException());
      auto exceeded = storage.SetItem(u8"bigger", big);
      REQUIRE(exceeded.HasException());
      CHECK(exceeded.GetException().Code() == dom::ExceptionCode::QuotaExceededError);
      REQUIRE_FALSE(storage.Clear().HasException());
    }

    SECTION("opaque origins have no storage")
    {
      WindowFixture opaque;
      opaque.Document->SetURL(url::URL(u8"data:text/html,hi"));
      CHECK(opaque.Window().LocalStorage().HasException());
      CHECK(opaque.Window().SessionStorage().HasException());
    }
  }

  TEST_CASE("Event handler attributes", "[Page][LocalDOMWindow][EventHandlers]")
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
      body.SetOnHashChange([&](dom::Event &) {});
      CHECK(window.OnResize() == body.OnResize());
      CHECK(window.OnResize() != nullptr);
      CHECK(window.OnHashChange() != nullptr);
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
