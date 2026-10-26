#include "Krystal.UI/View.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/PointerEvent.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/History/BackForwardCache.hpp"
#include "Krystal.Booey/History/BackForwardController.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcherManager.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Loader/DocumentWriter.hpp"
#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Booey/Loader/FrameLoader.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Loader/NavigationAction.hpp"
#include "Krystal.Booey/Loader/SubstituteData.hpp"
#include "Krystal.Booey/Page/DragController.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Navigator.hpp"
#include "Krystal.Booey/Page/PointerCaptureController.hpp"
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Platform/DragData.hpp"
#include "Krystal.Booey/Platform/Pasteboard.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Platform/PlatformWheelEvent.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Values/PointerEvents/TouchAction.hpp"
#include "Krystal.UI/ColorChooserOverlay.hpp"
#include "Krystal.UI/Compositing/GraphicsLayerTextureMapper.hpp"
#include "Krystal.UI/DateTimeChooserOverlay.hpp"
#include "Krystal.UI/EditorClient.hpp"
#include "Krystal.UI/FileLoaderStrategy.hpp"
#include "Krystal.UI/GraphicsContextRender2D.hpp"
#include "Krystal.UI/Input.hpp"
#include "Krystal.UI/PopupMenuOverlay.hpp"
#include "Krystal.UI/SystemPasteboardStrategy.hpp"
#include <algorithm>
#include <cmath>
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
  /// @brief Runs the drags that start in the page: the view keeps what's dragged and moves, drops or cancels
  /// the drag as the mouse and the keys say.
  class View::DragClient final : public boo::DragClient
  {
  private:
    View *_view {nullptr};

  public:
    void SetView(View *view) noexcept
    {
      _view = view;
    }

    void WillPerformDragDestinationAction(boo::DragDestinationAction, const boo::DragData &) noexcept override
    {
    }

    void WillPerformDragSourceAction(boo::DragSourceAction, const boo::IntPoint &,
                                     boo::dom::DataTransfer &) noexcept override
    {
    }

    KRYS_NODISCARD EnumSet<boo::DragSourceAction>
      DragSourceActionMaskForPoint(const boo::IntPoint &) noexcept override
    {
      return boo::AnyDragSourceAction();
    }

    void StartDrag(boo::DragItem, boo::dom::DataTransfer &dataTransfer, boo::LocalFrame &) noexcept override
    {
      if (_view)
      {
        _view->DragStarted(dataTransfer);
      }
    }
  };

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

    /// @brief The page's editor client, noted in @p editorClient as it's handed to the page.
    KRYS_NODISCARD UniquePtr<boo::EditorClient> CreateEditorClient(EditorClient *&editorClient) noexcept
    {
      auto client = CreateUnique<EditorClient>();
      editorClient = client.get();
      return client;
    }

    /// @brief The page's drag client, noted in @p dragClient as it's handed to the page.
    KRYS_NODISCARD UniquePtr<boo::DragClient> CreateDragClient(View::DragClient *&dragClient) noexcept
    {
      auto client = CreateUnique<View::DragClient>();
      dragClient = client.get();
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
        _page(boo::Page::Create(CreateChromeClient(_chromeClient), CreateEditorClient(_editorClient),
                                CreateFrameLoaderClient(_frameLoaderClient), _backForwardList,
                                CreateDragClient(_dragClient))),
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
    _dragClient->SetView(this);
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
    _dragClient->SetView(nullptr);
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
    // The old document's gestures and fling end with it.
    ResetGesture();
    _fling = null;
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

    // The fling moves on with the host's time, before the rendering update shows it.
    if (_fling)
    {
      StepFling(now);
    }

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

    // While a drag runs, the mouse moves it.
    if (_drag)
    {
      DragMoved(point, modifiers);
      PerformMicrotaskCheckpoint();
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
    // A drag that the move started enters the page at once.
    if (_drag)
    {
      DragMoved(point, modifiers);
    }
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

    // Other buttons do nothing while a drag runs.
    if (_drag)
    {
      _buttons |= ButtonsBit(button);
      return;
    }

    // A press ends the composition: it's confirmed where it is, wherever the press puts the selection.
    ConfirmCompositionForHost();

    (void)CountClick(point, button);

    _buttons |= ButtonsBit(button);
    _lastMousePosition = point;
    auto event = MouseEvent(boo::PlatformEvent::Type::MousePressed, point, button, _clickCount,
                            input::ToBooey(modifiers));
    (void)_frame->GetEventHandler().HandleMousePressEvent(event);
    // The right button's press asks for a context menu: there's none, but the page gets `contextmenu`.
    if (button == boo::MouseButton::Right)
    {
      (void)_frame->GetEventHandler().SendContextMenuEvent(event);
    }
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

    // The release of the button that's dragging drops the drag; the page doesn't see it, as it doesn't see
    // the mouse while a system drag runs.
    if (_drag)
    {
      if (button == boo::MouseButton::Left)
      {
        EndDrag(point, true);
        PerformMicrotaskCheckpoint();
      }
      return;
    }

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

  int32 View::CountClick(const boo::IntPoint &point, boo::MouseButton button, int32 distance) noexcept
  {
    // A press soon after the last, near it and with the same button, counts on from it.
    auto now = MonotonicTime::Now();
    bool continuesClicks = _clickCount > 0 && button == _lastClickButton
                           && now - _lastClickTime <= DoubleClickTime
                           && std::abs(point.X() - _lastClickPosition.X()) <= distance
                           && std::abs(point.Y() - _lastClickPosition.Y()) <= distance;
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
        ConfirmCompositionForHost();
        // https://w3c.github.io/pointerevents/#the-button-property
        _penButton = input.Eraser         ? boo::MouseButton::Other
                     : input.BarrelButton ? boo::MouseButton::Right
                                          : boo::MouseButton::Left;
        _penButtons |= _penButton == boo::MouseButton::Other ? uint16 {32} : ButtonsBit(_penButton);
        int32 clickCount = CountClick(intPoint, _penButton);
        auto event = PenEvent(boo::PlatformEvent::Type::MousePressed, point, _penButton, clickCount, input);
        (void)handler.HandleMousePressEvent(event);
        if (_penButton == boo::MouseButton::Right)
        {
          (void)handler.SendContextMenuEvent(event);
        }
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
      ConfirmCompositionForHost();
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

    uint32 id = it->Id;
    if (state == boo::PlatformTouchPoint::TouchReleased || state == boo::PlatformTouchPoint::TouchCancelled)
    {
      _touches.erase(it);
    }

    bool handled = _frame->GetEventHandler().HandleTouchEvent(event);
    PerformMicrotaskCheckpoint();
    RecogniseGesture(id, input.Action, position, handled);
    return handled;
  }

  void View::RecogniseGesture(uint32 id, pal::PointerAction action, const boo::DoublePoint &position,
                              bool cancelled) noexcept
  {
    switch (action)
    {
      case pal::PointerAction::Pressed:
      {
        if (_touches.size() == 1)
        {
          // A touch stops the fling, and can't tap while it does.
          bool stoppedFling = std::exchange(_fling, null).has_value();
          ResetGesture();
          if (cancelled)
          {
            break;
          }
          _gestureState = GestureState::MayTap;
          _gestureTouchId = id;
          _gestureStart = _gestureLast = position;
          _gestureTouchActions = TouchActionsAt(position);
          _gestureCanTap = !stoppedFling;
          _longPressTimer.StartOneShot(LongPressDuration);
          break;
        }

        // A second touch pinches, where the page can be pinched.
        bool canPinch = _gestureTouchActions.ContainsAny(
          {boo::TouchAction::Auto, boo::TouchAction::Manipulation, boo::TouchAction::PinchZoom});
        if (_touches.size() == 2 && !cancelled && canPinch
            && (_gestureState == GestureState::MayTap || _gestureState == GestureState::Pan))
        {
          _longPressTimer.Stop();
          for (auto &touch : _touches)
          {
            CancelTouchPointer(touch.Id, touch.Point.Pos());
          }
          auto first = _touches[0].Point.Pos();
          auto second = _touches[1].Point.Pos();
          auto centre = boo::DoublePoint((first.X() + second.X()) / 2, (first.Y() + second.Y()) / 2);
          _pinchStartScale = _page->PageScaleFactor();
          _pinchStartDistance = std::hypot(second.X() - first.X(), second.Y() - first.Y());
          auto scroll = ScrollPosition();
          _pinchDocumentPoint = boo::DoublePoint((scroll.X() + centre.X()) / _pinchStartScale,
                                                 (scroll.Y() + centre.Y()) / _pinchStartScale);
          _gestureState = _pinchStartDistance > 0 ? GestureState::Pinch : GestureState::None;
          break;
        }
        if (_gestureState != GestureState::Pinch)
        {
          ResetGesture();
        }
        break;
      }
      case pal::PointerAction::Moved:
      {
        if (_gestureState == GestureState::Pinch)
        {
          UpdatePinch();
          break;
        }
        if (id != _gestureTouchId)
        {
          break;
        }
        if (_gestureState == GestureState::MayTap)
        {
          // The page takes the touch by cancelling its moves before it pans.
          if (cancelled)
          {
            ResetGesture();
            break;
          }
          auto delta = position - _gestureStart;
          if (std::hypot(delta.Width(), delta.Height()) < PanThreshold)
          {
            break;
          }

          // The touch pans if `touch-action` allows its direction where it began; otherwise it's the page's.
          bool any =
            _gestureTouchActions.ContainsAny({boo::TouchAction::Auto, boo::TouchAction::Manipulation});
          _panX = any || _gestureTouchActions.Contains(boo::TouchAction::PanX);
          _panY = any || _gestureTouchActions.Contains(boo::TouchAction::PanY);
          bool horizontal = std::abs(delta.Width()) >= std::abs(delta.Height());
          _longPressTimer.Stop();
          if (horizontal ? !_panX : !_panY)
          {
            ResetGesture();
            break;
          }
          CancelTouchPointer(id, position);
          _gestureState = GestureState::Pan;
          _panSamples = {{boo::html::EventLoop::Current().Now(), _gestureStart}};
        }
        if (_gestureState == GestureState::Pan)
        {
          PanTo(position);
        }
        break;
      }
      case pal::PointerAction::Released:
      {
        if (_gestureState == GestureState::Pinch)
        {
          // Once a touch lifts, the rest of the pinch does nothing.
          ResetGesture();
          break;
        }
        if (id != _gestureTouchId)
        {
          break;
        }
        auto state = _gestureState;
        bool canTap = _gestureCanTap;
        if (state == GestureState::Pan)
        {
          PanTo(position);
          StartFling();
        }
        ResetGesture();
        // A cancelled touchend stops the tap's mouse events, the click among them, and the long press's
        // context menu.
        if (state == GestureState::MayTap && canTap && !cancelled && _touches.empty())
        {
          Tap(id, position);
        }
        else if (state == GestureState::LongPress && !cancelled)
        {
          RequestContextMenu(id, position);
        }
        break;
      }
      case pal::PointerAction::Cancelled:
      {
        ResetGesture();
        break;
      }
      default: break;
    }
  }

  EnumSet<boo::TouchAction> View::TouchActionsAt(const boo::DoublePoint &point) noexcept
  {
    auto *element = HitTest(boo::FlooredIntPoint(point));
    auto *renderer = element ? element->Renderer() : nullptr;
    if (!renderer)
    {
      return boo::TouchAction::Auto;
    }
    return boo::style::ToPlatform<boo::style::TouchAction> {}(renderer->Style().UsedTouchAction());
  }

  void View::ResetGesture() noexcept
  {
    _gestureState = GestureState::None;
    _longPressTimer.Stop();
    _panRemainder = {};
    _panSamples.clear();
  }

  void View::CancelTouchPointer(uint32 id, const boo::DoublePoint &position) noexcept
  {
    if (auto *view = _frame->View())
    {
      boo::PlatformTouchPoint point(id, boo::PlatformTouchPoint::TouchCancelled, position, position, {}, 0,
                                    0);
      _page->GetPointerCaptureController().CancelPointer(
        boo::dom::PointerEvent::PointerIdForTouch(point),
        view->WindowToContents(boo::FlooredIntPoint(position)));
      PerformMicrotaskCheckpoint();
    }
  }

  void View::PanTo(const boo::DoublePoint &position) noexcept
  {
    auto movement = position - _gestureLast;
    _gestureLast = position;
    boo::DoubleSize delta(_panX ? movement.Width() : 0, _panY ? movement.Height() : 0);

    // The samples within the fling's window of this one.
    auto now = boo::html::EventLoop::Current().Now();
    _panSamples.emplace_back(now, position);
    std::erase_if(_panSamples, [&](const auto &sample) { return now - sample.first > FlingSampleWindow; });

    // Scrolls are in whole pixels; the rest waits for the next move.
    auto remainder =
      boo::DoubleSize(_panRemainder.Width() + delta.Width(), _panRemainder.Height() + delta.Height());
    boo::DoubleSize step(std::round(remainder.Width()), std::round(remainder.Height()));
    _panRemainder = boo::DoubleSize(remainder.Width() - step.Width(), remainder.Height() - step.Height());
    if (step.Width() || step.Height())
    {
      (void)ScrollForTouch(_gestureStart, step);
    }
  }

  bool View::ScrollForTouch(const boo::DoublePoint &position, const boo::DoubleSize &delta) noexcept
  {
    auto point = boo::FlooredIntPoint(position);
    bool scrolled = _frame->GetEventHandler().HandleTouchScroll(boo::PlatformWheelEvent(
      point, point, static_cast<float>(delta.Width()), static_cast<float>(delta.Height()), 0, 0,
      boo::PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {}, MonotonicTime::Now()));
    PerformMicrotaskCheckpoint();
    _needsPaint = true;
    return scrolled;
  }

  void View::StartFling() noexcept
  {
    if (_panSamples.size() < 2)
    {
      return;
    }
    auto &[firstTime, firstPosition] = _panSamples.front();
    auto &[lastTime, lastPosition] = _panSamples.back();
    double seconds = std::chrono::duration<double>(lastTime - firstTime).count();
    if (seconds <= 0)
    {
      return;
    }
    auto distance = lastPosition - firstPosition;
    boo::DoubleSize velocity(_panX ? distance.Width() / seconds : 0, _panY ? distance.Height() / seconds : 0);
    if (std::hypot(velocity.Width(), velocity.Height()) < MinimumFlingVelocity)
    {
      return;
    }
    _fling = Fling {_gestureStart, velocity, {}, boo::html::EventLoop::Current().Now()};
  }

  void View::StepFling(MonotonicTime::duration now) noexcept
  {
    double seconds = std::chrono::duration<double>(now - _fling->LastTime).count();
    if (seconds <= 0)
    {
      return;
    }
    _fling->LastTime = now;

    // The velocity decays exponentially; the distance is its integral over the step.
    double decay = std::exp(-seconds / FlingTimeConstant);
    auto &velocity = _fling->Velocity;
    auto remainder =
      boo::DoubleSize(_fling->Remainder.Width() + velocity.Width() * FlingTimeConstant * (1 - decay),
                      _fling->Remainder.Height() + velocity.Height() * FlingTimeConstant * (1 - decay));
    velocity = velocity.ScaledBy(decay);

    boo::DoubleSize step(std::round(remainder.Width()), std::round(remainder.Height()));
    _fling->Remainder = boo::DoubleSize(remainder.Width() - step.Width(), remainder.Height() - step.Height());
    bool scrolled = (!step.Width() && !step.Height()) || ScrollForTouch(_fling->Position, step);
    // It stops once it's slow, or at the end of what it scrolls.
    if (!scrolled || std::hypot(velocity.Width(), velocity.Height()) < StopFlingVelocity)
    {
      _fling = null;
    }
  }

  void View::UpdatePinch() noexcept
  {
    if (_touches.size() < 2)
    {
      return;
    }
    auto first = _touches[0].Point.Pos();
    auto second = _touches[1].Point.Pos();
    auto centre = boo::DoublePoint((first.X() + second.X()) / 2, (first.Y() + second.Y()) / 2);
    double distance = std::hypot(second.X() - first.X(), second.Y() - first.Y());
    float scale = std::clamp(static_cast<float>(_pinchStartScale * distance / _pinchStartDistance),
                             MinimumPageScale, MaximumPageScale);

    // The document point that was between the touches stays between them.
    boo::IntPoint origin(static_cast<int32>(std::lround(_pinchDocumentPoint.X() * scale - centre.X())),
                         static_cast<int32>(std::lround(_pinchDocumentPoint.Y() * scale - centre.Y())));
    _page->SetPageScaleFactor(scale, origin);
    PerformMicrotaskCheckpoint();
    _needsPaint = true;
  }

  void View::Tap(uint32 id, const boo::DoublePoint &position) noexcept
  {
    // A tap is the mouse moving there and clicking, as one finger's tap, from the touch's pointer: its
    // compatibility mouse events don't fire if the page cancelled its pointerdown.
    auto point = boo::FlooredIntPoint(position);
    int32 clickCount = CountClick(point, boo::MouseButton::Left, DoubleTapDistance);
    boo::PlatformTouchPoint touchPoint(id, boo::PlatformTouchPoint::TouchReleased, position, position, {}, 0,
                                       0);
    auto pointerId = boo::dom::PointerEvent::PointerIdForTouch(touchPoint);
    auto event = [&](boo::PlatformEvent::Type type, boo::MouseButton button, uint16 buttons, int32 count)
    {
      // FIXME: The screen position, once the host reports where the page is on the screen.
      return boo::PlatformMouseEvent(
        boo::DoublePoint(point), boo::DoublePoint(point), button, type, count, {}, MonotonicTime::Now(),
        buttons, {}, 0, boo::SyntheticClickType::OneFingerTap, pointerId, boo::dom::TouchPointerEventType());
    };

    auto &handler = _frame->GetEventHandler();
    (void)handler.MouseMoved(event(boo::PlatformEvent::Type::MouseMoved, boo::MouseButton::None, 0, 0));
    (void)handler.HandleMousePressEvent(
      event(boo::PlatformEvent::Type::MousePressed, boo::MouseButton::Left, 1, clickCount));
    (void)handler.HandleMouseReleaseEvent(
      event(boo::PlatformEvent::Type::MouseReleased, boo::MouseButton::Left, 0, clickCount));
    PerformMicrotaskCheckpoint();
  }

  void View::LongPressed() noexcept
  {
    // It asks for the context menu as it lifts, as Windows' press and hold does.
    if (_gestureState == GestureState::MayTap)
    {
      _gestureState = GestureState::LongPress;
    }
  }

  void View::RequestContextMenu(uint32 id, const boo::DoublePoint &position) noexcept
  {
    // As the right button's press asks for a context menu: there's none, but the page gets `contextmenu`.
    auto point = boo::FlooredIntPoint(position);
    boo::PlatformTouchPoint touchPoint(id, boo::PlatformTouchPoint::TouchReleased, position, position, {}, 0,
                                       0);
    // FIXME: The screen position, once the host reports where the page is on the screen.
    boo::PlatformMouseEvent event(boo::DoublePoint(point), boo::DoublePoint(point), boo::MouseButton::Right,
                                  boo::PlatformEvent::Type::MousePressed, 1, {}, MonotonicTime::Now(), 0, {},
                                  0, boo::SyntheticClickType::NoTap,
                                  boo::dom::PointerEvent::PointerIdForTouch(touchPoint),
                                  boo::dom::TouchPointerEventType());
    (void)_frame->GetEventHandler().SendContextMenuEvent(event);
    PerformMicrotaskCheckpoint();
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
    if (_drag)
    {
      if (!_drag->InView || !_drag->Operation)
      {
        return pal::CursorShape::DragNone;
      }
      switch (*_drag->Operation)
      {
        case boo::DragOperation::Copy: return pal::CursorShape::DragCopy;
        case boo::DragOperation::Link: return pal::CursorShape::DragLink;
        default:                       return pal::CursorShape::DragMove;
      }
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
    // While a drag runs, the keys go to it: Escape cancels it, and the modifiers held choose what a drop
    // would do.
    if (_drag)
    {
      if (key == pal::Key::Escape)
      {
        EndDrag(_drag->Position, false);
      }
      else
      {
        DragMoved(_drag->Position, modifiers);
      }
      PerformMicrotaskCheckpoint();
      return true;
    }

    // While an overlay is open, it takes the keys.
    if (_overlay)
    {
      RefPtr<Overlay> overlay = _overlay;
      bool handled = overlay->KeyPressed(key, modifiers);
      PerformMicrotaskCheckpoint();
      return handled;
    }

    // The input method's keys reach the page as the composition changes they make.
    if (text.HandledByInputMethod)
    {
      return true;
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
    // While a drag runs, the keys go to it: the modifiers held choose what a drop would do.
    if (_drag)
    {
      DragMoved(_drag->Position, modifiers);
      PerformMicrotaskCheckpoint();
      return true;
    }

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

  bool View::InsertText(const utf8_string &text) noexcept
  {
    if (_overlay || text.empty())
    {
      return false;
    }
    bool handled = _frame->GetEventHandler().KeyEvent(input::TextInputEvent(text));
    PerformMicrotaskCheckpoint();
    return handled;
  }

  void View::SetComposition(const utf8_string &text, Span<const pal::CompositionClause> clauses,
                            uint32 caret) noexcept
  {
    if (_overlay)
    {
      return;
    }
    (void)_frame->GetEventHandler().KeyEvent(
      input::InputMethodKeyboardEvent(text, input::CompositionUnderlines(text, clauses), caret, 0));
    PerformMicrotaskCheckpoint();
  }

  void View::ConfirmComposition(const utf8_string &text) noexcept
  {
    if (_overlay)
    {
      return;
    }
    (void)_frame->GetEventHandler().KeyEvent(input::InputMethodKeyboardEvent(text, null));
    PerformMicrotaskCheckpoint();
  }

  bool View::AcceptsTextInput() const noexcept
  {
    return !_overlay && _editorClient->AcceptsTextInput();
  }

  Maybe<boo::IntRect> View::TextInputRect() noexcept
  {
    auto *view = _frame->View();
    if (!view)
    {
      return null;
    }

    auto &document = Document();
    (void)document.UpdateLayout();
    auto &editor = document.GetEditor();
    boo::IntRect rect;
    if (auto range = editor.CompositionRange())
    {
      rect = editor.FirstRectForRange(*range);
    }
    else
    {
      rect = document.Selection().AbsoluteCaretBounds();
    }
    if (rect == boo::IntRect())
    {
      return null;
    }
    return view->ContentsToWindow(rect);
  }

  void View::SetCompositionDiscardedHandler(Func<void()> &&handler) noexcept
  {
    _editorClient->SetDiscardedCompositionHandler(krys::move(handler));
  }

  void View::ConfirmCompositionForHost() noexcept
  {
    auto &editor = Document().GetEditor();
    if (!editor.HasComposition())
    {
      return;
    }
    editor.ConfirmComposition();
    PerformMicrotaskCheckpoint();
    _editorClient->DiscardedComposition(Document());
  }

  void View::GamepadInput(Span<const pal::Gamepad> gamepads, MonotonicTime::duration now) noexcept
  {
    if (!_gamepadNavigationEnabled || !_focused)
    {
      ResetGamepadState();
      return;
    }

    using pal::GamepadAxis;
    using pal::GamepadButton;
    using FocusDirection = boo::dom::FocusDirection;
    auto pressed = [gamepads](GamepadButton button)
    {
      return std::ranges::any_of(gamepads, [button](const pal::Gamepad &gamepad)
                                 { return gamepad.Connected() && gamepad.IsButtonPressed(button); });
    };

    // The direction a pad holds: its D-pad, or else its left stick past the threshold, along its main axis.
    auto heldDirection = [](const pal::Gamepad &gamepad)
    {
      if (gamepad.IsButtonHeld(GamepadButton::DPadUp))
      {
        return FocusDirection::Up;
      }
      if (gamepad.IsButtonHeld(GamepadButton::DPadDown))
      {
        return FocusDirection::Down;
      }
      if (gamepad.IsButtonHeld(GamepadButton::DPadLeft))
      {
        return FocusDirection::Left;
      }
      if (gamepad.IsButtonHeld(GamepadButton::DPadRight))
      {
        return FocusDirection::Right;
      }
      float x = gamepad.AxisValue(GamepadAxis::LeftX);
      float y = gamepad.AxisValue(GamepadAxis::LeftY);
      if (std::max(std::abs(x), std::abs(y)) < GamepadStickThreshold)
      {
        return FocusDirection::None;
      }
      if (std::abs(x) > std::abs(y))
      {
        return x > 0 ? FocusDirection::Right : FocusDirection::Left;
      }
      return y > 0 ? FocusDirection::Down : FocusDirection::Up;
    };

    // The first pad that holds a direction, or pushes the right stick, has its way. The stick's push is
    // squared, so a light push scrolls slowly.
    auto direction = FocusDirection::None;
    double scrollX = 0;
    double scrollY = 0;
    for (auto &gamepad : gamepads)
    {
      if (!gamepad.Connected())
      {
        continue;
      }
      if (direction == FocusDirection::None)
      {
        direction = heldDirection(gamepad);
      }
      if (!scrollX && !scrollY)
      {
        double x = gamepad.AxisValue(GamepadAxis::RightX);
        double y = gamepad.AxisValue(GamepadAxis::RightY);
        scrollX = x * std::abs(x);
        scrollY = y * std::abs(y);
      }
    }

    // A new direction moves the focus at once; one held moves it again after a delay, then repeatedly.
    if (direction != _gamepadDirection)
    {
      _gamepadDirection = direction;
      if (direction != FocusDirection::None)
      {
        _gamepadNextRepeat = now + GamepadRepeatDelay;
        GamepadMove(direction);
      }
    }
    else if (direction != FocusDirection::None && now >= _gamepadNextRepeat)
    {
      _gamepadNextRepeat = now + GamepadRepeatInterval;
      GamepadMove(direction);
    }

    // A clicks the focused element, with a press and release before the click, as an access key does.
    if (pressed(GamepadButton::South) && !GamepadOverlayKey(pal::Key::Enter))
    {
      GamepadInteraction(true);
      if (RefPtr<boo::dom::Element> element = ShareRefPtr(Document().FocusedElement()))
      {
        (void)element->DispatchSimulatedClick(
          nullptr, boo::dom::SimulatedClickMouseEventOptions::SendMouseUpDownEvents);
      }
      PerformMicrotaskCheckpoint();
    }

    // B is a close request, as Escape is.
    if (pressed(GamepadButton::East) && !GamepadOverlayKey(pal::Key::Escape))
    {
      GamepadInteraction(false);
      if (RefPtr<boo::LocalDOMWindow> window = ShareRefPtr(Document().Window()))
      {
        (void)window->GetCloseWatcherManager().ProcessCloseWatchers();
      }
      PerformMicrotaskCheckpoint();
    }

    // The bumpers move the focus in sequence, as Tab and Shift+Tab do.
    if (pressed(GamepadButton::RightShoulder) && !GamepadOverlayKey(pal::Key::Tab))
    {
      GamepadInteraction(true);
      Ref document = ShareRef(Document());
      (void)boo::html::FocusAlgorithms::SequentialNavigation(*document, FocusDirection::Forward);
      PerformMicrotaskCheckpoint();
    }
    if (pressed(GamepadButton::LeftShoulder)
        && !GamepadOverlayKey(pal::Key::Tab, pal::KeyModifiers {pal::KeyModifier::Shift}))
    {
      GamepadInteraction(true);
      Ref document = ShareRef(Document());
      (void)boo::html::FocusAlgorithms::SequentialNavigation(*document, FocusDirection::Backward);
      PerformMicrotaskCheckpoint();
    }

    // The right stick scrolls for the time since the last frame it was pushed in.
    if ((scrollX || scrollY) && !_overlay)
    {
      if (_gamepadLastScroll)
      {
        auto elapsed = std::min<MonotonicTime::duration>(now - *_gamepadLastScroll, GamepadMaximumScrollStep);
        double distance = GamepadScrollSpeed * std::chrono::duration<double>(elapsed).count();
        GamepadScroll(boo::DoubleSize(scrollX * distance, scrollY * distance));
      }
      _gamepadLastScroll = now;
    }
    else
    {
      _gamepadLastScroll = null;
      _gamepadScrollRemainder = {};
    }
  }

  void View::SetGamepadNavigationEnabled(bool enabled) noexcept
  {
    _gamepadNavigationEnabled = enabled;
    if (!enabled)
    {
      ResetGamepadState();
    }
  }

  void View::GamepadInteraction(bool activates) noexcept
  {
    Ref document = ShareRef(Document());

    // A press other than a close request is an activation-triggering input, as a key press is.
    if (auto *window = document->Window(); window && activates)
    {
      window->NotifyActivated();
    }

    // As with the keyboard, the focused element matches :focus-visible.
    if (auto *element = document->FocusedElement())
    {
      element->SetHasFocusVisible(true);
    }
  }

  bool View::GamepadOverlayKey(pal::Key key, pal::KeyModifiers modifiers) noexcept
  {
    RefPtr<Overlay> overlay = _overlay;
    if (!overlay)
    {
      return false;
    }
    (void)overlay->KeyPressed(key, modifiers);
    PerformMicrotaskCheckpoint();
    return true;
  }

  void View::GamepadMove(boo::dom::FocusDirection direction) noexcept
  {
    using FocusDirection = boo::dom::FocusDirection;
    pal::Key key = pal::Key::DownArrow;
    switch (direction)
    {
      case FocusDirection::Up:    key = pal::Key::UpArrow; break;
      case FocusDirection::Down:  key = pal::Key::DownArrow; break;
      case FocusDirection::Left:  key = pal::Key::LeftArrow; break;
      case FocusDirection::Right: key = pal::Key::RightArrow; break;
      default:                    krys_debug_assert(false); return;
    }
    if (GamepadOverlayKey(key))
    {
      return;
    }

    GamepadInteraction(true);
    Ref document = ShareRef(Document());
    (void)boo::html::FocusAlgorithms::DirectionalNavigation(*document, direction);
    PerformMicrotaskCheckpoint();
    _needsPaint = true;
  }

  void View::GamepadScroll(const boo::DoubleSize &delta) noexcept
  {
    // Scrolls are in whole pixels; the rest waits for the next frame.
    boo::DoubleSize remainder(_gamepadScrollRemainder.Width() + delta.Width(),
                              _gamepadScrollRemainder.Height() + delta.Height());
    boo::DoubleSize step(std::trunc(remainder.Width()), std::trunc(remainder.Height()));
    _gamepadScrollRemainder =
      boo::DoubleSize(remainder.Width() - step.Width(), remainder.Height() - step.Height());

    Ref document = ShareRef(Document());
    (void)document->UpdateLayout();
    RefPtr<boo::LocalFrameView> view = ShareRefPtr(_frame->View());

    // Each axis scrolls the focused element's box (or else the nearest box around it that can), or else the
    // viewport, as the arrow keys do.
    auto scrollAxis = [&](double amount, boo::ScrollDirection backward, boo::ScrollDirection forward)
    {
      if (!amount)
      {
        return;
      }
      auto scrollDirection = amount < 0 ? backward : forward;
      auto stepCount = static_cast<uint32>(std::abs(amount));
      auto *element = document->FocusedElement();
      auto *renderer = element ? element->Renderer() : nullptr;
      if (renderer
          && renderer->EnclosingBox().Scroll(scrollDirection, boo::ScrollGranularity::Pixel, stepCount))
      {
        return;
      }
      if (view)
      {
        (void)view->Scroll(scrollDirection, boo::ScrollGranularity::Pixel, stepCount);
      }
    };
    scrollAxis(step.Width(), boo::ScrollDirection::ScrollLeft, boo::ScrollDirection::ScrollRight);
    scrollAxis(step.Height(), boo::ScrollDirection::ScrollUp, boo::ScrollDirection::ScrollDown);
    PerformMicrotaskCheckpoint();
    _needsPaint = true;
  }

  void View::ResetGamepadState() noexcept
  {
    _gamepadDirection = boo::dom::FocusDirection::None;
    _gamepadLastScroll = null;
    _gamepadScrollRemainder = {};
  }

  void View::SetFocused(bool focused) noexcept
  {
    if (!focused && _overlay)
    {
      RefPtr<Overlay> overlay = _overlay;
      overlay->Hide();
    }
    // The composition is confirmed, and a drag cancelled, as the window loses focus.
    if (!focused)
    {
      ConfirmCompositionForHost();
      if (_drag)
      {
        EndDrag(_drag->Position, false);
      }
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

  namespace
  {
    /// @brief The drag's flags for @p modifiers: Ctrl makes a drop copy.
    KRYS_NODISCARD EnumSet<boo::DragApplicationFlags> DragFlags(pal::KeyModifiers modifiers) noexcept
    {
      if (modifiers.Contains(pal::KeyModifier::Control))
      {
        return {boo::DragApplicationFlags::IsCopyKeyDown};
      }
      return {};
    }
  }

  void View::DragStarted(boo::dom::DataTransfer &dataTransfer) noexcept
  {
    ActiveDrag drag;
    drag.Data = dataTransfer.GetPasteboard().GetSelectionData();
    drag.SourceOperationMask = dataTransfer.SourceOperationMask();
    drag.Position = _lastMousePosition.value_or(boo::IntPoint());
    _drag = krys::move(drag);
  }

  void View::DragMoved(const boo::IntPoint &point, pal::KeyModifiers modifiers) noexcept
  {
    auto &drag = *_drag;
    drag.Position = point;
    drag.Modifiers = modifiers;
    boo::DragData data(&drag.Data, point, point, drag.SourceOperationMask, DragFlags(modifiers));

    auto size = ViewportSize();
    bool inView = point.X() >= 0 && point.Y() >= 0 && point.X() < size.Width() && point.Y() < size.Height();
    if (inView)
    {
      drag.Operation = _page->GetDragController().DragEnteredOrUpdated(*_frame, krys::move(data));
      drag.InView = true;
    }
    else if (drag.InView)
    {
      _page->GetDragController().DragExited(*_frame, krys::move(data));
      drag.InView = false;
      drag.Operation = null;
    }
  }

  void View::EndDrag(const boo::IntPoint &point, bool drop) noexcept
  {
    // The drag's data outlives the drop, which reads it.
    auto drag = krys::move(*_drag);
    _drag = null;

    auto &controller = _page->GetDragController();
    EnumSet<boo::DragOperation> operation;
    if (drag.InView)
    {
      boo::DragData data(&drag.Data, point, point, drag.SourceOperationMask, DragFlags(drag.Modifiers));
      if (drop && drag.Operation)
      {
        if (controller.PerformDragOperation(krys::move(data)))
        {
          operation = *drag.Operation;
        }
      }
      else
      {
        controller.DragExited(*_frame, krys::move(data));
      }
    }

    controller.DragEnded();
    _frame->GetEventHandler().DragSourceEndedAt(MouseEvent(boo::PlatformEvent::Type::MouseMoved, point,
                                                           boo::MouseButton::Left, 0,
                                                           input::ToBooey(drag.Modifiers)),
                                                operation);
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
