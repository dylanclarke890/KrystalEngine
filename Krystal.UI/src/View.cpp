#include "Krystal.UI/View.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/History/BackForwardCache.hpp"
#include "Krystal.Booey/History/BackForwardController.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Loader/DocumentWriter.hpp"
#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Loader/NavigationAction.hpp"
#include "Krystal.Booey/Loader/SubstituteData.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Navigator.hpp"
#include "Krystal.Booey/Page/PointerCaptureController.hpp"
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.UI/ColorChooserOverlay.hpp"
#include "Krystal.UI/Compositing/GraphicsLayerTextureMapper.hpp"
#include "Krystal.UI/DateTimeChooserOverlay.hpp"
#include "Krystal.UI/EditorClient.hpp"
#include "Krystal.UI/FileLoaderStrategy.hpp"
#include "Krystal.UI/GraphicsContextRender2D.hpp"
#include "Krystal.UI/Input.hpp"
#include "Krystal.UI/PopupMenuOverlay.hpp"
#include "Krystal.UI/SystemPasteboardStrategy.hpp"
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <limits>
#include <mutex>
#include <utility>

namespace krys::ui
{
  /// @brief Hands Booey the graphics layers the view composites, and keeps the root of the page's layer tree.
  class View::ChromeClient final : public boo::ChromeClient
  {
  private:
    View *_view {nullptr};
    GraphicsLayerFactoryTextureMapper _factory;
    GraphicsLayerTextureMapper *_rootLayer {nullptr};
    Func<void()> _focusRequestHandler;
    Func<void()> _closeRequestHandler;
    Func<void(const boo::FloatRect &)> _windowRectRequestHandler;
    Maybe<boo::FloatRect> _windowRect;
    Func<void(const utf8_string &)> _alertHandler;
    Func<bool(const utf8_string &)> _confirmHandler;
    Func<Maybe<utf8_string>(const utf8_string &, const utf8_string &)> _promptHandler;
    Func<void()> _printHandler;
    Func<void(const ConsoleMessage &)> _consoleMessageHandler;
    Func<bool(const utf8_string &)> _beforeUnloadHandler;
    Func<void(const utf8_string &, const utf8_string &, bool)> _protocolHandlerRequestHandler;

  public:
    void RegisterProtocolHandler(const boo::dom::DOMString &scheme,
                                 const boo::dom::DOMString &url) noexcept override
    {
      if (_protocolHandlerRequestHandler)
      {
        _protocolHandlerRequestHandler(scheme, url, true);
      }
    }

    void UnregisterProtocolHandler(const boo::dom::DOMString &scheme,
                                   const boo::dom::DOMString &url) noexcept override
    {
      if (_protocolHandlerRequestHandler)
      {
        _protocolHandlerRequestHandler(scheme, url, false);
      }
    }

    void SetProtocolHandlerRequestHandler(
      Func<void(const utf8_string &, const utf8_string &, bool)> &&handler) noexcept
    {
      _protocolHandlerRequestHandler = krys::move(handler);
    }

    void RunJavaScriptAlert(boo::LocalFrame &, const boo::dom::DOMString &message) noexcept override
    {
      if (_alertHandler)
      {
        _alertHandler(message);
      }
    }

    KRYS_NODISCARD bool RunJavaScriptConfirm(boo::LocalFrame &,
                                             const boo::dom::DOMString &message) noexcept override
    {
      return _confirmHandler && _confirmHandler(message);
    }

    KRYS_NODISCARD Maybe<boo::dom::DOMString>
      RunJavaScriptPrompt(boo::LocalFrame &, const boo::dom::DOMString &message,
                          const boo::dom::DOMString &defaultValue) noexcept override
    {
      if (!_promptHandler)
      {
        return null;
      }
      auto result = _promptHandler(message, defaultValue);
      return result ? Maybe<boo::dom::DOMString>(boo::dom::DOMString(*result)) : null;
    }

    KRYS_NODISCARD bool RunBeforeUnloadConfirmPanel(const boo::dom::DOMString &message,
                                                    boo::LocalFrame &) noexcept override
    {
      return !_beforeUnloadHandler || _beforeUnloadHandler(message);
    }

    void SetBeforeUnloadHandler(Func<bool(const utf8_string &)> &&handler) noexcept
    {
      _beforeUnloadHandler = krys::move(handler);
    }

    void Print(boo::LocalFrame &) noexcept override
    {
      if (_printHandler)
      {
        _printHandler();
      }
    }

    void AddMessageToConsole(boo::MessageSource source, boo::MessageLevel level,
                             const boo::dom::DOMString &message, uint32 lineNumber, uint32 columnNumber,
                             const boo::dom::DOMString &sourceID) noexcept override
    {
      if (_consoleMessageHandler)
      {
        _consoleMessageHandler(ConsoleMessage {source, level, message, sourceID, lineNumber, columnNumber});
      }
    }

    void SetAlertHandler(Func<void(const utf8_string &)> &&handler) noexcept
    {
      _alertHandler = krys::move(handler);
    }

    void SetConfirmHandler(Func<bool(const utf8_string &)> &&handler) noexcept
    {
      _confirmHandler = krys::move(handler);
    }

    void
      SetPromptHandler(Func<Maybe<utf8_string>(const utf8_string &, const utf8_string &)> &&handler) noexcept
    {
      _promptHandler = krys::move(handler);
    }

    void SetPrintHandler(Func<void()> &&handler) noexcept
    {
      _printHandler = krys::move(handler);
    }

    void SetConsoleMessageHandler(Func<void(const ConsoleMessage &)> &&handler) noexcept
    {
      _consoleMessageHandler = krys::move(handler);
    }

    KRYS_NODISCARD boo::FloatRect WindowRect() const noexcept override
    {
      if (_windowRect)
      {
        return *_windowRect;
      }
      return _view ? boo::FloatRect(boo::FloatPoint(), boo::FloatSize(_view->ViewportSize()))
                   : boo::FloatRect();
    }

    void SetWindowRect(const boo::FloatRect &rect) noexcept override
    {
      if (_windowRectRequestHandler)
      {
        _windowRectRequestHandler(rect);
      }
    }

    void SetHostWindowRect(const boo::FloatRect &rect) noexcept
    {
      _windowRect = rect;
    }

    void SetWindowRectRequestHandler(Func<void(const boo::FloatRect &)> &&handler) noexcept
    {
      _windowRectRequestHandler = krys::move(handler);
    }

    void Focus() noexcept override
    {
      if (_focusRequestHandler)
      {
        _focusRequestHandler();
      }
    }

    void CloseWindow() noexcept override
    {
      if (_closeRequestHandler)
      {
        _closeRequestHandler();
      }
    }

