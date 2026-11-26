#include "Krystal.Booey/View.hpp"
#include "Krystal.Booey/ColorChooserOverlay.hpp"
#include "Krystal.Booey/Compositing/GraphicsLayerTextureMapper.hpp"
#include "Krystal.Booey/DataListSuggestionsOverlay.hpp"
#include "Krystal.Booey/DateTimeChooserOverlay.hpp"
#include "Krystal.Booey/EditorClient.hpp"
#include "Krystal.Booey/FileLoaderStrategy.hpp"
#include "Krystal.Booey/GraphicsContextRender2D.hpp"
#include "Krystal.Booey/Input.hpp"
#include "Krystal.Booey/PathStrokerStrategy.hpp"
#include "Krystal.Booey/PopupMenuOverlay.hpp"
#include "Krystal.Booey/SystemPasteboardStrategy.hpp"
#include "Krystal.Booey/ValidationMessageOverlay.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Types/SharedBuffer.hpp"
#include "Krystal.Net/Url.hpp"
#include "Krystal.Web/Accessibility/AXObjectCache.hpp"
#include "Krystal.Web/Css/Values/ValuePool.hpp"
#include "Krystal.Web/Dom/Algorithms/NameValidation.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Editing/Editor.hpp"
#include "Krystal.Web/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Web/Events/Pointer/PointerEvent.hpp"
#include "Krystal.Web/Graphics/Color/DestinationColorSpace.hpp"
#include "Krystal.Web/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Web/Graphics/Fonts/FontCache.hpp"
#include "Krystal.Web/Graphics/Fonts/FontDatabase.hpp"
#include "Krystal.Web/Html/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Web/Html/Algorithms/RenderBlocking.hpp"
#include "Krystal.Web/Html/CloseWatcher/CloseWatcherManager.hpp"
#include "Krystal.Web/Html/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Web/Html/DragAndDrop/DataTransfer.hpp"
#include "Krystal.Web/Html/EventLoop/EventLoop.hpp"
#include "Krystal.Web/Html/Parser/DocumentParser.hpp"
#include "Krystal.Web/Html/Window.hpp"
#include "Krystal.Web/Input/PlatformWheelEvent.hpp"
#include "Krystal.Web/Input/PointerCaptureController.hpp"
#include "Krystal.Web/Input/PointerLockController.hpp"
#include "Krystal.Web/Loader/DocumentLoader.hpp"
#include "Krystal.Web/Loader/DocumentWriter.hpp"
#include "Krystal.Web/Loader/FormSubmission.hpp"
#include "Krystal.Web/Loader/FrameLoader.hpp"
#include "Krystal.Web/Loader/FrameLoadRequest.hpp"
#include "Krystal.Web/Loader/NavigationAction.hpp"
#include "Krystal.Web/Loader/SubstituteData.hpp"
#include "Krystal.Web/Page/Drag/DragController.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Platform/Pasteboard/DragData.hpp"
#include "Krystal.Web/Platform/Pasteboard/Pasteboard.hpp"
#include "Krystal.Web/Platform/PlatformStrategies.hpp"
#include "Krystal.Web/Rendering/RenderBox.hpp"
#include "Krystal.Web/Rendering/RenderElement.hpp"
#include "Krystal.Web/Style/Scope/Scope.hpp"
#include "Krystal.Web/Style/Values/PointerEvents/TouchAction.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <limits>
#include <mutex>
#include <utility>

namespace krys::boo
{
  /// @brief Hands Krystal.Web the graphics layers the view composites, and keeps the root of the page's layer tree.
  class View::WebClient final : public web::WebClient
  {
  private:
    View *_view {nullptr};
    GraphicsLayerFactoryTextureMapper _factory;
    GraphicsLayerTextureMapper *_rootLayer {nullptr};
    Func<void()> _focusRequestHandler;
    Func<void(Ref<web::FileChooser>)> _fileChooserHandler;
    RefPtr<ValidationMessageOverlay> _validationMessage;

  public:
    void Focus() noexcept override
    {
      if (_focusRequestHandler)
      {
        _focusRequestHandler();
      }
    }

    void SetFocusRequestHandler(Func<void()> &&handler) noexcept
    {
      _focusRequestHandler = krys::move(handler);
    }

    // The display's colours, as the host says.
    KRYS_NODISCARD int32 ScreenDepthPerComponent() const noexcept override
    {
      return _view ? static_cast<int32>(_view->_displayColour.BitsPerComponent) : 8;
    }

    KRYS_NODISCARD bool ScreenSupportsExtendedColor() const noexcept override
    {
      return _view && _view->_displayColour.Gamut != pal::DisplayGamut::SRGB;
    }

    KRYS_NODISCARD bool ScreenSupportsRec2020Color() const noexcept override
    {
      return _view && _view->_displayColour.Gamut == pal::DisplayGamut::Rec2020;
    }

    KRYS_NODISCARD bool ScreenSupportsHighDynamicRange() const noexcept override
    {
      return _view && _view->_displayColour.HighDynamicRange;
    }

    void RunOpenPanel(web::Frame &, web::FileChooser &chooser) noexcept override
    {
      if (_fileChooserHandler)
      {
        _fileChooserHandler(ShareRef(chooser));
        return;
      }
      chooser.Cancel();
    }

    void SetFileChooserHandler(Func<void(Ref<web::FileChooser>)> &&handler) noexcept
    {
      _fileChooserHandler = krys::move(handler);
    }

    void ShowValidationMessage(web::Element &anchor, const web::DOMString &message) noexcept override
    {
      if (!_view)
      {
        return;
      }
      if (!_validationMessage)
      {
        _validationMessage = CreateRef<ValidationMessageOverlay>(*_view);
      }
      _validationMessage->Show(anchor, message);
    }

    void HideValidationMessage(const web::Element &anchor) noexcept override
    {
      if (_validationMessage && _validationMessage->IsShownAt(anchor))
      {
        _validationMessage->Hide();
      }
    }

    void HideAnyValidationMessage() noexcept override
    {
      if (_validationMessage)
      {
        _validationMessage->Hide();
      }
    }

    KRYS_NODISCARD bool IsValidationMessageVisible(const web::Element &anchor) const noexcept override
    {
      return _validationMessage && _validationMessage->IsShownAt(anchor);
    }

    void AttachRootGraphicsLayer(web::Frame &, web::GraphicsLayer *layer) noexcept override
    {
      _rootLayer = layer ? &Downcast<GraphicsLayerTextureMapper>(*layer) : nullptr;
    }

    KRYS_NODISCARD RefPtr<web::ImageBuffer>
      CreateImageBuffer(const SizeF &size, web::RenderingMode mode, web::RenderingPurpose purpose,
                        float resolutionScale, const web::DestinationColorSpace &) const noexcept override
    {
      // Accelerated buffers draw into textures with Render2D; the rest record into display lists.
      // FIXME: The colour space (sRGB, 8 bits a channel).
      if (!_view || mode != web::RenderingMode::Accelerated)
      {
        return nullptr;
      }
      return _view->_acceleratedImageBuffers->Create(size, purpose, resolutionScale);
    }

    KRYS_NODISCARD web::GraphicsLayerFactory *GetGraphicsLayerFactory() const noexcept override
    {
      return const_cast<GraphicsLayerFactoryTextureMapper *>(&_factory);
    }

    KRYS_NODISCARD RefPtr<web::PopupMenu>
      CreatePopupMenu(web::PopupMenuClient &client) const noexcept override
    {
      if (!_view)
      {
        return nullptr;
      }
      return CreateRef<PopupMenuOverlay>(*_view, client);
    }

    KRYS_NODISCARD RefPtr<web::ColorChooser>
      CreateColorChooser(web::ColorChooserClient &client, const web::Color &initialColor) noexcept override
    {
      if (!_view)
      {
        return nullptr;
      }
      return CreateRef<ColorChooserOverlay>(*_view, client, initialColor);
    }

    KRYS_NODISCARD RefPtr<web::DataListSuggestionPicker>
      CreateDataListSuggestionPicker(web::DataListSuggestionsClient &client) noexcept override
    {
      if (!_view)
      {
        return nullptr;
      }
      return CreateRef<DataListSuggestionsOverlay>(*_view, client);
    }

    KRYS_NODISCARD bool CanShowDataListSuggestionLabels() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD RefPtr<web::DateTimeChooser>
      CreateDateTimeChooser(web::DateTimeChooserClient &client) noexcept override
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

    // The mouse is the primary pointing device if there's one, else the touch screen, else the pen.
    KRYS_NODISCARD web::PointerCharacteristics PrimaryPointingDevice() const noexcept override
    {
      if (!_view)
      {
        return web::WebClient::PrimaryPointingDevice();
      }
      auto &devices = _view->_pointingDevices;
      if (devices.Mouse || (!devices.Touch && devices.Pen))
      {
        return web::PointerCharacteristic::Fine;
      }
      return devices.Touch ? web::PointerCharacteristic::Coarse : web::PointerCharacteristics {};
    }

    KRYS_NODISCARD bool HoverSupportedByPrimaryPointingDevice() const noexcept override
    {
      return PrimaryPointingDevice() == web::PointerCharacteristic::Fine;
    }

    KRYS_NODISCARD web::PointerCharacteristics AllAvailablePointingDevices() const noexcept override
    {
      if (!_view)
      {
        return web::WebClient::AllAvailablePointingDevices();
      }
      auto &devices = _view->_pointingDevices;
      auto characteristics = web::PointerCharacteristics {};
      if (devices.Mouse || devices.Pen)
      {
        characteristics = characteristics | web::PointerCharacteristic::Fine;
      }
      if (devices.Touch)
      {
        characteristics = characteristics | web::PointerCharacteristic::Coarse;
      }
      return characteristics;
    }

