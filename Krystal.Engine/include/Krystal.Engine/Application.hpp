#pragma once

#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/IRenderer.hpp"
#include "Krystal.IO/VirtualFileSystem.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/Events/EventManager.hpp"
#include "Krystal.Lib/Expected.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Lib/String.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Log/ILogger.hpp"
#include "Krystal.Platform/Events.hpp"
#include "Krystal.Platform/Input.hpp"
#include "Krystal.Platform/IWindow.hpp"
#include "Krystal.Platform/Platform.hpp"
#include <algorithm>
#include <exception>
#include <utility>

namespace Krys
{
  /// @brief Create a new `Application`.
  /// @tparam TApplication The derived `Application` type.
  /// @param argc Command line argument count.
  /// @param argv Command line arguments.
  /// @param settings Application settings.
  template <DerivedFrom<Application> TApplication, typename... Args>
  static Expected<Unique<TApplication>> CreateApplication(int argc, char **argv,
                                                          const struct ApplicationSettings &settings,
                                                          Args &&...args) noexcept
  {
    try
    {
      return CreateUnique<TApplication>(argc, argv, settings, std::forward<Args>(args)...);
    }
    catch (const std::exception &e)
    {
      return Unexpected("Exception during application creation: " + string(e.what()));
    }
  }

  struct ApplicationSettings
  {
    string Name {};

    Log::LoggerSettings GlobalLoggerSettings {};

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
    Unique<Platform::Input> Input {};
    Unique<IO::VirtualFileSystem> VFS {};
    Unique<Gfx::IContext> GraphicsContext {};
    Unique<Gfx::IRenderer> Renderer {};
    StringInterner Strings {};
  };

  /// @brief Base class for a Krystal application.
  class Application
  {
    NO_COPY_MOVE(Application)

  protected:
    bool _running, _isWindowMinimised;
    Unique<ApplicationContext> _context;

  public:
    /// @brief Constructs an `Application`.
    Application(int argc, char **argv, const ApplicationSettings &settings);

    virtual ~Application() noexcept;

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

  private:
    void CreateServices(int argc, char **argv, const ApplicationSettings &settings);

    Platform::WindowCallbacks CreateWindowCallbacks() noexcept;

    /// @brief CPU friendly way to cap the frame rate.
    void ClampFramerate(double &elapsedMs, const double startTime);
  };
}