    void SetFocusRequestHandler(Func<void()> &&handler) noexcept
    {
      _focusRequestHandler = krys::move(handler);
    }

    void SetCloseRequestHandler(Func<void()> &&handler) noexcept
    {
      _closeRequestHandler = krys::move(handler);
    }

    void AttachRootGraphicsLayer(boo::LocalFrame &, boo::GraphicsLayer *layer) noexcept override
    {
      _rootLayer = layer ? &Downcast<GraphicsLayerTextureMapper>(*layer) : nullptr;
    }

    KRYS_NODISCARD boo::GraphicsLayerFactory *GetGraphicsLayerFactory() const noexcept override
    {
      return const_cast<GraphicsLayerFactoryTextureMapper *>(&_factory);
    }

    KRYS_NODISCARD RefPtr<boo::PopupMenu>
      CreatePopupMenu(boo::PopupMenuClient &client) const noexcept override
    {
      if (!_view)
      {
        return nullptr;
      }
      return CreateRef<PopupMenuOverlay>(*_view, client);
    }

    KRYS_NODISCARD RefPtr<boo::ColorChooser>
      CreateColorChooser(boo::ColorChooserClient &client, const boo::Color &initialColor) noexcept override
    {
      if (!_view)
      {
        return nullptr;
      }
      return CreateRef<ColorChooserOverlay>(*_view, client, initialColor);
    }

    KRYS_NODISCARD RefPtr<boo::DateTimeChooser>
      CreateDateTimeChooser(boo::DateTimeChooserClient &client) noexcept override
    {
      if (!_view)
      {
        return nullptr;
      }
      return CreateRef<DateTimeChooserOverlay>(*_view, client);
    }

    KRYS_NODISCARD bool HasAccessoryMousePointingDevice() const noexcept override
    {
      return _view && _view->_pointingDevices.Mouse;
    }

    void RequestPointerLock(Func<void(boo::PointerLockRequestResult)> &&completionHandler) noexcept override
    {
      completionHandler(_view ? _view->LockPointer() : boo::PointerLockRequestResult::Unsupported);
    }

    void RequestPointerUnlock(Func<void(bool)> &&completionHandler) noexcept override
    {
      completionHandler(_view && _view->UnlockPointer());
    }

    void SetView(View *view) noexcept
    {
      _view = view;
    }

    KRYS_NODISCARD GraphicsLayerTextureMapper *RootLayer() const noexcept
    {
      return _rootLayer;
    }
  };

  /// @brief Asks the host's navigation handler about the main frame's navigations.
  class View::FrameLoaderClient final : public boo::LocalFrameLoaderClient
  {
  private:
    View *_view {nullptr};
    Func<NavigationPolicy(const NavigationRequest &)> _navigationHandler;
    Maybe<utf8_string> _userAgent;
    Func<void(const LoadEvent &)> _loadHandler;

  public:
    void DispatchDidClearWindowObject() noexcept override
    {
      if (_view)
      {
        _view->DidClearWindowObject();
      }
    }

    void DispatchDidStartProvisionalLoad() noexcept override
    {
      Notify({LoadEventType::Started, {}});
    }

    void DispatchDidCommitLoad() noexcept override
    {
      // A page from the back/forward cache commits once it's restored, so the host sees its document.
      if (_view && _view->Frame().Loader().IsLoadingFromCachedPage())
      {
        return;
      }
      if (_view)
      {
        _view->DidCommitLoad();
      }
      Notify({LoadEventType::Committed, {}});
    }

    void DidRestoreFromBackForwardCache() noexcept override
    {
      if (_view)
      {
        _view->DidCommitLoad();
      }
      Notify({LoadEventType::Committed, {}, true});
    }

    void DispatchDidFinishLoad() noexcept override
    {
      Notify({LoadEventType::Finished, {}});
    }

    void DispatchDidFailProvisionalLoad(const boo::ResourceError &error) noexcept override
    {
      Notify({LoadEventType::Failed, error});
    }

    void DispatchDidFailLoad(const boo::ResourceError &error) noexcept override
    {
      Notify({LoadEventType::Failed, error});
    }

    void SetLoadHandler(Func<void(const LoadEvent &)> &&handler) noexcept
    {
      _loadHandler = krys::move(handler);
    }

    void Notify(const LoadEvent &event) const noexcept
    {
      if (_loadHandler)
      {
        _loadHandler(event);
      }
    }

    KRYS_NODISCARD boo::dom::DOMString UserAgent(const boo::url::URL &url) const noexcept override
    {
      return _userAgent ? boo::dom::DOMString(*_userAgent) : boo::LocalFrameLoaderClient::UserAgent(url);
    }

    void SetUserAgent(utf8_string &&userAgent) noexcept
    {
      _userAgent = krys::move(userAgent);
    }

    KRYS_NODISCARD boo::PolicyAction
      DispatchDecidePolicyForNavigationAction(const boo::NavigationAction &action,
                                              const boo::ResourceRequest &request,
                                              const boo::FormSubmission *formSubmission) noexcept override
    {
      return Decide(action, request, formSubmission, {});
    }

    KRYS_NODISCARD boo::PolicyAction DispatchDecidePolicyForNewWindowAction(
      const boo::NavigationAction &action, const boo::ResourceRequest &request,
      const boo::FormSubmission *formSubmission, boo::dom::DOMStringView frameName) noexcept override
    {
      return Decide(action, request, formSubmission, frameName);
    }

    void SetView(View *view) noexcept
    {
      _view = view;
    }

    void SetNavigationHandler(Func<NavigationPolicy(const NavigationRequest &)> &&handler) noexcept
    {
      _navigationHandler = krys::move(handler);
    }

