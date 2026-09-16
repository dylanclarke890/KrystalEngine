#pragma once

#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Events/EventManager.hpp"
#include "Krystal.Core/IO/VirtualFileSystem.hpp"
#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Time/Duration.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Gfx/IContext.hpp"
#include "Krystal.Gfx/IRenderer.hpp"
#include "Krystal.PAL/Events.hpp"
#include "Krystal.PAL/Input.hpp"
#include "Krystal.PAL/IWindow.hpp"
#include "Krystal.PAL/Platform.hpp"
#include <algorithm>
#include <exception>
#include <utility>

namespace krys
{
  /// @brief Create a new `Application`.
  /// @tparam TApplication The derived `Application` type.
  /// @param argc Command line argument count.
  /// @param argv Command line arguments.
  /// @param settings Application settings.
  template <DerivedFrom<Application> TApplication, typename... Args>
  static Expected<UniquePtr<TApplication>> CreateApplication(int argc, char **argv,
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

    log::LoggerSettings GlobalLoggerSettings {};

    pal::WindowSettings WindowSettings {};

    uint32 RenderFramerate {60};

    uint32 PhysicsFramerate {30};

    /// @brief Maximum number of physics updates to process per frame if the accumulated time exceeds the
    /// fixed time step. Keep this low to avoid stuttering.
    uint32 MaxPhysicsUpdatesPerFrame {5};
  };

  struct ApplicationContext
  {
    List<string> CommandLineArgs {};
    ApplicationSettings Settings {};
    UniquePtr<log::ILogger> Logger {};
    UniquePtr<EventManager> Events {};
    UniquePtr<pal::IWindow> Window {};
    UniquePtr<pal::Input> Input {};
    UniquePtr<io::VirtualFileSystem> VFS {};
    UniquePtr<Gfx::IContext> GraphicsContext {};
    UniquePtr<Gfx::IRenderer> Renderer {};
  };

  /// @brief Base class for a Krystal application.
  class Application
  {
    KRYS_NON_COPY_MOVABLE(Application);

  protected:
    bool _running;
    bool _isWindowMinimised;
    UniquePtr<ApplicationContext> _context;

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
    virtual void OnUpdate(seconds dt) noexcept;

    /// @brief Fixed rate update method.
    /// @param dt The fixed time step, in seconds. Does not change unless explicitly set.
    virtual void OnFixedUpdate(seconds dt) noexcept;

    /// @brief Per-frame render method.
    virtual void OnRender() noexcept;

    /// @brief Called once after the application stops running, before all services are shut down.
    virtual void OnShutdown() noexcept;

  private:
    void CreateServices(int argc, char **argv, const ApplicationSettings &settings);

    pal::WindowCallbacks CreateWindowCallbacks() noexcept;

    /// @brief CPU friendly way to cap the frame rate.
    void ClampFramerate(const nanoseconds start, nanoseconds &elapsed);
  };
}