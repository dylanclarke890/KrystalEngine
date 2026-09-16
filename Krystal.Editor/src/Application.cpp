#include "Krystal.Editor/Application.hpp"
#include "Krystal.Core/IO/Backends/NativeFileBackend.hpp"
#include "Krystal.Core/Maths/Random.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include "Krystal.Core/Utils/Move.hpp"
#include "Krystal.Core/Utils/ScopedProfiler.hpp"
#include "Krystal.PAL/Input.hpp"
#include "Krystal.PAL/Platform.hpp"
#include <cassert>
#include <thread>
#include <utility>

namespace krys
{
  Application::Application(int argc, char **argv, const ApplicationSettings &settings)
      : _context(CreateUnique<ApplicationContext>()), _running(false), _isWindowMinimised(false)
  {
    pal::Initialise();
    Random::Initialise();

    CreateServices(argc, argv, settings);

    _context->Window->SetCallbacks(CreateWindowCallbacks());
    _context->GraphicsContext->Startup();
    _context->Renderer->Startup();
  }

  Application::~Application() noexcept
  {
    _context->Renderer->Shutdown();
    _context->GraphicsContext->Shutdown();

    _context->Events.reset();
    _context->Input.reset();
    _context->Renderer.reset();
    _context->GraphicsContext.reset();
    _context->VFS.reset();
    _context->Window.reset();
    _context->Logger.reset();
    _context.reset();

    pal::Shutdown();
  }

  void Application::Run() noexcept
  {
    OnInit();
    {
      _running = true;

      nanoseconds elapsed {};
      nanoseconds accumulated {};
      while (_running)
      {
        const nanoseconds frameStart = MonotonicTime::Now();

        _context->Input->BeginFrame();
        _context->Window->ProcessMessages();
        _context->Events->DispatchAll();

        // Fixed update loop.
        const auto fixedUpdateSizeNs = nanoseconds(1s) / _context->Settings.PhysicsFramerate;
        const uint32 maxPhysicsSteps = _context->Settings.MaxPhysicsUpdatesPerFrame;
        {
          uint32 physicsStepCount = 0;
          while (accumulated >= fixedUpdateSizeNs && physicsStepCount < maxPhysicsSteps)
          {
            OnFixedUpdate(seconds(fixedUpdateSizeNs));
            accumulated -= fixedUpdateSizeNs;
            physicsStepCount++;
          }
        }

        // Per-frame update and render.
        {
          OnUpdate(seconds(elapsed));
          if (!_isWindowMinimised)
          {
            _context->Renderer->BeginFrame();
            OnRender();
            _context->Renderer->EndFrame();
            _context->GraphicsContext->Present();
          }
        }

        // We'll only manually cap the frame rate if vsync is disabled.
        if (!_context->Settings.WindowSettings.VSync)
        {
          ClampFramerate(frameStart, elapsed);
        }

        // Calculate the elapsed time since the last frame.
        elapsed = MonotonicTime::Now() - frameStart;
        accumulated = std::min(accumulated + elapsed,
                               fixedUpdateSizeNs * maxPhysicsSteps); // Cap to avoid spiral of death.

        // KRYS_LOG_INFO("Frame Time: {:.2f} ms", std::chrono::duration_cast<Milliseconds>(elapsed).count());
      }
    }
    OnShutdown();
  }

  void Application::Stop() noexcept
  {
    _running = false;
  }