  private:
    /// @param newWindowName The browsing context a new window would be; null (a null view) for the view's
    /// own.
    KRYS_NODISCARD boo::PolicyAction Decide(const boo::NavigationAction &action,
                                            const boo::ResourceRequest &request,
                                            const boo::FormSubmission *formSubmission,
                                            boo::dom::DOMStringView newWindowName) const noexcept
    {
      // The host's own loads aren't asked about.
      if (!_navigationHandler || action.IsRequestFromClientOrUserInput())
      {
        return boo::PolicyAction::Use;
      }

      NavigationRequest navigation;
      navigation.URL = request.URL().String();
      navigation.OpensNewWindow = newWindowName.data() != nullptr;
      navigation.Target = utf8_string(newWindowName);
      navigation.SourceElement = ShareRefPtr(action.SourceElement());
      if (auto &mouse = action.GetMouseEventData())
      {
        navigation.Button = mouse->Button;
      }
      if (auto &keys = action.KeyStateEventData())
      {
        navigation.ShiftKey = keys->ShiftKey;
        navigation.CtrlKey = keys->CtrlKey;
        navigation.AltKey = keys->AltKey;
        navigation.MetaKey = keys->MetaKey;
      }
      if (action.DownloadAttribute() != boo::dom::DOMStringAtom::Null())
      {
        navigation.Download = utf8_string(action.DownloadAttribute().View());
      }
      navigation.Features = action.GetWindowFeatures();
      navigation.Method = request.HTTPMethod();
      if (formSubmission)
      {
        navigation.IsFormSubmission = true;
        navigation.Target = formSubmission->Target();
        if (!navigation.SourceElement)
        {
          navigation.SourceElement = ShareRefPtr<boo::dom::Element>(formSubmission->Form());
        }
        if (formSubmission->GetMethod() == boo::FormSubmission::Method::Post)
        {
          navigation.ContentType = formSubmission->ContentType();
          if (!formSubmission->Boundary().empty())
          {
            navigation.ContentType += u8"; boundary=" + formSubmission->Boundary();
          }
          if (auto *data = formSubmission->Data())
          {
            navigation.Body = data->FlattenToString();
          }
        }
      }
      if (_view && !navigation.OpensNewWindow && navigation.Method == u8"GET")
      {
        navigation.IsSameDocument =
          !boo::FrameLoader::ShouldReload(_view->Document().GetURL(), request.URL());
      }

      return _navigationHandler(navigation) == NavigationPolicy::Allow ? boo::PolicyAction::Use
                                                                       : boo::PolicyAction::Ignore;
    }
  };

  namespace
  {
    /// @brief The main frame's loader client, noted in @p frameLoaderClient as it's handed to the page.
    KRYS_NODISCARD UniquePtr<boo::LocalFrameLoaderClient>
      CreateFrameLoaderClient(View::FrameLoaderClient *&frameLoaderClient) noexcept
    {
      auto client = CreateUnique<View::FrameLoaderClient>();
      frameLoaderClient = client.get();
      return client;
    }

    /// @brief The page's chrome client, noted in @p chromeClient as it's handed to the page.
    KRYS_NODISCARD UniquePtr<boo::ChromeClient> CreateChromeClient(View::ChromeClient *&chromeClient) noexcept
    {
      auto client = CreateUnique<View::ChromeClient>();
      chromeClient = client.get();
      return client;
    }
  }

  View::View(const boo::IntSize &viewportSize) noexcept
      : _backForwardList(boo::BackForwardList::Create()),
        _page(boo::Page::Create(CreateChromeClient(_chromeClient), CreateUnique<EditorClient>(),
                                CreateFrameLoaderClient(_frameLoaderClient), _backForwardList)),
        _frame(ShareRef(_page->MainFrame()))
  {
    static std::once_flag valuePool;
    std::call_once(valuePool, [] { boo::css::StaticValuePool::Init(); });

    // The back/forward cache keeps a few pages, unless the host sizes it.
    static std::once_flag backForwardCache;
    std::call_once(backForwardCache,
                   [] { boo::BackForwardCache::Singleton().SetMaxSize(DefaultBackForwardCacheCapacity); });

    auto &strategies = boo::PlatformStrategies::Singleton();
    if (!strategies.GetLoaderStrategy())
    {
      static FileLoaderStrategy fileLoader;
      strategies.SetLoaderStrategy(&fileLoader);
    }
    if (!strategies.GetPasteboardStrategy())
    {
      static SystemPasteboardStrategy systemPasteboard;
      strategies.SetPasteboardStrategy(&systemPasteboard);
    }

    _chromeClient->SetView(this);
    _frameLoaderClient->SetView(this);
    _frame->CreateView(viewportSize);
    // The frame starts with its initial empty document, at once.
    _frame->Loader().Init();
  }

  View::~View() noexcept
  {
    if (_overlay)
    {
      _overlay->Hide();
    }
    _chromeClient->SetView(nullptr);
    _frameLoaderClient->SetView(nullptr);
    // The layers go before the texture mapper that holds their textures.
    _frame->SetDocument(nullptr);
  }

  boo::dom::HTMLDocument &View::Document() const noexcept
  {
    return Downcast<boo::dom::HTMLDocument>(*_frame->Document());
  }

  void View::LoadHTML(utf8_string &&markup, boo::url::URL &&url, boo::CachePolicy cachePolicy) noexcept
  {
    _frame->SetSubresourceCachePolicy(cachePolicy);
    auto requestURL = url.IsNull() ? boo::url::URL(u8"about:blank") : url;
    boo::FrameLoadRequest request(ShareRef<boo::dom::Document>(Document()),
                                  boo::ResourceRequest(krys::move(requestURL)),
                                  boo::dom::DOMStringAtom::Null());
    request.SetSubstituteData(boo::SubstituteData(krys::move(markup), u8"text/html", krys::move(url)));
    _frame->Loader().Load(krys::move(request));
  }

