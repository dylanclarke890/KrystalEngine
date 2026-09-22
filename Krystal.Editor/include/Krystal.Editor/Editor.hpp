#pragma once

#ifndef KRYS_ENABLE_PROFILING
  #define KRYS_ENABLE_PROFILING
#endif

#include "Krystal.Core/Log/ILogger.hpp"
#include "Krystal.Core/Maths/Convert.hpp"
#include "Krystal.Core/Maths/Random.hpp"
#include "Krystal.Core/Utils/ScopedProfiler.hpp"
#include "Krystal.Editor/Application.hpp"
#include "Krystal.Gfx/Cameras/FirstPersonCamera.hpp"
#include "Krystal.PAL/Events.hpp"
#include "Krystal.PAL/Keys.hpp"

namespace krys
{
  class Editor : public Application
  {
  private:
    uint32 _width {0u};
    uint32 _height {0u};
    Gfx::FirstPersonCamera _camera;

  public:
    explicit Editor(int argc, char **argv, const ApplicationSettings &settings) noexcept
        : Application(argc, argv, settings), _width(settings.WindowSettings.Size.Width),
          _height(settings.WindowSettings.Size.Height),
          _camera({0.f, 0.f, 5.f}, {0.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, Radians(45.f),
                  (float)_width / (float)_height, 0.1f, 100.f)
    {
    }

    void OnInit() noexcept override
    {
      _context->Logger->SetLevel(log::Level::Info);
      _context->Logger->Info("Initialising Krystal Editor...");

      BindEvents();
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

          if (event.Key() == pal::Key::Escape)
          {
            _context->Logger->Info("ESC key pressed, quitting application...");
            Stop();
          }

          return true;
        });

      _context->Events->On<WindowResizeEvent>(
        [&](const auto &event)
        {
          _width = event.Width();
          _height = event.Height();
          _context->GraphicsContext->Resize(_width, _height);
          _camera.OnResize(_width, _height);
          return true;
        });

      _context->Events->On<WindowCloseEvent>(
        [&](const auto &)
        {
          Stop();
          return true;
        });
    }

    void OnRender() noexcept override
    {
      _context->GraphicsContext->Render(_camera);
    }

    void OnUpdate(seconds dt) noexcept override
    {
      _camera.Update(dt.count(), *_context->Input);
    }
  };
}