  void Application::CreateServices(int argc, char **argv, const ApplicationSettings &settings)
  {
    assert(settings.PhysicsFramerate > 0);
    assert(settings.RenderFramerate > 0);
    assert(settings.MaxPhysicsUpdatesPerFrame > 0);

    _context->Settings = settings;
    _context->CommandLineArgs.resize(argc);
    std::transform(argv, argv + argc, _context->CommandLineArgs.begin(),
                   [](const char *arg) -> string { return arg; });

    auto logger = log::CreateLogger(_context->Settings.GlobalLoggerSettings);
    if (!logger.has_value())
    {
      throw std::runtime_error("Failed to create logger: " + logger.error());
    }
    _context->Logger = krys::move(logger.value());
    log::SetGlobalLogger(_context->Logger);

    _context->Events = CreateUnique<EventManager>();
    if (!_context->Events)
    {
      throw std::runtime_error("Failed to create event manager");
    }

    _context->Input = CreateUnique<pal::Input>(_context->Events.get());
    if (!_context->Input)
    {
      throw std::runtime_error("Failed to create input manager");
    }

    auto window = pal::CreateWindow(_context->Settings.WindowSettings);
    if (!window.has_value())
    {
      throw std::runtime_error("Failed to create window: " + window.error());
    }
    _context->Window = krys::move(window.value());

    // TODO(feat): this needs to be configurable on startup.
    auto cwd = std::filesystem::current_path();
    auto shadersDirectory = cwd / "data/shaders/opengl";
    auto texturesDirectory = cwd / "data/assets";
    _context->VFS = io::VirtualFileSystemBuilder()
                      .Mount<io::NativeFileBackend>(io::Path("/"), io::Path(cwd))
                      .Mount<io::NativeFileBackend>(io::Path("/shaders"), io::Path(shadersDirectory))
                      .Mount<io::NativeFileBackend>(io::Path("/textures"), io::Path(texturesDirectory))
                      .Build();
    if (!_context->VFS)
    {
      throw std::runtime_error("Failed to create virtual file system");
    }

    Gfx::ContextSettings contextSettings {
      .WindowHandle = _context->Window->GetWindowHandle(),
      .Width = _context->Settings.WindowSettings.Size.Width,
      .Height = _context->Settings.WindowSettings.Size.Height,
      .VFS = _context->VFS.get(),
    };
    auto gfxContext = Gfx::CreateContext(contextSettings);
    if (!gfxContext.has_value())
    {
      throw std::runtime_error("Failed to create graphics context: " + gfxContext.error());
    }
    _context->GraphicsContext = krys::move(gfxContext.value());

    auto uiRenderer = CreateRenderer(*_context->GraphicsContext);
    if (!uiRenderer.has_value())
    {
      throw std::runtime_error("Failed to create renderer");
    }
    _context->Renderer = krys::move(uiRenderer.value());
  }

  pal::WindowCallbacks Application::CreateWindowCallbacks() noexcept
  {
    using namespace pal;

    return {
      .OnMouseMove =
        [&](WindowHandle window, float deltaX, float deltaY, float clientX, float clientY) noexcept
      { _context->Input->OnMouseMoveEvent(window, deltaX, deltaY, clientX, clientY); },

      .OnMouseButton = [&](WindowHandle window, MouseButton button, MouseButtonState state) noexcept
      { _context->Input->OnMouseButtonEvent(window, button, state); },

      .OnMouseScroll = [&](WindowHandle window, float delta) noexcept
      { _context->Input->OnMouseScrollEvent(window, delta); },

      .OnKey = [&](WindowHandle window, Key key, KeyState state) noexcept
      { _context->Input->OnKeyboardEvent(window, key, state); },

      .OnClose = [&](WindowHandle window) noexcept
      { _context->Events->Enqueue(CreateUnique<WindowCloseEvent>(window)); },

      .OnResize =
        [&](WindowHandle window, uint32 width, uint32 height) noexcept
      {
        _context->Events->Enqueue(CreateUnique<WindowResizeEvent>(window, width, height));
        _context->GraphicsContext->Resize(width, height);
      },
      .OnMinimise =
        [&](WindowHandle window) noexcept
      {
        _context->Events->Enqueue(CreateUnique<WindowMinimiseEvent>(window));
        _isWindowMinimised = true;
      },
      .OnRestore =
        [&](WindowHandle window) noexcept
      {
        _context->Events->Enqueue(CreateUnique<WindowRestoreEvent>(window));
        _isWindowMinimised = false;
      },
      .OnDPIChange =
        [&](WindowHandle window, int dpi) noexcept
      {
        _context->Events->Enqueue(CreateUnique<WindowDPIChangeEvent>(window, dpi));
        _context->GraphicsContext->DPIChanged(dpi);
      },
    };
  }

  void Application::ClampFramerate(const nanoseconds start, nanoseconds &elapsed)
  {
    const nanoseconds targetFrameTime = nanoseconds(1s) / _context->Settings.RenderFramerate;
    while (elapsed < (targetFrameTime - 2ms))
    {
      pal::Sleep(1ms);
      elapsed = MonotonicTime::Now() - start;
    }

    // Yield the thread to the OS for the remaining time, avoids busy waiting.
    do
    {
      std::this_thread::yield();
      elapsed = MonotonicTime::Now() - start;
    } while (elapsed < targetFrameTime);
  }

#pragma region Lifecycle Methods

  void Application::OnInit() noexcept
  {
  }

  void Application::OnUpdate(seconds) noexcept
  {
  }

  void Application::OnFixedUpdate(seconds) noexcept
  {
  }

  void Application::OnRender() noexcept
  {
  }

  void Application::OnShutdown() noexcept
  {
  }

#pragma endregion
}