  Expected<void> View::LoadFile(const std::filesystem::path &path, boo::CachePolicy cachePolicy) noexcept
  {
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error))
    {
      return Unexpected(std::format("Couldn't open {}", path.string()));
    }
    auto absolutePath = std::filesystem::absolute(path).generic_u8string();
    Load(boo::url::URL::FileURLWithFileSystemPath(absolutePath), cachePolicy);
    return {};
  }

  void View::Load(boo::url::URL &&url, boo::CachePolicy cachePolicy) noexcept
  {
    _frame->SetSubresourceCachePolicy(cachePolicy);
    boo::FrameLoadRequest request(ShareRef<boo::dom::Document>(Document()),
                                  boo::ResourceRequest(krys::move(url)), boo::dom::DOMStringAtom::Null());
    _frame->Loader().Load(krys::move(request));
  }

  bool View::IsLoading() const noexcept
  {
    return _frame->Loader().IsLoading();
  }

  void View::Stop() noexcept
  {
    _frame->Loader().StopForUserCancel();
  }

  void View::SetLoadHandler(Func<void(const LoadEvent &)> &&handler) noexcept
  {
    _frameLoaderClient->SetLoadHandler(krys::move(handler));
  }

  bool View::CanGoBack() const noexcept
  {
    return _page->BackForward().CanGoBackOrForward(-1);
  }

  bool View::CanGoForward() const noexcept
  {
    return _page->BackForward().CanGoBackOrForward(1);
  }

  bool View::GoBack() noexcept
  {
    bool wentBack = _page->BackForward().GoBack();
    PerformMicrotaskCheckpoint();
    return wentBack;
  }

  bool View::GoForward() noexcept
  {
    bool wentForward = _page->BackForward().GoForward();
    PerformMicrotaskCheckpoint();
    return wentForward;
  }

  void View::GoToItem(boo::HistoryItem &item) noexcept
  {
    _page->GoToItem(*_frame, item, boo::FrameLoadType::IndexedBackForward);
    PerformMicrotaskCheckpoint();
  }

  void View::Reload() noexcept
  {
    _frame->Loader().Reload(true);
    PerformMicrotaskCheckpoint();
  }

  void View::DidCommitLoad() noexcept
  {
    // A committed document (a new one, or one the back/forward cache restored) takes the window's focus and
    // is painted; the old one's overlay goes.
    if (_overlay)
    {
      _overlay->Hide();
    }
    Document().SetFocusedAndActive(_focused);
    _needsPaint = true;
  }

  void View::DidClearWindowObject() noexcept
  {
    // A new document: it takes the window's focus, and the view's custom elements are defined before it's
    // parsed, so the parser creates them as it goes.
    if (_overlay)
    {
      _overlay->Hide();
    }
    auto &document = Document();
    document.SetFocusedAndActive(_focused);
    _needsPaint = true;
    for (auto &definition : _customElements)
    {
      (void)document.Window()->CustomElements().Define(definition.Name,
                                                       boo::html::CustomElementFactory(definition.Factory),
                                                       boo::html::CustomElementOptions(definition.Options));
    }
  }

  void View::ReplaceDocument(utf8_string &&markup) noexcept
  {
    // The document is parsed in the frame, as a navigation's is, so it's fully active as its elements are
    // inserted; it replaces the current one at once, without a load.
    boo::DocumentWriter writer;
    writer.SetFrame(*_frame);
    if (!writer.Begin(boo::url::URL(u8"about:blank")))
    {
      return;
    }
    writer.AddData(Span<const uint8>(reinterpret_cast<const uint8 *>(markup.data()), markup.size()));
    writer.End();
  }

  Expected<void> View::DefineCustomElement(boo::dom::DOMStringAtom name,
                                           boo::html::CustomElementFactory &&factory,
                                           boo::html::CustomElementOptions &&options) noexcept
  {
    if (!boo::dom::NameValidation::IsValidCustomElementName(name.View()))
    {
      return Unexpected(std::format("{} isn't a valid custom element name",
                                    std::string(name.View().begin(), name.View().end())));
    }
    for (auto &definition : _customElements)
    {
      if (definition.Name == name)
      {
        return Unexpected(
          std::format("{} is already defined", std::string(name.View().begin(), name.View().end())));
      }
    }

    auto &definition = _customElements.emplace_back(name, krys::move(factory), krys::move(options));
    (void)Document().Window()->CustomElements().Define(definition.Name,
                                                       boo::html::CustomElementFactory(definition.Factory),
                                                       boo::html::CustomElementOptions(definition.Options));
    return {};
  }

  uint32 View::AddFontDirectory(const std::filesystem::path &directory)
  {
    auto count = boo::FontDatabase::Singleton().AddFontDirectory(directory);
    if (count)
    {
      boo::FontCache::InvalidateAllFontCaches();
    }
    return count;
  }

  void View::Resize(const boo::IntSize &viewportSize) noexcept
  {
    // The overlay was placed in the old viewport.
    if (_overlay)
    {
      _overlay->Hide();
    }
    if (auto *view = _frame->View())
    {
      view->Resize(viewportSize);
    }
    _needsPaint = true;
  }

  boo::IntSize View::ViewportSize() const noexcept
  {
    auto *view = _frame->View();
    return view ? boo::IntSize(view->Width(), view->Height()) : boo::IntSize();
  }

  bool View::Update(MonotonicTime::duration now, Maybe<MonotonicTime::duration> budget) noexcept
  {
    auto start = MonotonicTime::Now();
    boo::html::EventLoop::Current().Run(now, budget);

    // The overlay's page shares the event loop, and updates with the page.
    bool overlayUpdated = false;
    if (_overlay && _overlay->ContentView().GetPage().IsRenderingUpdateScheduled())
    {
      _overlay->ContentView().UpdateRendering();
      overlayUpdated = true;
    }

    // Each frame is a rendering opportunity.
    bool updated = overlayUpdated;
    if (_page->IsRenderingUpdateScheduled())
    {
      _page->UpdateRendering();
      _needsPaint = true;
      updated = true;
    }

    // The rest of the frame's budget is idle, up to 50ms (the idle period's limit).
    // https://w3c.github.io/requestidlecallback/#start-an-idle-period-algorithm
    constexpr auto maximumIdlePeriod = std::chrono::milliseconds(50);
    auto deadline = budget ? start + std::min<MonotonicTime::duration>(*budget, maximumIdlePeriod)
                           : MonotonicTime::Now() + maximumIdlePeriod;
    _page->OpportunisticallyRunIdleCallbacks(deadline);
    return updated;
  }

  void View::PerformMicrotaskCheckpoint() noexcept
  {
    // Input is dispatched as a task would be, so a microtask checkpoint follows it.
    // https://html.spec.whatwg.org/#event-loop-processing-model
    boo::html::EventLoop::Current().PerformMicrotaskCheckpoint();
  }

  void View::UpdateRendering() noexcept
  {
    _page->UpdateRendering();
    _needsPaint = true;
  }

  boo::IntPoint View::ScrollPosition() const noexcept
  {
    auto *view = _frame->View();
    return view ? view->GetScrollPosition() : boo::IntPoint();
  }

  void View::ScrollTo(const boo::IntPoint &position) noexcept
  {
    (void)Document().UpdateLayout();
    if (auto *view = _frame->View())
    {
      view->SetScrollPosition(position);
    }
    _needsPaint = true;
  }

  Ref<const boo::display_list::DisplayList> View::Record() noexcept
  {
    // The rendering update runs in Update; painting only needs the layout.
    (void)Document().UpdateLayout();
    const auto size = ViewportSize();
    boo::display_list::RecorderImpl recorder(
      boo::FloatSize(static_cast<float>(size.Width()), static_cast<float>(size.Height())));
    if (auto *view = _frame->View())
    {
      // The composited layers paint into the recording too.
      auto paintBehavior = view->GetPaintBehavior();
      auto flattened = paintBehavior;
      flattened.Add(boo::PaintBehavior::FlattenCompositingLayers);
      view->SetPaintBehavior(flattened);
      view->Paint(recorder, boo::IntRect(0, 0, size.Width(), size.Height()));
      view->SetPaintBehavior(paintBehavior);
    }
    if (_overlay)
    {
      auto overlayList = _overlay->ContentView().Record();
      auto &rect = _overlay->Rect();
      boo::GraphicsContextStateSaver stateSaver(recorder);
      recorder.Translate(static_cast<float>(rect.X()), static_cast<float>(rect.Y()));
      recorder.Clip(
        boo::FloatRect(0, 0, static_cast<float>(rect.Width()), static_cast<float>(rect.Height())));
      recorder.DrawDisplayList(*overlayList);
    }
    return recorder.TakeDisplayList();
  }

  bool View::NeedsPaint() const noexcept
  {
    auto *view = _frame->View();
    return _needsPaint || (view && view->HasDamage()) || (_overlay && _overlay->ContentView().NeedsPaint());
  }

  void View::Paint(gfx::Renderer2D &renderer) noexcept
  {
    const auto size = ViewportSize();
    auto *view = _frame->View();
    if (!view || size.IsEmpty() || !renderer.GetCommandBuffer())
    {
      return;
    }

    if (!_textureMapper)
    {
      auto textureMapper = TextureMapper::Create(renderer.GetDevice(), _fonts);
      if (!textureMapper)
      {
        return;
      }
      _textureMapper = krys::move(*textureMapper);
    }

    // The rendering update runs in Update; painting only needs the layout and the layers up to date.
    (void)Document().UpdateLayout({boo::dom::LayoutOptions::UpdateCompositingLayers});
    (void)view->FlushCompositingStateIncludingSubframes();

    // What the view repaints is in its layers: the damage it tracks for a page that isn't composited is
    // dropped.
    (void)view->TakeDamage();
    _needsPaint = false;

    auto *root = _chromeClient->RootLayer();
    if (!root)
    {
      return;
    }

    // Layers that animate are composited at the frame's time, and ask for the next frame while they run.
    (void)root->Layer().ApplyAnimationsRecursively(boo::html::EventLoop::Current().Now());

    _textureMapper->BeginPainting(renderer, boo::IntRect(0, 0, size.Width(), size.Height()));
    root->UpdateBackingStoreIncludingSubLayers(*_textureMapper);
    root->Layer().Paint(*_textureMapper);
    _textureMapper->EndPainting();

    _needsPaint = root->Layer().DescendantsOrSelfHaveRunningAnimations();
    PaintOverlay(renderer);
  }

  void View::PaintOverlay(gfx::Renderer2D &renderer) noexcept
  {
    if (!_overlay)
    {
      return;
    }

    auto &rect = _overlay->Rect();
    renderer.Save();
    renderer.Translate(static_cast<float>(rect.X()), static_cast<float>(rect.Y()));
    renderer.ClipRect({0.f, 0.f, static_cast<float>(rect.Width()), static_cast<float>(rect.Height())});
    _overlay->ContentView().Paint(renderer);
    renderer.Restore();
  }

  void View::ShowOverlay(Overlay &overlay) noexcept
  {
    if (_overlay && _overlay.get() != &overlay)
    {
      RefPtr<Overlay> previous = _overlay;
      previous->Hide();
    }
    _overlay = ShareRefPtr(&overlay);
    _pageOwnsPress = _buttons != 0;
    _needsPaint = true;
  }

  void View::OverlayHidden(Overlay &overlay) noexcept
  {
    if (_overlay.get() == &overlay)
    {
      _overlay = nullptr;
      _needsPaint = true;
    }
  }

  void View::Replay(const boo::display_list::DisplayList &displayList, gfx::Renderer2D &renderer) noexcept
  {
    const auto size = ViewportSize();
    if (!_imageBuffers)
    {
      _imageBuffers = CreateUnique<ImageBufferTextures>(renderer.GetDevice(), _fonts);
    }
    {
      GraphicsContextRender2D context(
        renderer, _fonts, {0.f, 0.f, static_cast<float>(size.Width()), static_cast<float>(size.Height())},
        _imageBuffers.get());
      context.DrawDisplayList(displayList);
    }
    _imageBuffers->Release();
  }

  boo::dom::Element *View::HitTest(const boo::IntPoint &point) noexcept
  {
    return Document().ElementFromPoint(point.X(), point.Y()).get();
  }

  void View::MouseMoved(const boo::IntPoint &point, pal::KeyModifiers modifiers) noexcept
  {
    // Windows reports moves when the mouse hasn't moved (raw input reports presses and releases as moves
    // too), and one while a button is held would start a selection.
    auto lastPosition = std::exchange(_lastMousePosition, point);
    if (lastPosition == point)
    {
      return;
    }

    // While an overlay is open, the page doesn't see the mouse move.
    if (_overlay)
    {
      auto &rect = _overlay->Rect();
      if (rect.Contains(point))
      {
        _overlay->MouseMoved(boo::IntPoint(point.X() - rect.X(), point.Y() - rect.Y()));
      }
      return;
    }

    // The move's movement is how far the mouse went since the last.
    boo::DoublePoint movement;
    if (lastPosition)
    {
      movement = boo::DoublePoint(point.X() - lastPosition->X(), point.Y() - lastPosition->Y());
    }
    (void)_frame->GetEventHandler().MouseMoved(MouseEvent(
      boo::PlatformEvent::Type::MouseMoved, point, HeldButton(), 0, input::ToBooey(modifiers), movement));
    PerformMicrotaskCheckpoint();
  }

  void View::LockedMouseMoved(const boo::DoublePoint &movement, const boo::DoublePoint &unadjustedMovement,
                              pal::KeyModifiers modifiers) noexcept
  {
    if (!_pointerLocked)
    {
      return;
    }
    // The mouse stays where it was when the pointer was locked.
    (void)_frame->GetEventHandler().MouseMoved(
      MouseEvent(boo::PlatformEvent::Type::MouseMoved, _lastMousePosition.value_or(boo::IntPoint()),
                 HeldButton(), 0, input::ToBooey(modifiers), movement, unadjustedMovement));
    PerformMicrotaskCheckpoint();
  }

  boo::MouseButton View::HeldButton() const noexcept
  {
    if (_buttons & ButtonsBit(boo::MouseButton::Left))
    {
      return boo::MouseButton::Left;
    }
    if (_buttons & ButtonsBit(boo::MouseButton::Middle))
    {
      return boo::MouseButton::Middle;
    }
    if (_buttons & ButtonsBit(boo::MouseButton::Right))
    {
      return boo::MouseButton::Right;
    }
    return boo::MouseButton::None;
  }

  void View::MousePressed(const boo::IntPoint &point, pal::MouseButton palButton,
                          pal::KeyModifiers modifiers) noexcept
  {
    auto button = input::ToBooey(palButton);
    if (button == boo::MouseButton::None)
    {
      return;
    }

    // A press outside the overlay closes it, and goes no further.
    if (_overlay)
    {
      _buttons |= ButtonsBit(button);
      _lastMousePosition = point;
      if (!_overlay->Rect().Contains(point))
      {
        RefPtr<Overlay> overlay = _overlay;
        overlay->Hide();
        PerformMicrotaskCheckpoint();
      }
      return;
    }

    (void)CountClick(point, button);

    _buttons |= ButtonsBit(button);
    _lastMousePosition = point;
    (void)_frame->GetEventHandler().HandleMousePressEvent(MouseEvent(
      boo::PlatformEvent::Type::MousePressed, point, button, _clickCount, input::ToBooey(modifiers)));
    PerformMicrotaskCheckpoint();
  }

  void View::MouseReleased(const boo::IntPoint &point, pal::MouseButton palButton,
                           pal::KeyModifiers modifiers) noexcept
  {
    auto button = input::ToBooey(palButton);
    if (button == boo::MouseButton::None)
    {
      return;
    }

    _buttons &= static_cast<uint16>(~ButtonsBit(button));
    _lastMousePosition = point;

    // A release over the overlay goes to it (a popup chooses the item under it); the release of the press
    // that opened it goes to the page.
    if (_overlay && !std::exchange(_pageOwnsPress, false))
    {
      auto &rect = _overlay->Rect();
      if (rect.Contains(point))
      {
        RefPtr<Overlay> overlay = _overlay;
        overlay->MouseReleased(boo::IntPoint(point.X() - rect.X(), point.Y() - rect.Y()));
        PerformMicrotaskCheckpoint();
      }
      return;
    }

    bool handled = _frame->GetEventHandler().HandleMouseReleaseEvent(MouseEvent(
      boo::PlatformEvent::Type::MouseReleased, point, button, _clickCount, input::ToBooey(modifiers)));
    PerformMicrotaskCheckpoint();

    // The mouse's back and forward buttons go back and forward, unless the page handled them.
    if (!handled && button == boo::MouseButton::Back)
    {
      (void)GoBack();
    }
    else if (!handled && button == boo::MouseButton::Forward)
    {
      (void)GoForward();
    }
  }

  void View::MouseLeft() noexcept
  {
    // A move outside the viewport hits nothing, so the element under the mouse is left and nothing is
    // hovered.
    MouseMoved(boo::IntPoint(-1, -1));
  }

  bool View::MouseWheel(const boo::IntPoint &point, float notchesX, float notchesY,
                        pal::KeyModifiers modifiers, const pal::WheelScrollSettings &settings) noexcept
  {
    if (!notchesX && !notchesY)
    {
      return false;
    }
    if (_overlay)
    {
      auto &rect = _overlay->Rect();
      return rect.Contains(point)
             && _overlay->MouseWheel(boo::IntPoint(point.X() - rect.X(), point.Y() - rect.Y()), notchesX,
                                     notchesY);
    }
    bool handled = _frame->GetEventHandler().HandleWheelEvent(
      input::WheelEvent(point, notchesX, notchesY, modifiers, settings));
    PerformMicrotaskCheckpoint();
    return handled;
  }

  int32 View::CountClick(const boo::IntPoint &point, boo::MouseButton button) noexcept
  {
    // A press soon after the last, near it and with the same button, counts on from it.
    auto now = MonotonicTime::Now();
    bool continuesClicks = _clickCount > 0 && button == _lastClickButton
                           && now - _lastClickTime <= DoubleClickTime
                           && std::abs(point.X() - _lastClickPosition.X()) <= DoubleClickDistance
                           && std::abs(point.Y() - _lastClickPosition.Y()) <= DoubleClickDistance;
    _clickCount = continuesClicks ? _clickCount + 1 : 1;
    _lastClickButton = button;
    _lastClickTime = now;
    _lastClickPosition = point;
    return _clickCount;
  }

  void View::PenInput(const boo::DoublePoint &point, const pal::PointerInput &input) noexcept
  {
    auto intPoint = boo::FlooredIntPoint(point);

    // While an overlay is open, the pen is the mouse.
    if (_overlay)
    {
      switch (input.Action)
      {
        case pal::PointerAction::Pressed:
          MousePressed(intPoint, pal::MouseButton::Left, input.Modifiers);
          break;
        case pal::PointerAction::Released:
          MouseReleased(intPoint, pal::MouseButton::Left, input.Modifiers);
          break;
        case pal::PointerAction::Entered:
        case pal::PointerAction::Moved:   MouseMoved(intPoint, input.Modifiers); break;
        default:                          break;
      }
      return;
    }

    auto &handler = _frame->GetEventHandler();
    switch (input.Action)
    {
      case pal::PointerAction::Entered:
      case pal::PointerAction::Moved:
      {
        (void)handler.MouseMoved(PenEvent(boo::PlatformEvent::Type::MouseMoved, point, _penButton, 0, input));
        break;
      }
      case pal::PointerAction::Pressed:
      {
        // https://w3c.github.io/pointerevents/#the-button-property
        _penButton = input.Eraser         ? boo::MouseButton::Other
                     : input.BarrelButton ? boo::MouseButton::Right
                                          : boo::MouseButton::Left;
        _penButtons |= _penButton == boo::MouseButton::Other ? uint16 {32} : ButtonsBit(_penButton);
        int32 clickCount = CountClick(intPoint, _penButton);
        (void)handler.HandleMousePressEvent(
          PenEvent(boo::PlatformEvent::Type::MousePressed, point, _penButton, clickCount, input));
        break;
      }
      case pal::PointerAction::Released:
      {
        auto button = std::exchange(_penButton, boo::MouseButton::None);
        if (button == boo::MouseButton::None)
        {
          break;
        }
        _penButtons = 0;
        (void)handler.HandleMouseReleaseEvent(
          PenEvent(boo::PlatformEvent::Type::MouseReleased, point, button, _clickCount, input));
        break;
      }
      case pal::PointerAction::Left:
      {
        // A move outside the viewport hits nothing, so the element under the pen is left.
        (void)handler.MouseMoved(PenEvent(boo::PlatformEvent::Type::MouseMoved, boo::DoublePoint(-1, -1),
                                          boo::MouseButton::None, 0, input));
        break;
      }
      case pal::PointerAction::Cancelled:
      {
        // FIXME: End the event handler's press too, without the release's events.
        _penButton = boo::MouseButton::None;
        _penButtons = 0;
        _page->GetPointerCaptureController().CancelPointer(boo::PenPointerID,
                                                           _frame->View()->WindowToContents(intPoint));
        break;
      }
    }
    PerformMicrotaskCheckpoint();
  }

  bool View::TouchInput(const boo::DoublePoint &point, const pal::PointerInput &input) noexcept
  {
    if (_overlay)
    {
      return false;
    }

    auto it = std::ranges::find(_touches, input.Id, &ActiveTouch::HostId);
    if (input.Action == pal::PointerAction::Pressed && it == _touches.end())
    {
      // The lowest id no other touch has.
      uint32 id = 0;
      while (std::ranges::find(_touches, id, &ActiveTouch::Id) != _touches.end())
      {
        ++id;
      }
      _touches.push_back({input.Id, id, {}});
      it = _touches.end() - 1;
    }
    if (it == _touches.end())
    {
      return false;
    }

    boo::PlatformEvent::Type type = boo::PlatformEvent::Type::TouchMove;
    boo::PlatformTouchPoint::State state = boo::PlatformTouchPoint::TouchMoved;
    switch (input.Action)
    {
      case pal::PointerAction::Pressed:
        type = boo::PlatformEvent::Type::TouchStart;
        state = boo::PlatformTouchPoint::TouchPressed;
        break;
      case pal::PointerAction::Released:
        type = boo::PlatformEvent::Type::TouchEnd;
        state = boo::PlatformTouchPoint::TouchReleased;
        break;
      case pal::PointerAction::Cancelled:
        type = boo::PlatformEvent::Type::TouchCancel;
        state = boo::PlatformTouchPoint::TouchCancelled;
        break;
      default: break;
    }

    // A cancellation from the host may come without a position: the touch stays where it was.
    bool hasPosition = input.Action != pal::PointerAction::Cancelled || point.X() || point.Y();
    auto &previous = it->Point;
    // FIXME: The screen position, once the host reports where the page is on the screen.
    boo::DoublePoint position = hasPosition ? point : previous.Pos();
    boo::DoubleSize radius =
      hasPosition ? boo::DoubleSize(input.Width / 2, input.Height / 2) : previous.Radius();
    float force = hasPosition ? input.Pressure.value_or(0.f) : previous.Force();
    it->Point = boo::PlatformTouchPoint(it->Id, state, position, position, radius, 0, force);

    List<boo::PlatformTouchPoint> points;
    for (auto &touch : _touches)
    {
      if (&touch == &*it)
      {
        points.push_back(touch.Point);
      }
      else
      {
        points.push_back(boo::PlatformTouchPoint(
          touch.Id, boo::PlatformTouchPoint::TouchStationary, touch.Point.ScreenPos(), touch.Point.Pos(),
          touch.Point.Radius(), touch.Point.RotationAngle(), touch.Point.Force()));
      }
    }

    boo::PlatformTouchEvent event(type, krys::move(points), input::ToBooey(input.Modifiers),
                                  MonotonicTime::Now());
    // Moves aren't merged, so each stands for itself alone (its pointermove's coalesced events).
    if (type == boo::PlatformEvent::Type::TouchMove)
    {
      event.SetCoalescedEvents({event});
    }

    if (state == boo::PlatformTouchPoint::TouchReleased || state == boo::PlatformTouchPoint::TouchCancelled)
    {
      _touches.erase(it);
    }

    bool handled = _frame->GetEventHandler().HandleTouchEvent(event);
    PerformMicrotaskCheckpoint();
    return handled;
  }

  void View::SetPointingDevices(const pal::PointingDevices &devices) noexcept
  {
    using boo::css::mq::PointerCharacteristics;
    _pointingDevices = devices;
    auto environment = _frame->GetMediaEnvironment();

    // The mouse is the primary pointing device if there's one, else the touch screen, else the pen.
    if (devices.Mouse || (!devices.Touch && devices.Pen))
    {
      environment.PrimaryPointingDevice = PointerCharacteristics::Fine;
      environment.HoverSupportedByPrimaryPointingDevice = true;
    }
    else
    {
      environment.PrimaryPointingDevice =
        devices.Touch ? PointerCharacteristics::Coarse : PointerCharacteristics::None;
      environment.HoverSupportedByPrimaryPointingDevice = false;
    }

    environment.AllAvailablePointingDevices = PointerCharacteristics::None;
    if (devices.Mouse || devices.Pen)
    {
      environment.AllAvailablePointingDevices =
        environment.AllAvailablePointingDevices | PointerCharacteristics::Fine;
    }
    if (devices.Touch)
    {
      environment.AllAvailablePointingDevices =
        environment.AllAvailablePointingDevices | PointerCharacteristics::Coarse;
    }
    // A pen hovers, as the mouse does.
    environment.HoverSupportedByAnyAvailablePointingDevice = devices.Mouse || devices.Pen;
    environment.MaxTouchPoints = devices.Touch ? devices.MaxTouchPoints : 0;
    _frame->SetMediaEnvironment(environment);
  }

  pal::CursorShape View::CurrentCursor() const noexcept
  {
    if (_pointerLocked)
    {
      return pal::CursorShape::Hidden;
    }
    return input::ToCursorShape(_frame->GetEventHandler().CurrentMouseCursor());
  }

  void View::SetPointerLockHandler(Func<bool(bool)> &&handler) noexcept
  {
    _pointerLockHandler = krys::move(handler);
  }

  boo::PointerLockRequestResult View::LockPointer() noexcept
  {
    if (!_pointerLockHandler)
    {
      return boo::PointerLockRequestResult::Unsupported;
    }
    // A window without focus can't take the pointer.
    if (!_focused || !_pointerLockHandler(true))
    {
      return boo::PointerLockRequestResult::Failure;
    }
    _pointerLocked = true;
    return boo::PointerLockRequestResult::Success;
  }

  bool View::UnlockPointer() noexcept
  {
    if (std::exchange(_pointerLocked, false) && _pointerLockHandler)
    {
      (void)_pointerLockHandler(false);
    }
    return true;
  }

  bool View::KeyPressed(pal::Key key, pal::KeyModifiers modifiers, const pal::KeyText &text,
                        bool isAutoRepeat) noexcept
  {
    // While an overlay is open, it takes the keys.
    if (_overlay)
    {
      RefPtr<Overlay> overlay = _overlay;
      bool handled = overlay->KeyPressed(key, modifiers);
      PerformMicrotaskCheckpoint();
      return handled;
    }

    auto event = input::KeyboardEvent(boo::PlatformEvent::Type::KeyDown, key, modifiers, text, isAutoRepeat);
    _heldKeyValues.insert_or_assign(key, event.Key());
    bool handled = _frame->GetEventHandler().KeyEvent(event);
    PerformMicrotaskCheckpoint();

    // Alt+Left and Alt+Right go back and forward, unless the page handled them.
    if (!handled && modifiers == pal::KeyModifiers {pal::KeyModifier::Alt})
    {
      if (key == pal::Key::LeftArrow)
      {
        return GoBack();
      }
      if (key == pal::Key::RightArrow)
      {
        return GoForward();
      }
    }
    return handled;
  }

  bool View::KeyReleased(pal::Key key, pal::KeyModifiers modifiers) noexcept
  {
    utf8_string keyValue;
    if (auto held = _heldKeyValues.find(key); held != _heldKeyValues.end())
    {
      keyValue = krys::move(held->second);
      _heldKeyValues.erase(held);
    }
    bool handled = _frame->GetEventHandler().KeyEvent(
      input::KeyboardEvent(boo::PlatformEvent::Type::KeyUp, key, modifiers, {}, false, keyValue));
    PerformMicrotaskCheckpoint();
    return handled;
  }

  void View::SetFocused(bool focused) noexcept
  {
    if (!focused && _overlay)
    {
      RefPtr<Overlay> overlay = _overlay;
      overlay->Hide();
    }
    _focused = focused;
    boo::html::FocusAlgorithms::SystemFocusChanged(Document(), focused);

    // The pointer lock ends with the window's focus.
    if (!focused && _pointerLocked)
    {
      (void)UnlockPointer();
      _page->GetPointerLockController().DidLosePointerLock();
    }
    PerformMicrotaskCheckpoint();
  }

  void View::SetNavigationHandler(Func<NavigationPolicy(const NavigationRequest &)> &&handler) noexcept
  {
    _frameLoaderClient->SetNavigationHandler(krys::move(handler));
  }

  void View::SetFocusRequestHandler(Func<void()> &&handler) noexcept
  {
    _chromeClient->SetFocusRequestHandler(krys::move(handler));
  }

  void View::SetCloseRequestHandler(Func<void()> &&handler) noexcept
  {
    _chromeClient->SetCloseRequestHandler(krys::move(handler));
  }

  void View::SetWindowRect(const boo::FloatRect &rect) noexcept
  {
    _chromeClient->SetHostWindowRect(rect);
  }

  void View::SetWindowRectRequestHandler(Func<void(const boo::FloatRect &)> &&handler) noexcept
  {
    _chromeClient->SetWindowRectRequestHandler(krys::move(handler));
  }

  void View::SetAlertHandler(Func<void(const utf8_string &)> &&handler) noexcept
  {
    _chromeClient->SetAlertHandler(krys::move(handler));
  }

  void View::SetConfirmHandler(Func<bool(const utf8_string &)> &&handler) noexcept
  {
    _chromeClient->SetConfirmHandler(krys::move(handler));
  }

  void View::SetPromptHandler(
    Func<Maybe<utf8_string>(const utf8_string &, const utf8_string &)> &&handler) noexcept
  {
    _chromeClient->SetPromptHandler(krys::move(handler));
  }

  void View::SetPrintHandler(Func<void()> &&handler) noexcept
  {
    _chromeClient->SetPrintHandler(krys::move(handler));
  }

  void View::SetBeforeUnloadHandler(Func<bool(const utf8_string &)> &&handler) noexcept
  {
    _chromeClient->SetBeforeUnloadHandler(krys::move(handler));
  }

  void View::SetConsoleMessageHandler(Func<void(const ConsoleMessage &)> &&handler) noexcept
  {
    _chromeClient->SetConsoleMessageHandler(krys::move(handler));
  }

  void View::SetProtocolHandlerRequestHandler(
    Func<void(const utf8_string &scheme, const utf8_string &url, bool registered)> &&handler) noexcept
  {
    _chromeClient->SetProtocolHandlerRequestHandler(krys::move(handler));
  }

  void View::SetUserAgent(utf8_string &&userAgent) noexcept
  {
    _frameLoaderClient->SetUserAgent(krys::move(userAgent));
    if (auto *window = Document().Window())
    {
      window->GetNavigator().UserAgentChanged();
    }
  }

  boo::PlatformMouseEvent View::MouseEvent(boo::PlatformEvent::Type type, const boo::IntPoint &point,
                                           boo::MouseButton button, int32 clickCount,
                                           EnumSet<boo::PlatformEvent::Modifier> modifiers,
                                           const boo::DoublePoint &movement,
                                           const boo::DoublePoint &unadjustedMovement) const noexcept
  {
    // FIXME: The screen position, once the host reports where the page is on the screen.
    boo::PlatformMouseEvent event(boo::DoublePoint(point), boo::DoublePoint(point), button, type, clickCount,
                                  modifiers, MonotonicTime::Now(), _buttons, movement);
    event.SetUnadjustedMovementDelta(unadjustedMovement);
    // Moves aren't merged, so each stands for itself alone (its pointermove's coalesced events).
    if (type == boo::PlatformEvent::Type::MouseMoved)
    {
      event.SetCoalescedEvents({event});
    }
    return event;
  }

  boo::PlatformMouseEvent View::PenEvent(boo::PlatformEvent::Type type, const boo::DoublePoint &point,
                                         boo::MouseButton button, int32 clickCount,
                                         const pal::PointerInput &input) const noexcept
  {
    // A pen without pressure presses at half pressure.
    double force = input.Pressure ? *input.Pressure : (input.InContact ? 0.5 : 0);
    // FIXME: The screen position, once the host reports where the page is on the screen.
    boo::PlatformMouseEvent event(point, point, button, type, clickCount, input::ToBooey(input.Modifiers),
                                  MonotonicTime::Now(), _penButtons, {}, force,
                                  boo::SyntheticClickType::NoTap, boo::PenPointerID,
                                  boo::dom::PenPointerEventType());
    event.SetPenOrientation({static_cast<int32>(std::round(input.TiltX)),
                             static_cast<int32>(std::round(input.TiltY)),
                             static_cast<int32>(std::round(input.Twist))});
    // Moves aren't merged, so each stands for itself alone (its pointermove's coalesced events).
    if (type == boo::PlatformEvent::Type::MouseMoved)
    {
      event.SetCoalescedEvents({event});
    }
    return event;
  }

  uint16 View::ButtonsBit(boo::MouseButton button) noexcept
  {
    // The DOM's buttons: https://w3c.github.io/uievents/#dom-mouseevent-buttons
    switch (button)
    {
      case boo::MouseButton::Left:    return 1;
      case boo::MouseButton::Right:   return 2;
      case boo::MouseButton::Middle:  return 4;
      case boo::MouseButton::Back:    return 8;
      case boo::MouseButton::Forward: return 16;
      default:                        return 0;
    }
  }
}
