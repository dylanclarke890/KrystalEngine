#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Core/Events/EventManager.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Platform/IInput.hpp"
#include "Krystal.Platform/IWindow.hpp"
#include "Krystal.Platform/Platform.hpp"

#include <algorithm>
#include <utility>

namespace Krys
{
  struct ApplicationSettings
  {
    string Title;
    Platform::WindowSettings WindowSettings;
    uint32 RenderFramerate {30};
    uint32 PhysicsFramerate {30};
    /// @brief Maximum number of physics updates to process per frame if the accumulated time exceeds the
    /// fixed time step. Keep this low to avoid stuttering.
    uint32 MaxPhysicsUpdatesPerFrame {5};
  };

  struct ApplicationContext
  {
    List<string> CommandLineArgs {};
    ApplicationSettings Settings {};
    Unique<Platform::IWindow> Window {};
    Unique<Platform::IInput> Input {};
    Unique<EventManager> Events {};
    Unique<Gfx::IContext> GraphicsContext {};
    Unique<Log::ILogger> Logger {};
  };

  /// @brief Base class for a Krystal application.
  class Application
  {
    NO_COPY_MOVE(Application)

  protected:
    bool _running;
    Unique<ApplicationContext> _context;

  public:
    /// @brief Constructs an `Application`.
    Application(Unique<ApplicationContext> context) noexcept;

    virtual ~Application() noexcept = default;

    /// @brief Runs the application. Will not return until the app stops running.
    void Run() noexcept;

    /// @brief Stops the application then shuts down all services.
    void Stop() noexcept;

    /// @brief Called once before the application starts running, after all services have been initialized.
    virtual void OnInit() noexcept;

    /// @brief Per-frame update method.
    /// @param dt Delta time since the last frame, in seconds.
    virtual void OnUpdate(double dt) noexcept;

    /// @brief Fixed rate update method.
    /// @param dt The fixed time step, in seconds. Does not change unless explicitly set.
    virtual void OnFixedUpdate(double dt) noexcept;

    /// @brief Per-frame render method.
    virtual void OnRender() noexcept;

    /// @brief Called once after the application stops running, before all services are shut down.
    virtual void OnShutdown() noexcept;

    /// @brief Create a new `Application`.
    /// @tparam TApplication The derived `Application` type.
    /// @param argc Command line argument count.
    /// @param argv Command line arguments.
    /// @param settings Application settings.
    template <DerivedFrom<Application> TApplication>
    static Unique<Application> Create(int argc, char **argv, const ApplicationSettings &settings) noexcept
    {
      Platform::Initialise();

      Unique<ApplicationContext> context = CreateUnique<ApplicationContext>();

      context->CommandLineArgs.resize(argc);
      std::transform(argv, argv + argc, context->CommandLineArgs.begin(),
                     [](const char *arg) -> string { return arg; });

      context->Settings = settings;
      context->Events = CreateUnique<EventManager>();
      context->Input = Platform::CreateInput(context->Events.get());
      context->Window =
        Platform::CreateWindow(settings.WindowSettings, context->Input.get(), context->Events.get());

      context->GraphicsContext = Gfx::CreateContext(context->Window->GetNativeHandle());
      context->GraphicsContext->Initialise();

      context->Logger = Log::CreateLogger(settings.WindowSettings.Title);
      context->Logger->AddConsoleSink();
      context->Logger->AddFileSink("logs/log.txt");
      Log::SetGlobalLogger(context->Logger);

      return CreateUnique<TApplication>(std::move(context));
    }

  private:
    /// @brief CPU friendly way to cap the frame rate.
    void ClampFramerate(double &elapsedMs, const double startTime);
  };
}