#include "Krystal.Engine/Application.hpp"

#include "Krystal.Core/Core.hpp"
#include "Krystal.Engine/Debug/ScopedProfiler.hpp"
#include "Krystal.Platform/Platform.hpp"

#include <cassert>
#include <thread>
#include <utility>

namespace Krys
{
  Application::Application(Unique<ApplicationContext> context) noexcept
      : _context(std::move(context)), _running(false)
  {
  }

  void Application::Run() noexcept
  {
    assert(_context->Settings.PhysicsFramerate > 0);
    assert(_context->Settings.RenderFramerate > 0);
    assert(_context->Settings.MaxPhysicsUpdatesPerFrame > 0);

    OnInit();
    {
      _running = true;

      double elapsedMs = 0;
      double accumulatedMs = 0;
      while (_running)
      {
        KRYS_SCOPED_PROFILER("Frame", *_context->Logger);
        const double startTime = Platform::GetTimeMilliseconds();

        _context->Input->BeginFrame();
        _context->Window->ProcessMessages();
        _context->Input->PollDevices();
        _context->Events->DispatchAll();

        // Fixed update loop.
        {
          const auto physicsStepMs = 1'000.0 / _context->Settings.PhysicsFramerate;
          const uint32 maxPhysicsSteps = _context->Settings.MaxPhysicsUpdatesPerFrame;
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
          OnRender();
        }

        _context->GraphicsContext->Present();

        // We'll only manually cap the frame rate if vsync is disabled.
        if (!_context->Settings.WindowSettings.VSync)
        {
          ClampFramerate(elapsedMs, startTime);
        }

        // Calculate the elapsed time since the last frame.
        elapsedMs = Platform::GetTimeMilliseconds() - startTime;
        accumulatedMs += elapsedMs;
      }
    }
    OnShutdown();

    // Here we can add any cleanup code that needs to run after the main loop ends.
  }

  void Application::Stop() noexcept
  {
    _running = false;
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