    // A pen hovers, as the mouse does.
    KRYS_NODISCARD bool HoverSupportedByAnyAvailablePointingDevice() const noexcept override
    {
      return !_view || _view->_pointingDevices.Mouse || _view->_pointingDevices.Pen;
    }

    // The root view is the view, wherever the host put it on the screen.
    KRYS_NODISCARD PointI RootViewToScreen(const PointI &point) const noexcept override
    {
      return _view ? _view->ToScreen(point) : point;
    }

    KRYS_NODISCARD PointI ScreenToRootView(const PointI &point) const noexcept override
    {
      if (!_view)
      {
        return point;
      }
      return point - ToSizeI(_view->ToPage(_view->_screenOrigin));
    }

    void RequestPointerLock(Func<void(web::PointerLockRequestResult)> completionHandler) noexcept override
    {
      completionHandler(_view ? _view->LockPointer() : web::PointerLockRequestResult::Unsupported);
    }

    void RequestPointerUnlock(Func<void(bool)> completionHandler) noexcept override
    {
      completionHandler(_view && _view->UnlockPointer());
    }

    void PostAccessibilityNotification(web::AXObject &object,
                                       web::AXNotification notification) noexcept override
    {
      if (auto *accessibility = Accessibility())
      {
        accessibility->NotificationPosted(object, notification);
      }
    }

    void AccessibilityObjectDetached(web::AXObject &object, web::AXDetachmentType) noexcept override
    {
      if (auto *accessibility = Accessibility())
      {
        accessibility->ObjectDetached(object);
      }
    }

    void PostAccessibilityTextChange(web::AXObject &object, web::AXTextChange change, uint64 offset,
                                     utf8_stringview text) noexcept override
    {
      if (auto *accessibility = Accessibility())
      {
        accessibility->TextChanged(object, change, offset, text);
      }
    }

    /// @brief What exposes the page: the view's, or for an overlay's page, its owner's.
    KRYS_NODISCARD ViewAccessibility *Accessibility() const noexcept
    {
      if (!_view)
      {
        return nullptr;
      }
      if (_view->_accessibility)
      {
        return _view->_accessibility.get();
      }
      return _view->_overlayOwner ? _view->_overlayOwner->_accessibility.get() : nullptr;
    }

    void SetView(View *view) noexcept
    {
      if (auto validationMessage = std::exchange(_validationMessage, nullptr))
      {
        validationMessage->Detach();
      }
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
  class View::DragClient final : public web::DragClient
  {
  private:
    View *_view {nullptr};

  public:
    void SetView(View *view) noexcept
    {
      _view = view;
    }

    void WillPerformDragDestinationAction(web::DragDestinationAction, const web::DragData &) noexcept override
    {
    }

    void WillPerformDragSourceAction(web::DragSourceAction, const PointI &,
                                     web::DataTransfer &) noexcept override
    {
    }

    KRYS_NODISCARD EnumSet<web::DragSourceAction>
      DragSourceActionMaskForPoint(const PointI &) noexcept override
    {
      return web::AnyDragSourceAction();
    }

    void StartDrag(web::DragItem item, web::DataTransfer &dataTransfer, web::Frame &) noexcept override
    {
      if (_view)
      {
        _view->DragStarted(krys::move(item), dataTransfer);
      }
    }
  };

  class View::FrameLoaderClient final : public web::FrameLoaderClient
  {
  private:
    View *_view {nullptr};
    Func<void(const NavigationRequest &)> _navigationHandler;
    Func<void(const LoadEvent &)> _loadHandler;
    Func<void(const PageIcon &)> _iconHandler;

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
      if (_view)
      {
        _view->DidCommitLoad();
      }
      Notify({LoadEventType::Committed, {}});
    }

    void DispatchDidFinishLoad() noexcept override
    {
      Notify({LoadEventType::Finished, {}});
    }

    void DispatchDidFailProvisionalLoad(const net::ResourceError &error) noexcept override
    {
      Notify({LoadEventType::Failed, error});
    }

    void DispatchDidFailLoad(const net::ResourceError &error) noexcept override
    {
      Notify({LoadEventType::Failed, error});
    }

    void SetLoadHandler(Func<void(const LoadEvent &)> &&handler) noexcept
    {
      _loadHandler = krys::move(handler);
    }

    void SetIconHandler(Func<void(const PageIcon &)> &&handler) noexcept
    {
      _iconHandler = krys::move(handler);
    }

    void GetLoadDecisionForIcons(Span<const web::LinkIconLoadDecision> icons) noexcept override
    {
      RefPtr<web::DocumentLoader> documentLoader =
        _view ? ShareRefPtr(_view->_frame->Loader().GetDocumentLoader()) : nullptr;
      if (!documentLoader)
      {
        return;
      }
      // Every icon loads for a handler; none without one.
      for (const auto &decision : icons)
      {
        documentLoader->DidGetLoadDecisionForIcon(
          !!_iconHandler, decision.Identifier,
          [this, icon = decision.Icon](const SharedBuffer *data)
          {
            if (data && _iconHandler)
            {
              _iconHandler({icon.Url.String(), utf8_string(icon.MimeType), icon.Size, data->GetSpan()});
            }
          });
      }
    }

    void Notify(const LoadEvent &event) const noexcept
    {
      if (_loadHandler)
      {
        _loadHandler(event);
      }
    }

    void DispatchNavigationRequested(const web::NavigationAction &action, const net::ResourceRequest &request,
                                     const web::FormSubmission *formSubmission) noexcept override
    {
      Request(action, request, formSubmission, {});
    }

    void DispatchNewWindowRequested(const web::NavigationAction &action, const net::ResourceRequest &request,
                                    const web::FormSubmission *formSubmission,
                                    web::DOMStringView frameName) noexcept override
    {
      Request(action, request, formSubmission, frameName);
    }

    void SetView(View *view) noexcept
    {
      _view = view;
    }

    void SetNavigationHandler(Func<void(const NavigationRequest &)> &&handler) noexcept
    {
      _navigationHandler = krys::move(handler);
    }

  private:
    /// @param newWindowName The browsing context a new window would be; null (a null view) for the view's
    /// own.
    void Request(const web::NavigationAction &action, const net::ResourceRequest &request,
                 const web::FormSubmission *formSubmission, web::DOMStringView newWindowName) const noexcept
    {
      if (!_navigationHandler)
      {
        return;
      }

      NavigationRequest navigation;
      navigation.Url = request.Url().String();
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
      if (action.DownloadAttribute() != null)
      {
        navigation.Download = action.DownloadAttribute().ToString();
      }
      navigation.Method = request.HttpMethod();
      if (formSubmission)
      {
        navigation.IsFormSubmission = true;
        navigation.Target = formSubmission->Target();
        if (!navigation.SourceElement)
        {
          navigation.SourceElement = ShareRefPtr<web::Element>(formSubmission->Form());
        }
        navigation.EncodingType = formSubmission->EncodingType();
        navigation.Entries = ShareRefPtr(formSubmission->EntryList());
      }
      _navigationHandler(navigation);
    }
  };

  namespace
  {
    /// @brief The main frame's loader client, noted in @p frameLoaderClient as it's handed to the page.
    KRYS_NODISCARD UniquePtr<web::FrameLoaderClient>
      CreateFrameLoaderClient(View::FrameLoaderClient *&frameLoaderClient) noexcept
    {
      auto client = CreateUnique<View::FrameLoaderClient>();
      frameLoaderClient = client.get();
      return client;
    }

    /// @brief The page's editor client, noted in @p editorClient as it's handed to the page.
    KRYS_NODISCARD UniquePtr<web::EditorClient> CreateEditorClient(EditorClient *&editorClient) noexcept
    {
      auto client = CreateUnique<EditorClient>();
      editorClient = client.get();
      return client;
    }

    /// @brief The page's drag client, noted in @p dragClient as it's handed to the page.
    KRYS_NODISCARD UniquePtr<web::DragClient> CreateDragClient(View::DragClient *&dragClient) noexcept
    {
      auto client = CreateUnique<View::DragClient>();
      dragClient = client.get();
      return client;
    }

    /// @brief The page's Krystal.Web client, noted in @p webClient as it's handed to the page.
    KRYS_NODISCARD UniquePtr<web::WebClient> CreateWebClient(View::WebClient *&webClient) noexcept
    {
      auto client = CreateUnique<View::WebClient>();
      webClient = client.get();
      return client;
    }
  }

  View::View(const SizeI &viewportSize) noexcept
      : _frame(web::Frame::Create(CreateWebClient(_webClient), CreateEditorClient(_editorClient),
                                  CreateFrameLoaderClient(_frameLoaderClient), CreateDragClient(_dragClient)))
  {
    static std::once_flag valuePool;
    std::call_once(valuePool, [] { web::css::StaticValuePool::Init(); });

    auto &strategies = web::PlatformStrategies::Singleton();
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
    if (!strategies.GetPathStrokeStrategy())
    {
      static PathStrokerStrategy pathStroker;
      strategies.SetPathStrokeStrategy(&pathStroker);
    }

    _webClient->SetView(this);
    _frameLoaderClient->SetView(this);
    _dragClient->SetView(this);
    _viewportPixels = viewportSize;
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
    _accessibility.reset();
    _webClient->SetView(nullptr);
    _frameLoaderClient->SetView(nullptr);
    _dragClient->SetView(nullptr);
    // The layers go before the texture mapper that holds their textures.
    _frame->SetDocument(nullptr);
  }

