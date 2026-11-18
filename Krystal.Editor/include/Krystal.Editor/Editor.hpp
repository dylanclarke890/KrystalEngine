#pragma once

#ifndef KRYS_ENABLE_PROFILING
  #define KRYS_ENABLE_PROFILING
#endif

#include "Krystal.Booey/DOM/Collections/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/Events/CommandEvent.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Platform/PlatformStrategies.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollbarThemeAdwaita.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Core/Utils/ScopedProfiler.hpp"
#include "Krystal.Editor/Application.hpp"
#include "Krystal.Editor/BeanRating.hpp"
#include "Krystal.Editor/PageAnimations.hpp"
#include "Krystal.Editor/PageCanvas2D.hpp"
#include "Krystal.Editor/PageCube.hpp"
#include "Krystal.Editor/PageDragAndDrop.hpp"
#include "Krystal.Editor/PageObservers.hpp"
#include "Krystal.Editor/PagePointers.hpp"
#include "Krystal.Editor/PageSpatialNavigation.hpp"
#include "Krystal.Editor/PageTextInput.hpp"
#include "Krystal.Gfx/Render2D/Renderer2D.hpp"
#include "Krystal.PAL/CursorShapes.hpp"
#include "Krystal.PAL/Events.hpp"
#include "Krystal.PAL/Keys.hpp"
#include "Krystal.Text/FreeTypeFace.hpp"
#include "Krystal.UI/FileLoaderStrategy.hpp"
#include "Krystal.UI/View.hpp"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <tuple>

namespace krys
{
  /// @brief The editor: an HTML page from `Krystal.Editor/pages`, painted by Booey below a header.
  class Editor : public Application
  {
  private:
    /// @brief The height of the header the page is drawn under.
    constexpr static float HeaderHeight = 56.f;

    uint32 _width {0u};
    uint32 _height {0u};
    UniquePtr<gfx::Renderer2D> _renderer;
    UniquePtr<gfx::FontFace2D> _font;
    UniquePtr<gfx::FontFace2D> _boldFont;
    /// @brief Booey's loader, which reads files on the application's workers.
    UniquePtr<ui::FileLoaderStrategy> _fileLoader;
    UniquePtr<ui::View> _pageView;
    /// @brief The page's listeners, which go when the page is loaded again.
    boo::EventListenerHandle _cardPicker;
    boo::EventListenerHandle _keyLogger;
    boo::EventListenerHandle _commandHandler;
    boo::EventListenerHandle _dialogLogger;
    /// @brief The page's observers, made once it has loaded: the header shows the chapter most in view.
    UniquePtr<editor::PageObservers> _observers;
    /// @brief The page's animation from C++, made once it has loaded.
    UniquePtr<editor::PageAnimations> _animations;
    /// @brief The page's pointer samples from C++, made once it has loaded.
    UniquePtr<editor::PagePointers> _pointers;
    /// @brief The page's spatial navigation sample from C++, made once it has loaded.
    UniquePtr<editor::PageSpatialNavigation> _spatialNavigation;
    /// @brief The page's input method sample from C++, made once it has loaded.
    UniquePtr<editor::PageTextInput> _textInput;
    /// @brief The page's drag and drop sample from C++, made once it has loaded.
    UniquePtr<editor::PageDragAndDrop> _dragAndDrop;
    UniquePtr<editor::PageCube> _cube;
    UniquePtr<editor::PageCanvas2D> _canvas2D;
    /// @brief The last load's events, which the pages show in their `#load-events` element.
    utf8_string _loadEvents;
    /// @brief The page's time, which only moves while the editor runs (Booey's timers follow it).
    MonotonicTime::duration _pageTime {};
    /// @brief Counts the seconds the page has been open in its `#clock` element.
    boo::Timer _clockTimer {[this] { TickClock(); }};
    uint32 _secondsOpen {0};
    bool _focused {true};
    /// @brief Where the window's input method was last told the page's text input is, in the window.
    Maybe<boo::IntRect> _textInputRect;
    /// @brief The mouse in page view coordinates: below the header.
    boo::IntPoint _pointer;

