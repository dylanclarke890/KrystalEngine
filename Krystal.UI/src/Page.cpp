#include "Krystal.UI/Page.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.UI/GraphicsContextRender2D.hpp"
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <mutex>

namespace krys::ui
{
  namespace
  {
    KRYS_NODISCARD Ref<boo::dom::HTMLDocument> Parse(utf8_string &&markup) noexcept
    {
      boo::html::EventLoop::Current().RunUntilIdle();
      auto document = CreateRef<boo::dom::HTMLDocument>();
      auto parser = CreateRef<boo::html::HTMLDocumentParser>(*document);
      parser->InputStream().Append(krys::move(markup), boo::IsEOF(true));
      parser->PumpTokenizer();
      document->StyleScope().FlushPendingUpdate();
      return document;
    }
  }

  Page::Page(const boo::IntSize &viewportSize) noexcept : _frame(boo::LocalFrame::Create())
  {
    static std::once_flag valuePool;
    std::call_once(valuePool, [] { boo::css::StaticValuePool::Init(); });

    _frame->CreateView(viewportSize);
    LoadHTML(u8"<!DOCTYPE html>");
  }

  Page::~Page() noexcept = default;

  void Page::LoadHTML(utf8_string &&markup) noexcept
  {
    _document = Parse(krys::move(markup));
    _document->SetFocusedAndActive(_focused);
    _frame->SetDocument(ShareRefPtr<boo::dom::Document>(_document.get()));
  }

  Expected<void> Page::LoadFile(const std::filesystem::path &path) noexcept
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
    LoadHTML(utf8_string(bytes.begin(), bytes.end()));
    return {};
  }

  uint32 Page::AddFontDirectory(const std::filesystem::path &directory)
  {
    auto count = boo::FontDatabase::Singleton().AddFontDirectory(directory);
    if (count)
    {
      boo::FontCache::InvalidateAllFontCaches();
    }
    return count;
  }

  void Page::Resize(const boo::IntSize &viewportSize) noexcept
  {
    if (auto *view = _frame->View())
    {
      view->Resize(viewportSize);
    }
  }

  boo::IntSize Page::ViewportSize() const noexcept
  {
    auto *view = _frame->View();
    return view ? boo::IntSize(view->Width(), view->Height()) : boo::IntSize();
  }

  void Page::UpdateRendering() noexcept
  {
    (void)_document->UpdateLayout();
  }

  Ref<const boo::display_list::DisplayList> Page::Record() noexcept
  {
    UpdateRendering();
    const auto size = ViewportSize();
    boo::display_list::RecorderImpl recorder(
      boo::FloatSize(static_cast<float>(size.Width()), static_cast<float>(size.Height())));
    if (auto *view = _frame->View())
    {
      view->PaintContents(recorder, boo::IntRect(0, 0, size.Width(), size.Height()));
    }
    return recorder.TakeDisplayList();
  }

  void Page::Paint(gfx::Renderer2D &renderer) noexcept
  {
    Replay(*Record(), renderer);
  }

  void Page::Replay(const boo::display_list::DisplayList &displayList, gfx::Renderer2D &renderer) noexcept
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

  boo::dom::Element *Page::HitTest(const boo::IntPoint &point) noexcept
  {
    return _document->ElementFromPoint(point.X(), point.Y()).get();
  }

  void Page::MouseMoved(const boo::IntPoint &point, EnumSet<boo::PlatformEvent::Modifier> modifiers) noexcept
  {
    (void)_frame->GetEventHandler().MouseMoved(
      MouseEvent(boo::PlatformEvent::Type::MouseMoved, point, boo::MouseButton::None, 0, modifiers));
  }

  void Page::MousePressed(const boo::IntPoint &point, boo::MouseButton button,
                          EnumSet<boo::PlatformEvent::Modifier> modifiers) noexcept
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

    _buttons |= ButtonsBit(button);
    (void)_frame->GetEventHandler().HandleMousePressEvent(
      MouseEvent(boo::PlatformEvent::Type::MousePressed, point, button, _clickCount, modifiers));
  }

  void Page::MouseReleased(const boo::IntPoint &point, boo::MouseButton button,
                           EnumSet<boo::PlatformEvent::Modifier> modifiers) noexcept
  {
    _buttons &= static_cast<uint16>(~ButtonsBit(button));
    (void)_frame->GetEventHandler().HandleMouseReleaseEvent(
      MouseEvent(boo::PlatformEvent::Type::MouseReleased, point, button, _clickCount, modifiers));
  }

  void Page::MouseLeft() noexcept
  {
    // A move outside the viewport hits nothing, so the element under the mouse is left and nothing is
    // hovered.
    MouseMoved(boo::IntPoint(-1, -1));
  }

  boo::Cursor Page::CurrentCursor() const noexcept
  {
    return _frame->GetEventHandler().CurrentMouseCursor();
  }

  void Page::SetFocused(bool focused) noexcept
  {
    _focused = focused;
    boo::html::FocusAlgorithms::SystemFocusChanged(*_document, focused);
  }

  boo::PlatformMouseEvent Page::MouseEvent(boo::PlatformEvent::Type type, const boo::IntPoint &point,
                                           boo::MouseButton button, int32 clickCount,
                                           EnumSet<boo::PlatformEvent::Modifier> modifiers) const noexcept
  {
    // FIXME: The screen position, once the host reports where the page is on the screen.
    return boo::PlatformMouseEvent(boo::DoublePoint(point), boo::DoublePoint(point), button, type, clickCount,
                                   modifiers, MonotonicTime::Now(), _buttons);
  }

  uint16 Page::ButtonsBit(boo::MouseButton button) noexcept
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