  void View::AttachAccessibility(pal::IWindow &window, const PointI &origin) noexcept
  {
    if (!_accessibility)
    {
      _accessibility = CreateUnique<ViewAccessibility>(*this);
    }
    _accessibility->Attach(window, origin);
  }

  void View::DetachAccessibility() noexcept
  {
    _accessibility.reset();
  }

  web::HTMLDocument &View::Document() const noexcept
  {
    return Downcast<web::HTMLDocument>(*_frame->Document());
  }

  void View::LoadHTML(utf8_string markup, net::Url &&url, web::CachePolicy cachePolicy) noexcept
  {
    _frame->SetSubresourceCachePolicy(cachePolicy);
    auto requestUrl = url.IsNull() ? net::Url(u8"about:blank") : url;
    web::FrameLoadRequest request(ShareRef<web::Document>(Document()),
                                  net::ResourceRequest(krys::move(requestUrl)), null);
    request.SetSubstituteData(web::SubstituteData(krys::move(markup), u8"text/html", krys::move(url)));
    _frame->Loader().Load(krys::move(request));
  }

  Expected<void> View::LoadFile(const std::filesystem::path &path, web::CachePolicy cachePolicy) noexcept
  {
    std::error_code error;
    if (!std::filesystem::is_regular_file(path, error))
    {
      return Unexpected<string>(std::format("Couldn't open {}", path.string()));
    }
    auto absolutePath = std::filesystem::absolute(path).generic_u8string();
    Load(net::Url::FileUrlWithFileSystemPath(absolutePath), cachePolicy);
    return {};
  }

  void View::Load(net::Url &&url, web::CachePolicy cachePolicy) noexcept
  {
    _frame->SetSubresourceCachePolicy(cachePolicy);
    web::FrameLoadRequest request(ShareRef<web::Document>(Document()), net::ResourceRequest(krys::move(url)),
                                  null);
    _frame->Loader().Load(krys::move(request));
  }

  bool View::IsLoading() const noexcept
  {
    return _frame->Loader().IsLoading();
  }

  void View::Stop() noexcept
  {
    _frame->Loader().StopAllLoaders();
  }

  void View::SetLoadHandler(Func<void(const LoadEvent &)> &&handler) noexcept
  {
    _frameLoaderClient->SetLoadHandler(krys::move(handler));
  }

  void View::SetIconHandler(Func<void(const PageIcon &)> &&handler) noexcept
  {
    _frameLoaderClient->SetIconHandler(krys::move(handler));
  }

  void View::Reload() noexcept
  {
    _frame->Loader().Reload();
    PerformMicrotaskCheckpoint();
  }

