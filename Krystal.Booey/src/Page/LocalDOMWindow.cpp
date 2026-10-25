#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/CSSComputedStyleDeclaration.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Internals/HTMLCollection/LiveHTMLCollection.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcherManager.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/ErrorEvent.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/MessageEvent.hpp"
#include "Krystal.Booey/HTML/URLDecomposition.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
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
#include "Krystal.Booey/Storage/Storage.hpp"
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
  }

  LocalDOMWindow::LocalDOMWindow(dom::Document &document) noexcept
      : EventTarget(dom::EventTargetFlags::IsWindow), _document(WeakPtr<dom::Document>::WithRef(document)),
        _timeOrigin(html::EventLoop::Current().Now()),
        _lastActivationTimestamp(std::numeric_limits<double>::infinity()),
        _lastHistoryActionActivationTimestamp(std::numeric_limits<double>::infinity())
  {
    krys_assert(document.Frame());
    AddLanguageChangeObserver(this, [this] { LanguagesChanged(); });
    AllWindows().push_back(this);
  }

  LocalDOMWindow::~LocalDOMWindow() noexcept
  {
    RemoveLanguageChangeObserver(this);
    std::erase(AllWindows(), this);
  }

  void LocalDOMWindow::ForEachWindow(const Func<void(LocalDOMWindow &)> &function) noexcept
  {
    auto windows = AllWindows();
    for (auto *window : windows)
    {
      function(*window);
    }
  }

  dom::ExceptionOr<Storage *> LocalDOMWindow::SessionStorage() noexcept
  {
    // If this's associated Document's session storage holder is non-null, then return it.
    if (_sessionStorage)
    {
      return _sessionStorage.get();
    }

    // Let map be the result of running obtain a session storage bottle map; if map is failure (an opaque
    // origin, or no browsing session), then throw a "SecurityError" DOMException.
    auto *document = Document();
    auto *page = GetPage();
    auto origin = Origin();
    if (!document || !page || origin == u8"null")
    {
      return dom::ExceptionCode::SecurityError;
    }

    _sessionStorage = Storage::Create(*this, page->SessionStorageArea(origin));
    return _sessionStorage.get();
  }

  dom::ExceptionOr<Storage *> LocalDOMWindow::LocalStorage() noexcept
  {
    if (_localStorage)
    {
      return _localStorage.get();
    }

    auto *document = Document();
    auto origin = Origin();
    if (!document || !Frame() || origin == u8"null")
    {
      return dom::ExceptionCode::SecurityError;
    }

    auto *provider = PlatformStrategies::Singleton().GetStorageProvider();
    StorageProvider &storageProvider = provider ? *provider : MemoryStorageProvider::Singleton();
    _localStorage = Storage::Create(*this, storageProvider.LocalStorageArea(origin));
    return _localStorage.get();
  }

  Storage *LocalDOMWindow::OptionalSessionStorage() const noexcept
  {
    return _sessionStorage.get();
  }

  Storage *LocalDOMWindow::OptionalLocalStorage() const noexcept
  {
    return _localStorage.get();
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

  dom::DOMString LocalDOMWindow::Name() const noexcept
  {
    // If this's navigable is null, then return the empty string.
    auto *frame = Frame();
    if (!frame)
    {
      return {};
    }
    // Return this's navigable's target name.
    return frame->SpecifiedName();
  }

  void LocalDOMWindow::SetName(dom::DOMString &&name) noexcept
  {
    // If this's navigable is null, then return.
    auto *frame = Frame();
    if (!frame)
    {
      return;
    }
    // Set this's navigable's active session history entry's document state's navigable target name to the
    // given value.
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

  void LocalDOMWindow::Close() noexcept
  {
    // Let thisTraversable be this's navigable; if it isn't a top-level traversable (a frame is), or its is
    // closing is true, then return.
    RefPtr<Page> page = ShareRefPtr(GetPage());
    if (!page || page->IsClosing())
    {
      return;
    }

    // FIXME: Only if the page is script-closable (opened by script, or with one session history entry, which
    // a page always has until there's history (B3c)); a warning on the console otherwise.

    // The page may ask to stay.
    if (auto *frame = Frame(); frame && !frame->Loader().ShouldClose())
    {
      return;
    }

    // Set thisTraversable's is closing to true, and queue a task on the DOM manipulation task source to
    // definitely close thisTraversable.
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

  void LocalDOMWindow::Stop() noexcept
  {
    // If this's navigable is null, then return.
    if (!Frame())
    {
      return;
    }
    // FIXME: Stop loading this's navigable, once there's document loading (B3b).
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

  LocalDOMWindow *LocalDOMWindow::Top() noexcept
  {
    // If this's navigable is null, then return null. Return this's navigable's top-level traversable's active
    // WindowProxy: a frame is the only one in its tree.
    return Frame() ? this : nullptr;
  }

  LocalDOMWindow *LocalDOMWindow::Parent() noexcept
  {
    // If this's navigable is null, then return null. A frame has no parent, so return its active WindowProxy.
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
    // Page zoom applies to the whole page, so it's included in devicePixelRatio.
    // FIXME: The frame scale factor, once frames can be scaled.
    return frame->GetMediaEnvironment().DeviceScaleFactor * frame->PageZoomFactor();
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
    /// @see https://infra.spec.whatwg.org/#normalize-newlines
    KRYS_NODISCARD dom::DOMString NormalizeNewlines(const dom::DOMString &message) noexcept
    {
      dom::DOMString result;
      result.reserve(message.size());
      for (size_t i = 0; i < message.size(); ++i)
      {
        if (message[i] == u8'\r')
        {
          result += u8'\n';
          if (i + 1 < message.size() && message[i + 1] == u8'\n')
          {
            ++i;
          }
          continue;
        }
        result += message[i];
      }
      return result;
    }

    /// @brief Whether the user agent can't show @p window's simple dialogs now.
    /// @see https://html.spec.whatwg.org/#cannot-show-simple-dialogs
    KRYS_NODISCARD bool CannotShowSimpleDialogs(const LocalDOMWindow &window) noexcept
    {
      // If the document's unload counter is greater than 0 (beforeunload, pagehide or unload is being
      // dispatched), return true.
      // FIXME: The sandboxed modals flag, once there's sandboxing.
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

  void LocalDOMWindow::Alert(const dom::DOMString &message) noexcept
  {
    // If we cannot show simple dialogs for this, then return.
    if (CannotShowSimpleDialogs(*this))
    {
      return;
    }

    // Set message to the result of normalizing newlines given message, and show it to the user.
    (void)Document()->UpdateStyleIfNeeded();
    GetPage()->GetPointerLockController().RequestPointerUnlock();
    GetPage()->GetChromeClient().RunJavaScriptAlert(*Frame(), NormalizeNewlines(message));
  }

  bool LocalDOMWindow::Confirm(const dom::DOMString &message) noexcept
  {
    if (CannotShowSimpleDialogs(*this))
    {
      return false;
    }

    (void)Document()->UpdateStyleIfNeeded();
    GetPage()->GetPointerLockController().RequestPointerUnlock();
    return GetPage()->GetChromeClient().RunJavaScriptConfirm(*Frame(), NormalizeNewlines(message));
  }

  Maybe<dom::DOMString> LocalDOMWindow::Prompt(const dom::DOMString &message,
                                               const dom::DOMString &defaultValue) noexcept
  {
    if (CannotShowSimpleDialogs(*this))
    {
      return null;
    }

    (void)Document()->UpdateStyleIfNeeded();
    GetPage()->GetPointerLockController().RequestPointerUnlock();
    return GetPage()->GetChromeClient().RunJavaScriptPrompt(*Frame(), NormalizeNewlines(message),
                                                            defaultValue);
  }

  void LocalDOMWindow::Print() noexcept
  {
    // https://html.spec.whatwg.org/#printing-steps
    RefPtr<LocalFrame> frame = ShareRefPtr(Frame());
    if (!frame || !GetPage())
    {
      return;
    }

    // Not while the document is unloading.
    if (Document()->UnloadCounter() > 0)
    {
      return;
    }

    // FIXME: Printing once the document has loaded (shouldPrintWhenFinishedLoading).
    (void)dom::EventDispatcher::FireEvent(dom::DOMStringAtom(EventNames::BeforePrint), *this);
    if (auto *page = GetPage())
    {
      page->GetChromeClient().Print(*frame);
    }
    (void)dom::EventDispatcher::FireEvent(dom::DOMStringAtom(EventNames::AfterPrint), *this);
  }

  dom::ExceptionOr<LocalDOMWindow *> LocalDOMWindow::Open(utf8_stringview urlString,
                                                          dom::DOMStringAtom target,
                                                          dom::DOMStringView featuresString) noexcept
  {
    // https://html.spec.whatwg.org/#window-open-steps
    if (!IsCurrentlyDisplayedInFrame())
    {
      return static_cast<LocalDOMWindow *>(nullptr);
    }

    RefPtr<dom::Document> document = ShareRefPtr(Document());
    RefPtr<LocalFrame> frame = ShareRefPtr(Frame());

    // If url is not the empty string, then parse it relative to the entry settings object; on failure, throw
    // a "SyntaxError" DOMException.
    url::URL urlRecord;
    if (!urlString.empty())
    {
      urlRecord = document->CompleteURL(urlString);
      if (!urlRecord.IsValid())
      {
        return dom::ExceptionCode::SyntaxError;
      }
    }

    // If target is the empty string, then set target to "_blank".
    if (target.View().empty())
    {
      target = dom::DOMStringAtom(u8"_blank");
    }

    auto features = ParseWindowFeatures(featuresString);
    auto *targetFrame = frame->Loader().FindFrameForNavigation(target.View());

    // A new window needs transient activation: popups are blocked otherwise.
    if (!targetFrame && !HasTransientActivation())
    {
      return static_cast<LocalDOMWindow *>(nullptr);
    }

    auto referrerPolicy =
      features.WantsNoReferrer() ? ReferrerPolicy::NoReferrer : ReferrerPolicy::EmptyString;

    if (targetFrame)
    {
      // An existing browsing context: navigate it, unless there's no URL.
      if (!urlRecord.IsNull())
      {
        frame->Loader().ChangeLocation(urlRecord, dom::DOMStringAtom(u8"_self"), nullptr, referrerPolicy,
                                       null, dom::DOMStringAtom::Null());
      }
      return this;
    }

    // A new browsing context: the host opens it, asked by the client's new window policy.
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

  void LocalDOMWindow::NotifyActivated() noexcept
  {
    // Set the last activation timestamp of the window to the current high resolution time. There are no
    // ancestor or descendant frames to notify.
    _lastActivationTimestamp = NowTimestamp();
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

  bool LocalDOMWindow::ConsumeTransientActivation() noexcept
  {
    if (!HasTransientActivation())
    {
      return false;
    }
    // For each window in the frame tree (this one), if its last activation timestamp is not positive
    // infinity, set it to negative infinity.
    if (_lastActivationTimestamp != std::numeric_limits<double>::infinity())
    {
      _lastActivationTimestamp = -std::numeric_limits<double>::infinity();
    }
    return true;
  }

  bool LocalDOMWindow::ConsumeHistoryActionUserActivation() noexcept
  {
    if (!HasHistoryActionActivation())
    {
      return false;
    }
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
    // https://html.spec.whatwg.org/#report-an-exception
    // If target is in error reporting mode, then return.
    if (_errorReportingMode)
    {
      return;
    }

    // Set target's error reporting mode to true, and fire an event named error at target, using ErrorEvent,
    // with the cancelable attribute initialized to true.
    // FIXME: The script's location (filename, line and column), once there's scripting.
    _errorReportingMode = true;
    Ref<dom::Event> event = html::ErrorEvent::Create(dom::DOMString(message), {}, 0, 0, krys::move(error));
    Ref<LocalDOMWindow> protectedThis = ShareRef(*this);
    bool notHandled = dom::EventDispatcher::DispatchToTarget(*event, this);
    _errorReportingMode = false;

    // If notHandled is true, then report the exception to the developer console.
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

  RefPtr<html::SerializedScriptValue>
    LocalDOMWindow::StructuredClone(html::SerializedScriptValue *value) const noexcept
  {
    return value ? RefPtr<html::SerializedScriptValue>(value->Clone()) : nullptr;
  }

  dom::ExceptionOr<void> LocalDOMWindow::PostMessage(RefPtr<html::SerializedScriptValue> &&message,
                                                     utf8_stringview targetOrigin) noexcept
  {
    WindowPostMessageOptions options;
    options.TargetOrigin = dom::USVString(targetOrigin);
    return PostMessage(krys::move(message), options);
  }

  dom::ExceptionOr<void> LocalDOMWindow::PostMessage(RefPtr<html::SerializedScriptValue> &&message,
                                                     const WindowPostMessageOptions &options) noexcept
  {
    // https://html.spec.whatwg.org/#window-post-message-steps
    // The incumbent settings object is this window's: messages come from the window itself.
    auto *document = Document();
    if (!document)
    {
      return {};
    }

    // Let targetOrigin be options["targetOrigin"]. If it's "/", set it to the incumbent's origin. Otherwise,
    // if it isn't "*", parse it; on failure, throw a "SyntaxError"; set it to the parsed URL's origin.
    Maybe<dom::USVString> targetOrigin;
    utf8_stringview requested = options.TargetOrigin;
    if (requested == u8"/")
    {
      targetOrigin = Origin();
    }
    else if (requested != u8"*")
    {
      url::URL parsedURL(requested);
      if (!parsedURL.IsValid())
      {
        return dom::ExceptionCode::SyntaxError;
      }
      targetOrigin = html::URLDecomposition::SerializedOrigin(parsedURL);
    }

    // Let origin be the serialization of incumbentSettings's origin, and source be its WindowProxy. Queue a
    // global task on the posted message task source to fire the message, unless the target's origin differs.
    // FIXME: Message ports and transfers, once there are any.
    auto *globalDocument = Document();
    if (!globalDocument)
    {
      return {};
    }
    html::EventLoop::Current().QueueTask(
      html::TaskSource::PostedMessage, *globalDocument,
      [protectedThis = ShareRef(*this), targetOrigin = krys::move(targetOrigin),
       message = krys::move(message)]() mutable
      {
        auto *document = protectedThis->Document();
        if (!document || (targetOrigin && *targetOrigin != protectedThis->Origin()))
        {
          return;
        }
        auto event = html::MessageEvent::Create(krys::move(message), protectedThis->Origin(),
                                                RefPtr<LocalDOMWindow>(protectedThis));
        (void)dom::EventDispatcher::DispatchToTarget(*event, protectedThis.get());
      });
    return {};
  }

  int32 LocalDOMWindow::SetTimeout(Func<void()> &&handler, int32 timeout) noexcept
  {
    auto *document = Document();
    if (!document)
    {
      return 0;
    }
    return DOMTimer::Install(*document, krys::move(handler), std::chrono::milliseconds(std::max(timeout, 0)),
                             DOMTimer::Type::SingleShot);
  }

  void LocalDOMWindow::ClearTimeout(int32 id) noexcept
  {
    if (auto *document = Document())
    {
      DOMTimer::RemoveById(*document, id);
    }
  }

  int32 LocalDOMWindow::SetInterval(Func<void()> &&handler, int32 timeout) noexcept
  {
    auto *document = Document();
    if (!document)
    {
      return 0;
    }
    return DOMTimer::Install(*document, krys::move(handler), std::chrono::milliseconds(std::max(timeout, 0)),
                             DOMTimer::Type::Repeating);
  }

  void LocalDOMWindow::ClearInterval(int32 id) noexcept
  {
    if (auto *document = Document())
    {
      DOMTimer::RemoveById(*document, id);
    }
  }

  void LocalDOMWindow::QueueMicrotask(Func<void()> &&callback) noexcept
  {
    // Queue a microtask to invoke callback; if it throws an exception, report it (C++ callbacks don't).
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

  double LocalDOMWindow::NowTimestamp() const noexcept
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

  double LocalDOMWindow::FrozenNowTimestamp() const noexcept
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
