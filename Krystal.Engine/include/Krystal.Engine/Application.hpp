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
    string Name {};

    Log::LoggerSettings GlobalLoggerSettings {Log::LoggerSettings::Default()};

    Platform::WindowSettings WindowSettings {};

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
    Unique<Log::ILogger> Logger {};
    Unique<EventManager> Events {};
    Unique<Platform::IWindow> Window {};
    Unique<Platform::IInput> Input {};
    Unique<Gfx::IContext> GraphicsContext {};
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
    template <DerivedFrom<Application> TApplication, typename... Args>
    static Expected<Unique<TApplication>> Create(int argc, char **argv, const ApplicationSettings &settings,
                                                 Args &&...args) noexcept
    {
      Unique<ApplicationContext> context = CreateUnique<ApplicationContext>();

      context->Settings = settings;
      context->CommandLineArgs.resize(argc);
      std::transform(argv, argv + argc, context->CommandLineArgs.begin(),
                     [](const char *arg) -> string { return arg; });

      Platform::Initialise();

      try
      {
        auto logger = Log::CreateLogger(context->Settings.GlobalLoggerSettings);
        if (!logger.has_value())
          return Unexpected("Failed to create logger: " + logger.error());
        context->Logger = std::move(logger.value());
        Log::SetGlobalLogger(context->Logger);

        context->Events = CreateUnique<EventManager>();
        if (!context->Events)
          return Unexpected("Failed to create event manager");

        auto input = Platform::CreateInput(context->Events.get());
        if (!input.has_value())
          return Unexpected("Failed to create input manager: " + input.error());
        context->Input = std::move(input.value());

        auto window = Platform::CreateWindow(context->Settings.WindowSettings, context->Input.get(),
                                             context->Events.get());
        if (!window.has_value())
          return Unexpected("Failed to create window: " + window.error());
        context->Window = std::move(window.value());

        auto gfxContext = Gfx::CreateContext(context->Window->GetNativeHandle());
        if (!gfxContext.has_value())
          return Unexpected("Failed to create graphics context: " + gfxContext.error());
        context->GraphicsContext = std::move(gfxContext.value());
      }
      catch (const std::exception &e)
      {
        return Unexpected("Exception during application creation: " + string(e.what()));
      }

      return CreateUnique<TApplication>(std::move(context), std::forward<Args>(args)...);
    }

  private:
    /// @brief CPU friendly way to cap the frame rate.
    void ClampFramerate(double &elapsedMs, const double startTime);
  };
}