  public:
    explicit Editor(int argc, char **argv, const ApplicationSettings &settings) noexcept
        : Application(argc, argv, settings), _width(_context->Window->GetSize().Width),
          _height(_context->Window->GetSize().Height)
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
      LoadFonts();
      CreatePage();
    }

    void OnShutdown() noexcept override
    {
      boo::PlatformStrategies::Singleton().SetWorkQueue(nullptr);
      boo::PlatformStrategies::Singleton().SetLoaderStrategy(nullptr);
      _observers = nullptr;
      _animations = nullptr;
      _pointers = nullptr;
      _spatialNavigation = nullptr;
      _textInput = nullptr;
      _dragAndDrop = nullptr;
      _cube = nullptr;
      _canvas2D = nullptr;
      _pageView = nullptr;
      _renderer = nullptr;
      _boldFont = nullptr;
      _font = nullptr;
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

          if (event.Key() == pal::Key::F5 && event.State() == pal::KeyState::Pressed)
          {
            // Reloading loads the page's style sheets and images again, so edits to them show.
            if (_pageView)
            {
              _pageView->Reload();
            }
          }

          // Escape cancels a drag in the page rather than quitting.
          if (event.Key() == pal::Key::Escape && !(_pageView && _pageView->IsDragging()))
          {
            _context->Logger->Info("ESC key pressed, quitting application...");
            Stop();
            return true;
          }

          // The other keys go to the page view's focused element.
          if (_pageView && event.Key() != pal::Key::F5)
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
            // The keys change a drag's cursor.
            UpdateCursor();
          }
          return true;
        });

      // Text typed without a key press, and the input method's composition, go to the page view.
      _context->Events->On<TextInputEvent>(
        [&](const auto &event)
        {
          if (_pageView)
          {
            (void)_pageView->InsertText(event.Text());
          }
          return true;
        });

      _context->Events->On<CompositionInputEvent>(
        [&](const auto &event)
        {
          if (!_pageView)
          {
            return true;
          }
          const auto &input = event.Input();
          switch (input.Action)
          {
            case CompositionAction::Updated:
              _pageView->SetComposition(input.Text, input.Clauses, input.Caret);
              break;
            case CompositionAction::Committed: _pageView->ConfirmComposition(input.Text); break;
            default:                           break;
          }
          return true;
        });

      _context->Events->On<WindowDPIChangeEvent>(
        [&](const auto &event)
        {
          if (_pageView)
          {
            _pageView->SetDeviceScaleFactor(static_cast<float>(event.DPI()) / 96.f);
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

      // The page is hidden while the window is minimised.
      _context->Events->On<WindowMinimiseEvent>(
        [&](const auto &)
        {
          if (_pageView)
          {
            _pageView->SetVisible(false);
          }
          return true;
        });

      _context->Events->On<WindowRestoreEvent>(
        [&](const auto &)
        {
          if (_pageView)
          {
            _pageView->SetVisible(true);
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
          if (_pageView)
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

      // While the page has locked the pointer, the mouse's movement goes to it instead.
      _context->Events->On<LockedMouseMoveEvent>(
        [&](const auto &event)
        {
          if (_pageView)
          {
            _pageView->LockedMouseMoved(boo::DoublePoint(event.DeltaX(), event.DeltaY()),
                                        boo::DoublePoint(event.RawDeltaX(), event.RawDeltaY()),
                                        event.Modifiers());
          }
          return true;
        });

      _context->Events->On<MouseButtonEvent>(
        [&](const auto &event)
        {
          if (!_pageView)
          {
            return true;
          }
          if (event.IsPressed() && _pointer.Y() >= 0)
          {
            _pageView->MousePressed(_pointer, event.Button(), event.Modifiers());
          }
          else if (event.WasReleased())
          {
            _pageView->MouseReleased(_pointer, event.Button(), event.Modifiers());
          }
          UpdateCursor();
          return true;
        });

      // Pens and touches go to the page view as pointer events (a pen's with its compatibility mouse events,
      // a touch's with touch events).
      _context->Events->On<PointerInputEvent>(
        [&](const auto &event)
        {
          if (!_pageView)
          {
            return true;
          }
          auto &input = event.Input();
          boo::DoublePoint point(input.ClientX, input.ClientY - HeaderHeight);
          if (input.Type == pal::PointerType::Pen)
          {
            _pageView->PenInput(point, input);
            UpdateCursor();
          }
          else
          {
            (void)_pageView->TouchInput(point, input);
          }
          return true;
        });

      // The wheel scrolls the page view: the box under the mouse, or else the page.
      _context->Events->On<MouseScrollEvent>(
        [&](const auto &event)
        {
          if (!_pageView)
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

      // A touchpad's pans scroll the page view as the wheel does; their ends go to it wherever the cursor is,
      // so what they scrolled snaps.
      _context->Events->On<TouchpadPanEvent>(
        [&](const auto &event)
        {
          if (!_pageView)
          {
            return true;
          }
          auto &input = event.Input();
          boo::IntPoint point(static_cast<int32>(input.ClientX),
                              static_cast<int32>(input.ClientY - HeaderHeight));
          bool moves = input.DeltaX || input.DeltaY;
          if (point.Y() >= 0 || !moves)
          {
            (void)_pageView->TouchpadPan(point, input);
          }
          return true;
        });

      // Drags from other applications go to the page view, which answers what a drop would do; over the
      // header, they're off the page.
      _context->Events->On<DragInputEvent>(
        [&](const auto &event)
        {
          if (!_pageView)
          {
            return true;
          }
          auto &input = event.Input();
          boo::IntPoint point(static_cast<int32>(input.ClientX),
                              static_cast<int32>(input.ClientY - HeaderHeight));
          _context->Window->SetDropOperation(_pageView->SystemDragInput(point, input));
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
      // Booey's event loop runs on the page's time: timers, then tasks and microtasks.
      _pageTime += std::chrono::duration_cast<MonotonicTime::duration>(dt);
      if (_pageView)
      {
        // The page's animations align their updates to the rate frames are drawn at: the display's with
        // vsync, or else the cap.
        const auto &settings = _context->Settings;
        _pageView->SetFramesPerSecond(
          settings.WindowSettings.VSync
            ? _context->Window->DisplayRefreshRate().value_or(settings.RenderFramerate)
            : settings.RenderFramerate);
        // The page's media features follow the colours of the display the window is on.
        _pageView->SetDisplayColour(_context->Window->GetDisplayColour());
        // The page view sits below the header, wherever the window is on the screen.
        auto origin = _context->Window->ClientOriginOnScreen();
        _pageView->SetScreenOrigin(boo::IntPoint(origin.X, origin.Y + static_cast<int32>(HeaderHeight)));
        // The pads drive the page as the keyboard does, before its frame runs.
        _pageView->GamepadInput(_context->Input->GetGamepads(), _pageTime);
        if (_cube)
        {
          _cube->Update(static_cast<float>(dt.count()));
        }
        if (_canvas2D)
        {
          _canvas2D->Update(static_cast<float>(dt.count()));
        }
        _pageView->Update(_pageTime);
      }
      UpdateTextInput();
    }

    void OnRender(gfx::CommandBuffer &commands) noexcept override
    {
      if (!_renderer || _width == 0u || _height == 0u)
      {
        return;
      }

      // The page's live 3D view is drawn into its texture before the page is painted.
      if (_cube)
      {
        _cube->Render(commands);
      }

      _renderer->Begin(commands, {.Swapchain = _context->Swapchain,
                                  .Width = _width,
                                  .Height = _height,
                                  .Format = gfx::TextureFormat::RGBA8Unorm,
                                  .ClearColour = Array<float, 4> {0.933f, 0.945f, 0.965f, 1.f}});

      DrawPage();
      // The header goes over the page, which scrolls under it.
      DrawHeader();

      _renderer->End();
    }

  private:
    /// @brief The window's input method follows the page view: it's on while the page's focus takes text,
    /// with its windows by the composition or the caret.
    void UpdateTextInput() noexcept
    {
      bool enabled = _pageView && _focused && _pageView->AcceptsTextInput();
      _context->Window->SetTextInputEnabled(enabled);
      if (!enabled)
      {
        _textInputRect = null;
        return;
      }

      auto rect = _pageView->TextInputRect();
      if (!rect)
      {
        return;
      }
      // The page is below the header.
      rect->Move(0, static_cast<int32>(HeaderHeight));
      if (rect != _textInputRect)
      {
        _textInputRect = rect;
        _context->Window->SetTextInputRect({rect->X(), rect->Y(), rect->Width(), rect->Height()});
      }
    }

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
    /// starts) and once it has loaded, and each page shows the last load's events.
    void HandleLoad(const ui::LoadEvent &event) noexcept
    {
      auto &document = _pageView->Document();
      switch (event.Type)
      {
        case ui::LoadEventType::Started: _loadEvents = u8"started"; break;
        case ui::LoadEventType::Committed:
          _loadEvents += u8", committed";
          AttachPage();
          _observers = nullptr;
          _animations = nullptr;
          _pointers = nullptr;
          _spatialNavigation = nullptr;
          _textInput = nullptr;
          _dragAndDrop = nullptr;
          _cube = nullptr;
          _canvas2D = nullptr;
          break;
        case ui::LoadEventType::Finished:
          _loadEvents += u8", finished";
          _observers = CreateUnique<editor::PageObservers>(document);
          _animations = CreateUnique<editor::PageAnimations>(document);
          _pointers = CreateUnique<editor::PagePointers>(document);
          _spatialNavigation = CreateUnique<editor::PageSpatialNavigation>(document);
          _textInput = CreateUnique<editor::PageTextInput>(document);
          _dragAndDrop = CreateUnique<editor::PageDragAndDrop>(document);
          _cube = nullptr;
          _cube = CreateUnique<editor::PageCube>(document, *_context->Device);
          _canvas2D = CreateUnique<editor::PageCanvas2D>(document);
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

    /// @brief Sets up the page as it commits: the card picker, the key logger and the dialogs.
    void AttachPage() noexcept
    {
      auto &document = _pageView->Document();
      // Toggles `picked` on the card a click lands in.
      _cardPicker =
        document.AddEventListener(u8"click",
                                  [](boo::Event &event)
                                  {
                                    auto *node = DynamicDowncast<boo::Node>(event.Target().get());
                                    auto *target = node ? DynamicDowncast<boo::Element>(*node) : nullptr;
                                    if (!target)
                                    {
                                      return;
                                    }
                                    auto card = target->Closest(u8".card");
                                    if (card.HasValue() && card.Value())
                                    {
                                      (void)card.Value()->ClassList().Toggle(boo::DOMString(u8"picked"));
                                    }
                                  });
      // Shows the `key` of each key pressed on the page in its `#last-key` element.
      _keyLogger = document.AddEventListener<boo::KeyboardEvent>(
        u8"keydown",
        [this](boo::KeyboardEvent &event)
        {
          if (auto label = _pageView->Document().GetElementById(u8"last-key"))
          {
            auto key = event.Key() == u8" " ? boo::DOMString(u8"Space") : event.Key();
            (void)label->TextContent(krys::move(key));
          }
        });
      // Carries out the page's custom `--show` command, which shows a dialog without making it modal (there's
      // no built-in command for that). `command` doesn't bubble, so this listens as it's captured.
      _commandHandler = document.AddEventListener<boo::CommandEvent>(
        u8"command",
        [](boo::CommandEvent &event)
        {
          auto *node = DynamicDowncast<boo::Node>(event.Target().get());
          auto *dialog = node ? DynamicDowncast<boo::HTMLDialogElement>(*node) : nullptr;
          if (dialog && event.Command() == u8"--show")
          {
            (void)dialog->Show();
          }
        },
        true);
      // Submits the settings page's form once its `#confirm-save` dialog closes with "save", and shows how
      // the last dialog closed in the first page's `#dialog-result` element (`close` doesn't bubble, so this
      // listens as it's captured).
      _dialogLogger = document.AddEventListener(
        u8"close",
        [this](boo::Event &event)
        {
          auto *node = DynamicDowncast<boo::Node>(event.Target().get());
          auto *dialog = node ? DynamicDowncast<boo::HTMLDialogElement>(*node) : nullptr;
          if (!dialog)
          {
            return;
          }
          const auto &result = dialog->ReturnValue();
          if (dialog->Id() == u8"confirm-save")
          {
            auto *form =
              DynamicDowncast<boo::HTMLFormElement>(_pageView->Document().GetElementById(u8"settings").get());
            if (form && result == u8"save")
            {
              (void)form->RequestSubmit();
            }
            // Closing without a value (Escape) keeps the last one, so it's cleared for the next time.
            dialog->ReturnValue({});
            return;
          }
          auto label = _pageView->Document().GetElementById(u8"dialog-result");
          if (!label)
          {
            return;
          }
          (void)label->TextContent(result.empty() ? boo::DOMString(u8"Closed without an answer.")
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
        (void)clock->TextContent(boo::DOMString(reinterpret_cast<const char8_t *>(text.data()), text.size()));
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
        : topic == u8"fragments"
          ? u8"A link to a fragment of the page scrolls to it without asking the editor; every "
            u8"other link goes to the editor, which loads its own pages and logs the rest."
        : topic == u8"routing"
          ? u8"These topics never load: the editor's navigation handler shows them in place, so "
            u8"only this paragraph changes."
          : u8"Pick a topic.";
      (void)view->TextContent(krys::move(text));
      for (utf8_stringview name : {u8"pages", u8"fragments", u8"routing"})
      {
        if (auto link = document.GetElementById(u8"topic-" + utf8_string(name)))
        {
          (void)link->ClassList().Toggle(boo::DOMString(u8"current"), name == topic);
        }
      }
    }

    /// @brief Shows the page's cursor (a system one or its image) over the page view, and the arrow
    /// everywhere else; a drag in the page shows its cursor wherever the mouse is.
    void UpdateCursor() noexcept
    {
      bool overPage = _pageView && (_pointer.Y() >= 0 || _pageView->IsDragging());
      if (overPage)
      {
        if (auto image = _pageView->CurrentCursorImage())
        {
          Ref<pal::CursorImage> cursorImage = krys::move(image);
          _context->Window->SetCursorImage(krys::move(cursorImage));
          return;
        }
      }
      _context->Window->SetCursor(overPage ? _pageView->CurrentCursor() : pal::CursorShape::Arrow);
    }

    /// @brief The page view and the page it shows.
    void CreatePage() noexcept
    {
      // Booey's work that can leave the main thread runs on the application's workers, its file reads as
      // jobs that may block.
      boo::PlatformStrategies::Singleton().SetWorkQueue(_context->Jobs.get());
      _fileLoader = CreateUnique<ui::FileLoaderStrategy>(_context->Jobs.get());
      boo::PlatformStrategies::Singleton().SetLoaderStrategy(_fileLoader.get());
      // Overlay scrollbars, which fade in while the mouse is over a scroller or it scrolls.
      boo::ScrollbarThemeAdwaita::SetUsesOverlayScrollbars(true);
      (void)ui::View::AddFontDirectory(std::filesystem::path {KRYS_EDITOR_FONTS_DIR});
      _pageView = CreateUnique<ui::View>(PageSize());
      // The page is laid out and painted at the window's DPI, which `WindowDPIChangeEvent` follows.
      _pageView->SetDeviceScaleFactor(
        static_cast<float>(pal::GetDPIForWindow(_context->Window->GetWindowHandle())) / 96.f);
      _pageView->SetFocused(_focused);
      _pageView->SetPointingDevices(pal::GetPointingDevices());
      AttachPageAccessibility();
      // The window hides and confines the cursor while the page has locked the pointer.
      _pageView->SetPointerLockHandler(
        [&](bool locked)
        {
          bool done = _context->Window->SetPointerLocked(locked);
          UpdateCursor();
          return done;
        });
      // The page dropping its composition drops the input method's.
      _pageView->SetCompositionDiscardedHandler([&] { _context->Window->CancelComposition(); });
      _pageView->SetNavigationHandler([this](const ui::NavigationRequest &request)
                                      { HandleNavigation(request); });
      _pageView->SetLoadHandler([this](const ui::LoadEvent &event) { HandleLoad(event); });
      // The page's custom elements, which its parser creates.
      std::ignore = _pageView->DefineCustomElement(
        u8"bean-rating", [](boo::HTMLElement &element) { return CreateUnique<BeanRating>(element); },
        BeanRating::Options());
      LoadPage();
    }

    /// @brief Exposes the page to screen readers through the window, below the header.
    void AttachPageAccessibility() noexcept
    {
      _pageView->AttachAccessibility(*_context->Window, {0, static_cast<int32>(HeaderHeight)});
    }

    /// @brief Shows the guide's topics in place and loads links to the editor's other pages; logs any other
    /// navigation (a link or a form submission), which the editor doesn't follow, and shows where it leads in
    /// the page's `#last-submission` element.
    void HandleNavigation(const ui::NavigationRequest &request) noexcept
    {
      boo::url::URL url(request.URL);
      if (!request.IsFormSubmission && !request.Download && !request.OpensNewWindow
          && url.ProtocolIs(u8"file"))
      {
        // A guide topic of this page is shown without a load.
        if (url.Path() == _pageView->Document().GetURL().Path() && !GuideTopic(url).empty())
        {
          ShowGuide(url);
          return;
        }
        _pageView->Load(krys::move(url));
        return;
      }

      // A form's entries, as name=value (a file by its name).
      utf8_string entries;
      if (request.Entries)
      {
        for (auto &item : request.Entries->Items())
        {
          auto *value = std::get_if<boo::DOMString>(&item.Data);
          auto *file = std::get_if<RefPtr<boo::File>>(&item.Data);
          entries += (entries.empty() ? u8"" : u8"&") + item.Name + u8"="
                     + (value   ? *value
                        : *file ? utf8_string((*file)->Name())
                                : utf8_string());
        }
      }
      _context->Logger->Info("Navigation: {} {} {}{}", Narrow(request.Method), Narrow(request.URL),
                             Narrow(entries), request.OpensNewWindow ? " (new window)" : "");
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
        positions.emplace_back(pen, 0.f);
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
        .Stops = {{.Offset = 0.f, .Colour = Hex(0x4F6DF5)}, {.Offset = 1.f, .Colour = Hex(0xB04FF5)}}};
      _renderer->Save();
      _renderer->SetDropShadow({.Offset = {0.f, 2.f}, .BlurRadius = 10.f, .Colour = Rgb(0x2A2050, 70)});
      _renderer->FillRect({0.f, 0.f, width, 56.f}, header);
      _renderer->Restore();
      _renderer->FillRoundedRect(Rounded(20.f, 16.f, 24.f, 24.f, 12.f), Rgb(0xFFFFFF));
      _renderer->Save();
      _renderer->SetDropShadow({.Offset = {0.f, 1.f}, .BlurRadius = 4.f, .Colour = Rgb(0x1A1040, 120)});
      DrawText(U"Krystal", 56.f, 36.f, 22.f, Rgb(0xFFFFFF), true);
      _renderer->Restore();
      DrawText(U"F5 reloads the page", width - 150.f, 34.f, 13.f, Rgb(0xFFFFFF, 170));

      // The page's chapter most in view, from its intersection observer.
      if (_observers && !_observers->CurrentChapter().empty())
      {
        std::u32string chapter = U"Reading: ";
        for (text::CodePointIterator it(utf8_stringview(_observers->CurrentChapter())); !it.AtEnd(); ++it)
        {
          chapter.push_back(*it);
        }
        DrawText(chapter.c_str(), 170.f, 34.f, 15.f, Rgb(0xFFFFFF), true);
      }
    }
  };
}
