#include "Krystal.Engine/Application.hpp"
#include "Krystal.Debug/ScopedProfiler.hpp"
#include "Krystal.IO/Backends/NativeFileBackend.hpp"
#include "Krystal.Maths/Random.hpp"
#include "Krystal.Platform/Input.hpp"
#include "Krystal.Platform/Platform.hpp"
#include <cassert>
#include <filesystem>
#include <thread>
#include <utility>

namespace Krys
{
  Application::Application(int argc, char **argv, const ApplicationSettings &settings)
      : _context(CreateUnique<ApplicationContext>()), _running(false), _isWindowMinimised(false)
  {
    Platform::Initialise();
    Maths::Random::Initialise();

    CreateServices(argc, argv, settings);

    _context->Window->SetCallbacks(CreateWindowCallbacks());
    _context->Renderer->Startup();
  }

  Application::~Application() noexcept
  {
    _context->Renderer->Shutdown();

    _context->Events.reset();
    _context->Input.reset();
    _context->Renderer.reset();
    _context->GraphicsContext.reset();
    _context->VFS.reset();
    _context->Window.reset();
    _context->Logger.reset();
    _context.reset();

    Platform::Shutdown();
  }

  void Application::Run() noexcept
  {
    OnInit();
    {
      _running = true;

      double elapsedMs = 0;
      double accumulatedMs = 0;
      while (_running)
      {
        // auto profiler = Debug::ScopedProfiler("Frame");
        const double startTime = Platform::GetTimeMilliseconds();

        _context->Input->BeginFrame();
        _context->Window->ProcessMessages();
        _context->Events->DispatchAll();

        // Fixed update loop.
        const double physicsStepMs = 1'000.0 / _context->Settings.PhysicsFramerate;
        const uint32 maxPhysicsSteps = _context->Settings.MaxPhysicsUpdatesPerFrame;
        {
          uint32 physicsSteps = 0;
          while (accumulatedMs >= physicsStepMs && physicsSteps < maxPhysicsSteps)
          {
            OnFixedUpdate(physicsStepMs / 1'000.0);
            accumulatedMs -= physicsStepMs;
            physicsSteps++;
          }
        }

        // Per-frame update and render.
        {
          OnUpdate(elapsedMs / 1'000.0);
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
          ClampFramerate(elapsedMs, startTime);
        }

        // Calculate the elapsed time since the last frame.
        elapsedMs = Platform::GetTimeMilliseconds() - startTime;
        accumulatedMs = std::min(accumulatedMs + elapsedMs,
                                 physicsStepMs * maxPhysicsSteps); // Cap to avoid spiral of death.
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

    auto logger = Log::CreateLogger(_context->Settings.GlobalLoggerSettings);
    if (!logger.has_value())
    {
      throw std::runtime_error("Failed to create logger: " + logger.error());
    }
    _context->Logger = std::move(logger.value());
    Log::SetGlobalLogger(_context->Logger);

    _context->Events = CreateUnique<EventManager>();
    if (!_context->Events)
    {
      throw std::runtime_error("Failed to create event manager");
    }

    _context->Input = CreateUnique<Platform::Input>(_context->Events.get());
    if (!_context->Input)
    {
      throw std::runtime_error("Failed to create input manager");
    }

    auto window = Platform::CreateWindow(_context->Settings.WindowSettings);
    if (!window.has_value())
    {
      throw std::runtime_error("Failed to create window: " + window.error());
    }
    _context->Window = std::move(window.value());

    auto cwd = std::filesystem::current_path();
    auto shadersDirectory = cwd / "data/shaders/opengl";
    auto texturesDirectory = cwd / "data/assets";
    _context->VFS = IO::VirtualFileSystemBuilder()
                      .Mount<IO::NativeFileBackend>(IO::Path("/"), IO::Path(cwd))
                      .Mount<IO::NativeFileBackend>(IO::Path("/shaders"), IO::Path(shadersDirectory))
                      .Mount<IO::NativeFileBackend>(IO::Path("/textures"), IO::Path(texturesDirectory))
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
    _context->GraphicsContext = std::move(gfxContext.value());

    auto uiRenderer = CreateRenderer(*_context->GraphicsContext);
    if (!uiRenderer.has_value())
    {
      throw std::runtime_error("Failed to create renderer");
    }
    _context->Renderer = std::move(uiRenderer.value());
  }

  Platform::WindowCallbacks Application::CreateWindowCallbacks() noexcept
  {
    using namespace Platform;
    using namespace Events;

    return {.OnMouseMove =
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
            }};
  }

  void Application::ClampFramerate(double &elapsedMs, const double startTime)
  {
    const double targetFrameTimeMs = 1'000.0f / _context->Settings.RenderFramerate;
    while (elapsedMs < targetFrameTimeMs - 2)
    {
      Platform::Sleep(1);
      elapsedMs = Platform::GetTimeMilliseconds() - startTime;
    }

    // Yield the thread to the OS for the remaining time, avoids busy waiting.
    do
    {
      std::this_thread::yield();
      elapsedMs = Platform::GetTimeMilliseconds() - startTime;
    } while (elapsedMs < targetFrameTimeMs);
  }

#pragma region Lifecycle Methods

  void Application::OnInit() noexcept
  {
  }

  void Application::OnUpdate(double) noexcept
  {
  }

  void Application::OnFixedUpdate(double) noexcept
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