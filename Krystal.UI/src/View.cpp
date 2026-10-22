#include "Krystal.UI/View.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.UI/Compositing/GraphicsLayerTextureMapper.hpp"
#include "Krystal.UI/EditorClient.hpp"
#include "Krystal.UI/FileLoaderStrategy.hpp"
#include "Krystal.UI/GraphicsContextRender2D.hpp"
#include "Krystal.UI/Input.hpp"
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
    GraphicsLayerFactoryTextureMapper _factory;
    GraphicsLayerTextureMapper *_rootLayer {nullptr};
    Func<void(const FormSubmissionRequest &)> _formSubmissionHandler;

  public:
    void AttachRootGraphicsLayer(boo::LocalFrame &, boo::GraphicsLayer *layer) noexcept override
    {
      _rootLayer = layer ? &Downcast<GraphicsLayerTextureMapper>(*layer) : nullptr;
    }

    KRYS_NODISCARD boo::GraphicsLayerFactory *GetGraphicsLayerFactory() const noexcept override
    {
      return const_cast<GraphicsLayerFactoryTextureMapper *>(&_factory);
    }

    void SubmitForm(boo::LocalFrame &, boo::FormSubmission &submission) noexcept override
    {
      if (!_formSubmissionHandler)
      {
        return;
      }

      FormSubmissionRequest request;
      switch (submission.GetMethod())
      {
        case boo::FormSubmission::Method::Get:    request.Method = u8"GET"; break;
        case boo::FormSubmission::Method::Post:   request.Method = u8"POST"; break;
        case boo::FormSubmission::Method::Dialog: request.Method = u8"DIALOG"; break;
      }
      request.URL = submission.RequestURL().String();
      request.Target = submission.Target();
      if (submission.GetMethod() == boo::FormSubmission::Method::Post)
      {
        request.ContentType = submission.ContentType();
        if (!submission.Boundary().empty())
        {
          request.ContentType += u8"; boundary=" + submission.Boundary();
        }
        if (auto *data = submission.Data())
        {
          request.Body = data->FlattenToString();
        }
      }
      _formSubmissionHandler(request);
    }

    void SetFormSubmissionHandler(Func<void(const FormSubmissionRequest &)> &&handler) noexcept
    {
      _formSubmissionHandler = krys::move(handler);
    }

    KRYS_NODISCARD GraphicsLayerTextureMapper *RootLayer() const noexcept
    {
      return _rootLayer;
    }
  };

  namespace
  {
    /// @brief The page's chrome client, noted in @p chromeClient as it's handed to the page.
    KRYS_NODISCARD UniquePtr<boo::ChromeClient> CreateChromeClient(View::ChromeClient *&chromeClient) noexcept
    {
      auto client = CreateUnique<View::ChromeClient>();
      chromeClient = client.get();
      return client;
    }
  }

  View::View(const boo::IntSize &viewportSize) noexcept
      : _page(boo::Page::Create(CreateChromeClient(_chromeClient), CreateUnique<EditorClient>())),
        _frame(ShareRef(_page->MainFrame()))
  {
    static std::once_flag valuePool;
    std::call_once(valuePool, [] { boo::css::StaticValuePool::Init(); });

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

    _frame->CreateView(viewportSize);
    LoadHTML(u8"<!DOCTYPE html>");
  }

  View::~View() noexcept
  {
    // The layers go before the texture mapper that holds their textures.
    _frame->SetDocument(nullptr);
  }

  void View::LoadHTML(utf8_string &&markup, boo::url::URL &&url, boo::CachePolicy cachePolicy) noexcept
  {
    _frame->SetSubresourceCachePolicy(cachePolicy);
    // The document is parsed in the frame, as a navigation's is, so it's fully active as its elements are
    // inserted (autofocus candidates need that).
    _document = CreateRef<boo::dom::HTMLDocument>();
    if (!url.IsNull())
    {
      _document->SetURL(krys::move(url));
    }
    _document->SetFocusedAndActive(_focused);
    _frame->SetDocument(ShareRefPtr<boo::dom::Document>(_document.get()));
    _needsPaint = true;

    auto parser = CreateRef<boo::html::HTMLDocumentParser>(*_document);
    parser->InputStream().Append(krys::move(markup), boo::IsEOF(true));
    parser->PumpTokenizer();
    _document->StyleScope().FlushPendingUpdate();
  }

  Expected<void> View::LoadFile(const std::filesystem::path &path, boo::CachePolicy cachePolicy) noexcept
  {
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
      return Unexpected(std::format("Couldn't open {}", path.string()));
    }
    std::string bytes {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    // Decoding UTF-8 removes its byte order mark, which would otherwise be text in the document.
    if (bytes.starts_with("\xEF\xBB\xBF"))
    {
      bytes.erase(0, 3);
    }
    auto absolutePath = std::filesystem::absolute(path).generic_u8string();
    LoadHTML(utf8_string(bytes.begin(), bytes.end()), boo::url::URL::FileURLWithFileSystemPath(absolutePath),
             cachePolicy);
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
    boo::html::EventLoop::Current().Run(now, budget);

    // Each frame is a rendering opportunity.
    if (!_page->IsRenderingUpdateScheduled())
    {
      return false;
    }
    _page->UpdateRendering();
    _needsPaint = true;
    return true;
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
    (void)_document->UpdateLayout();
    if (auto *view = _frame->View())
    {
      view->SetScrollPosition(position);
    }
    _needsPaint = true;
  }

  Ref<const boo::display_list::DisplayList> View::Record() noexcept
  {
    // The rendering update runs in Update; painting only needs the layout.
    (void)_document->UpdateLayout();
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
    return recorder.TakeDisplayList();
  }

  bool View::NeedsPaint() const noexcept
  {
    auto *view = _frame->View();
    return _needsPaint || (view && view->HasDamage());
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
    (void)_document->UpdateLayout({boo::dom::LayoutOptions::UpdateCompositingLayers});
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
    return _document->ElementFromPoint(point.X(), point.Y()).get();
  }

  void View::MouseMoved(const boo::IntPoint &point, pal::KeyModifiers modifiers) noexcept
  {
    // Windows reports moves when the mouse hasn't moved (raw input reports presses and releases as moves
    // too), and one while a button is held would start a selection.
    if (std::exchange(_lastMousePosition, point) == point)
    {
      return;
    }

    // A move carries the button held, as Windows' do: the left, then the middle, then the right.
    auto button = boo::MouseButton::None;
    if (_buttons & ButtonsBit(boo::MouseButton::Left))
    {
      button = boo::MouseButton::Left;
    }
    else if (_buttons & ButtonsBit(boo::MouseButton::Middle))
    {
      button = boo::MouseButton::Middle;
    }
    else if (_buttons & ButtonsBit(boo::MouseButton::Right))
    {
      button = boo::MouseButton::Right;
    }
    (void)_frame->GetEventHandler().MouseMoved(
      MouseEvent(boo::PlatformEvent::Type::MouseMoved, point, button, 0, input::ToBooey(modifiers)));
    PerformMicrotaskCheckpoint();
  }

  void View::MousePressed(const boo::IntPoint &point, pal::MouseButton palButton,
                          pal::KeyModifiers modifiers) noexcept
  {
    auto button = input::ToBooey(palButton);
    if (button == boo::MouseButton::None)
    {
      return;
    }

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
    (void)_frame->GetEventHandler().HandleMouseReleaseEvent(MouseEvent(
      boo::PlatformEvent::Type::MouseReleased, point, button, _clickCount, input::ToBooey(modifiers)));
    PerformMicrotaskCheckpoint();
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
    bool handled = _frame->GetEventHandler().HandleWheelEvent(
      input::WheelEvent(point, notchesX, notchesY, modifiers, settings));
    PerformMicrotaskCheckpoint();
    return handled;
  }

  pal::CursorShape View::CurrentCursor() const noexcept
  {
    return input::ToCursorShape(_frame->GetEventHandler().CurrentMouseCursor());
  }

  bool View::KeyPressed(pal::Key key, pal::KeyModifiers modifiers, const pal::KeyText &text,
                        bool isAutoRepeat) noexcept
  {
    auto event = input::KeyboardEvent(boo::PlatformEvent::Type::KeyDown, key, modifiers, text, isAutoRepeat);
    _heldKeyValues.insert_or_assign(key, event.Key());
    bool handled = _frame->GetEventHandler().KeyEvent(event);
    PerformMicrotaskCheckpoint();
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
    _focused = focused;
    boo::html::FocusAlgorithms::SystemFocusChanged(*_document, focused);
    PerformMicrotaskCheckpoint();
  }

  void View::SetFormSubmissionHandler(Func<void(const FormSubmissionRequest &)> &&handler) noexcept
  {
    _chromeClient->SetFormSubmissionHandler(krys::move(handler));
  }

  boo::PlatformMouseEvent View::MouseEvent(boo::PlatformEvent::Type type, const boo::IntPoint &point,
                                           boo::MouseButton button, int32 clickCount,
                                           EnumSet<boo::PlatformEvent::Modifier> modifiers) const noexcept
  {
    // FIXME: The screen position, once the host reports where the page is on the screen.
    return boo::PlatformMouseEvent(boo::DoublePoint(point), boo::DoublePoint(point), button, type, clickCount,
                                   modifiers, MonotonicTime::Now(), _buttons);
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
