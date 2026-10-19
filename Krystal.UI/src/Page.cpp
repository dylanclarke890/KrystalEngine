#include "Krystal.UI/Page.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.UI/GraphicsContextRender2D.hpp"
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
}