  void View::DidCommitLoad() noexcept
  {
    // A committed document takes the window's focus and is painted; the old one's overlay goes.
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
                                                       web::html::CustomElementFactory(definition.Factory),
                                                       web::html::CustomElementOptions(definition.Options));
    }
  }

  void View::ReplaceDocument(utf8_string markup) noexcept
  {
    // The document is parsed in the frame, as a navigation's is, so it's fully active as its elements are
    // inserted; it replaces the current one at once, without a load.
    web::DocumentWriter writer;
    writer.SetFrame(*_frame);
    if (!writer.Begin(net::Url(u8"about:blank")))
    {
      return;
    }
    writer.AddData(Span<const uint8>(reinterpret_cast<const uint8 *>(markup.data()), markup.size()));
    writer.End();
  }

  Expected<void> View::DefineCustomElement(const web::DOMStringAtom &name,
                                           web::html::CustomElementFactory &&factory,
                                           web::html::CustomElementOptions &&options) noexcept
  {
    if (!web::dom::NameValidation::IsValidCustomElementName(name.View()))
    {
      return Unexpected<string>(std::format("{} isn't a valid custom element name",
                                            std::string(name.View().begin(), name.View().end())));
    }
    for (auto &definition : _customElements)
    {
      if (definition.Name == name)
      {
        return Unexpected<string>(
          std::format("{} is already defined", std::string(name.View().begin(), name.View().end())));
      }
    }

    auto &definition = _customElements.emplace_back(name, krys::move(factory), krys::move(options));
    (void)Document().Window()->CustomElements().Define(definition.Name,
                                                       web::html::CustomElementFactory(definition.Factory),
                                                       web::html::CustomElementOptions(definition.Options));
    return {};
  }

  uint32 View::AddFontDirectory(const std::filesystem::path &directory)
  {
    auto count = web::FontDatabase::Singleton().AddFontDirectory(directory);
    if (count)
    {
      web::FontCache::InvalidateAllFontCaches();
    }
    return count;
  }

  void View::Resize(const SizeI &viewportSize) noexcept
  {
    // The overlay was placed in the old viewport.
    if (_overlay)
    {
      _overlay->Hide();
    }
    _viewportPixels = viewportSize;
    if (auto *view = _frame->View())
    {
      // The page lays out over as many CSS pixels as fit, to the nearest.
      auto scale = DeviceScaleFactor();
      view->Resize(SizeI(static_cast<int32>(std::lround(viewportSize.Width() / scale)),
                         static_cast<int32>(std::lround(viewportSize.Height() / scale))));
    }
    _needsPaint = true;
  }

  void View::ResizeToLayoutSize(const SizeI &size) noexcept
  {
    auto scale = DeviceScaleFactor();
    _viewportPixels = SizeI(static_cast<int32>(std::lround(size.Width() * scale)),
                            static_cast<int32>(std::lround(size.Height() * scale)));
    if (auto *view = _frame->View())
    {
      view->Resize(size);
    }
    _needsPaint = true;
  }

  SizeI View::ViewportSize() const noexcept
  {
    auto *view = _frame->View();
    return view ? SizeI(view->Width(), view->Height()) : SizeI();
  }

  void View::SetDeviceScaleFactor(float scale) noexcept
  {
    if (!std::isfinite(scale) || scale <= 0 || scale == DeviceScaleFactor())
    {
      return;
    }
    _frame->SetDeviceScaleFactor(scale);
    // The viewport's CSS pixels change with it.
    Resize(_viewportPixels);
  }

  float View::DeviceScaleFactor() const noexcept
  {
    return _frame->DeviceScaleFactor();
  }

  PointI View::ToPage(const PointI &point) const noexcept
  {
    return FlooredPointI(ToPage(PointD(point)));
  }

  PointD View::ToPage(const PointD &point) const noexcept
  {
    double scale = DeviceScaleFactor();
    return PointD(point.X() / scale, point.Y() / scale);
  }

  pal::PointerInput View::ToPage(const pal::PointerInput &input) const noexcept
  {
    auto scale = DeviceScaleFactor();
    auto result = input;
    result.Width /= scale;
    result.Height /= scale;
    return result;
  }

  pal::TouchpadPanInput View::ToPage(const pal::TouchpadPanInput &input) const noexcept
  {
    auto scale = DeviceScaleFactor();
    auto result = input;
    result.DeltaX /= scale;
    result.DeltaY /= scale;
    return result;
  }

  RectI View::ToViewPixels(const RectI &rect) const noexcept
  {
    RectF scaled(rect);
    scaled.Scale(DeviceScaleFactor());
    return EnclosingRectI(scaled);
  }

  bool View::Update(MonotonicTime::duration now, Maybe<MonotonicTime::duration> budget) noexcept
  {
    web::html::EventLoop::Current().Run(now, budget);

    // The fling moves on with the host's time, before the rendering update shows it.
    if (_fling)
    {
      StepFling(now);
    }

    // The overlay's page shares the event loop, and updates with the page.
    bool overlayUpdated = false;
    if (_overlay && _overlay->ContentView().Frame().IsVisible()
        && _overlay->ContentView().Frame().IsRenderingUpdateScheduled())
    {
      _overlay->ContentView().UpdateRendering();
      overlayUpdated = true;
    }

    // Each frame is a rendering opportunity, except for a hidden page, whose update waits until it's shown.
    bool updated = overlayUpdated;
    if (_frame->IsVisible() && _frame->IsRenderingUpdateScheduled())
    {
      _frame->UpdateRendering();
      _needsPaint = true;
      updated = true;
    }
    return updated;
  }

  void View::PerformMicrotaskCheckpoint() noexcept
  {
    // Input is dispatched as a task would be, so a microtask checkpoint follows it.
    // https://html.spec.whatwg.org/multipage/webappapis.html#event-loop-processing-model
    web::html::EventLoop::Current().PerformMicrotaskCheckpoint();
  }

  void View::UpdateRendering() noexcept
  {
    _frame->UpdateRendering();
    _needsPaint = true;
  }

  PointI View::ScrollPosition() const noexcept
  {
    auto *view = _frame->View();
    return view ? view->GetScrollPosition() : PointI();
  }

  void View::ScrollTo(const PointI &position) noexcept
  {
    (void)Document().UpdateLayout();
    if (auto *view = _frame->View())
    {
      view->SetScrollPosition(position);
    }
    _needsPaint = true;
  }

  Ref<const web::display_list::DisplayList> View::Record() noexcept
  {
    // The rendering update runs in Update; painting only needs the layout.
    (void)Document().UpdateLayout();
    const auto size = ViewportSize();
    web::display_list::RecorderImpl recorder(
      SizeF(static_cast<float>(size.Width()), static_cast<float>(size.Height())));
    if (auto *view = _frame->View())
    {
      // The composited layers paint into the recording too, as a snapshot: canvases that show their pixels
      // as layer contents paint them.
      auto paintBehavior = view->GetPaintBehavior();
      auto flattened = paintBehavior;
      flattened.Add({web::PaintBehavior::FlattenCompositingLayers, web::PaintBehavior::Snapshotting});
      view->SetPaintBehavior(flattened);
      view->Paint(recorder, RectI(0, 0, size.Width(), size.Height()));
      view->SetPaintBehavior(paintBehavior);
    }
    if (_overlay)
    {
      auto overlayList = _overlay->ContentView().Record();
      auto &rect = _overlay->Rect();
      web::GraphicsContextStateSaver stateSaver(recorder);
      recorder.Translate(static_cast<float>(rect.X()), static_cast<float>(rect.Y()));
      recorder.Clip(RectF(0, 0, static_cast<float>(rect.Width()), static_cast<float>(rect.Height())));
      recorder.DrawDisplayList(*overlayList);
    }
    DrawDragImage(recorder);
    return recorder.TakeDisplayList();
  }

  bool View::NeedsPaint() const noexcept
  {
    auto *view = _frame->View();
    return _needsPaint || (view && view->HasDamage()) || (_overlay && _overlay->ContentView().NeedsPaint());
  }

  void View::SetDevice(gfx::Device &device) noexcept
  {
    _acceleratedImageBuffers->SetDevice(device);
  }

  void View::Paint(gfx::Renderer2D &renderer) noexcept
  {
    const auto size = ViewportSize();
    auto *view = _frame->View();
    if (!view || size.IsEmpty() || !renderer.GetCommandBuffer())
    {
      return;
    }
    SetDevice(renderer.GetDevice());
    // The host has run the last frame's commands, which may have drawn canvases' snapshots now gone.
    _acceleratedImageBuffers->DestroyRetiredTextures();

    if (!_textureMapper)
    {
      auto textureMapper = TextureMapper::Create(renderer.GetDevice(), _fonts);
      if (!textureMapper)
      {
        return;
      }
      _textureMapper = krys::move(*textureMapper);
    }

    // A render-blocked document isn't shown: the last frame stays until it's unblocked.
    if (web::html::RenderBlocking::IsRenderBlocked(Document()))
    {
      return;
    }

    // The rendering update runs in Update; painting only needs the layout and the layers up to date.
    (void)Document().UpdateLayout({web::layout::LayoutOptions::UpdateCompositingLayers});
    (void)view->FlushCompositingState();

    // What the view repaints is in its layers: the damage it tracks for a page that isn't composited is
    // dropped.
    (void)view->TakeDamage();
    _needsPaint = false;

    auto *root = _webClient->RootLayer();
    if (!root)
    {
      return;
    }

    // Layers that animate are composited at the frame's time, and ask for the next frame while they run.
    (void)root->Layer().ApplyAnimationsRecursively(web::html::EventLoop::Current().Now());

    // The layers are in CSS pixels, and their backing stores are drawn at the scale.
    renderer.Save();
    auto scale = DeviceScaleFactor();
    renderer.Scale(scale, scale);
    _textureMapper->BeginPainting(renderer, RectI(0, 0, size.Width(), size.Height()));
    root->UpdateBackingStoreIncludingSubLayers(*_textureMapper);
    root->Layer().Paint(*_textureMapper);
    _textureMapper->EndPainting();
    renderer.Restore();

    _needsPaint = root->Layer().DescendantsOrSelfHaveRunningAnimations();
    PaintOverlay(renderer);
    PaintDragImage(renderer);
  }

  void View::PaintOverlay(gfx::Renderer2D &renderer) noexcept
  {
    if (!_overlay)
    {
      return;
    }

    // The overlay's view paints at its own scale, the same as this one's.
    auto scale = DeviceScaleFactor();
    auto &rect = _overlay->Rect();
    renderer.Save();
    renderer.Translate(static_cast<float>(rect.X()) * scale, static_cast<float>(rect.Y()) * scale);
    renderer.ClipRect(
      {0.f, 0.f, static_cast<float>(rect.Width()) * scale, static_cast<float>(rect.Height()) * scale});
    _overlay->ContentView().Paint(renderer);
    renderer.Restore();
  }

  void View::PaintDragImage(gfx::Renderer2D &renderer) noexcept
  {
    if (!_drag || !_drag->Image)
    {
      return;
    }

    const auto size = ViewportSize();
    web::display_list::RecorderImpl recorder(
      SizeF(static_cast<float>(size.Width()), static_cast<float>(size.Height())));
    DrawDragImage(recorder);
    auto displayList = recorder.TakeDisplayList();
    renderer.Save();
    auto scale = DeviceScaleFactor();
    renderer.Scale(scale, scale);
    Replay(*displayList, renderer);
    renderer.Restore();
  }

  void View::DrawDragImage(web::GraphicsContext &context) const noexcept
  {
    if (!_drag || !_drag->Image)
    {
      return;
    }

    auto &image = *_drag->Image.get();
    // The image is in device pixels.
    SizeF size(web::DragImageSize(_drag->Image.get()));
    size.Scale(1 / _frame->Document()->DeviceScaleFactor());
    web::GraphicsContextStateSaver stateSaver(context);
    if (_drag->FadeImage)
    {
      context.SetAlpha(web::DragController::DragImageAlpha);
    }
    context.DrawImageBuffer(image, RectF(PointF(_drag->Position + _drag->ImageOffset), size));
  }

  void View::ShowOverlay(Overlay &overlay) noexcept
  {
    if (_overlay && _overlay.get() != &overlay)
    {
      RefPtr<Overlay> previous = _overlay;
      previous->Hide();
    }
    bool isNew = _overlay.get() != &overlay;
    _overlay = ShareRefPtr(&overlay);
    _pageOwnsPress = _buttons != 0;
    _needsPaint = true;
    if (isNew)
    {
      overlay.ContentView()._overlayOwner = this;
      if (_accessibility)
      {
        _accessibility->OverlayShown(overlay);
      }
    }
  }

  void View::OverlayHidden(Overlay &overlay) noexcept
  {
    if (_overlay.get() == &overlay)
    {
      _overlay = nullptr;
      _needsPaint = true;
      overlay.ContentView()._overlayOwner = nullptr;
      if (_accessibility)
      {
        _accessibility->OverlayHidden(overlay);
      }
    }
  }

  void View::Replay(const web::display_list::DisplayList &displayList, gfx::Renderer2D &renderer) noexcept
  {
    const auto size = ViewportSize();
    SetDevice(renderer.GetDevice());
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

  web::Element *View::HitTest(const PointI &point) noexcept
  {
    return HitTestInPage(ToPage(point));
  }

  web::Element *View::HitTestInPage(const PointI &point) const noexcept
  {
    return Document().ElementFromPoint(point.X(), point.Y()).get();
  }

  void View::MouseMoved(const PointI &viewPoint, pal::KeyModifiers modifiers) noexcept
  {
    auto point = ToPage(viewPoint);
    // Windows reports moves when the mouse hasn't moved (raw input reports presses and releases as moves
    // too), and one while a button is held would start a selection.
    auto lastPosition = std::exchange(_lastMousePosition, point);
    if (lastPosition == point)
    {
      return;
    }

    // While a modal overlay is open, the page doesn't see the mouse move; it sees it outside one that isn't.
    if (_overlay)
    {
      auto &rect = _overlay->Rect();
      if (rect.Contains(point))
      {
        _overlay->MouseMoved(PointI(point.X() - rect.X(), point.Y() - rect.Y()));
        return;
      }
      if (_overlay->IsModal())
      {
        return;
      }
    }

    // While a drag runs, the mouse moves it.
    if (_drag)
    {
      DragMoved(point, modifiers);
      PerformMicrotaskCheckpoint();
      return;
    }

    // The move's movement is how far the mouse went since the last.
    PointD movement;
    if (lastPosition)
    {
      movement = PointD(point.X() - lastPosition->X(), point.Y() - lastPosition->Y());
    }
    (void)_frame->GetEventHandler().MouseMoved(MouseEvent(
      web::PlatformEvent::Type::MouseMoved, point, HeldButton(), 0, input::ToWeb(modifiers), movement));
    // A drag that the move started enters the page at once.
    if (_drag)
    {
      DragMoved(point, modifiers);
    }
    PerformMicrotaskCheckpoint();
  }

  void View::LockedMouseMoved(const PointD &movement, const PointD &unadjustedMovement,
                              pal::KeyModifiers modifiers) noexcept
  {
    if (!_pointerLocked)
    {
      return;
    }
    // The mouse stays where it was when the pointer was locked.
    (void)_frame->GetEventHandler().MouseMoved(
      MouseEvent(web::PlatformEvent::Type::MouseMoved, _lastMousePosition.value_or(PointI()), HeldButton(), 0,
                 input::ToWeb(modifiers), ToPage(movement), unadjustedMovement));
    PerformMicrotaskCheckpoint();
  }

  web::MouseButton View::HeldButton() const noexcept
  {
    if (_buttons & ButtonsBit(web::MouseButton::Left))
    {
      return web::MouseButton::Left;
    }
    if (_buttons & ButtonsBit(web::MouseButton::Middle))
    {
      return web::MouseButton::Middle;
    }
    if (_buttons & ButtonsBit(web::MouseButton::Right))
    {
      return web::MouseButton::Right;
    }
    return web::MouseButton::None;
  }

  void View::MousePressed(const PointI &viewPoint, pal::MouseButton palButton,
                          pal::KeyModifiers modifiers) noexcept
  {
    auto point = ToPage(viewPoint);
    auto button = input::ToWeb(palButton);
    if (button == web::MouseButton::None)
    {
      return;
    }

    // A press outside the overlay closes it, and goes no further unless the overlay isn't modal.
    if (_overlay)
    {
      if (_overlay->Rect().Contains(point) || _overlay->IsModal())
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
      RefPtr<Overlay> overlay = _overlay;
      overlay->Hide();
      PerformMicrotaskCheckpoint();
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
    auto event = MouseEvent(web::PlatformEvent::Type::MousePressed, point, button, _clickCount,
                            input::ToWeb(modifiers));
    (void)_frame->GetEventHandler().HandleMousePressEvent(event);
    // The right button's press asks for a context menu: there's none, but the page gets `contextmenu`.
    if (button == web::MouseButton::Right)
    {
      (void)_frame->GetEventHandler().SendContextMenuEvent(event);
    }
    PerformMicrotaskCheckpoint();
  }

  void View::MouseReleased(const PointI &viewPoint, pal::MouseButton palButton,
                           pal::KeyModifiers modifiers) noexcept
  {
    auto point = ToPage(viewPoint);
    auto button = input::ToWeb(palButton);
    if (button == web::MouseButton::None)
    {
      return;
    }

    _buttons &= static_cast<uint16>(~ButtonsBit(button));
    _lastMousePosition = point;

    // The release of the button that's dragging drops the drag; the page doesn't see it, as it doesn't see
    // the mouse while a system drag runs.
    if (_drag)
    {
      if (button == web::MouseButton::Left)
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
        overlay->MouseReleased(PointI(point.X() - rect.X(), point.Y() - rect.Y()));
        PerformMicrotaskCheckpoint();
      }
      return;
    }

    (void)_frame->GetEventHandler().HandleMouseReleaseEvent(MouseEvent(
      web::PlatformEvent::Type::MouseReleased, point, button, _clickCount, input::ToWeb(modifiers)));
    PerformMicrotaskCheckpoint();
  }

  void View::MouseLeft() noexcept
  {
    // A move outside the viewport hits nothing, so the element under the mouse is left and nothing is
    // hovered.
    MouseMoved(PointI(-1, -1));
  }

  bool View::MouseWheel(const PointI &point, float notchesX, float notchesY, pal::KeyModifiers modifiers,
                        const pal::WheelScrollSettings &settings) noexcept
  {
    return MouseWheelInPage(ToPage(point), notchesX, notchesY, modifiers, settings);
  }

  bool View::MouseWheelInPage(const PointI &point, float notchesX, float notchesY,
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
             && _overlay->MouseWheel(PointI(point.X() - rect.X(), point.Y() - rect.Y()), notchesX, notchesY);
    }
    bool handled = _frame->GetEventHandler().HandleWheelEvent(
      input::WheelEvent(point, ToScreen(point), notchesX, notchesY, modifiers, settings));
    PerformMicrotaskCheckpoint();
    return handled;
  }

  bool View::TouchpadPan(const PointI &viewPoint, const pal::TouchpadPanInput &viewInput) noexcept
  {
    auto point = ToPage(viewPoint);
    auto input = ToPage(viewInput);
    if (_overlay)
    {
      // Overlays take the wheel's notches; the content moving down is the wheel turning away.
      constexpr float pixelsPerNotch = 3 * web::ScrollbarPixelsPerLine;
      float notchesX = -input.DeltaX / pixelsPerNotch;
      float notchesY = input.DeltaY / pixelsPerNotch;
      auto &rect = _overlay->Rect();
      return (notchesX || notchesY) && rect.Contains(point)
             && _overlay->MouseWheel(PointI(point.X() - rect.X(), point.Y() - rect.Y()), notchesX, notchesY);
    }
    bool handled =
      _frame->GetEventHandler().HandleWheelEvent(input::TouchpadPanEvent(point, ToScreen(point), input));
    PerformMicrotaskCheckpoint();
    return handled;
  }

  int32 View::CountClick(const PointI &point, web::MouseButton button, int32 distance) noexcept
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

  void View::PenInput(const PointD &viewPoint, const pal::PointerInput &viewInput) noexcept
  {
    // While an overlay is open, the pen is the mouse.
    if (_overlay)
    {
      auto mousePoint = FlooredPointI(viewPoint);
      switch (viewInput.Action)
      {
        case pal::PointerAction::Pressed:
          MousePressed(mousePoint, pal::MouseButton::Left, viewInput.Modifiers);
          break;
        case pal::PointerAction::Released:
          MouseReleased(mousePoint, pal::MouseButton::Left, viewInput.Modifiers);
          break;
        case pal::PointerAction::Entered:
        case pal::PointerAction::Moved:   MouseMoved(mousePoint, viewInput.Modifiers); break;
        default:                          break;
      }
      return;
    }

    auto point = ToPage(viewPoint);
    auto input = ToPage(viewInput);
    auto intPoint = FlooredPointI(point);

    auto &handler = _frame->GetEventHandler();
    switch (input.Action)
    {
      case pal::PointerAction::Entered:
      case pal::PointerAction::Moved:
      {
        (void)handler.MouseMoved(PenEvent(web::PlatformEvent::Type::MouseMoved, point, _penButton, 0, input));
        break;
      }
      case pal::PointerAction::Pressed:
      {
        ConfirmCompositionForHost();
        // https://w3c.github.io/pointerevents/#the-button-property
        _penButton = input.Eraser         ? web::MouseButton::Other
                     : input.BarrelButton ? web::MouseButton::Right
                                          : web::MouseButton::Left;
        _penButtons |= _penButton == web::MouseButton::Other ? uint16 {32} : ButtonsBit(_penButton);
        int32 clickCount = CountClick(intPoint, _penButton);
        auto event = PenEvent(web::PlatformEvent::Type::MousePressed, point, _penButton, clickCount, input);
        (void)handler.HandleMousePressEvent(event);
        if (_penButton == web::MouseButton::Right)
        {
          (void)handler.SendContextMenuEvent(event);
        }
        break;
      }
      case pal::PointerAction::Released:
      {
        auto button = std::exchange(_penButton, web::MouseButton::None);
        if (button == web::MouseButton::None)
        {
          break;
        }
        _penButtons = 0;
        (void)handler.HandleMouseReleaseEvent(
          PenEvent(web::PlatformEvent::Type::MouseReleased, point, button, _clickCount, input));
        break;
      }
      case pal::PointerAction::Left:
      {
        // A move outside the viewport hits nothing, so the element under the pen is left.
        (void)handler.MouseMoved(
          PenEvent(web::PlatformEvent::Type::MouseMoved, PointD(-1, -1), web::MouseButton::None, 0, input));
        break;
      }
      case pal::PointerAction::Cancelled:
      {
        _penButton = web::MouseButton::None;
        _penButtons = 0;
        handler.CancelMousePress();
        _frame->GetPointerCaptureController().CancelPointer(web::PenPointerId,
                                                            _frame->View()->WindowToContents(intPoint));
        break;
      }
    }
    PerformMicrotaskCheckpoint();
  }

  bool View::TouchInput(const PointD &viewPoint, const pal::PointerInput &viewInput) noexcept
  {
    if (_overlay)
    {
      return false;
    }
    auto point = ToPage(viewPoint);
    auto input = ToPage(viewInput);

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

    web::PlatformEvent::Type type = web::PlatformEvent::Type::TouchMove;
    web::PlatformTouchPoint::State state = web::PlatformTouchPoint::TouchMoved;
    switch (input.Action)
    {
      case pal::PointerAction::Pressed:
        type = web::PlatformEvent::Type::TouchStart;
        state = web::PlatformTouchPoint::TouchPressed;
        break;
      case pal::PointerAction::Released:
        type = web::PlatformEvent::Type::TouchEnd;
        state = web::PlatformTouchPoint::TouchReleased;
        break;
      case pal::PointerAction::Cancelled:
        type = web::PlatformEvent::Type::TouchCancel;
        state = web::PlatformTouchPoint::TouchCancelled;
        break;
      default: break;
    }

    // A cancellation from the host may come without a position: the touch stays where it was.
    bool hasPosition = input.Action != pal::PointerAction::Cancelled || point.X() || point.Y();
    auto &previous = it->Point;
    PointD position = hasPosition ? point : previous.Pos();
    SizeD radius = hasPosition ? SizeD(input.Width / 2, input.Height / 2) : previous.Radius();
    float force = hasPosition ? input.Pressure.value_or(0.f) : previous.Force();
    it->Point = web::PlatformTouchPoint(it->Id, state, ToScreen(position), position, radius, 0, force);

    List<web::PlatformTouchPoint> points;
    for (auto &touch : _touches)
    {
      if (&touch == &*it)
      {
        points.push_back(touch.Point);
      }
      else
      {
        points.emplace_back(touch.Id, web::PlatformTouchPoint::TouchStationary, touch.Point.ScreenPos(),
                            touch.Point.Pos(), touch.Point.Radius(), touch.Point.RotationAngle(),
                            touch.Point.Force());
      }
    }

    web::PlatformTouchEvent event(type, krys::move(points), input::ToWeb(input.Modifiers),
                                  web::html::EventLoop::Current().Now());
    // Moves aren't merged, so each stands for itself alone (its pointermove's coalesced events).
    if (type == web::PlatformEvent::Type::TouchMove)
    {
      event.SetCoalescedEvents({event});
    }

    uint32 id = it->Id;
    if (state == web::PlatformTouchPoint::TouchReleased || state == web::PlatformTouchPoint::TouchCancelled)
    {
      _touches.erase(it);
    }

    bool handled = _frame->GetEventHandler().HandleTouchEvent(event);
    PerformMicrotaskCheckpoint();
    RecogniseGesture(id, input.Action, position, handled);
    return handled;
  }

  void View::RecogniseGesture(uint32 id, pal::PointerAction action, const PointD &position,
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
          if (stoppedFling)
          {
            _frame->GetEventHandler().DidEndGestureScroll();
          }
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
          {web::TouchAction::Auto, web::TouchAction::Manipulation, web::TouchAction::PinchZoom});
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
          auto centre = PointD((first.X() + second.X()) / 2, (first.Y() + second.Y()) / 2);
          _pinchStartScale = _frame->PageScaleFactor();
          _pinchStartDistance = std::hypot(second.X() - first.X(), second.Y() - first.Y());
          auto scroll = ScrollPosition();
          _pinchDocumentPoint = PointD((scroll.X() + centre.X()) / _pinchStartScale,
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
            _gestureTouchActions.ContainsAny({web::TouchAction::Auto, web::TouchAction::Manipulation});
          _panX = any || _gestureTouchActions.contains(web::TouchAction::PanX);
          _panY = any || _gestureTouchActions.contains(web::TouchAction::PanY);
          bool horizontal = std::abs(delta.Width()) >= std::abs(delta.Height());
          _longPressTimer.Stop();
          if (horizontal ? !_panX : !_panY)
          {
            ResetGesture();
            break;
          }
          CancelTouchPointer(id, position);
          _gestureState = GestureState::Pan;
          _panSamples = {{web::html::EventLoop::Current().Now(), _gestureStart}};
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
          if (!_fling)
          {
            _frame->GetEventHandler().DidEndGestureScroll();
          }
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

  EnumSet<web::TouchAction> View::TouchActionsAt(const PointD &point) noexcept
  {
    auto *element = HitTestInPage(FlooredPointI(point));
    auto *renderer = element ? element->Renderer() : nullptr;
    if (!renderer)
    {
      return web::TouchAction::Auto;
    }
    return web::style::ToPlatform<web::style::TouchAction> {}(renderer->Style().UsedTouchAction());
  }

  void View::ResetGesture() noexcept
  {
    _gestureState = GestureState::None;
    _longPressTimer.Stop();
    _panRemainder = {};
    _panSamples.clear();
  }

  void View::CancelTouchPointer(uint32 id, const PointD &position) noexcept
  {
    if (auto *view = _frame->View())
    {
      web::PlatformTouchPoint point(id, web::PlatformTouchPoint::TouchCancelled, position, position, {}, 0,
                                    0);
      _frame->GetPointerCaptureController().CancelPointer(web::PointerEvent::PointerIdForTouch(point),
                                                          view->WindowToContents(FlooredPointI(position)));
      PerformMicrotaskCheckpoint();
    }
  }

  void View::PanTo(const PointD &position) noexcept
  {
    auto movement = position - _gestureLast;
    _gestureLast = position;
    SizeD delta(_panX ? movement.Width() : 0, _panY ? movement.Height() : 0);

    // The samples within the fling's window of this one.
    auto now = web::html::EventLoop::Current().Now();
    _panSamples.emplace_back(now, position);
    std::erase_if(_panSamples, [&](const auto &sample) { return now - sample.first > FlingSampleWindow; });

    // Scrolls are in whole pixels; the rest waits for the next move.
    auto remainder = SizeD(_panRemainder.Width() + delta.Width(), _panRemainder.Height() + delta.Height());
    SizeD step(std::round(remainder.Width()), std::round(remainder.Height()));
    _panRemainder = SizeD(remainder.Width() - step.Width(), remainder.Height() - step.Height());
    if (step.Width() || step.Height())
    {
      (void)ScrollForTouch(_gestureStart, step);
    }
  }

  bool View::ScrollForTouch(const PointD &position, const SizeD &delta) noexcept
  {
    auto point = FlooredPointI(position);
    bool scrolled = _frame->GetEventHandler().HandleTouchScroll(web::PlatformWheelEvent(
      point, ToScreen(point), static_cast<float>(delta.Width()), static_cast<float>(delta.Height()), 0, 0,
      web::PlatformWheelEventGranularity::ScrollByPixelWheelEvent, {},
      web::html::EventLoop::Current().Now()));
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
    SizeD velocity(_panX ? distance.Width() / seconds : 0, _panY ? distance.Height() / seconds : 0);
    if (std::hypot(velocity.Width(), velocity.Height()) < MinimumFlingVelocity)
    {
      return;
    }
    _fling = Fling {_gestureStart, velocity, {}, web::html::EventLoop::Current().Now()};
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
    auto remainder = SizeD(_fling->Remainder.Width() + velocity.Width() * FlingTimeConstant * (1 - decay),
                           _fling->Remainder.Height() + velocity.Height() * FlingTimeConstant * (1 - decay));
    velocity = velocity.ScaledBy(decay);

    SizeD step(std::round(remainder.Width()), std::round(remainder.Height()));
    _fling->Remainder = SizeD(remainder.Width() - step.Width(), remainder.Height() - step.Height());
    bool scrolled = (!step.Width() && !step.Height()) || ScrollForTouch(_fling->Position, step);
    // It stops once it's slow, or at the end of what it scrolls.
    if (!scrolled || std::hypot(velocity.Width(), velocity.Height()) < StopFlingVelocity)
    {
      _fling = null;
      _frame->GetEventHandler().DidEndGestureScroll();
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
    auto centre = PointD((first.X() + second.X()) / 2, (first.Y() + second.Y()) / 2);
    double distance = std::hypot(second.X() - first.X(), second.Y() - first.Y());
    float scale = std::clamp(static_cast<float>(_pinchStartScale * distance / _pinchStartDistance),
                             MinimumPageScale, MaximumPageScale);

    // The document point that was between the touches stays between them.
    PointI origin(static_cast<int32>(std::lround(_pinchDocumentPoint.X() * scale - centre.X())),
                  static_cast<int32>(std::lround(_pinchDocumentPoint.Y() * scale - centre.Y())));
    _frame->SetPageScaleFactor(scale, origin);
    PerformMicrotaskCheckpoint();
    _needsPaint = true;
  }

  void View::Tap(uint32 id, const PointD &position) noexcept
  {
    // A tap is the mouse moving there and clicking, as one finger's tap, from the touch's pointer: its
    // compatibility mouse events don't fire if the page cancelled its pointerdown.
    auto point = FlooredPointI(position);
    int32 clickCount = CountClick(point, web::MouseButton::Left, DoubleTapDistance);
    web::PlatformTouchPoint touchPoint(id, web::PlatformTouchPoint::TouchReleased, ToScreen(position),
                                       position, {}, 0, 0);
    auto pointerId = web::PointerEvent::PointerIdForTouch(touchPoint);
    auto event = [&](web::PlatformEvent::Type type, web::MouseButton button, uint16 buttons, int32 count)
    {
      return web::PlatformMouseEvent(PointD(point), PointD(ToScreen(point)), button, type, count, {},
                                     web::html::EventLoop::Current().Now(), buttons, {}, 0,
                                     web::SyntheticClickType::OneFingerTap, pointerId,
                                     web::pointer_events::TouchPointerEventType());
    };

    auto &handler = _frame->GetEventHandler();
    (void)handler.MouseMoved(event(web::PlatformEvent::Type::MouseMoved, web::MouseButton::None, 0, 0));
    (void)handler.HandleMousePressEvent(
      event(web::PlatformEvent::Type::MousePressed, web::MouseButton::Left, 1, clickCount));
    (void)handler.HandleMouseReleaseEvent(
      event(web::PlatformEvent::Type::MouseReleased, web::MouseButton::Left, 0, clickCount));
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

  void View::RequestContextMenu(uint32 id, const PointD &position) noexcept
  {
    // As the right button's press asks for a context menu: there's none, but the page gets `contextmenu`.
    auto point = FlooredPointI(position);
    web::PlatformTouchPoint touchPoint(id, web::PlatformTouchPoint::TouchReleased, ToScreen(position),
                                       position, {}, 0, 0);
    web::PlatformMouseEvent event(
      PointD(point), PointD(ToScreen(point)), web::MouseButton::Right, web::PlatformEvent::Type::MousePressed,
      1, {}, web::html::EventLoop::Current().Now(), 0, {}, 0, web::SyntheticClickType::NoTap,
      web::PointerEvent::PointerIdForTouch(touchPoint), web::pointer_events::TouchPointerEventType());
    (void)_frame->GetEventHandler().SendContextMenuEvent(event);
    PerformMicrotaskCheckpoint();
  }

  void View::SetPointingDevices(const pal::PointingDevices &devices) noexcept
  {
    if (devices.Mouse == _pointingDevices.Mouse && devices.Pen == _pointingDevices.Pen
        && devices.Touch == _pointingDevices.Touch
        && devices.MaxTouchPoints == _pointingDevices.MaxTouchPoints)
    {
      return;
    }

    // The Krystal.Web client answers the pointing device media features from these.
    _pointingDevices = devices;
    _frame->UpdateStyleAfterChangeInEnvironment();
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
        case web::DragOperation::Copy: return pal::CursorShape::DragCopy;
        case web::DragOperation::Link: return pal::CursorShape::DragLink;
        default:                       return pal::CursorShape::DragMove;
      }
    }
    return input::ToCursorShape(_frame->GetEventHandler().CurrentMouseCursor());
  }

  RefPtr<pal::CursorImage> View::CurrentCursorImage() const noexcept
  {
    if (_pointerLocked || _drag)
    {
      return nullptr;
    }
    auto cursor = _frame->GetEventHandler().CurrentMouseCursor();
    auto *image = cursor.GetImage();
    auto frame = image ? image->CurrentNativeImage() : nullptr;
    auto *platformImage = frame ? frame->GetPlatformImage().get() : nullptr;
    if (!platformImage || platformImage->Pixels().empty())
    {
      return nullptr;
    }

    // The hot spot is in the image's natural size; the cursor's pixels are the image's.
    auto scale = cursor.ImageScaleFactor();
    PointI hotSpot(static_cast<int32>(cursor.HotSpot().X() * scale),
                   static_cast<int32>(cursor.HotSpot().Y() * scale));
    if (_cursorImage && _cursorImageFrame == frame && _cursorImageHotSpot == hotSpot)
    {
      return _cursorImage;
    }

    // The window takes straight alpha.
    List<uint32> pixels(platformImage->Pixels().begin(), platformImage->Pixels().end());
    if (platformImage->PremultipliedAlpha())
    {
      for (auto &pixel : pixels)
      {
        uint32 alpha = pixel >> 24;
        if (!alpha || alpha == 255)
        {
          continue;
        }
        auto unpremultiply = [&](uint32 shift)
        {
          uint32 channel = (pixel >> shift) & 0xFF;
          return std::min<uint32>(255, (channel * 255 + alpha / 2) / alpha) << shift;
        };
        pixel = (alpha << 24) | unpremultiply(16) | unpremultiply(8) | unpremultiply(0);
      }
    }
    auto size = platformImage->Size();
    _cursorImage =
      CreateRef<pal::CursorImage>(static_cast<uint32>(size.Width()), static_cast<uint32>(size.Height()),
                                  krys::move(pixels), hotSpot.X(), hotSpot.Y());
    _cursorImageFrame = frame;
    _cursorImageHotSpot = hotSpot;
    return _cursorImage;
  }

  void View::SetPointerLockHandler(Func<bool(bool)> &&handler) noexcept
  {
    _pointerLockHandler = krys::move(handler);
  }

  web::PointerLockRequestResult View::LockPointer() noexcept
  {
    if (!_pointerLockHandler)
    {
      return web::PointerLockRequestResult::Unsupported;
    }
    // A window without focus can't take the pointer.
    if (!_focused || !_pointerLockHandler(true))
    {
      return web::PointerLockRequestResult::Failure;
    }
    _pointerLocked = true;
    return web::PointerLockRequestResult::Success;
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

    // While a modal overlay is open, it takes the keys.
    if (_overlay && _overlay->IsModal())
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

    // Ctrl with a Shift key on its own chooses the direction of the text being edited, when they're let go:
    // the right Shift right to left, the left one left to right. Any other key on the way cancels it.
    bool isShift = key == pal::Key::Shift || key == pal::Key::RightShift;
    if (isShift && modifiers.contains(pal::KeyModifier::Control) && !modifiers.contains(pal::KeyModifier::Alt)
        && !modifiers.contains(pal::KeyModifier::Meta))
    {
      _pendingTextDirection =
        key == pal::Key::RightShift ? web::WritingDirection::RightToLeft : web::WritingDirection::LeftToRight;
    }
    else if (!isShift && key != pal::Key::Control && key != pal::Key::RightControl)
    {
      _pendingTextDirection = null;
    }

    auto event = input::KeyboardEvent(web::PlatformEvent::Type::KeyDown, key, modifiers, text, isAutoRepeat);
    _heldKeyValues.insert_or_assign(key, event.Key());
    bool handled = _frame->GetEventHandler().KeyEvent(event);
    PerformMicrotaskCheckpoint();
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
      input::KeyboardEvent(web::PlatformEvent::Type::KeyUp, key, modifiers, {}, false, keyValue));

    if (_pendingTextDirection
        && (key == pal::Key::Shift || key == pal::Key::RightShift || key == pal::Key::Control
            || key == pal::Key::RightControl))
    {
      auto direction = *std::exchange(_pendingTextDirection, null);
      if (auto *document = _frame->Document(); document && document->GetEditor().CanEdit())
      {
        document->GetEditor().SetBaseWritingDirection(direction);
      }
    }
    PerformMicrotaskCheckpoint();
    return handled;
  }

  bool View::HasModalOverlay() const noexcept
  {
    return _overlay && _overlay->IsModal();
  }

  bool View::InsertText(const utf8_string &text) noexcept
  {
    if (HasModalOverlay() || text.empty())
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
    if (HasModalOverlay())
    {
      return;
    }
    (void)_frame->GetEventHandler().KeyEvent(
      input::InputMethodKeyboardEvent(text, input::CompositionUnderlines(text, clauses), caret, 0));
    PerformMicrotaskCheckpoint();
  }

  void View::ConfirmComposition(const utf8_string &text) noexcept
  {
    if (HasModalOverlay())
    {
      return;
    }
    (void)_frame->GetEventHandler().KeyEvent(input::InputMethodKeyboardEvent(text, null));
    PerformMicrotaskCheckpoint();
  }

  bool View::AcceptsTextInput() const noexcept
  {
    return !HasModalOverlay() && _editorClient->AcceptsTextInput();
  }

  Maybe<RectI> View::TextInputRect() noexcept
  {
    auto *view = _frame->View();
    if (!view)
    {
      return null;
    }

    auto &document = Document();
    (void)document.UpdateLayout();
    auto &editor = document.GetEditor();
    RectI rect;
    if (auto range = editor.CompositionRange())
    {
      rect = editor.FirstRectForRange(*range);
    }
    else
    {
      rect = document.Selection().AbsoluteCaretBounds();
    }
    if (rect == RectI())
    {
      return null;
    }
    return ToViewPixels(view->ContentsToWindow(rect));
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
    using FocusDirection = web::dom::FocusDirection;
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
      if (RefPtr<web::Element> element = ShareRefPtr(Document().FocusedElement()))
      {
        (void)element->DispatchSimulatedClick(
          nullptr, web::html::SimulatedClickMouseEventOptions::SendMouseUpDownEvents);
      }
      PerformMicrotaskCheckpoint();
    }

    // B is a close request, as Escape is.
    if (pressed(GamepadButton::East) && !GamepadOverlayKey(pal::Key::Escape))
    {
      GamepadInteraction(false);
      if (RefPtr<web::Window> window = ShareRefPtr(Document().Window()))
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
      (void)web::html::FocusAlgorithms::SequentialNavigation(*document, FocusDirection::Forward);
      PerformMicrotaskCheckpoint();
    }
    if (pressed(GamepadButton::LeftShoulder) && !GamepadOverlayKey(pal::Key::Tab, {pal::KeyModifier::Shift}))
    {
      GamepadInteraction(true);
      Ref document = ShareRef(Document());
      (void)web::html::FocusAlgorithms::SequentialNavigation(*document, FocusDirection::Backward);
      PerformMicrotaskCheckpoint();
    }

    // The right stick scrolls for the time since the last frame it was pushed in.
    if ((scrollX || scrollY) && !_overlay)
    {
      if (_gamepadLastScroll)
      {
        auto elapsed = std::min<MonotonicTime::duration>(now - *_gamepadLastScroll, GamepadMaximumScrollStep);
        double distance = GamepadScrollSpeed * std::chrono::duration<double>(elapsed).count();
        GamepadScroll(SizeD(scrollX * distance, scrollY * distance));
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

  void View::GamepadMove(web::dom::FocusDirection direction) noexcept
  {
    using FocusDirection = web::dom::FocusDirection;
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
    (void)web::html::FocusAlgorithms::DirectionalNavigation(*document, direction);
    PerformMicrotaskCheckpoint();
    _needsPaint = true;
  }

  void View::GamepadScroll(const SizeD &delta) noexcept
  {
    // Scrolls are in whole pixels; the rest waits for the next frame.
    SizeD remainder(_gamepadScrollRemainder.Width() + delta.Width(),
                    _gamepadScrollRemainder.Height() + delta.Height());
    SizeD step(std::trunc(remainder.Width()), std::trunc(remainder.Height()));
    _gamepadScrollRemainder = SizeD(remainder.Width() - step.Width(), remainder.Height() - step.Height());

    Ref document = ShareRef(Document());
    (void)document->UpdateLayout();
    RefPtr<web::FrameView> view = ShareRefPtr(_frame->View());

    // Each axis scrolls the focused element's box (or else the nearest box around it that can), or else the
    // viewport, as the arrow keys do.
    auto scrollAxis = [&](double amount, web::ScrollDirection backward, web::ScrollDirection forward)
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
          && renderer->EnclosingBox().Scroll(scrollDirection, web::ScrollGranularity::Pixel, stepCount))
      {
        return;
      }
      if (view)
      {
        (void)view->Scroll(scrollDirection, web::ScrollGranularity::Pixel, stepCount);
      }
    };
    scrollAxis(step.Width(), web::ScrollDirection::ScrollLeft, web::ScrollDirection::ScrollRight);
    scrollAxis(step.Height(), web::ScrollDirection::ScrollUp, web::ScrollDirection::ScrollDown);
    PerformMicrotaskCheckpoint();
    _needsPaint = true;
  }

  void View::ResetGamepadState() noexcept
  {
    _gamepadDirection = web::dom::FocusDirection::None;
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
    web::html::FocusAlgorithms::SystemFocusChanged(Document(), focused);

    // The pointer lock ends with the window's focus.
    if (!focused && _pointerLocked)
    {
      (void)UnlockPointer();
      _frame->GetPointerLockController().DidLosePointerLock();
    }
    PerformMicrotaskCheckpoint();
  }

  void View::SetVisible(bool visible) noexcept
  {
    _frame->SetIsVisible(visible);
  }

  bool View::IsVisible() const noexcept
  {
    return _frame->IsVisible();
  }

  void View::SetDisplayColour(const pal::DisplayColour &colour) noexcept
  {
    if (_displayColour == colour)
    {
      return;
    }
    _displayColour = colour;
    _frame->ScreenPropertiesDidChange();
  }

  void View::SetFramesPerSecond(uint32 framesPerSecond) noexcept
  {
    _frame->SetDisplayNominalFramesPerSecond(framesPerSecond);
  }

  void View::SetImageAnimationEnabled(bool enabled) noexcept
  {
    _frame->SetImageAnimationEnabled(enabled);
  }

  void View::SetNavigationHandler(Func<void(const NavigationRequest &)> &&handler) noexcept
  {
    _frameLoaderClient->SetNavigationHandler(krys::move(handler));
  }

  void View::SetFocusRequestHandler(Func<void()> &&handler) noexcept
  {
    _webClient->SetFocusRequestHandler(krys::move(handler));
  }

  void View::SetFileChooserHandler(Func<void(Ref<web::FileChooser>)> &&handler) noexcept
  {
    _webClient->SetFileChooserHandler(krys::move(handler));
  }

  namespace
  {
    /// @brief The drag's flags for @p modifiers: Ctrl makes a drop copy.
    KRYS_NODISCARD EnumSet<web::DragApplicationFlags> DragFlags(pal::KeyModifiers modifiers) noexcept
    {
      if (modifiers.contains(pal::KeyModifier::Control))
      {
        return {web::DragApplicationFlags::IsCopyKeyDown};
      }
      return {};
    }

    /// @brief What's dragged in from another application, as the page's pasteboard reads it.
    KRYS_NODISCARD web::SelectionData SelectionDataForDrag(const pal::DragContent &content) noexcept
    {
      web::SelectionData data;
      if (content.Text)
      {
        data.SetText(*content.Text);
      }
      if (content.HTML)
      {
        data.SetMarkup(*content.HTML);
      }
      if (content.Url)
      {
        net::Url url(*content.Url);
        if (url.IsValid())
        {
          data.SetUrl(url, content.UrlTitle);
        }
      }
      // Files come as a URI list of their file URLs.
      utf8_string uriList;
      for (const auto &path : content.Files)
      {
        if (!uriList.empty())
        {
          uriList += u8"\r\n";
        }
        uriList += net::Url::FileUrlWithFileSystemPath(path).String();
      }
      if (!uriList.empty())
      {
        data.SetURIList(uriList);
      }
      return data;
    }

    /// @brief What a drop of @p data may do, of what its source allows (@p operations).
    KRYS_NODISCARD EnumSet<web::DragOperation> SourceOperationMask(pal::DropOperations operations,
                                                                   const web::SelectionData &data) noexcept
    {
      auto mask = input::ToWeb(operations);
      // A page only reads the files dropped on it, and a source told they moved removes them (Windows'
      // file manager deletes files a drop moved).
      if (data.HasFilenames())
      {
        mask.Remove(web::DragOperation::Move);
      }
      return mask;
    }
  }

  void View::DragStarted(web::DragItem item, web::DataTransfer &dataTransfer) noexcept
  {
    ActiveDrag drag;
    drag.Data = dataTransfer.GetPasteboard().GetSelectionData();
    drag.SourceOperationMask = dataTransfer.SourceOperationMask();
    drag.Position = _lastMousePosition.value_or(PointI());
    // The image starts where the page put it and follows the mouse from where it was pressed.
    drag.ImageOffset = item.DragLocationInContentCoordinates - item.EventPositionInContentCoordinates;
    drag.FadeImage =
      item.SourceAction == web::DragSourceAction::DHTML || item.SourceAction == web::DragSourceAction::Link;
    drag.Image = krys::move(item.Image);
    _drag = krys::move(drag);
    _needsPaint = true;
  }

  void View::DragMoved(const PointI &point, pal::KeyModifiers modifiers) noexcept
  {
    auto &drag = *_drag;
    if (drag.Image && drag.Position != point)
    {
      _needsPaint = true;
    }
    drag.Position = point;
    drag.Modifiers = modifiers;
    web::DragData data(&drag.Data, point, ToScreen(point), drag.SourceOperationMask, DragFlags(modifiers),
                       input::ToWeb(modifiers));

    auto size = ViewportSize();
    bool inView = point.X() >= 0 && point.Y() >= 0 && point.X() < size.Width() && point.Y() < size.Height();
    if (inView)
    {
      drag.Operation = _frame->GetDragController().DragEnteredOrUpdated(*_frame, krys::move(data));
      drag.InView = true;
    }
    else if (drag.InView)
    {
      _frame->GetDragController().DragExited(*_frame, krys::move(data));
      drag.InView = false;
      drag.Operation = null;
    }
  }

  void View::EndDrag(const PointI &point, bool drop) noexcept
  {
    // The drag's data outlives the drop, which reads it.
    auto drag = krys::move(*_drag);
    _drag = null;
    if (drag.Image)
    {
      _needsPaint = true;
    }

    auto &controller = _frame->GetDragController();
    EnumSet<web::DragOperation> operation;
    if (drag.InView)
    {
      web::DragData data(&drag.Data, point, ToScreen(point), drag.SourceOperationMask,
                         DragFlags(drag.Modifiers), input::ToWeb(drag.Modifiers));
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
    _frame->GetEventHandler().DragSourceEndedAt(MouseEvent(web::PlatformEvent::Type::MouseMoved, point,
                                                           web::MouseButton::Left, 0,
                                                           input::ToWeb(drag.Modifiers)),
                                                operation);
  }

  Maybe<pal::DropOperation> View::SystemDragInput(const PointI &viewPoint,
                                                  const pal::DragInput &input) noexcept
  {
    auto point = ToPage(viewPoint);
    auto &controller = _frame->GetDragController();
    if (input.Action == pal::DragAction::Entered)
    {
      _systemDrag = SystemDrag {};
      _systemDrag->Data = SelectionDataForDrag(input.Content);
    }
    if (!_systemDrag)
    {
      return null;
    }

    auto &drag = *_systemDrag;
    auto size = ViewportSize();
    bool inView = point.X() >= 0 && point.Y() >= 0 && point.X() < size.Width() && point.Y() < size.Height();
    auto operationMask = SourceOperationMask(input.AllowedOperations, drag.Data);

    switch (input.Action)
    {
      case pal::DragAction::Entered:
      case pal::DragAction::Moved:
      {
        drag.Position = point;
        web::DragData data(&drag.Data, point, point, operationMask, DragFlags(input.Modifiers),
                           input::ToWeb(input.Modifiers));
        if (inView)
        {
          drag.Operation = controller.DragEnteredOrUpdated(*_frame, krys::move(data));
          drag.InView = true;
        }
        else if (drag.InView)
        {
          controller.DragExited(*_frame, krys::move(data));
          drag.InView = false;
          drag.Operation = null;
        }
        return drag.Operation ? input::ToDropOperation(*drag.Operation) : null;
      }
      case pal::DragAction::Left:
      {
        if (drag.InView)
        {
          web::DragData data(&drag.Data, drag.Position, drag.Position, {});
          controller.DragExited(*_frame, krys::move(data));
        }
        _systemDrag = null;
        return null;
      }
      case pal::DragAction::Dropped:
      {
        // The drag's data outlives the drop, which reads it.
        auto dropped = krys::move(drag);
        _systemDrag = null;
        if (!dropped.InView)
        {
          return null;
        }

        web::DragData data(&dropped.Data, point, point, operationMask, DragFlags(input.Modifiers),
                           input::ToWeb(input.Modifiers));
        if (inView && dropped.Operation)
        {
          if (controller.PerformDragOperation(krys::move(data)))
          {
            return input::ToDropOperation(*dropped.Operation);
          }
          return null;
        }
        controller.DragExited(*_frame, krys::move(data));
        return null;
      }
    }
    return null;
  }

  web::PlatformMouseEvent View::MouseEvent(web::PlatformEvent::Type type, const PointI &point,
                                           web::MouseButton button, int32 clickCount,
                                           EnumSet<web::PlatformEvent::Modifier> modifiers,
                                           const PointD &movement,
                                           const PointD &unadjustedMovement) const noexcept
  {
    web::PlatformMouseEvent event(PointD(point), PointD(ToScreen(point)), button, type, clickCount, modifiers,
                                  web::html::EventLoop::Current().Now(), _buttons, movement);
    event.SetUnadjustedMovementDelta(unadjustedMovement);
    // Moves aren't merged, so each stands for itself alone (its pointermove's coalesced events).
    if (type == web::PlatformEvent::Type::MouseMoved)
    {
      event.SetCoalescedEvents({event});
    }
    return event;
  }

  web::PlatformMouseEvent View::PenEvent(web::PlatformEvent::Type type, const PointD &point,
                                         web::MouseButton button, int32 clickCount,
                                         const pal::PointerInput &input) const noexcept
  {
    // A pen without pressure presses at half pressure.
    double force = input.Pressure ? *input.Pressure : (input.InContact ? 0.5 : 0);
    web::PlatformMouseEvent event(point, ToScreen(point), button, type, clickCount,
                                  input::ToWeb(input.Modifiers), web::html::EventLoop::Current().Now(),
                                  _penButtons, {}, force, web::SyntheticClickType::NoTap, web::PenPointerId,
                                  web::pointer_events::PenPointerEventType());
    event.SetPenOrientation({static_cast<int32>(std::round(input.TiltX)),
                             static_cast<int32>(std::round(input.TiltY)),
                             static_cast<int32>(std::round(input.Twist))});
    // Moves aren't merged, so each stands for itself alone (its pointermove's coalesced events).
    if (type == web::PlatformEvent::Type::MouseMoved)
    {
      event.SetCoalescedEvents({event});
    }
    return event;
  }

  uint16 View::ButtonsBit(web::MouseButton button) noexcept
  {
    // The DOM's buttons: https://w3c.github.io/pointerevents/#dom-mouseevent-buttons
    switch (button)
    {
      case web::MouseButton::Left:    return 1;
      case web::MouseButton::Right:   return 2;
      case web::MouseButton::Middle:  return 4;
      case web::MouseButton::Back:    return 8;
      case web::MouseButton::Forward: return 16;
      default:                        return 0;
    }
  }
}
