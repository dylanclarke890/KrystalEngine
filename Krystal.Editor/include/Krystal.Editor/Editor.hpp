#pragma once

#ifndef KRYS_ENABLE_PROFILING
  #define KRYS_ENABLE_PROFILING
#endif

#include "Krystal.Booey/DOM/CommandEvent.hpp"
#include "Krystal.Booey/DOM/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/Graphics/DisplayLists/DisplayListRecorderImpl.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/HTML/HTMLDialogElement.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/NavigateEvent.hpp"
#include "Krystal.Booey/Page/Navigation.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Core/Utils/ScopedProfiler.hpp"
#include "Krystal.Editor/Application.hpp"
#include "Krystal.Editor/BeanRating.hpp"
#include "Krystal.Editor/BooeyScene.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.PAL/CursorShapes.hpp"
#include "Krystal.PAL/Events.hpp"
#include "Krystal.PAL/Keys.hpp"
#include "Krystal.Text/FreeTypeFace.hpp"
#include "Krystal.UI/View.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace krys
{
  /// @brief What the editor shows below its header; F2 moves to the next.
  enum class EditorView : uint8
  {
    // An HTML page from `Krystal.Editor/pages`, painted by Booey.
    Page,
    // A dashboard drawn with Render2D directly.
    Render2D,
    // A scene drawn through Booey's graphics context.
    BooeyScene,
  };

  class Editor : public Application
  {
  private:
    /// @brief The height of the header the views are drawn under.
    static constexpr float HeaderHeight = 56.f;

    uint32 _width {0u};
    uint32 _height {0u};
    float _time {0.f};
    UniquePtr<gfx::Renderer2D> _renderer {};
    gfx::TextureHandle _image {};
    UniquePtr<gfx::FontFace2D> _font {};
    UniquePtr<gfx::FontFace2D> _boldFont {};
    UniquePtr<ui::View> _pageView {};
    /// @brief The page's listeners, which go when the page is loaded again.
    boo::dom::EventListenerHandle _cardPicker;
    boo::dom::EventListenerHandle _keyLogger;
    boo::dom::EventListenerHandle _commandHandler;
    boo::dom::EventListenerHandle _dialogLogger;
    /// @brief Routes the guide's topics within the page, on the window's navigation.
    boo::dom::EventListenerHandle _router;
    /// @brief The last load's events, which the pages show in their `#load-events` element.
    utf8_string _loadEvents;
    /// @brief The page's time, which only moves while the editor runs (Booey's timers follow it).
    MonotonicTime::duration _pageTime {};
    /// @brief Counts the seconds the page has been open in its `#clock` element.
    boo::Timer _clockTimer {[this] { TickClock(); }};
    uint32 _secondsOpen {0};
    bool _focused {true};
    editor::BooeyFonts _booeyFonts {};
    EditorView _view {EditorView::Page};
    /// @brief The mouse in page view coordinates: below the header.
    boo::IntPoint _pointer {};

  public:
    explicit Editor(int argc, char **argv, const ApplicationSettings &settings) noexcept
        : Application(argc, argv, settings), _width(settings.WindowSettings.Size.Width),
          _height(settings.WindowSettings.Size.Height)
    {
    }

    void OnInit() noexcept override
    {
      _context->Logger->SetLevel(log::Level::Info);
      _context->Logger->Info("Initialising Krystal Editor...");
      BindEvents();

      auto renderer = gfx::Renderer2D::Create(*_context->Device);
      if (!renderer)
      {
        _context->Logger->Error("{}", renderer.error());
        return;
      }
      _renderer = krys::move(*renderer);
      CreateImage();
      LoadFonts();
      CreatePage();
    }

    void OnShutdown() noexcept override
    {
      _pageView = nullptr;
      _booeyFonts = {};
      _renderer = nullptr;
      _boldFont = nullptr;
      _font = nullptr;
      if (_image.IsValid())
      {
        _context->Device->Destroy(_image);
      }
    }

    /// @brief A 16-by-16 colour gradient with a checkerboard over it, since the editor has no image assets.
    void CreateImage() noexcept
    {
      constexpr uint32 size = 16u;
      Array<Array<uint8, 4>, size * size> texels {};
      for (uint32 y = 0u; y < size; ++y)
      {
        for (uint32 x = 0u; x < size; ++x)
        {
          const bool light = (x / 4u + y / 4u) % 2u == 0u;
          const auto shade = static_cast<uint8>(light ? 255u : 170u);
          texels[y * size + x] = {static_cast<uint8>(x * shade / (size - 1u)),
                                  static_cast<uint8>(y * shade / (size - 1u)), shade, 255u};
        }
      }

      auto image =
        _context->Device->CreateTexture({.Width = size, .Height = size, .DebugName = u8"EditorImage"});
      if (!image)
      {
        _context->Logger->Error("{}", image.error());
        return;
      }
      _image = *image;
      _context->Device->WriteTexture(
        _image, gfx::TextureRegion::Whole2D(size, size),
        Span<const byte>(reinterpret_cast<const byte *>(texels.data()), sizeof(texels)));
    }

    /// @brief Liberation Sans, regular and synthetically bold, for the labels.
    void LoadFonts() noexcept
    {
      std::ifstream file(std::filesystem::path {KRYS_EDITOR_FONTS_DIR} / "LiberationSans-Regular.ttf",
                         std::ios::binary);
      if (!file)
      {
        _context->Logger->Error("Couldn't open Liberation Sans.");
        return;
      }
      const List<char> bytes {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
      List<byte> data(bytes.size());
      std::memcpy(data.data(), bytes.data(), bytes.size());
      auto face = text::FreeTypeFace::Create(text::FontFileData::Create(krys::move(data)), 0);
      if (!face)
      {
        _context->Logger->Error("Couldn't read Liberation Sans.");
        return;
      }
      Ref<text::FreeTypeFace> shared {krys::move(face)};
      if (auto font = gfx::FontFace2D::Create(shared))
      {
        _font = krys::move(*font);
      }
      if (auto font = gfx::FontFace2D::Create(shared, {.SyntheticBold = true}))
      {
        _boldFont = krys::move(*font);
      }
    }

    void BindEvents() noexcept
    {
      using namespace pal;

      _context->Events->On<KeyboardEvent>(
        [&](const auto &event)
        {
          if (event.State() == pal::KeyState::Pressed)
          {
            _context->Logger->Info("Key Pressed '{}'", event.Key());
          }

          if (event.Key() == pal::Key::F2 && event.State() == pal::KeyState::Pressed)
          {
            _view = static_cast<EditorView>((static_cast<uint8>(_view) + 1u) % 3u);
          }

          if (event.Key() == pal::Key::F5 && event.State() == pal::KeyState::Pressed)
          {
            // Reloading loads the page's style sheets and images again, so edits to them show.
            if (_pageView)
            {
              _pageView->Reload();
            }
          }

          if (event.Key() == pal::Key::Escape)
          {
            _context->Logger->Info("ESC key pressed, quitting application...");
            Stop();
            return true;
          }

          // The other keys go to the page view's focused element.
          if (_pageView && _view == EditorView::Page && event.Key() != pal::Key::F2
              && event.Key() != pal::Key::F5)
          {
            if (event.State() == pal::KeyState::Released)
            {
              (void)_pageView->KeyReleased(event.Key(), event.Modifiers());
            }
            else
            {
              (void)_pageView->KeyPressed(event.Key(), event.Modifiers(), event.Text(),
                                          event.State() == pal::KeyState::Held);
            }
          }
          return true;
        });

      _context->Events->On<WindowResizeEvent>(
        [&](const auto &event)
        {
          _width = event.Width();
          _height = event.Height();
          if (_pageView)
          {
            _pageView->Resize(PageSize());
          }
          return true;
        });

      // `:focus` matches, and the focused element gets `focus` and `blur`, as the window gains and loses
      // focus.
      _context->Events->On<WindowFocusEvent>(
        [&](const auto &event)
        {
          _focused = event.Focused();
          if (_pageView)
          {
            _pageView->SetFocused(_focused);
          }
          return true;
        });

      // The page view's mouse: mouse events, hover and active state and the cursor follow it.
      _context->Events->On<MouseMoveEvent>(
        [&](const auto &event)
        {
          _pointer = boo::IntPoint(static_cast<int32>(event.ClientX()),
                                   static_cast<int32>(event.ClientY() - HeaderHeight));
          if (_pageView && _view == EditorView::Page)
          {
            if (_pointer.Y() < 0)
            {
              _pageView->MouseLeft();
            }
            else
            {
              _pageView->MouseMoved(_pointer, event.Modifiers());
            }
          }
          UpdateCursor();
          return true;
        });

      _context->Events->On<MouseButtonEvent>(
        [&](const auto &event)
        {
          if (!_pageView || _view != EditorView::Page)
          {
            return true;
          }
          if (event.IsPressed() && _pointer.Y() >= 0)
          {
            _pageView->MousePressed(_pointer, event.Button(), event.Modifiers());
          }
          else if (event.IsPressed() && event.Button() == pal::MouseButton::Left)
          {
            ClickHeader(static_cast<float>(_pointer.X()), static_cast<float>(_pointer.Y()) + HeaderHeight);
          }
          else if (event.WasReleased())
          {
            _pageView->MouseReleased(_pointer, event.Button(), event.Modifiers());
          }
          UpdateCursor();
          return true;
        });

      // The wheel scrolls the page view: the box under the mouse, or else the page.
      _context->Events->On<MouseScrollEvent>(
        [&](const auto &event)
        {
          if (!_pageView || _view != EditorView::Page)
          {
            return true;
          }
          boo::IntPoint point(static_cast<int32>(event.ClientX()),
                              static_cast<int32>(event.ClientY() - HeaderHeight));
          if (point.Y() >= 0)
          {
            (void)_pageView->MouseWheel(point, event.DeltaX(), event.DeltaY(), event.Modifiers());
          }
          return true;
        });

      _context->Events->On<WindowCloseEvent>(
        [&](const auto &)
        {
          Stop();
          return true;
        });
    }

    void OnUpdate(seconds dt) noexcept override
    {
      _time += static_cast<float>(dt.count());

      // Booey's event loop runs on the page's time: timers, then tasks and microtasks.
      _pageTime += std::chrono::duration_cast<MonotonicTime::duration>(dt);
      if (_pageView)
      {
        _pageView->Update(_pageTime);
      }
    }

    void OnRender(gfx::CommandBuffer &commands) noexcept override
    {
      if (!_renderer || _width == 0u || _height == 0u)
      {
        return;
      }

      _renderer->Begin(commands, {.Swapchain = _context->Swapchain,
                                  .Width = _width,
                                  .Height = _height,
                                  .Format = gfx::TextureFormat::RGBA8Unorm,
                                  .ClearColour = Array<float, 4> {0.933f, 0.945f, 0.965f, 1.f}});

      switch (_view)
      {
        case EditorView::Page: DrawPage(); break;
        case EditorView::Render2D:
          DrawCard(U"Mask clipping", 24.f, 80.f, 392.f, 616.f);
          DrawCard(U"Shadows", 444.f, 80.f, 392.f, 616.f);
          DrawCard(U"Compositing", 864.f, 80.f, 392.f, 616.f);
          DrawClipping(24.f, 80.f);
          DrawShadows(444.f, 80.f);
          DrawCompositing(864.f, 80.f);
          break;
        case EditorView::BooeyScene: DrawBooey(); break;
      }
      // The header goes over the page, which scrolls under it.
      DrawHeader();

      _renderer->End();
    }

  private:
    /// @brief The size of the page view: the window below the header.
    KRYS_NODISCARD boo::IntSize PageSize() const noexcept
    {
      const auto height = std::max(static_cast<float>(_height) - HeaderHeight, 0.f);
      return boo::IntSize(static_cast<int32>(_width), static_cast<int32>(height));
    }

    /// @brief Loads the page view's first page.
    void LoadPage() noexcept
    {
      if (!_pageView)
      {
        return;
      }
      const auto path = std::filesystem::path {KRYS_EDITOR_PAGES_DIR} / "index.html";
      if (auto loaded = _pageView->LoadFile(path); !loaded)
      {
        _context->Logger->Error("{}", loaded.error());
        return;
      }

      _secondsOpen = 0;
      _clockTimer.StartRepeating(std::chrono::seconds(1));
    }

    /// @brief Hears the page view's loads: a page is set up as it commits (on a later frame than the load
    /// starts, or as the back/forward cache restores it), and each page shows the last load's events.
    void HandleLoad(const ui::LoadEvent &event) noexcept
    {
      auto &document = _pageView->Document();
      switch (event.Type)
      {
        case ui::LoadEventType::Started: _loadEvents = u8"started"; break;
        case ui::LoadEventType::Committed:
          // A page from the back/forward cache has already loaded.
          _loadEvents +=
            event.FromBackForwardCache ? u8", committed from the back/forward cache" : u8", committed";
          AttachPage();
          break;
        case ui::LoadEventType::Finished:
          _loadEvents += u8", finished";
          _context->Logger->Info("Loaded {}", Narrow(document.GetURL().String()));
          ShowGuide(document.GetURL());
          break;
        case ui::LoadEventType::Failed:
          _loadEvents += u8", failed";
          _context->Logger->Error("Couldn't load: {}", Narrow(event.Error.LocalizedDescription()));
          break;
      }
      if (auto label = document.GetElementById(u8"load-events"))
      {
        (void)label->TextContent(u8"Last load of " + utf8_string(document.GetURL().LastPathComponent())
                                 + u8": " + _loadEvents);
      }
    }

    KRYS_NODISCARD static std::string_view Narrow(utf8_stringview text) noexcept
    {
      return std::string_view(reinterpret_cast<const char *>(text.data()), text.size());
    }

    /// @brief Sets up the page as it commits: the card picker, the key logger, the dialogs and the guide's
    /// router.
    void AttachPage() noexcept
    {
      auto &document = _pageView->Document();
      if (auto *window = document.Window())
      {
        _router = window->GetNavigation().AddEventListener<boo::NavigateEvent>(
          u8"navigate", [this](boo::NavigateEvent &event) { RouteGuide(event); });
      }
      // Toggles `picked` on the card a click lands in.
      _cardPicker =
        document.AddEventListener(u8"click",
                                  [](boo::dom::Event &event)
                                  {
                                    auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
                                    auto *target = node ? DynamicDowncast<boo::dom::Element>(*node) : nullptr;
                                    if (!target)
                                    {
                                      return;
                                    }
                                    auto card = target->Closest(u8".card");
                                    if (card.HasValue() && card.Value())
                                    {
                                      (void)card.Value()->ClassList().Toggle(boo::dom::DOMString(u8"picked"));
                                    }
                                  });
      // Shows the `key` of each key pressed on the page in its `#last-key` element.
      _keyLogger = document.AddEventListener<boo::dom::KeyboardEvent>(
        u8"keydown",
        [this](boo::dom::KeyboardEvent &event)
        {
          if (auto label = _pageView->Document().GetElementById(u8"last-key"))
          {
            auto key = event.Key() == u8" " ? boo::dom::DOMString(u8"Space") : event.Key();
            (void)label->TextContent(krys::move(key));
          }
        });
      // Carries out the page's custom `--show` command, which shows a dialog without making it modal (there's
      // no built-in command for that). `command` doesn't bubble, so this listens as it's captured.
      _commandHandler = document.AddEventListener<boo::dom::CommandEvent>(
        u8"command",
        [](boo::dom::CommandEvent &event)
        {
          auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
          auto *dialog = node ? DynamicDowncast<boo::html::HTMLDialogElement>(*node) : nullptr;
          if (dialog && event.Command() == u8"--show")
          {
            (void)dialog->Show();
          }
        },
        true);
      // Shows how the last dialog closed in the page's `#dialog-result` element (`close` doesn't bubble, so
      // this listens as it's captured).
      _dialogLogger = document.AddEventListener(
        u8"close",
        [this](boo::dom::Event &event)
        {
          auto *node = DynamicDowncast<boo::dom::Node>(event.Target().get());
          auto *dialog = node ? DynamicDowncast<boo::html::HTMLDialogElement>(*node) : nullptr;
          auto label = _pageView->Document().GetElementById(u8"dialog-result");
          if (!dialog || !label)
          {
            return;
          }
          const auto &result = dialog->ReturnValue();
          (void)label->TextContent(result.empty() ? boo::dom::DOMString(u8"Closed without an answer.")
                                                  : u8"Closed with \u201c" + result + u8"\u201d.");
        },
        true);
    }

    /// @brief Shows how long the page has been open, once a second.
    void TickClock() noexcept
    {
      ++_secondsOpen;
      if (auto clock = _pageView ? _pageView->Document().GetElementById(u8"clock") : nullptr)
      {
        auto text = std::format("Open for {}:{:02}", _secondsOpen / 60, _secondsOpen % 60);
        (void)clock->TextContent(
          boo::dom::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size()));
      }
    }

    /// @brief The guide topic @p url names in its `guide` query parameter; empty if none.
    KRYS_NODISCARD static utf8_string GuideTopic(const boo::url::URL &url) noexcept
    {
      constexpr utf8_stringview key = u8"guide=";
      auto query = url.Query();
      for (size_t start = 0; start <= query.size();)
      {
        auto end = std::min(query.find(u8'&', start), query.size());
        auto parameter = query.substr(start, end - start);
        if (parameter.starts_with(key))
        {
          return utf8_string(parameter.substr(key.size()));
        }
        start = end + 1;
      }
      return {};
    }

    /// @brief The guide's router: intercepts navigations to the page's guide topics (and back and forward
    /// between the page's entries), so the URL and the history change without a load, and shows the topic.
    void RouteGuide(boo::NavigateEvent &event) noexcept
    {
      if (!event.CanIntercept() || event.HashChange()
          || event.NavigationType() == boo::NavigationNavigationType::Reload)
      {
        return;
      }
      auto &document = _pageView->Document();
      const auto &destination = event.Destination()->URL();
      if (destination.Path() != document.GetURL().Path())
      {
        return;
      }
      if (GuideTopic(destination).empty()
          && event.NavigationType() != boo::NavigationNavigationType::Traverse)
      {
        return;
      }
      boo::NavigateEvent::NavigationInterceptOptions options;
      options.Handler = [this, destination](Func<void(boo::dom::ExceptionOr<void>)> &&completion)
      {
        ShowGuide(destination);
        completion({});
      };
      (void)event.Intercept(document, krys::move(options));
    }

    /// @brief Shows the guide topic @p url names, and marks its link.
    void ShowGuide(const boo::url::URL &url) noexcept
    {
      auto &document = _pageView->Document();
      auto view = document.GetElementById(u8"guide-view");
      if (!view)
      {
        return;
      }
      auto topic = GuideTopic(url);
      utf8_string text =
        topic == u8"pages"
          ? u8"Each document the view loads is a page of its own: the settings link loads one in "
            u8"place of this, and the load's events show under the order form."
        : topic == u8"history"
          ? u8"The view keeps a session history: back and forward go through it, and the pages "
            u8"left wait in the back/forward cache, so they come back as they were."
        : topic == u8"routing"
          ? u8"These topics never load: the editor's router intercepts their navigations, so "
            u8"only the URL, the history and this paragraph change."
          : u8"Pick a topic.";
      (void)view->TextContent(krys::move(text));
      for (utf8_stringview name : {u8"pages", u8"history", u8"routing"})
      {
        if (auto link = document.GetElementById(u8"topic-" + utf8_string(name)))
        {
          (void)link->ClassList().Toggle(boo::dom::DOMString(u8"current"), name == topic);
        }
      }
    }

    /// @brief Shows the page's cursor over the page view, and the arrow everywhere else.
    void UpdateCursor() noexcept
    {
      bool overPage = _pageView && _view == EditorView::Page && _pointer.Y() >= 0;
      _context->Window->SetCursor(overPage ? _pageView->CurrentCursor() : pal::CursorShape::Arrow);
    }

    /// @brief The page shown in the page view, which Booey's drawing is also replayed through, and Liberation
    /// Sans as Booey's fonts.
    void CreatePage() noexcept
    {
      (void)ui::View::AddFontDirectory(std::filesystem::path {KRYS_EDITOR_FONTS_DIR});
      _pageView = CreateUnique<ui::View>(PageSize());
      _pageView->SetFocused(_focused);
      _pageView->SetNavigationHandler([this](const ui::NavigationRequest &request)
                                      { return HandleNavigation(request); });
      _pageView->SetLoadHandler([this](const ui::LoadEvent &event) { HandleLoad(event); });
      // The page's custom elements, which its parser creates.
      (void)_pageView->DefineCustomElement(
        u8"bean-rating", [](boo::html::HTMLElement &element) { return CreateUnique<BeanRating>(element); },
        BeanRating::Options());
      LoadPage();
      auto &database = boo::FontDatabase::Singleton();
      auto faces = database.FacesInFamily(u8"Liberation Sans");
      if (faces.empty())
      {
        _context->Logger->Error("Booey couldn't load Liberation Sans.");
        return;
      }
      _booeyFonts.Regular =
        boo::Font::Create(boo::FontPlatformData {ShareRef(*faces.front()), 14, false, false});
      _booeyFonts.Bold = boo::Font::Create(boo::FontPlatformData {ShareRef(*faces.front()), 14, true, false});
    }

    /// @brief Lets the page navigate within itself and follow links to the editor's other pages; logs any
    /// other navigation (a link or a form submission), which the editor doesn't follow, and shows where it
    /// leads in the page's `#last-submission` element.
    KRYS_NODISCARD ui::NavigationPolicy HandleNavigation(const ui::NavigationRequest &request) noexcept
    {
      if (request.IsSameDocument)
      {
        return ui::NavigationPolicy::Allow;
      }
      if (!request.IsFormSubmission && !request.Download && !request.OpensNewWindow
          && boo::url::URL(request.URL).ProtocolIs(u8"file"))
      {
        return ui::NavigationPolicy::Allow;
      }

      _context->Logger->Info("Navigation: {} {} {}{}", Narrow(request.Method), Narrow(request.URL),
                             Narrow(request.Body), request.OpensNewWindow ? " (new window)" : "");
      if (auto label = _pageView->Document().GetElementById(u8"last-submission"))
      {
        utf8_string text = request.IsFormSubmission ? request.Method + u8" " + request.URL
                           : request.Download       ? u8"Download " + request.URL
                                                    : u8"Link to " + request.URL;
        if (request.OpensNewWindow)
        {
          text += u8" (new window)";
        }
        (void)label->TextContent(krys::move(text));
      }
      return ui::NavigationPolicy::Ignore;
    }

    /// @brief The page view: the page laid out and painted by Booey, below the header.
    void DrawPage() noexcept
    {
      if (!_pageView)
      {
        return;
      }
      _renderer->Save();
      _renderer->Translate(0.f, HeaderHeight);
      _pageView->Paint(*_renderer);
      _renderer->Restore();
    }

    /// @brief The Booey scene view: a scene drawn through Booey's graphics context, recorded into a display
    /// list and replayed into Render2D.
    void DrawBooey() noexcept
    {
      if (!_pageView)
      {
        return;
      }
      boo::display_list::RecorderImpl recorder(
        boo::FloatSize(static_cast<float>(_width), static_cast<float>(_height)));
      editor::BooeyScene(recorder, _booeyFonts, _time).Draw();
      _pageView->Replay(*recorder.TakeDisplayList(), *_renderer);
    }

    KRYS_NODISCARD static gfx::RoundedRect2D Rounded(float x, float y, float width, float height,
                                                     float radius) noexcept
    {
      return {{x, y, width, height}, gfx::CornerRadii2D::Uniform(radius)};
    }

    KRYS_NODISCARD static ExtendedSRGBA<float> Hex(uint32 rgb) noexcept
    {
      return {static_cast<float>((rgb >> 16u) & 255u) / 255.f, static_cast<float>((rgb >> 8u) & 255u) / 255.f,
              static_cast<float>(rgb & 255u) / 255.f, 1.f};
    }

    KRYS_NODISCARD static SRGBA<uint8> Rgb(uint32 rgb, uint8 alpha = 255u) noexcept
    {
      return {static_cast<uint8>((rgb >> 16u) & 255u), static_cast<uint8>((rgb >> 8u) & 255u),
              static_cast<uint8>(rgb & 255u), alpha};
    }

    /// @brief An outer `box-shadow` as a browser paints it: the box's shape, moved out of view, casts the
    /// shadow, clipped out of the box a pixel inside its edge (the box is opaque), so it shows only around
    /// it.
    void BoxShadow(const gfx::RoundedRect2D &box, Vec2f offset, float blur, SRGBA<uint8> colour) noexcept
    {
      constexpr float away = 10'000.f;
      auto clipOut = box;
      clipOut.Rect = {box.Rect.X + 1.f, box.Rect.Y + 1.f, box.Rect.Width - 2.f, box.Rect.Height - 2.f};
      auto moved = box;
      moved.Rect.X += away;
      _renderer->Save();
      _renderer->ClipOutRoundedRect(clipOut);
      _renderer->SetDropShadow({.Offset = {offset.x - away, offset.y}, .BlurRadius = blur, .Colour = colour});
      _renderer->FillRoundedRect(moved, SRGBA<uint8> {0, 0, 0});
      _renderer->Restore();
    }

    /// @brief An inset `box-shadow` as a browser paints it: a rect with the box as its hole, moved out of
    /// view, casts the shadow inside a clip to the box.
    void InsetShadow(const gfx::RoundedRect2D &box, Vec2f offset, float blur, SRGBA<uint8> colour) noexcept
    {
      constexpr float away = 10'000.f;
      const float reach = blur + std::abs(offset.x) + std::abs(offset.y) + 2.f;
      const auto &rect = box.Rect;
      _renderer->Save();
      _renderer->ClipRoundedRect(box);
      _renderer->SetDropShadow({.Offset = {offset.x - away, offset.y}, .BlurRadius = blur, .Colour = colour});
      _renderer->Translate(away, 0.f);
      _renderer->FillRectWithRoundedHole(
        {rect.X - reach, rect.Y - reach, rect.Width + 2.f * reach, rect.Height + 2.f * reach}, box,
        SRGBA<uint8> {0, 0, 0});
      _renderer->Restore();
    }

    /// @brief A white tile with a two-layer shadow, as design systems draw cards.
    void DrawTile(const gfx::RoundedRect2D &tile, SRGBA<uint8> colour = {255, 255, 255}) noexcept
    {
      BoxShadow(tile, {0.f, 1.f}, 3.f, Rgb(0x000000, 30));
      BoxShadow(tile, {0.f, 4.f}, 12.f, Rgb(0x000000, 22));
      _renderer->FillRoundedRect(tile, colour);
    }

    /// @brief A line of text on a baseline, left to right, placed by advances (Booey shapes text; this is
    /// just for labels until it draws the editor).
    void DrawText(const char32_t *text, float x, float baseline, float size, const gfx::Paint2D &paint,
                  bool bold = false) noexcept
    {
      const auto *font = bold ? _boldFont.get() : _font.get();
      if (font == nullptr)
      {
        return;
      }
      List<uint32> glyphs;
      List<Vec2f> positions;
      float pen = 0.f;
      for (const char32_t *character = text; *character != U'\0'; ++character)
      {
        glyphs.push_back(font->GlyphIndex(*character));
        positions.push_back({pen, 0.f});
        pen += font->Advance(glyphs.back(), size);
      }
      _renderer->DrawGlyphs(*font, size, glyphs, positions, {x, baseline}, paint);
    }

    void DrawHeader() noexcept
    {
      using namespace gfx;
      const float width = static_cast<float>(_width);
      const Gradient2D header {
        .Shape = LinearGradient2D {{0.f, 0.f}, {width, 0.f}},
        .Stops = {{.Offset = 0.f, .Colour = Hex(0x4f6df5)}, {.Offset = 1.f, .Colour = Hex(0xb04ff5)}}};
      _renderer->Save();
      _renderer->SetDropShadow({.Offset = {0.f, 2.f}, .BlurRadius = 10.f, .Colour = Rgb(0x2a2050, 70)});
      _renderer->FillRect({0.f, 0.f, width, 56.f}, header);
      _renderer->Restore();
      _renderer->FillRoundedRect(Rounded(20.f, 16.f, 24.f, 24.f, 12.f), Rgb(0xffffff));
      _renderer->Save();
      _renderer->SetDropShadow({.Offset = {0.f, 1.f}, .BlurRadius = 4.f, .Colour = Rgb(0x1a1040, 120)});
      DrawText(U"Krystal", 56.f, 36.f, 22.f, Rgb(0xffffff), true);
      _renderer->Restore();
      const char32_t *links[] {U"Page", U"Render2D", U"Booey scene"};
      for (int i = 0; i < 3; ++i)
      {
        const bool active = i == static_cast<int>(_view);
        DrawText(links[i], 170.f + static_cast<float>(i) * 96.f, 34.f, 15.f,
                 Rgb(0xffffff, active ? 255 : 150), active);
      }
      DrawText(U"F2 switches views, F5 reloads the page", width - 290.f, 34.f, 13.f, Rgb(0xffffff, 170));

      // The page view's back and forward buttons, faded when there's nowhere to go.
      if (_pageView && _view == EditorView::Page)
      {
        const bool can[] {_pageView->CanGoBack(), _pageView->CanGoForward()};
        const char32_t *arrows[] {U"\u2039", U"\u203a"};
        for (int i = 0; i < 2; ++i)
        {
          const auto button = HistoryButton(i);
          _renderer->FillRoundedRect(
            Rounded(button.X, button.Y, button.Width, button.Height, button.Width / 2),
            Rgb(0xffffff, can[i] ? 60 : 20));
          DrawText(arrows[i], button.X + 11.f, button.Y + 24.f, 24.f, Rgb(0xffffff, can[i] ? 255 : 90), true);
        }
      }
    }

    /// @brief The header's back (0) and forward (1) buttons.
    KRYS_NODISCARD gfx::Rect2D HistoryButton(int index) const noexcept
    {
      return {static_cast<float>(_width) - 380.f + static_cast<float>(index) * 40.f, 12.f, 32.f, 32.f};
    }

    /// @brief A click in the header at (@p x, @p y): its back and forward buttons go back and forward.
    void ClickHeader(float x, float y) noexcept
    {
      for (int i = 0; i < 2; ++i)
      {
        const auto button = HistoryButton(i);
        if (x >= button.X && x < button.X + button.Width && y >= button.Y && y < button.Y + button.Height)
        {
          (void)(i == 0 ? _pageView->GoBack() : _pageView->GoForward());
        }
      }
    }

    void DrawCard(const char32_t *title, float x, float y, float width, float height) noexcept
    {
      const auto card = Rounded(x, y, width, height, 16.f);
      BoxShadow(card, {0.f, 2.f}, 6.f, Rgb(0x1a2240, 26));
      BoxShadow(card, {0.f, 12.f}, 32.f, Rgb(0x1a2240, 30));
      _renderer->FillRoundedRect(card, Rgb(0xffffff));
      DrawText(title, x + 24.f, y + 34.f, 16.f, Rgb(0x5a6378), true);
    }

    /// @brief Mask clipping: a turning star window, a panel with orbiting holes, two path clips at once, a
    /// turning rounded clip, a ring chart and an avatar.
    void DrawClipping(float x, float y) noexcept
    {
      using namespace gfx;

      // The image scrolling behind a turning star.
      const Vec2f star {x + 110.f, y + 160.f};
      _renderer->Save();
      _renderer->Translate(star.x, star.y);
      _renderer->Rotate(_time * 0.4f);
      _renderer->ClipPath(Star(82.f), FillRule::NonZero);
      _renderer->SetTransform({});
      const float drift = std::fmod(_time * 20.f, 40.f);
      for (int row = 0; row < 6; ++row)
      {
        for (int column = 0; column < 6; ++column)
        {
          if (_image.IsValid())
          {
            _renderer->DrawImage(_image,
                                 {star.x - 100.f + static_cast<float>(column) * 40.f - drift,
                                  star.y - 100.f + static_cast<float>(row) * 40.f - drift, 40.f, 40.f},
                                 {.Filter = SamplerFilter::Nearest});
          }
        }
      }
      _renderer->Restore();

      // A gradient panel with three holes orbiting in it.
      const auto panel = Rounded(x + 222.f, y + 80.f, 148.f, 150.f, 18.f);
      Path2D holes;
      for (int i = 0; i < 3; ++i)
      {
        const float angle = _time * 0.9f + static_cast<float>(i) * 2.0944f;
        holes.AddEllipse({x + 296.f + 38.f * std::cos(angle), y + 155.f + 38.f * std::sin(angle)},
                         {16.f, 16.f});
      }
      const Gradient2D panelFill {
        .Shape = LinearGradient2D {{panel.Rect.X, panel.Rect.Y},
                                   {panel.Rect.X + panel.Rect.Width, panel.Rect.Y + panel.Rect.Height}},
        .Stops = {{.Offset = 0.f, .Colour = Hex(0x06d6a0)}, {.Offset = 1.f, .Colour = Hex(0x118ab2)}}};
      _renderer->Save();
      _renderer->ClipOutPath(holes, FillRule::NonZero);
      _renderer->FillRoundedRect(panel, panelFill);
      _renderer->Restore();

      // Two path clips at once: a circle and a turning star, their masks multiplied.
      const Vec2f both {x + 110.f, y + 350.f};
      Path2D circle;
      circle.AddEllipse(both, {80.f, 80.f});
      const Gradient2D wheel {.Shape = ConicGradient2D {both, 0.f},
                              .Stops = {{.Offset = 0.f, .Colour = Hex(0xef476f)},
                                        {.Offset = 0.33f, .Colour = Hex(0xffd166)},
                                        {.Offset = 0.66f, .Colour = Hex(0x06d6a0)},
                                        {.Offset = 1.f, .Colour = Hex(0xef476f)}}};
      _renderer->Save();
      _renderer->ClipPath(circle, FillRule::NonZero);
      _renderer->Translate(both.x, both.y);
      _renderer->Rotate(-_time * 0.3f);
      _renderer->ClipPath(Star(110.f), FillRule::NonZero);
      _renderer->SetTransform({});
      _renderer->FillRect({both.x - 90.f, both.y - 90.f, 180.f, 180.f}, wheel);
      _renderer->Restore();

      // Stripes behind a rounded clip that rocks.
      const Vec2f rocking {x + 296.f, y + 350.f};
      _renderer->Save();
      _renderer->Translate(rocking.x, rocking.y);
      _renderer->Rotate(std::sin(_time * 0.8f) * 0.5f);
      _renderer->ClipRoundedRect(Rounded(-66.f, -52.f, 132.f, 104.f, 24.f));
      _renderer->SetTransform({});
      _renderer->FillRect({rocking.x - 100.f, rocking.y - 100.f, 200.f, 200.f}, Rgb(0xffd166));
      const float shift = std::fmod(_time * 16.f, 28.f);
      for (int i = -8; i <= 8; ++i)
      {
        const float offset = static_cast<float>(i) * 28.f + shift;
        _renderer->StrokeLine({rocking.x + offset - 100.f, rocking.y - 100.f},
                              {rocking.x + offset + 100.f, rocking.y + 100.f}, {.Width = 10.f},
                              Rgb(0xef476f));
      }
      _renderer->Restore();

      // A ring chart: a conic gradient with hard stops, clipped to a ring (even-odd).
      const Vec2f ring {x + 110.f, y + 530.f};
      Path2D band;
      band.AddEllipse(ring, {62.f, 62.f});
      band.AddEllipse(ring, {38.f, 38.f});
      const float share = 0.35f + 0.15f * std::sin(_time * 0.7f);
      const Gradient2D chart {.Shape = ConicGradient2D {ring, 0.f},
                              .Stops = {{.Offset = 0.f, .Colour = Hex(0x4f6df5)},
                                        {.Offset = share, .Colour = Hex(0x4f6df5)},
                                        {.Offset = share, .Colour = Hex(0xffd166)},
                                        {.Offset = share + 0.25f, .Colour = Hex(0xffd166)},
                                        {.Offset = share + 0.25f, .Colour = Hex(0xe3e7ef)},
                                        {.Offset = 1.f, .Colour = Hex(0xe3e7ef)}}};
      _renderer->Save();
      _renderer->ClipPath(band, FillRule::EvenOdd);
      _renderer->FillRect({ring.x - 64.f, ring.y - 64.f, 128.f, 128.f}, chart);
      _renderer->Restore();

      // A progress bar: a gradient inside a rounded clip, with stripes sliding along it.
      const auto track = Rounded(x + 222.f, y + 480.f, 148.f, 20.f, 10.f);
      _renderer->FillRoundedRect(track, Rgb(0xe3e7ef));
      const float progress = 0.5f + 0.45f * std::sin(_time * 0.6f);
      const Gradient2D fill {
        .Shape = LinearGradient2D {{track.Rect.X, 0.f}, {track.Rect.X + track.Rect.Width, 0.f}},
        .Stops = {{.Offset = 0.f, .Colour = Hex(0x4f6df5)}, {.Offset = 1.f, .Colour = Hex(0xb04ff5)}}};
      _renderer->Save();
      _renderer->ClipRoundedRect(track);
      _renderer->FillRoundedRect(Rounded(track.Rect.X, track.Rect.Y, track.Rect.Width * progress, 20.f, 10.f),
                                 fill);
      _renderer->StrokeLine({track.Rect.X - 20.f, track.Rect.Y + 10.f},
                            {track.Rect.X + track.Rect.Width * progress, track.Rect.Y + 10.f},
                            {.Width = 20.f, .Dashes = {6.f, 10.f}, .DashOffset = -_time * 30.f},
                            Rgb(0xffffff, 60));
      _renderer->Restore();

      // An avatar: the image clipped to a circle, with a ring and a shadow.
      const Vec2f avatar {x + 296.f, y + 565.f};
      Path2D face;
      face.AddEllipse(avatar, {32.f, 32.f});
      _renderer->Save();
      _renderer->SetDropShadow({.Offset = {0.f, 3.f}, .BlurRadius = 10.f, .Colour = Rgb(0x000000, 70)});
      _renderer->FillPath(face, FillRule::NonZero, Rgb(0xffffff));
      _renderer->Restore();
      if (_image.IsValid())
      {
        _renderer->Save();
        _renderer->ClipPath(face, FillRule::NonZero);
        _renderer->DrawImage(_image, {avatar.x - 32.f, avatar.y - 32.f, 64.f, 64.f});
        _renderer->Restore();
      }
      _renderer->StrokePath(face, {.Width = 3.f}, Rgb(0xffffff));
    }

    /// @brief Shadows: elevations, sharp, inset, glowing and translucent shadows, shadows of paths and
    /// strokes, shadows under transforms, and a card lifting.
    void DrawShadows(float x, float y) noexcept
    {
      using namespace gfx;

      // Elevations, as a design system steps them.
      const float blurs[] {2.f, 6.f, 14.f, 28.f};
      const float drops[] {1.f, 3.f, 6.f, 12.f};
      for (int i = 0; i < 4; ++i)
      {
        const auto tile = Rounded(x + 24.f + static_cast<float>(i) * 90.f, y + 64.f, 74.f, 64.f, 10.f);
        BoxShadow(tile, {0.f, drops[i]}, blurs[i], Rgb(0x000000, 70));
        _renderer->FillRoundedRect(tile, Rgb(0xffffff));
      }

      // Sharp, inset, glowing on a dark tile, and translucent (its shadow fades with it).
      const float row = y + 160.f;
      const auto sharp = Rounded(x + 24.f, row, 74.f, 64.f, 0.f);
      BoxShadow(sharp, {6.f, 6.f}, 0.f, Rgb(0x4f6df5));
      _renderer->FillRoundedRect(sharp, Rgb(0xffffff));
      DrawText(U"Sharp", sharp.Rect.X + 12.f, row + 24.f, 13.f, Rgb(0x5a6378));

      const auto well = Rounded(x + 114.f, row, 74.f, 64.f, 14.f);
      _renderer->FillRoundedRect(well, Rgb(0xeef1f6));
      InsetShadow(well, {0.f, 3.f}, 10.f, Rgb(0x000000, 110));

      const auto dark = Rounded(x + 204.f, row, 74.f, 64.f, 14.f);
      _renderer->Save();
      _renderer->SetDropShadow(
        {.Offset = {}, .BlurRadius = 18.f + 6.f * std::sin(_time * 2.f), .Colour = Rgb(0x00e5ff, 200)});
      _renderer->FillRoundedRect(dark, Rgb(0x23283a));
      _renderer->Restore();

      const auto glass = Rounded(x + 294.f, row, 74.f, 64.f, 14.f);
      _renderer->Save();
      _renderer->SetDropShadow({.Offset = {0.f, 6.f}, .BlurRadius = 12.f, .Colour = Rgb(0x000000, 160)});
      _renderer->FillRoundedRect(glass, Rgb(0x4f6df5, 110));
      _renderer->Restore();

      // Shadows of paths and strokes: a star, a zigzag, dots and a ring.
      const float shapes = y + 300.f;
      _renderer->Save();
      _renderer->SetDropShadow({.Offset = {3.f, 5.f}, .BlurRadius = 8.f, .Colour = Rgb(0x000000, 90)});
      _renderer->Save();
      _renderer->Translate(x + 62.f, shapes);
      _renderer->FillPath(Star(36.f), FillRule::NonZero, Rgb(0xffd166));
      _renderer->Restore();
      Path2D zigzag;
      zigzag.MoveTo({x + 110.f, shapes + 20.f});
      zigzag.LineTo({x + 135.f, shapes - 20.f});
      zigzag.LineTo({x + 160.f, shapes + 20.f});
      zigzag.LineTo({x + 185.f, shapes - 20.f});
      _renderer->StrokePath(zigzag, {.Width = 8.f, .Cap = LineCap::Round, .Join = LineJoin::Round},
                            Rgb(0xef476f));
      _renderer->StrokeLine({x + 210.f, shapes}, {x + 280.f, shapes},
                            {.Width = 10.f, .Cap = LineCap::Round, .Dashes = {0.f, 20.f}}, Rgb(0x06d6a0));
      Path2D ring;
      ring.AddEllipse({x + 330.f, shapes}, {26.f, 26.f});
      _renderer->StrokePath(ring, {.Width = 6.f}, Rgb(0x4f6df5));
      _renderer->Restore();

      // Under transforms: the shadow turning with its card (current coordinates) and staying put
      // (ignoring transforms, as canvas does), and a card squashed by a scale.
      const float turned = y + 420.f;
      for (int i = 0; i < 2; ++i)
      {
        _renderer->Save();
        _renderer->Translate(x + 70.f + static_cast<float>(i) * 120.f, turned);
        _renderer->Rotate(std::sin(_time * 0.9f) * 0.6f);
        _renderer->SetShadowsIgnoreTransforms(i == 1);
        _renderer->SetDropShadow({.Offset = {0.f, 8.f}, .BlurRadius = 12.f, .Colour = Rgb(0x000000, 90)});
        _renderer->FillRoundedRect(Rounded(-38.f, -28.f, 76.f, 56.f, 10.f), Rgb(0xffffff));
        _renderer->Restore();
      }
      _renderer->Save();
      _renderer->Translate(x + 320.f, turned);
      _renderer->Scale(1.4f, 0.7f);
      _renderer->SetDropShadow({.Offset = {0.f, 8.f}, .BlurRadius = 12.f, .Colour = Rgb(0x000000, 90)});
      _renderer->FillRoundedRect(Rounded(-26.f, -36.f, 52.f, 72.f, 10.f), Rgb(0xffffff));
      _renderer->Restore();

      // A card lifting on its shadow, and a star whose shadow swings round it.
      const float lift = 0.5f + 0.5f * std::sin(_time * 1.7f);
      const auto lifted = Rounded(x + 24.f, y + 520.f - 4.f * lift, 160.f, 64.f, 12.f);
      BoxShadow(lifted, {0.f, 2.f + 8.f * lift}, 6.f + 18.f * lift, Rgb(0x000000, 80));
      _renderer->FillRoundedRect(lifted, Rgb(0xffffff));
      DrawText(U"Lifting card", lifted.Rect.X + 16.f, lifted.Rect.Y + 26.f, 14.f, Rgb(0x222733), true);
      DrawText(U"Its shadow grows as it rises", lifted.Rect.X + 16.f, lifted.Rect.Y + 46.f, 11.f,
               Rgb(0x8a93a6));

      _renderer->Save();
      _renderer->SetDropShadow({.Offset = {8.f * std::cos(_time), 8.f * std::sin(_time)},
                                .BlurRadius = 10.f,
                                .Colour = Rgb(0x1a1040, 110)});
      _renderer->Translate(x + 300.f, y + 552.f);
      _renderer->FillPath(Star(36.f), FillRule::NonZero, Rgb(0xb04ff5));
      _renderer->Restore();
    }

    /// @brief Compositing (there are no filters yet): blend modes over the image, opacity per draw against
    /// a group's, a layer composited with multiply, additive light, and gradients.
    void DrawCompositing(float x, float y) noexcept
    {
      using namespace gfx;

      // Six blend modes over the image.
      const auto backdrop = Rounded(x + 24.f, y + 60.f, 344.f, 110.f, 12.f);
      _renderer->Save();
      _renderer->ClipRoundedRect(backdrop);
      if (_image.IsValid())
      {
        _renderer->DrawImage(_image, backdrop.Rect);
      }
      const BlendMode modes[] {BlendMode::Multiply,   BlendMode::Screen, BlendMode::Overlay,
                               BlendMode::Difference, BlendMode::Hue,    BlendMode::Luminosity};
      for (int i = 0; i < 6; ++i)
      {
        const float bob = 12.f * std::sin(_time * 2.f + static_cast<float>(i));
        _renderer->Save();
        _renderer->SetBlendMode(modes[i]);
        _renderer->FillRoundedRect(
          Rounded(x + 36.f + static_cast<float>(i) * 56.f, y + 92.f + bob, 46.f, 46.f, 23.f), Rgb(0xff8c00));
        _renderer->Restore();
      }
      _renderer->Restore();

      // Opacity per draw (the overlap mixes) and for a group (it fades as one).
      const float circles = y + 200.f;
      _renderer->FillRoundedRect(Rounded(x + 24.f, circles, 56.f, 56.f, 28.f), Rgb(0xef476f, 128));
      _renderer->FillRoundedRect(Rounded(x + 54.f, circles + 18.f, 56.f, 56.f, 28.f), Rgb(0x4f6df5, 128));
      _renderer->BeginLayer(0.5f + 0.4f * std::sin(_time * 1.5f));
      _renderer->FillRoundedRect(Rounded(x + 130.f, circles, 56.f, 56.f, 28.f), Rgb(0xef476f));
      _renderer->FillRoundedRect(Rounded(x + 160.f, circles + 18.f, 56.f, 56.f, 28.f), Rgb(0x4f6df5));
      _renderer->EndLayer();

      // A layer composited with multiply over stripes.
      const auto stripes = Rounded(x + 244.f, circles, 124.f, 74.f, 10.f);
      _renderer->Save();
      _renderer->ClipRoundedRect(stripes);
      for (int i = 0; i < 6; ++i)
      {
        _renderer->FillRect({stripes.Rect.X + static_cast<float>(i) * 21.f, stripes.Rect.Y, 21.f, 74.f},
                            i % 2 == 0 ? Rgb(0xffd166) : Rgb(0x06d6a0));
      }
      _renderer->BeginLayer(1.f, BlendMode::Multiply);
      _renderer->FillRoundedRect(Rounded(stripes.Rect.X + 18.f, circles + 12.f, 50.f, 50.f, 8.f),
                                 Rgb(0x8ecae6));
      _renderer->FillRoundedRect(Rounded(stripes.Rect.X + 52.f, circles + 12.f, 50.f, 50.f, 8.f),
                                 Rgb(0xf4a6c6));
      _renderer->EndLayer();
      _renderer->Restore();

      // Additive light (plus-lighter) on a dark tile.
      const auto night = Rounded(x + 24.f, y + 300.f, 344.f, 120.f, 14.f);
      _renderer->FillRoundedRect(night, Rgb(0x14182a));
      _renderer->Save();
      _renderer->ClipRoundedRect(night);
      _renderer->SetBlendMode(BlendMode::PlusLighter);
      const uint32 lights[] {0xd02020, 0x20c040, 0x2040e0};
      for (int i = 0; i < 3; ++i)
      {
        const float angle = _time * 0.8f + static_cast<float>(i) * 2.0944f;
        const Vec2f centre {x + 196.f + 34.f * std::cos(angle), y + 360.f + 22.f * std::sin(angle)};
        const Gradient2D glow {.Shape = RadialGradient2D {centre, centre, 0.f, 56.f},
                               .Stops = {{.Offset = 0.f, .Colour = Hex(lights[i])},
                                         {.Offset = 1.f, .Colour = {0.f, 0.f, 0.f, 0.f}}}};
        _renderer->FillRect({centre.x - 56.f, centre.y - 56.f, 112.f, 112.f}, glow);
      }
      _renderer->Restore();

      // Gradients: a turning linear one, a radial one with a circling focus, a turning conic one, a bar in
      // OKLCH the long way round and a subtle grey bar that shows the dithering.
      const float tiles = y + 450.f;
      const Vec2f linearCentre {x + 57.f, tiles + 33.f};
      const Vec2f along {33.f * std::cos(_time), 33.f * std::sin(_time)};
      const Gradient2D linear {
        .Shape = LinearGradient2D {{linearCentre.x - along.x, linearCentre.y - along.y},
                                   {linearCentre.x + along.x, linearCentre.y + along.y}},
        .Stops = {{.Offset = 0.f, .Colour = Hex(0xff8c00)}, {.Offset = 1.f, .Colour = Hex(0x7333e6)}}};
      _renderer->FillRoundedRect(Rounded(x + 24.f, tiles, 66.f, 66.f, 12.f), linear);

      const Vec2f radialCentre {x + 143.f, tiles + 33.f};
      const Vec2f focus {radialCentre.x + 14.f * std::cos(_time * 1.3f),
                         radialCentre.y + 14.f * std::sin(_time * 1.3f)};
      const Gradient2D radial {
        .Shape = RadialGradient2D {focus, radialCentre, 0.f, 33.f},
        .Stops = {{.Offset = 0.f, .Colour = Hex(0xffffff)}, {.Offset = 1.f, .Colour = Hex(0x1a4dcc)}}};
      _renderer->FillRoundedRect(Rounded(x + 110.f, tiles, 66.f, 66.f, 33.f), radial);

      const Gradient2D conic {.Shape = ConicGradient2D {{x + 229.f, tiles + 33.f}, _time * 0.7f},
                              .Stops = {{.Offset = 0.f, .Colour = Hex(0xef476f)},
                                        {.Offset = 0.5f, .Colour = Hex(0xffd166)},
                                        {.Offset = 0.5f, .Colour = Hex(0x118ab2)},
                                        {.Offset = 1.f, .Colour = Hex(0xef476f)}}};
      _renderer->FillRoundedRect(Rounded(x + 196.f, tiles, 66.f, 66.f, 33.f), conic);

      const Gradient2D hues {
        .Shape = LinearGradient2D {{x + 24.f, 0.f}, {x + 368.f, 0.f}},
        .Stops = {{.Offset = 0.f, .Colour = Hex(0xff0000)}, {.Offset = 1.f, .Colour = Hex(0x0000ff)}},
        .Interpolation = {.ColourSpace = ColourInterpolationMethod::OKLCH {HueInterpolationMethod::Longer},
                          .AlphaPremultiplication = AlphaPremultiplication::Premultiplied}};
      _renderer->FillRoundedRect(Rounded(x + 24.f, tiles + 86.f, 344.f, 20.f, 6.f), hues);

      const Gradient2D greys {
        .Shape = LinearGradient2D {{x + 24.f, 0.f}, {x + 368.f, 0.f}},
        .Stops = {{.Offset = 0.f, .Colour = Hex(0xd8dce4)}, {.Offset = 1.f, .Colour = Hex(0xeef1f6)}}};
      _renderer->FillRoundedRect(Rounded(x + 24.f, tiles + 116.f, 344.f, 20.f, 6.f), greys);
    }

    /// @brief A five-pointed star drawn in one stroke, so its contour crosses itself.
    KRYS_NODISCARD static gfx::Path2D Star(float radius) noexcept
    {
      gfx::Path2D path;
      for (int i = 0; i < 5; ++i)
      {
        // Every second point of a pentagon: the classic pentagram.
        const float angle = -1.5707963f + static_cast<float>(i * 2) * 1.2566371f;
        const Vec2f point {radius * std::cos(angle), radius * std::sin(angle)};
        if (i == 0)
        {
          path.MoveTo(point);
        }
        else
        {
          path.LineTo(point);
        }
      }
      path.Close();
      return path;
    }
  };
}
