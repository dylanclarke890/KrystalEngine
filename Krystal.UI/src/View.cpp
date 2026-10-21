#include "Krystal.UI/View.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.UI/FileLoaderStrategy.hpp"
#include "Krystal.UI/GraphicsContextRender2D.hpp"
#include "Krystal.UI/Input.hpp"
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <limits>
#include <mutex>

namespace krys::ui
{
  View::View(const boo::IntSize &viewportSize) noexcept
      : _page(boo::Page::Create()), _frame(ShareRef(_page->MainFrame()))
  {
    static std::once_flag valuePool;
    std::call_once(valuePool, [] { boo::css::StaticValuePool::Init(); });

    auto &strategies = boo::PlatformStrategies::Singleton();
    if (!strategies.GetLoaderStrategy())
    {
      static FileLoaderStrategy fileLoader;
      strategies.SetLoaderStrategy(&fileLoader);
    }

    _frame->CreateView(viewportSize);
    LoadHTML(u8"<!DOCTYPE html>");
  }

  View::~View() noexcept
  {
    if (_device && _pageTexture.IsValid())
    {
      _device->Destroy(_pageTexture);
    }
    if (_device && _damageTexture.IsValid())
    {
      _device->Destroy(_damageTexture);
    }
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
      view->Paint(recorder, boo::IntRect(0, 0, size.Width(), size.Height()));
    }
    return recorder.TakeDisplayList();
  }

  bool View::NeedsPaint() const noexcept
  {
    auto *view = _frame->View();
    return !_pageTexture.IsValid() || _pageTextureSize != ViewportSize() || (view && view->HasDamage());
  }

  void View::Paint(gfx::Renderer2D &renderer) noexcept
  {
    const auto size = ViewportSize();
    auto *view = _frame->View();
    auto *commands = renderer.GetCommandBuffer();
    if (!view || size.IsEmpty() || !commands)
    {
      return;
    }

    auto &device = renderer.GetDevice();
    _device = &device;
    if (!_pageRenderer)
    {
      auto pageRenderer = gfx::Renderer2D::Create(device);
      if (!pageRenderer)
      {
        return;
      }
      _pageRenderer = krys::move(*pageRenderer);
    }

    // A new viewport size needs new textures, all of the page painted.
    if (!_pageTexture.IsValid() || _pageTextureSize != size)
    {
      for (auto *texture : {&_pageTexture, &_damageTexture})
      {
        if (texture->IsValid())
        {
          device.Destroy(*texture);
        }
        *texture = {};
      }
      auto createTexture = [&](utf8_stringview name)
      {
        return device.CreateTexture({.Format = gfx::TextureFormat::RGBA8Unorm,
                                     .Width = static_cast<uint32>(size.Width()),
                                     .Height = static_cast<uint32>(size.Height()),
                                     .Usage = gfx::TextureUsage::RenderTarget | gfx::TextureUsage::Sampled,
                                     .DebugName = name});
      };
      auto pageTexture = createTexture(u8"Page");
      auto damageTexture = createTexture(u8"Page damage");
      if (!pageTexture || !damageTexture)
      {
        for (auto &texture : {pageTexture, damageTexture})
        {
          if (texture)
          {
            device.Destroy(*texture);
          }
        }
        return;
      }
      _pageTexture = *pageTexture;
      _damageTexture = *damageTexture;
      _pageTextureSize = size;
      view->RepaintEntireView();
    }

    // The rendering update runs in Update; painting only needs the layout, which can damage the page too.
    (void)_document->UpdateLayout();
    if (view->HasDamage())
    {
      const boo::IntRect viewRect(0, 0, size.Width(), size.Height());
      auto damage = DamageRects(view->TakeDamage(), viewRect);
      if (!damage.empty())
      {
        PaintDamage(*commands, device, damage);
      }
    }

    renderer.DrawImage(_pageTexture, gfx::Rect2D {0.f, 0.f, static_cast<float>(size.Width()),
                                                  static_cast<float>(size.Height())});
  }

  List<boo::IntRect> View::DamageRects(const boo::Region &damage, const boo::IntRect &viewRect) noexcept
  {
    List<boo::IntRect> rects;
    for (const auto &rect : damage.Rects())
    {
      if (auto visible = boo::Intersection(rect, viewRect); !visible.IsEmpty())
      {
        rects.push_back(visible);
      }
    }

    auto area = [](const boo::IntRect &rect)
    {
      return static_cast<int64>(rect.Width()) * rect.Height();
    };
    // Rects are merged while some two are close enough that painting their bounds costs little more than
    // painting both (a region splits overlapping rects into bands, which merge back), or while there are
    // many: each one painted is a recording and a pass of its own.
    constexpr size_t maximumRects = 8;
    bool merged = true;
    while (merged && rects.size() > 1)
    {
      merged = false;
      size_t bestFirst = 0;
      size_t bestSecond = 0;
      int64 bestWaste = std::numeric_limits<int64>::max();
      for (size_t first = 0; first < rects.size(); ++first)
      {
        for (size_t second = first + 1; second < rects.size(); ++second)
        {
          auto bounds = boo::UnionRect(rects[first], rects[second]);
          auto waste = area(bounds) - area(rects[first]) - area(rects[second]);
          if (waste < bestWaste)
          {
            bestWaste = waste;
            bestFirst = first;
            bestSecond = second;
          }
        }
      }
      auto bestBounds = boo::UnionRect(rects[bestFirst], rects[bestSecond]);
      if (rects.size() > maximumRects || bestWaste <= area(rects[bestFirst]) + area(rects[bestSecond]))
      {
        rects[bestFirst] = bestBounds;
        rects.erase(rects.begin() + static_cast<ptrdiff_t>(bestSecond));
        merged = true;
      }
    }
    return rects;
  }

  void View::PaintDamage(gfx::CommandBuffer &commands, gfx::Device &device,
                         const List<boo::IntRect> &damage) noexcept
  {
    const auto size = ViewportSize();
    const boo::IntRect viewRect(0, 0, size.Width(), size.Height());
    auto *view = _frame->View();

    // Each rect is recorded on its own: a recording only holds what paints into its dirty rect.
    List<Ref<const boo::display_list::DisplayList>> displayLists;
    for (const auto &rect : damage)
    {
      boo::display_list::RecorderImpl recorder(
        boo::FloatSize(static_cast<float>(size.Width()), static_cast<float>(size.Height())));
      view->Paint(recorder, rect);
      displayLists.push_back(recorder.TakeDisplayList());
    }

    if (!_imageBuffers)
    {
      _imageBuffers = CreateUnique<ImageBufferTextures>(device, _fonts);
    }

    // The whole page is painted straight into the page texture. Parts of it are painted into the damage
    // texture, cleared, and copied over those parts of the page texture: what was there before is replaced,
    // as drawing over it would blend with it.
    const bool wholePage = damage.size() == 1u && damage.front() == viewRect;
    const auto target = wholePage ? _pageTexture : _damageTexture;
    _pageRenderer->Begin(commands, {.Texture = target,
                                    .Width = static_cast<uint32>(size.Width()),
                                    .Height = static_cast<uint32>(size.Height()),
                                    .Format = gfx::TextureFormat::RGBA8Unorm,
                                    .ClearColour = Array<float, 4> {0.f, 0.f, 0.f, 0.f}});
    for (size_t index = 0; index < damage.size(); ++index)
    {
      const auto &rect = damage[index];
      _pageRenderer->Save();
      _pageRenderer->ClipRect(gfx::Rect2D {static_cast<float>(rect.X()), static_cast<float>(rect.Y()),
                                           static_cast<float>(rect.Width()),
                                           static_cast<float>(rect.Height())});
      {
        GraphicsContextRender2D context(*_pageRenderer, _fonts, boo::FloatRect(rect), _imageBuffers.get());
        context.DrawDisplayList(*displayLists[index]);
      }
      _pageRenderer->Restore();
    }
    _pageRenderer->End();
    _imageBuffers->Release();

    if (!wholePage)
    {
      for (const auto &rect : damage)
      {
        const auto region = gfx::TextureRegion {.X = static_cast<uint32>(rect.X()),
                                                .Y = static_cast<uint32>(rect.Y()),
                                                .Width = static_cast<uint32>(rect.Width()),
                                                .Height = static_cast<uint32>(rect.Height())};
        commands.BlitTexture(_damageTexture, region, _pageTexture, region, gfx::SamplerFilter::Nearest);
      }
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
    return _document->ElementFromPoint(point.X(), point.Y()).get();
  }

  void View::MouseMoved(const boo::IntPoint &point, pal::KeyModifiers modifiers) noexcept
  {
    (void)_frame->GetEventHandler().MouseMoved(MouseEvent(
      boo::PlatformEvent::Type::MouseMoved, point, boo::MouseButton::None, 0, input::ToBooey(modifiers)));
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
