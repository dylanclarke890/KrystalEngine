#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/CSSComputedStyleDeclaration.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcherManager.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/ErrorEvent.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/MessageEvent.hpp"
#include "Krystal.Booey/HTML/URLDecomposition.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Modules/Gamepad/GamepadManager.hpp"
#include "Krystal.Booey/Page/BarProp.hpp"
#include "Krystal.Booey/Page/DOMSelection.hpp"
#include "Krystal.Booey/Page/DOMTimer.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/External.hpp"
#include "Krystal.Booey/Page/History.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Location.hpp"
#include "Krystal.Booey/Page/Navigation.hpp"
#include "Krystal.Booey/Page/Navigator.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Page/Screen.hpp"
#include "Krystal.Booey/Page/VisualViewport.hpp"
#include "Krystal.Booey/Page/WindowFeatures.hpp"
#include "Krystal.Booey/Platform/Language.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/Base64.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <cmath>
#include <limits>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD List<LocalDOMWindow *> &AllWindows() noexcept
    {
      static List<LocalDOMWindow *> windows;
      return windows;
    }

    /// @brief Whether @p type is a gamepad event's.
    KRYS_NODISCARD bool IsGamepadEventType(dom::DOMStringAtom type) noexcept
    {
      return type.View() == EventNames::GamepadConnected || type.View() == EventNames::GamepadDisconnected;
    }
  }

  LocalDOMWindow::LocalDOMWindow(dom::Document &document) noexcept
      : EventTarget(dom::EventTargetFlags::IsWindow), _document(WeakPtr<dom::Document>::WithRef(document)),
        _timeOrigin(html::EventLoop::Current().Now()),
        _lastActivationTimestamp(std::numeric_limits<dom::DOMHighResTimeStamp>::infinity()),
        _lastHistoryActionActivationTimestamp(std::numeric_limits<dom::DOMHighResTimeStamp>::infinity())
  {
    krys_assert(document.Frame());
    AddLanguageChangeObserver(this, [this] { LanguagesChanged(); });
    AllWindows().push_back(this);
  }

  LocalDOMWindow::~LocalDOMWindow() noexcept
  {
    if (_gamepadEventListenerCount)
    {
      GamepadManager::Singleton().UnregisterDOMWindow(*this);
    }

    RemoveLanguageChangeObserver(this);
    std::erase(AllWindows(), this);
  }

  void LocalDOMWindow::DidAddEventListener(const dom::EventListener &listener) noexcept
  {
    if (IsGamepadEventType(listener.Type()))
    {
      IncrementGamepadEventListenerCount();
    }
  }

  void LocalDOMWindow::DidRemoveEventListener(const dom::EventListener &listener) noexcept
  {
    if (IsGamepadEventType(listener.Type()))
    {
      DecrementGamepadEventListenerCount();
    }
  }

  void LocalDOMWindow::IncrementGamepadEventListenerCount() noexcept
  {
    if (++_gamepadEventListenerCount == 1)
    {
      GamepadManager::Singleton().RegisterDOMWindow(*this);
    }
  }

  void LocalDOMWindow::DecrementGamepadEventListenerCount() noexcept
  {
    krys_debug_assert(_gamepadEventListenerCount);
    if (!--_gamepadEventListenerCount)
    {
      GamepadManager::Singleton().UnregisterDOMWindow(*this);
    }
  }

  void LocalDOMWindow::ForEachWindow(const Func<void(LocalDOMWindow &)> &function) noexcept
  {
    auto windows = AllWindows();
    for (auto *window : windows)
    {
      function(*window);
    }
  }

  Ref<LocalDOMWindow> LocalDOMWindow::Create(dom::Document &document) noexcept
  {
    return AdoptRef(*new LocalDOMWindow(document));
  }

  dom::Document *LocalDOMWindow::Document() const noexcept
  {
    return _document.get();
  }

  LocalFrame *LocalDOMWindow::Frame() const noexcept
  {
    auto *document = Document();
    return document ? document->Frame() : nullptr;
  }

  Page *LocalDOMWindow::GetPage() const noexcept
  {
    auto *frame = Frame();
    return frame ? frame->GetPage() : nullptr;
  }

  bool LocalDOMWindow::IsCurrentlyDisplayedInFrame() const noexcept
  {
    auto *frame = Frame();
    return frame && frame->Document() == Document();
  }

  html::CustomElementRegistry &LocalDOMWindow::EnsureCustomElementRegistry() noexcept
  {
    if (!_customElementRegistry)
    {
      auto &document = *Document();
      _customElementRegistry = html::CustomElementRegistry::Create(document);
      // A document in a browsing context has its window's registry, unless it was given one.
      if (document.CustomElementRegistry() == nullptr)
      {
        document.SetCustomElementRegistry(RefPtr(_customElementRegistry));
      }
    }
    return *_customElementRegistry;
  }

  // https://html.spec.whatwg.org/#dom-name
  dom::DOMString LocalDOMWindow::Name() const noexcept
  {
    // 1. If this's navigable is null, then return the empty string.
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    // 2. Return this's navigable's target name.
    return frame->SpecifiedName();
  }

  // https://html.spec.whatwg.org/#dom-name
  void LocalDOMWindow::SetName(dom::DOMString name) noexcept
  {
    // 1. If this's navigable is null, then return.
    auto *frame = Frame();
    if (!frame)
    {
      return;
    }
    // 2. Set this's navigable's active session history entry's document state's navigable target name to the
    //    given value.
    frame->SetSpecifiedName(krys::move(name));
  }

  BarProp &LocalDOMWindow::Locationbar() const noexcept
  {
    if (!_locationbar)
    {
      _locationbar = BarProp::Create();
    }
    return *_locationbar;
  }

  BarProp &LocalDOMWindow::Menubar() const noexcept
  {
    if (!_menubar)
    {
      _menubar = BarProp::Create();
    }
    return *_menubar;
  }

  BarProp &LocalDOMWindow::Personalbar() const noexcept
  {
    if (!_personalbar)
    {
      _personalbar = BarProp::Create();
    }
    return *_personalbar;
  }

  BarProp &LocalDOMWindow::Scrollbars() const noexcept
  {
    if (!_scrollbars)
    {
      _scrollbars = BarProp::Create();
    }
    return *_scrollbars;
  }

  BarProp &LocalDOMWindow::Statusbar() const noexcept
  {
    if (!_statusbar)
    {
      _statusbar = BarProp::Create();
    }
    return *_statusbar;
  }

  BarProp &LocalDOMWindow::Toolbar() const noexcept
  {
    if (!_toolbar)
    {
      _toolbar = BarProp::Create();
    }
    return *_toolbar;
  }

  // https://html.spec.whatwg.org/#dom-window-close
  void LocalDOMWindow::Close() noexcept
  {
    // 1. Let thisTraversable be this's navigable.
    // 2. If thisTraversable is not a top-level traversable, then return.
    // 3. If thisTraversable's is closing is true, then return.
    // (The frame is the page's top-level traversable.)
    RefPtr<Page> page = ShareRefPtr(GetPage());
    if (!page || page->IsClosing())
    {
      return;
    }

    // 4. Let browsingContext be thisTraversable's active browsing context.
    // 5. Let sourceSnapshotParams be the result of snapshotting source snapshot params given
    // thisTraversable's
    //    active document.
    // 6. If all the following are true: thisTraversable is script-closable; the incumbent global object's
    //    browsing context is familiar with browsingContext; and the incumbent global object's navigable is
    //    allowed by sandboxing to navigate thisTraversable, given sourceSnapshotParams, then:
    // FIXME: Only if the page is script-closable (opened by script, or with one session history entry); a
    // warning on the console otherwise.

    // (The page may ask to stay, through beforeunload, as definitely closing would prompt.)
    if (auto *frame = Frame(); frame && !frame->Loader().ShouldClose())
    {
      return;
    }

    // 6a. Set thisTraversable's is closing to true.
    // 6b. Queue a task on the DOM manipulation task source to definitely close thisTraversable.
    page->SetIsClosing();
    html::EventLoop::Current().QueueTask(html::TaskSource::DOMManipulation,
                                         [weakPage = CreateWeakPtr(page.get())]
                                         {
                                           // Closing the window may destroy the page.
                                           if (RefPtr<Page> page = ShareRefPtr(weakPage.get()))
                                           {
                                             page->GetChromeClient().CloseWindow();
                                           }
                                         });
  }

  bool LocalDOMWindow::Closed() const noexcept
  {
    // Return true if this's browsing context is null or its is closing is true; otherwise false.
    if (!Frame())
    {
      return true;
    }
    auto *page = GetPage();
    return page && page->IsClosing();
  }

  // https://html.spec.whatwg.org/#dom-window-stop
  void LocalDOMWindow::Stop() noexcept
  {
    // 1. If this's navigable is null, then return.
    if (!Frame())
    {
      return;
    }
    // 2. Stop loading this's navigable.
    // FIXME: Stop loading this's navigable.
  }

  void LocalDOMWindow::Focus() noexcept
  {
    // Let current be this's navigable. If current is null, then return.
    auto *page = GetPage();
    if (!page)
    {
      return;
    }
    // If current is a top-level traversable, user agents are encouraged to trigger some sort of notification
    // to indicate to the user that the page is attempting to gain focus: the host decides whether to bring
    // its window to the front. The focusing steps would focus the document's viewport, but the focused
    // element is kept, as browsers do.
    page->GetChromeClient().Focus();
  }

  // https://html.spec.whatwg.org/#dom-top
  LocalDOMWindow *LocalDOMWindow::Top() noexcept
  {
    // 1. If this's navigable is null, then return null.
    // 2. Return this's navigable's top-level traversable's active WindowProxy.
    // (A frame is the only one in its tree.)
    return Frame() ? this : nullptr;
  }

  // https://html.spec.whatwg.org/#dom-parent
  LocalDOMWindow *LocalDOMWindow::Parent() noexcept
  {
    // 1. Let navigable be this's navigable.
    // 2. If navigable is null, then return null.
    // 3. If navigable's parent is not null, then set navigable to navigable's parent.
    // 4. Return navigable's active WindowProxy.
    // (A frame has no parent.)
    return Frame() ? this : nullptr;
  }

  bool LocalDOMWindow::IsNamedObject(const dom::Element &element, dom::DOMStringView name) noexcept
  {
    if (!Is<html::HTMLElement>(element) || name.empty())
    {
      return false;
    }

    // embed, form, img, or object elements that have a name content attribute whose value is name.
    auto localName = element.LocalName().View();
    if (localName == u8"embed" || localName == u8"form" || localName == u8"img" || localName == u8"object")
    {
      if (auto elementName = element.GetAttribute(u8"name"); elementName && *elementName == name)
      {
        return true;
      }
    }

    // HTML elements that have an id content attribute whose value is name.
    auto id = element.GetAttribute(u8"id");
    return id && *id == name;
  }

  Maybe<Variant<RefPtr<dom::Element>, RefPtr<dom::HTMLCollection>>>
    LocalDOMWindow::NamedItem(dom::DOMStringView name) const noexcept
  {
    // The named objects are in a document tree with window's associated Document as their root. There are no
    // child navigables.
    auto *document = Document();
    if (!document)
    {
      return {};
    }

    RefPtr<dom::Element> first;
    bool several = false;
    for (auto &node : dom::DescendantRange(*document))
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      if (!element || !IsNamedObject(*element, name))
      {
        continue;
      }
      if (first)
      {
        several = true;
        break;
      }
      first = ShareRefPtr(element);
    }

    if (!first)
    {
      return {};
    }

    // If objects has only one element, return that element.
    if (!several)
    {
      return Variant<RefPtr<dom::Element>, RefPtr<dom::HTMLCollection>>(krys::move(first));
    }

    // Otherwise return an HTMLCollection rooted at window's associated Document, whose filter matches only
    // named objects of window with the name name.
    RefPtr<dom::HTMLCollection> collection = CreateRef<dom::LiveHTMLCollection>(
      *document, [name = dom::DOMString(name)](const dom::Element &element)
      { return IsNamedObject(element, name) && element.IsInDocumentTree(); });
    return Variant<RefPtr<dom::Element>, RefPtr<dom::HTMLCollection>>(krys::move(collection));
  }

  List<dom::DOMString> LocalDOMWindow::SupportedPropertyNames() const noexcept
  {
    // The window's document-tree child navigable target name property set (empty: there are no child
    // navigables), then the values of the name content attributes of embed, form, img and object elements,
    // and of the id content attributes of HTML elements, that are non-empty and in the document tree, in tree
    // order, ignoring later duplicates.
    List<dom::DOMString> names;
    auto *document = Document();
    if (!document)
    {
      return names;
    }

    auto add = [&](const Maybe<dom::DOMString> &value)
    {
      if (value && !value->empty() && std::ranges::find(names, *value) == names.end())
      {
        names.push_back(*value);
      }
    };

    for (auto &node : dom::DescendantRange(*document))
    {
      auto *element = DynamicDowncast<dom::Element>(node);
      if (!element || !Is<html::HTMLElement>(*element))
      {
        continue;
      }
      auto localName = element->LocalName().View();
      if (localName == u8"embed" || localName == u8"form" || localName == u8"img" || localName == u8"object")
      {
        add(element->GetAttribute(u8"name"));
      }
      add(element->GetAttribute(u8"id"));
    }
    return names;
  }

  boo::Location &LocalDOMWindow::GetLocation() const noexcept
  {
    if (!_location)
    {
      _location = boo::Location::Create(const_cast<LocalDOMWindow &>(*this));
    }
    return *_location;
  }

  boo::History &LocalDOMWindow::GetHistory() const noexcept
  {
    if (!_history)
    {
      _history = boo::History::Create(const_cast<LocalDOMWindow &>(*this));
    }
    return *_history;
  }

  boo::Navigation &LocalDOMWindow::GetNavigation() const noexcept
  {
    if (!_navigation)
    {
      _navigation = boo::Navigation::Create(const_cast<LocalDOMWindow &>(*this));
    }
    return *_navigation;
  }

  boo::Navigation *LocalDOMWindow::NavigationIfExists() const noexcept
  {
    return _navigation.get();
  }

  html::CloseWatcherManager &LocalDOMWindow::GetCloseWatcherManager() const noexcept
  {
    if (!_closeWatcherManager)
    {
      _closeWatcherManager = html::CloseWatcherManager::Create();
    }
    return *_closeWatcherManager;
  }

  html::CloseWatcherManager *LocalDOMWindow::CloseWatcherManagerIfExists() const noexcept
  {
    return _closeWatcherManager.get();
  }

  Navigator &LocalDOMWindow::GetNavigator() const noexcept
  {
    if (!_navigator)
    {
      _navigator = Navigator::Create(const_cast<LocalDOMWindow &>(*this));
    }
    return *_navigator;
  }

  Navigator *LocalDOMWindow::NavigatorIfExists() const noexcept
  {
    return _navigator.get();
  }

  void LocalDOMWindow::LanguagesChanged() noexcept
  {
    if (auto *document = Document())
    {
      document->QueueTaskToDispatchEventOnWindow(
        html::TaskSource::DOMManipulation,
        dom::Event::Create(dom::DOMStringAtom(EventNames::LanguageChange), false, false));
    }
  }

  External &LocalDOMWindow::GetExternal() const noexcept
  {
    if (!_external)
    {
      _external = External::Create();
    }
    return *_external;
  }

  RefPtr<css::MediaQueryList> LocalDOMWindow::MatchMedia(dom::DOMStringView query) noexcept
  {
    auto *document = Document();
    return document ? RefPtr<css::MediaQueryList>(document->GetMediaQueryMatcher().MatchMedia(query))
                    : nullptr;
  }

  Screen &LocalDOMWindow::GetScreen() const noexcept
  {
    if (!_screen)
    {
      _screen = Screen::Create(const_cast<LocalDOMWindow &>(*this));
    }
    return *_screen;
  }

  VisualViewport &LocalDOMWindow::GetVisualViewport() const noexcept
  {
    if (!_visualViewport)
    {
      _visualViewport = VisualViewport::Create(const_cast<LocalDOMWindow &>(*this));
      _visualViewport->Update();
    }
    return *_visualViewport;
  }

  VisualViewport *LocalDOMWindow::OptionalVisualViewport() const noexcept
  {
    return _visualViewport.get();
  }

  FloatRect LocalDOMWindow::AdjustWindowRect(Page &page, const FloatRect &pendingChanges) noexcept
  {
    auto &environment = page.MainFrame().GetMediaEnvironment();
    FloatRect screen = environment.ScreenAvailableRect.IsEmpty()
                         ? FloatRect(FloatPoint(), environment.ScreenSize)
                         : environment.ScreenAvailableRect;
    FloatRect window = page.GetChromeClient().WindowRect();

    // Update window values if new requested values are not NaN.
    if (!std::isnan(pendingChanges.X()))
    {
      window.SetX(pendingChanges.X());
    }
    if (!std::isnan(pendingChanges.Y()))
    {
      window.SetY(pendingChanges.Y());
    }
    if (!std::isnan(pendingChanges.Width()))
    {
      window.SetWidth(pendingChanges.Width());
    }
    if (!std::isnan(pendingChanges.Height()))
    {
      window.SetHeight(pendingChanges.Height());
    }

    FloatSize minimumSize = page.GetChromeClient().MinimumWindowSize();
    window.SetWidth(std::min(std::max(minimumSize.Width(), window.Width()), screen.Width()));
    window.SetHeight(std::min(std::max(minimumSize.Height(), window.Height()), screen.Height()));

    // Constrain the window position within the valid screen area.
    window.SetX(std::max(screen.X(), std::min(window.X(), screen.MaxX() - window.Width())));
    window.SetY(std::max(screen.Y(), std::min(window.Y(), screen.MaxY() - window.Height())));

    return window;
  }

  bool LocalDOMWindow::AllowedToChangeWindowGeometry() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return false;
    }
    auto *page = frame->GetPage();
    if (!page || &page->MainFrame() != frame)
    {
      return false;
    }
    // Prevent web content from tricking the user into initiating a drag.
    return !frame->GetEventHandler().MousePressed();
  }

  void LocalDOMWindow::MoveBy(int32 x, int32 y) const noexcept
  {
    if (!AllowedToChangeWindowGeometry())
    {
      return;
    }

    auto &page = *GetPage();
    auto update = page.GetChromeClient().WindowRect();
    update.Move(static_cast<float>(x), static_cast<float>(y));
    page.GetChromeClient().SetWindowRect(AdjustWindowRect(page, update));
  }

  void LocalDOMWindow::MoveTo(int32 x, int32 y) const noexcept
  {
    if (!AllowedToChangeWindowGeometry())
    {
      return;
    }

    auto &page = *GetPage();
    auto update = page.GetChromeClient().WindowRect();
    update.SetLocation(FloatPoint(static_cast<float>(x), static_cast<float>(y)));
    page.GetChromeClient().SetWindowRect(AdjustWindowRect(page, update));
  }

  void LocalDOMWindow::ResizeBy(int32 x, int32 y) const noexcept
  {
    if (!AllowedToChangeWindowGeometry())
    {
      return;
    }

    auto &page = *GetPage();
    auto rect = page.GetChromeClient().WindowRect();
    FloatRect update(rect.Location(), rect.Size() + FloatSize(static_cast<float>(x), static_cast<float>(y)));
    page.GetChromeClient().SetWindowRect(AdjustWindowRect(page, update));
  }

  void LocalDOMWindow::ResizeTo(int32 width, int32 height) const noexcept
  {
    if (!AllowedToChangeWindowGeometry())
    {
      return;
    }

    auto &page = *GetPage();
    auto rect = page.GetChromeClient().WindowRect();
    FloatRect update(rect.Location(), FloatSize(static_cast<float>(width), static_cast<float>(height)));
    page.GetChromeClient().SetWindowRect(AdjustWindowRect(page, update));
  }

  int32 LocalDOMWindow::InnerWidth() const noexcept
  {
    auto *frame = Frame();
    auto *view = frame ? frame->View() : nullptr;
    if (!view)
    {
      return 0;
    }
    return view->MapFromLayoutToCSSUnits(
      view->UnobscuredContentRect(ScrollableArea::VisibleContentRectIncludesScrollbars::Yes).Width());
  }

  int32 LocalDOMWindow::InnerHeight() const noexcept
  {
    auto *frame = Frame();
    auto *view = frame ? frame->View() : nullptr;
    if (!view)
    {
      return 0;
    }
    return view->MapFromLayoutToCSSUnits(
      view->UnobscuredContentRect(ScrollableArea::VisibleContentRectIncludesScrollbars::Yes).Height());
  }

  int32 LocalDOMWindow::ScrollX() const noexcept
  {
    auto *frame = Frame();
    auto *view = frame ? frame->View() : nullptr;
    if (!view || !view->ContentsScrollPosition().X())
    {
      return 0;
    }

    (void)frame->Document()->UpdateLayoutIgnorePendingStylesheets();

    // Layout may have affected the current frame.
    auto *frameAfterLayout = Frame();
    auto *viewAfterLayout = frameAfterLayout ? frameAfterLayout->View() : nullptr;
    return viewAfterLayout
             ? viewAfterLayout->MapFromLayoutToCSSUnits(viewAfterLayout->ContentsScrollPosition().X())
             : 0;
  }

  int32 LocalDOMWindow::ScrollY() const noexcept
  {
    auto *frame = Frame();
    auto *view = frame ? frame->View() : nullptr;
    if (!view || !view->ContentsScrollPosition().Y())
    {
      return 0;
    }

    (void)frame->Document()->UpdateLayoutIgnorePendingStylesheets();

    // Layout may have affected the current frame.
    auto *frameAfterLayout = Frame();
    auto *viewAfterLayout = frameAfterLayout ? frameAfterLayout->View() : nullptr;
    return viewAfterLayout
             ? viewAfterLayout->MapFromLayoutToCSSUnits(viewAfterLayout->ContentsScrollPosition().Y())
             : 0;
  }

  void LocalDOMWindow::ScrollTo(double x, double y) const noexcept
  {
    ScrollTo(ScrollToOptions(x, y));
  }

  void LocalDOMWindow::ScrollTo(const ScrollToOptions &options) const noexcept
  {
    if (!IsCurrentlyDisplayedInFrame())
    {
      return;
    }

    RefPtr<LocalFrameView> view = ShareRefPtr(Frame()->View());
    if (!view)
    {
      return;
    }

    ScrollToOptions scrollToOptions = NormalizeNonFiniteCoordinatesOrFallBackTo(
      options, view->ContentsScrollPosition().X(), view->ContentsScrollPosition().Y());

    // This is an optimization for the common case of scrolling to (0, 0) when the scroller is already at the
    // origin. If an animated scroll is in progress, this optimization is skipped to ensure that the animated
    // scroll is really stopped.
    if (view->GetScrollAnimationStatus() == ScrollAnimationStatus::NotAnimating && !*scrollToOptions.Left
        && !*scrollToOptions.Top && view->ContentsScrollPosition().IsZero())
    {
      return;
    }

    view->CancelScheduledScrolls();
    (void)Document()->UpdateLayoutIgnorePendingStylesheets();

    IntPoint layoutPos(view->MapFromCSSToLayoutUnits(static_cast<int32>(*scrollToOptions.Left)),
                       view->MapFromCSSToLayoutUnits(static_cast<int32>(*scrollToOptions.Top)));

    // FIXME: Smooth scrolling (useSmoothScrolling) and scroll snapping, once there are scroll animations.
    view->SetContentsScrollPosition(layoutPos, ScrollPositionChangeOptions::CreateProgrammatic());
  }

  void LocalDOMWindow::ScrollBy(double x, double y) const noexcept
  {
    ScrollBy(ScrollToOptions(x, y));
  }

  void LocalDOMWindow::ScrollBy(const ScrollToOptions &options) const noexcept
  {
    if (!IsCurrentlyDisplayedInFrame())
    {
      return;
    }

    (void)Document()->UpdateLayoutIgnorePendingStylesheets();

    auto *view = Frame() ? Frame()->View() : nullptr;
    if (!view)
    {
      return;
    }

    ScrollToOptions scrollToOptions = NormalizeNonFiniteCoordinatesOrFallBackTo(options, 0, 0);
    *scrollToOptions.Left += view->MapFromLayoutToCSSUnits(view->ContentsScrollPosition().X());
    *scrollToOptions.Top += view->MapFromLayoutToCSSUnits(view->ContentsScrollPosition().Y());
    ScrollTo(scrollToOptions);
  }

  int32 LocalDOMWindow::ScreenX() const noexcept
  {
    auto *page = GetPage();
    return page ? static_cast<int32>(page->GetChromeClient().WindowRect().X()) : 0;
  }

  int32 LocalDOMWindow::ScreenY() const noexcept
  {
    auto *page = GetPage();
    return page ? static_cast<int32>(page->GetChromeClient().WindowRect().Y()) : 0;
  }

  int32 LocalDOMWindow::OuterWidth() const noexcept
  {
    auto *page = GetPage();
    return page ? static_cast<int32>(page->GetChromeClient().WindowRect().Width()) : 0;
  }

  int32 LocalDOMWindow::OuterHeight() const noexcept
  {
    auto *page = GetPage();
    return page ? static_cast<int32>(page->GetChromeClient().WindowRect().Height()) : 0;
  }

  double LocalDOMWindow::DevicePixelRatio() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return 0;
    }
    // Page zoom applies to all frames synchronously, so include it in devicePixelRatio
    // for both main frame and iframes. FrameScaleFactor() doesn't include page zoom
    // since page zoom is applied globally for rendering.
    return frame->GetMediaEnvironment().DeviceScaleFactor * frame->FrameScaleFactor()
           * frame->PageZoomFactor();
  }

  Ref<css::CSSStyleDeclaration>
    LocalDOMWindow::GetComputedStyle(dom::Element &element,
                                     const css::CSSOMString &pseudoElement) const noexcept
  {
    if (!pseudoElement.starts_with(u8':'))
    {
      return css::CSSComputedStyleDeclaration::Create(element, null);
    }

    // FIXME: This does not work for pseudo-elements that take arguments (webkit.org/b/264103).
    auto [pseudoElementIsParsable, pseudoElementIdentifier] = css::SelectorParser::ParsePseudoElement(
      pseudoElement, css::SelectorParserContext {element.NodeDocument()});
    if (!pseudoElementIsParsable)
    {
      return css::CSSComputedStyleDeclaration::CreateEmpty(element);
    }
    return css::CSSComputedStyleDeclaration::Create(element, pseudoElementIdentifier);
  }

  namespace
  {
    /// @brief Whether the user agent can't show @p window's simple dialogs now.
    /// @see https://html.spec.whatwg.org/#cannot-show-simple-dialogs
    KRYS_NODISCARD bool CannotShowSimpleDialogs(const LocalDOMWindow &window) noexcept
    {
      // 1. If the active sandboxing flag set of window's associated Document has the sandboxed modals flag
      //    set, then return true.
      // FIXME: The sandboxed modals flag, once there's sandboxing.
      // 2. If window's relevant agent's event loop's termination nesting level is nonzero, then optionally
      //    return true.
      // (There's no script to terminate.)
      // 3. Optionally, return true.
      // (A window without a page can't show them, nor can a document whose unload counter is greater than
      // 0, as beforeunload, pagehide or unload is being dispatched.)
      // 4. Return false.
      auto *document = window.Document();
      return !window.Frame() || !window.GetPage() || !document || document->UnloadCounter() > 0;
    }

    /// @see https://w3c.github.io/webappsec-secure-contexts/#is-url-trustworthy
    KRYS_NODISCARD bool IsPotentiallyTrustworthy(const url::URL &url) noexcept
    {
      // about:blank and about:srcdoc have their creator's origin; documents have no creators yet.
      if (url.IsNull() || url.ProtocolIsAbout())
      {
        return true;
      }
      if (url.ProtocolIsData())
      {
        return false;
      }
      if (url.ProtocolIs(u8"https") || url.ProtocolIs(u8"wss") || url.ProtocolIsFile())
      {
        return true;
      }
      // The local host: 127.0.0.0/8, ::1, localhost and its subdomains.
      auto host = url.Host();
      if (host == u8"localhost" || host.ends_with(u8".localhost") || host == u8"[::1]"
          || host.starts_with(u8"127."))
      {
        return true;
      }
      return false;
    }
  }

  // https://html.spec.whatwg.org/#dom-alert
  void LocalDOMWindow::Alert(const dom::DOMString &message) noexcept
  {
    // 1. If we cannot show simple dialogs for this, then return.
    if (CannotShowSimpleDialogs(*this))
    {
      return;
    }

    // 2. If the method was invoked with no arguments, then let message be the empty string; otherwise, let
    //    message be the method's first argument.
    // 3. Set message to the result of normalizing newlines given message.
    // 4. Set message to the result of optionally truncating message.
    // (The host may truncate it.)
    // 5. Let userPromptHandler be WebDriver BiDi user prompt opened with this, "alert", and message.
    // (There's no WebDriver, so userPromptHandler is "none".)
    // 6. If userPromptHandler is "none":
    // 6a. Show message to the user, treating U+000A LF as a line break.
    // 6b. Optionally, pause while waiting for the user to acknowledge the message.
    (void)Document()->UpdateStyleIfNeeded();
    GetPage()->GetPointerLockController().RequestPointerUnlock();
    GetPage()->GetChromeClient().RunJavaScriptAlert(*Frame(),
                                                    infra::StringAlgorithms::NormalizeNewlines(message));
    // 7. Invoke WebDriver BiDi user prompt closed with this, "alert", and true.
  }

  // https://html.spec.whatwg.org/#dom-confirm
  bool LocalDOMWindow::Confirm(const dom::DOMString &message) noexcept
  {
    // 1. If we cannot show simple dialogs for this, then return false.
    if (CannotShowSimpleDialogs(*this))
    {
      return false;
    }

    // 2. Set message to the result of normalizing newlines given message.
    // 3. Set message to the result of optionally truncating message.
    // 4. Show message to the user, treating U+000A LF as a line break, and ask the user to respond with a
    //    positive or negative response.
    // 5. Let userPromptHandler be WebDriver BiDi user prompt opened with this, "confirm", and message.
    // 6. Let accepted be false.
    // 7. If userPromptHandler is "none":
    // 7a. Pause until the user responds either positively or negatively.
    // 7b. If the user responded positively, then set accepted to true.
    // 8. If userPromptHandler is "accept", then set accepted to true.
    // 9. Invoke WebDriver BiDi user prompt closed with this, "confirm", and accepted.
    // 10. Return accepted.
    // (The host shows the message, may truncate it, and answers; there's no WebDriver.)
    (void)Document()->UpdateStyleIfNeeded();
    GetPage()->GetPointerLockController().RequestPointerUnlock();
    return GetPage()->GetChromeClient().RunJavaScriptConfirm(
      *Frame(), infra::StringAlgorithms::NormalizeNewlines(message));
  }

  // https://html.spec.whatwg.org/#dom-prompt
  Maybe<dom::DOMString> LocalDOMWindow::Prompt(const dom::DOMString &message,
                                               const dom::DOMString &defaultValue) noexcept
  {
    // 1. If we cannot show simple dialogs for this, then return null.
    if (CannotShowSimpleDialogs(*this))
    {
      return null;
    }

    // 2. Set message to the result of normalizing newlines given message.
    // 3. Set message to the result of optionally truncating message.
    // 4. Set default to the result of optionally truncating default.
    // 5. Show message to the user, treating U+000A LF as a line break, and ask the user to either respond
    //    with a string value or abort. The response must be defaulted to the value given by default.
    // 6. Let userPromptHandler be WebDriver BiDi user prompt opened with this, "prompt", and message.
    // 7. Let result be null.
    // 8. If userPromptHandler is "none":
    // 8a. Pause while waiting for the user's response.
    // 8b. If the user did not abort, then set result to the string that the user responded with.
    // 9. Otherwise, if userPromptHandler is "accept", then set result to the empty string.
    // 10. Invoke WebDriver BiDi user prompt closed with this, "prompt", false if result is null or true
    //     otherwise, and result.
    // 11. Return result.
    // (The host shows the message, may truncate it and the default, and answers; there's no WebDriver.)
    (void)Document()->UpdateStyleIfNeeded();
    GetPage()->GetPointerLockController().RequestPointerUnlock();
    return GetPage()->GetChromeClient().RunJavaScriptPrompt(
      *Frame(), infra::StringAlgorithms::NormalizeNewlines(message), defaultValue);
  }

  // https://html.spec.whatwg.org/#dom-print
  void LocalDOMWindow::Print() noexcept
  {
    // 1. Let document be this's associated Document.
    RefPtr<dom::Document> document = ShareRefPtr(Document());
    // 2. If document is not fully active, then return.
    RefPtr<LocalFrame> frame = ShareRefPtr(Frame());
    if (!frame || !GetPage() || !document || !document->IsFullyActive())
    {
      return;
    }

    // 3. If document's unload counter is greater than 0, then return.
    if (document->UnloadCounter() > 0)
    {
      return;
    }

    // 4. If document is ready for post-load tasks, then run the printing steps for document.
    // (The initial about:blank document is never loaded, so it's ready straight away.)
    if (document->LoadEventFinished() || frame->Loader().IsDisplayingInitialEmptyDocument())
    {
      _shouldPrintWhenFinishedLoading = false;
      RunPrintingSteps(*frame);
      return;
    }
    // 5. Otherwise, set document's print when loaded flag.
    _shouldPrintWhenFinishedLoading = true;
  }

  void LocalDOMWindow::FinishedLoading() noexcept
  {
    // https://html.spec.whatwg.org/#the-end
    // 10. If the Document's print when loaded flag is set, then run the printing steps.
    if (!_shouldPrintWhenFinishedLoading)
    {
      return;
    }
    _shouldPrintWhenFinishedLoading = false;
    if (RefPtr<LocalFrame> frame = ShareRefPtr(Frame()); frame && GetPage())
    {
      RunPrintingSteps(*frame);
    }
  }

  // https://html.spec.whatwg.org/#printing-steps
  void LocalDOMWindow::RunPrintingSteps(LocalFrame &frame) noexcept
  {
    // 1. The user agent may display a message to the user or return (or both).
    // 2. If the active sandboxing flag set of document has the sandboxed modals flag set, then return.
    // FIXME: The sandboxed modals flag, once there's sandboxing.
    // 3. The user agent must fire an event named beforeprint at the relevant global object of document, as
    //    well as any child navigable in it.
    // (There are no child navigables.)
    (void)dom::EventDispatcher::FireEvent(dom::DOMStringAtom(EventNames::BeforePrint), *this);
    // 4. The user agent should offer the user the opportunity to obtain a physical form (or the
    //    representation of a physical form) of document.
    if (auto *page = GetPage())
    {
      page->GetChromeClient().Print(frame);
    }
    // 5. The user agent must fire an event named afterprint at the relevant global object of document, as
    //    well as any child navigables in it.
    (void)dom::EventDispatcher::FireEvent(dom::DOMStringAtom(EventNames::AfterPrint), *this);
  }

  dom::ExceptionOr<LocalDOMWindow *> LocalDOMWindow::Open(utf8_stringview urlString,
                                                          dom::DOMStringAtom target,
                                                          dom::DOMStringView featuresString) noexcept
  {
    // https://html.spec.whatwg.org/#window-open-steps
    // 1. If the event loop's termination nesting level is nonzero, then return null.
    // (There's no script to terminate; a window that isn't shown in a frame has no navigable.)
    if (!IsCurrentlyDisplayedInFrame())
    {
      return static_cast<LocalDOMWindow *>(nullptr);
    }

    // 2. Let sourceDocument be the entry global object's associated Document.
    RefPtr<dom::Document> document = ShareRefPtr(Document());
    RefPtr<LocalFrame> frame = ShareRefPtr(Frame());

    // 3. Let urlRecord be null.
    // 4. If url is not the empty string:
    // 4a. Set urlRecord to the result of encoding-parsing a URL given url, relative to sourceDocument.
    // 4b. If urlRecord is failure, then throw a "SyntaxError" DOMException.
    url::URL urlRecord;
    if (!urlString.empty())
    {
      urlRecord = document->CompleteURL(urlString);
      if (!urlRecord.IsValid())
      {
        return dom::ExceptionCode::SyntaxError;
      }
    }

    // 5. If target is the empty string, then set target to "_blank".
    if (target.View().empty())
    {
      target = dom::DOMStringAtom(u8"_blank");
    }

    // 6. Let tokenizedFeatures be the result of tokenizing features.
    // 7.-10. (WindowFeatures parses noopener and noreferrer, and leaves them out of the popup check.)
    auto features = ParseWindowFeatures(featuresString);
    // 13. Let targetNavigable and windowType be the result of applying the rules for choosing a navigable
    //     given target, sourceDocument's node navigable, and noopener.
    auto *targetFrame = frame->Loader().FindFrameForNavigation(target.View());

    // 14. If targetNavigable is null, then return null.
    // (A new window needs transient activation: popups are blocked otherwise.)
    if (!targetFrame && !HasTransientActivation())
    {
      return static_cast<LocalDOMWindow *>(nullptr);
    }

    // 11. Let referrerPolicy be the empty string.
    // 12. If noreferrer is true, then set noopener to true and set referrerPolicy to "no-referrer".
    auto referrerPolicy =
      features.WantsNoReferrer() ? ReferrerPolicy::NoReferrer : ReferrerPolicy::EmptyString;

    // 16. Otherwise:
    if (targetFrame)
    {
      // 16a. If urlRecord is not null, then navigate targetNavigable to urlRecord using sourceDocument, with
      //      referrerPolicy set to referrerPolicy and exceptionsEnabled set to true.
      if (!urlRecord.IsNull())
      {
        frame->Loader().ChangeLocation(urlRecord, dom::DOMStringAtom(u8"_self"), nullptr, referrerPolicy,
                                       null, dom::DOMStringAtom::Null());
      }
      // 16b. If noopener is false, then set targetNavigable's active browsing context's opener browsing
      //      context to sourceDocument's browsing context.
      // (There are no other browsing contexts to be the opener.)
      // 18. If noopener is true and target is not an ASCII case-insensitive match for "_self", "_parent", or
      //     "_top", then return null.
      if (features.WantsNoOpener() && !text::IsASCIICaselessEqual(target.View(), utf8_stringview(u8"_self"))
          && !text::IsASCIICaselessEqual(target.View(), utf8_stringview(u8"_parent"))
          && !text::IsASCIICaselessEqual(target.View(), utf8_stringview(u8"_top")))
      {
        return static_cast<LocalDOMWindow *>(nullptr);
      }
      // 19. Return targetNavigable's active WindowProxy.
      return this;
    }

    // 15. If windowType is either "new and unrestricted" or "new with no opener": the host opens it, asked by
    //     the client's new window policy, with the features (the popup check and the browsing context
    //     features) and about:blank if there's no URL.
    // 17. If windowType is "new with no opener", then return null.
    // (The new window is never returned, as there are no other windows.)
    FrameLoadRequest request(ShareRef(*document),
                             ResourceRequest(urlRecord.IsNull() ? url::URL(u8"about:blank") : urlRecord),
                             target);
    request.SetReferrerPolicy(referrerPolicy);
    request.SetNewFrameOpenerPolicy(features.WantsNoOpener() ? NewFrameOpenerPolicy::Suppress
                                                             : NewFrameOpenerPolicy::Allow);
    request.SetWindowFeatures(Maybe<WindowFeatures>(krys::move(features)));
    frame->Loader().ChangeLocation(krys::move(request));
    return static_cast<LocalDOMWindow *>(nullptr);
  }

  // https://html.spec.whatwg.org/#activation-notification
  void LocalDOMWindow::NotifyActivated() noexcept
  {
    // 1. Assert: document is fully active.
    // 2. Let windows be « document's relevant global object ».
    // 3. Extend windows with the active window of each of document's ancestor navigables.
    // 4. Extend windows with the active window of each of document's descendant navigables, filtered to
    //    include only those navigables whose active document's origin is same origin with document's origin.
    // (There are no ancestor or descendant frames, so windows is this window.)
    // 5. For each window in windows:
    // 5.1. Set window's last activation timestamp to the current high resolution time.
    _lastActivationTimestamp = NowTimestamp();
    // 5.2. Notify the close watcher manager about user activation given window.
    GetCloseWatcherManager().NotifyAboutUserActivation();
  }

  bool LocalDOMWindow::HasStickyActivation() const noexcept
  {
    // When the current high resolution time given W is greater than or equal to the last activation timestamp
    // in W, W is said to have sticky activation.
    return NowTimestamp() >= _lastActivationTimestamp;
  }

  bool LocalDOMWindow::HasTransientActivation() const noexcept
  {
    // When the current high resolution time is greater than or equal to the last activation timestamp in W,
    // and less than the last activation timestamp in W plus the transient activation duration, then W is said
    // to have transient activation.
    auto now = NowTimestamp();
    return now >= _lastActivationTimestamp && now < _lastActivationTimestamp + TransientActivationDuration();
  }

  bool LocalDOMWindow::HasHistoryActionActivation() const noexcept
  {
    // When the last history-action activation timestamp of W is not equal to the last activation timestamp of
    // W, then W is said to have history-action activation.
    return _lastHistoryActionActivationTimestamp != _lastActivationTimestamp;
  }

  // https://html.spec.whatwg.org/#consume-user-activation
  bool LocalDOMWindow::ConsumeTransientActivation() noexcept
  {
    // (Without transient activation, consuming changes nothing observable, so it returns early to report it.)
    if (!HasTransientActivation())
    {
      return false;
    }
    // 1. If W's navigable is null, then return.
    // 2. Let top be W's navigable's top-level traversable.
    // 3. Let navigables be the inclusive descendant navigables of top's active document.
    // 4. Let windows be the list of Window objects constructed by taking the active window of each item in
    //    navigables.
    // (There are no other frames, so windows is this window.)
    // 5. For each window in windows, if window's last activation timestamp is not positive infinity, then set
    //    window's last activation timestamp to negative infinity.
    if (_lastActivationTimestamp != std::numeric_limits<dom::DOMHighResTimeStamp>::infinity())
    {
      _lastActivationTimestamp = -std::numeric_limits<dom::DOMHighResTimeStamp>::infinity();
    }
    return true;
  }

  // https://html.spec.whatwg.org/#consume-history-action-user-activation
  bool LocalDOMWindow::ConsumeHistoryActionUserActivation() noexcept
  {
    if (!HasHistoryActionActivation())
    {
      return false;
    }
    // 1. If W's navigable is null, then return.
    // 2. Let top be W's navigable's top-level traversable.
    // 3. Let navigables be the inclusive descendant navigables of top's active document.
    // 4. Let windows be the list of Window objects constructed by taking the active window of each item in
    //    navigables.
    // (There are no other frames, so windows is this window.)
    // 5. For each window in windows, set window's last history-action activation timestamp to window's last
    //    activation timestamp.
    _lastHistoryActionActivationTimestamp = _lastActivationTimestamp;
    return true;
  }

  dom::USVString LocalDOMWindow::Origin() const noexcept
  {
    auto *document = Document();
    return document ? html::URLDecomposition::SerializedOrigin(document->GetURL()) : dom::USVString();
  }

  bool LocalDOMWindow::IsSecureContext() const noexcept
  {
    auto *document = Document();
    return document && IsPotentiallyTrustworthy(document->GetURL());
  }

  void LocalDOMWindow::ReportError(const dom::DOMString &message,
                                   RefPtr<html::SerializedScriptValue> &&error) noexcept
  {
    // https://html.spec.whatwg.org/#dom-reporterror: report an exception e for this.
    // https://html.spec.whatwg.org/#report-an-exception
    // 1. Let notHandled be true.
    // 2. Let errorInfo be the result of extracting error information from exception.
    // (The message and the value; there's no script location.)
    // 3. Let script be a script found in an implementation-defined way, or null.
    // 4. If script is a classic script and script's muted errors is true, then ...
    // 5. If omitError is true, then set errorInfo[error] to null.
    // (There are no scripts.)
    // 6. If global is not in error reporting mode:
    if (_errorReportingMode)
    {
      return;
    }

    // 6a. Set global's in error reporting mode to true.
    // 6b. If global implements EventTarget, then set notHandled to the result of firing an event named error
    //     at global, using ErrorEvent, with the cancelable attribute initialized to true, and additional
    //     attributes initialized according to errorInfo.
    // FIXME: The script's location (filename, line and column), once there's scripting.
    _errorReportingMode = true;
    Ref<dom::Event> event = html::ErrorEvent::Create(message, {}, 0, 0, krys::move(error));
    Ref<LocalDOMWindow> protectedThis = ShareRef(*this);
    bool notHandled = dom::EventDispatcher::DispatchToTarget(*event, this);
    // 6c. Set global's in error reporting mode to false.
    _errorReportingMode = false;

    // 7. If notHandled is true:
    // 7a. Set errorInfo[error] to null.
    // 7c. Otherwise, the user agent may report exception to a developer console.
    if (notHandled)
    {
      if (auto *page = GetPage())
      {
        auto *document = Document();
        page->GetChromeClient().AddMessageToConsole(MessageSource::JS, MessageLevel::Error, message, 0, 0,
                                                    document ? dom::DOMString(document->GetURL().String())
                                                             : dom::DOMString());
      }
    }
  }

  dom::ExceptionOr<dom::DOMString> LocalDOMWindow::Btoa(dom::DOMStringView data) const noexcept
  {
    // If data contains any code point whose value is greater than U+00FF, throw an "InvalidCharacterError".
    List<uint8> bytes;
    bytes.reserve(data.size());
    for (text::CodePointIterator it(Span<const char8>(data.data(), data.size())); !it.AtEnd(); ++it)
    {
      char32 codePoint = *it;
      if (codePoint > 0xFF)
      {
        return dom::ExceptionCode::InvalidCharacterError;
      }
      bytes.push_back(static_cast<uint8>(codePoint));
    }

    // Return the result of running forgiving-base64 encode on data's code points as bytes.
    return dom::DOMString(Base64Encode(bytes));
  }

  dom::ExceptionOr<dom::DOMString> LocalDOMWindow::Atob(dom::DOMStringView data) const noexcept
  {
    // Let decodedData be the result of running forgiving-base64 decode on data; if failure, throw an
    // "InvalidCharacterError".
    auto decoded = Base64Decode(Span<const uint8>(reinterpret_cast<const uint8 *>(data.data()), data.size()),
                                {Base64DecodeOption::ValidatePadding, Base64DecodeOption::IgnoreWhitespace});
    if (!decoded)
    {
      return dom::ExceptionCode::InvalidCharacterError;
    }

    // Return decodedData as code points.
    StringBuilder builder;
    for (auto byte : *decoded)
    {
      builder.Append(static_cast<char32>(byte));
    }
    return dom::DOMString(builder.ToString());
  }

  dom::ExceptionOr<void> LocalDOMWindow::PostMessage(RefPtr<html::SerializedScriptValue> &&message,
                                                     utf8_stringview targetOrigin) noexcept
  {
    // https://html.spec.whatwg.org/#dom-window-postmessage
    // 1. Let options be «[ "targetOrigin" → targetOrigin, "transfer" → transfer ]».
    // 2. Run the window post message steps providing this, message, and options.
    WindowPostMessageOptions options;
    options.TargetOrigin = dom::USVString(targetOrigin);
    return PostMessage(krys::move(message), options);
  }

  // https://html.spec.whatwg.org/#dom-window-postmessage-options
  // https://html.spec.whatwg.org/#window-post-message-steps
  dom::ExceptionOr<void> LocalDOMWindow::PostMessage(RefPtr<html::SerializedScriptValue> &&message,
                                                     const WindowPostMessageOptions &options) noexcept
  {
    // 1. Let targetRealm be targetWindow's realm.
    // 2. Let incumbentSettings be the incumbent settings object.
    // (Messages come from the window itself, so it's the incumbent.)
    auto *document = Document();
    if (!document)
    {
      return {};
    }

    // 3. Let targetOrigin be options["targetOrigin"].
    Maybe<dom::USVString> targetOrigin;
    utf8_stringview requested = options.TargetOrigin;
    // 4. If targetOrigin is a single U+002F SOLIDUS character (/), then set targetOrigin to
    //    incumbentSettings's origin.
    if (requested == u8"/")
    {
      targetOrigin = Origin();
    }
    // 5. Otherwise, if targetOrigin is not a single U+002A ASTERISK character (*):
    else if (requested != u8"*")
    {
      // 5a. Let parsedURL be the result of running the URL parser on targetOrigin.
      url::URL parsedURL(requested);
      // 5b. If parsedURL is failure, then throw a "SyntaxError" DOMException.
      if (!parsedURL.IsValid())
      {
        return dom::ExceptionCode::SyntaxError;
      }
      // 5c. Set targetOrigin to parsedURL's origin.
      targetOrigin = html::URLDecomposition::SerializedOrigin(parsedURL);
    }

    // 6. Let transfer be options["transfer"].
    // 7. Let serializeWithTransferResult be StructuredSerializeWithTransfer(message, transfer).
    // (There's no structured serialization or transfer: the message is passed as it is.)
    // FIXME: Message ports and transfers, once there are any.

    // 8b. Let origin be incumbentSettings's origin.
    // 8c. Let source be the WindowProxy object corresponding to incumbentSettings's global object.
    // (Both are taken now, while the incumbent is the one posting.)
    dom::USVString origin = Origin();
    RefPtr<LocalDOMWindow> source = ShareRefPtr(this);

    // 8. Queue a global task on the posted message task source given targetWindow to run the following steps:
    html::EventLoop::Current().QueueTask(
      html::TaskSource::PostedMessage, *document,
      [protectedThis = ShareRef(*this), targetOrigin = krys::move(targetOrigin), origin = krys::move(origin),
       source = krys::move(source), message = krys::move(message)]() mutable
      {
        // 8a. If the targetOrigin argument is not a single literal U+002A ASTERISK character (*) and
        //     targetWindow's associated Document's origin is not same origin with targetOrigin, then return.
        auto *document = protectedThis->Document();
        if (!document || (targetOrigin && *targetOrigin != protectedThis->Origin()))
        {
          return;
        }
        // 8d.-8f. Let deserializeRecord be StructuredDeserializeWithTransfer(serializeWithTransferResult,
        //         targetRealm). [...] Let newPorts be a new frozen array consisting of all MessagePort
        //         objects in deserializeRecord.[[TransferredValues]], if any, maintaining their relative
        //         order.
        // (There's nothing to deserialize, and no ports.)
        // 8g. Fire an event named message at targetWindow, using MessageEvent, with its origin initialized to
        //     origin, the source attribute initialized to source, the data attribute initialized to
        //     messageClone, and the ports attribute initialized to newPorts.
        auto event = html::MessageEvent::Create(krys::move(message), krys::move(origin), krys::move(source));
        (void)dom::EventDispatcher::DispatchToTarget(*event, protectedThis.get());
      });
    return {};
  }

  // https://html.spec.whatwg.org/#dom-settimeout
  int32 LocalDOMWindow::SetTimeout(Func<void()> &&handler, int32 timeout) noexcept
  {
    // Return the result of running the timer initialization steps given this, handler, timeout, arguments,
    // and false.
    auto *document = Document();
    if (!document)
    {
      return 0;
    }
    // 5. If timeout is less than 0, then set timeout to 0.
    return DOMTimer::Install(*document, krys::move(handler), std::chrono::milliseconds(std::max(timeout, 0)),
                             DOMTimer::Type::SingleShot);
  }

  // https://html.spec.whatwg.org/#dom-cleartimeout
  void LocalDOMWindow::ClearTimeout(int32 id) noexcept
  {
    // Remove this's map of setTimeout and setInterval IDs[id].
    if (auto *document = Document())
    {
      DOMTimer::RemoveById(*document, id);
    }
  }

  // https://html.spec.whatwg.org/#dom-setinterval
  int32 LocalDOMWindow::SetInterval(Func<void()> &&handler, int32 timeout) noexcept
  {
    // Return the result of running the timer initialization steps given this, handler, timeout, arguments,
    // and true.
    auto *document = Document();
    if (!document)
    {
      return 0;
    }
    // 5. If timeout is less than 0, then set timeout to 0.
    return DOMTimer::Install(*document, krys::move(handler), std::chrono::milliseconds(std::max(timeout, 0)),
                             DOMTimer::Type::Repeating);
  }

  // https://html.spec.whatwg.org/#dom-clearinterval
  void LocalDOMWindow::ClearInterval(int32 id) noexcept
  {
    // Remove this's map of setTimeout and setInterval IDs[id].
    if (auto *document = Document())
    {
      DOMTimer::RemoveById(*document, id);
    }
  }

  // https://html.spec.whatwg.org/#dom-queuemicrotask
  void LocalDOMWindow::QueueMicrotask(Func<void()> &&callback) noexcept
  {
    // Queue a microtask to invoke callback with « » and "report".
    // (C++ callbacks don't throw.)
    html::EventLoop::Current().QueueMicrotask(krys::move(callback));
  }

  int32 LocalDOMWindow::RequestAnimationFrame(dom::FrameRequestCallback &&callback) noexcept
  {
    auto *document = Document();
    return document ? document->RequestAnimationFrame(krys::move(callback)) : 0;
  }

  void LocalDOMWindow::CancelAnimationFrame(int32 handle) noexcept
  {
    if (auto *document = Document())
    {
      document->CancelAnimationFrame(handle);
    }
  }

  int32 LocalDOMWindow::RequestIdleCallback(dom::IdleRequestCallback &&callback,
                                            const dom::IdleRequestOptions &options) noexcept
  {
    auto *document = Document();
    return document
             ? document->RequestIdleCallback(krys::move(callback), std::chrono::milliseconds(options.Timeout))
             : 0;
  }

  void LocalDOMWindow::CancelIdleCallback(int32 handle) noexcept
  {
    if (auto *document = Document())
    {
      document->CancelIdleCallback(handle);
    }
  }

  dom::DOMHighResTimeStamp LocalDOMWindow::NowTimestamp() const noexcept
  {
    return std::chrono::duration<double, std::milli>(html::EventLoop::Current().Now() - _timeOrigin).count();
  }

  void LocalDOMWindow::FreezeNowTimestamp() noexcept
  {
    _frozenNowTimestamp = NowTimestamp();
  }

  void LocalDOMWindow::UnfreezeNowTimestamp() noexcept
  {
    _frozenNowTimestamp = null;
  }

  dom::DOMHighResTimeStamp LocalDOMWindow::FrozenNowTimestamp() const noexcept
  {
    return _frozenNowTimestamp.value_or(NowTimestamp());
  }

  DOMSelection *LocalDOMWindow::GetSelection() const noexcept
  {
    // Return the associated Document's selection: the same object as document.getSelection(), which is null
    // without a browsing context.
    if (!IsCurrentlyDisplayedInFrame())
    {
      return nullptr;
    }
    return Document()->GetSelection();
  }
}
